#include <functional>
#include <sstream>
#include <utility>

#include <asio/io_service.hpp>

#include <json/value.h>

#include "repetier_action.hpp"
#include "repetier_client.hpp"

namespace gcu {
    namespace repetier {



        static std::string buildSocketUrl( std::string const& hostname, std::uint16_t port )
        {
            std::ostringstream os;
            os << "ws://" << hostname << ':' << port << "/socket";
            return os.str();
        }

        static std::string buildUploadUrl( std::string const& hostname, std::uint16_t port,
                                           std::string const& printer, std::string const& session,
                                           std::string const& name )
        {
            std::ostringstream os;
            os << "http://" << hostname << ':' << port << "/printer/model/" << printer << "?a=upload&sess="
               << session << "&name=" << name;
            return os.str();
        }

        Client::Client( std::string const& hostname, std::uint16_t port, std::string&& apikey,
                        Callback< void >&& callback )
        {
            wsclient_.clear_access_channels( websocketpp::log::alevel::all );
            wsclient_.clear_error_channels( websocketpp::log::elevel::all );

            wsclient_.init_asio();

            std::error_code ec;
            auto connection = wsclient_.get_connection( buildSocketUrl( hostname, port ), ec );
            if ( ec ) {
                throw std::system_error( ec );
            }
            wshandle_ = connection->get_handle();

            connection->set_open_handler( [this, apikey, callback]( auto&& ) {
                this->login( apikey, callback );
            } );
            connection->set_fail_handler( [this, callback]( auto&& ) {
                callback( wsclient_.get_con_from_hdl( wshandle_ )->get_ec() );
            } );
            connection->set_close_handler( [this, callback]( auto&& ) {
                callback( std::make_error_code( std::errc::invalid_argument) ) ; // TODO
            } );
            connection->set_message_handler( [this]( auto&&, auto&& message ) {
                this->handleMessage( std::forward< decltype( message ) >( message ) );
            } );

            // connection->set_open_handler( std::bind( &Client::handleOpen, this ) );
            //connection->set_fail_handler( std::bind( &Client::handleFail, this ) );
            //connection->set_close_handler( std::bind( &Client::handleClose, this ) );

            std::cerr << "INFO: Connecting to " << connection->get_uri()->str() << "\n";

            wsclient_.connect( connection );

            wsthread_ = std::thread( [this] { wsclient_.run(); } );
        }

        Client::~Client()
        {
            close( websocketpp::close::status::normal );
            wsthread_.join();
        }
/*
        void Client::handleFail()
        {
            auto connection = wsclient_.get_con_from_hdl( wshandle_ );
            std::cerr << "ERROR: Connection has failed: " << connection->get_ec().message() << "\n";
            propagateError( connection->get_ec() );
            forceClose();
        }

        void Client::handleClose()
        {
            auto connection = wsclient_.get_con_from_hdl( wshandle_ );
            if ( status_ != CLOSING ) {
                std::cerr << "ERROR: Connection closed by server: code "
                          << websocketpp::close::status::get_string( connection->get_remote_close_code())
                          << ", reason: " << connection->get_remote_close_reason() << "\n";
            }
            propagateError( std::make_error_code( std::errc::invalid_argument ) ); // TODO
            status_ = CLOSED;
        }
*/
        void Client::handleMessage( websocketclient::message_ptr message )
        {
            auto response = jsonContext_.toJson( message->get_payload() );
            auto callbackId = response[ Json::StaticString( "callback_id" ) ].asLargestInt();
            if ( callbackId != -1 ) {
                std::cerr << "<<< " << message->get_payload() << "\n";
                handleActionResponse( callbackId, std::move( response ) );
            }
        }

        void Client::handleActionResponse( std::intmax_t callbackId, Json::Value&& response )
        {
            auto it = actionHandlers_.find( callbackId );
            if ( it == actionHandlers_.end() ) {
                std::cerr << "WARN: Received response to unrequested callback " << callbackId << ", ignoring message\n";
                return;
            }

            auto& session = response[ Json::StaticString( "session" ) ];
            if ( !session.isNull() ) {
                session_ = session.asString();
            }

            auto handler = std::move( it->second );
            actionHandlers_.erase( it );
            handler( std::move( response[ Json::StaticString( "data" ) ] ), std::error_code() );
        }

        void Client::login( std::string const& apikey, Callback< void > const& callback )
        {
            std::cerr << "INFO: Connection established, logging in\n";

            action( "login" )
                    .arg( "apikey", apikey.c_str() )
                    .handle( action::checkOkFlag() )
                    .send( [this, callback]( std::error_code ec ) {
                        if ( ec ) {
                            std::cerr << "ERROR: Login failed, closing connection\n";
                            this->close( websocketpp::close::status::going_away );
                            callback( std::make_error_code( std::errc::invalid_argument ) ); // TODO
                        }
                        else {
                            std::cerr << "INFO: Login successful, connection completed\n";
                            status_ = CONNECTED;
                            callback( {} );
                        }
                    } );
        }

        void Client::close( websocketpp::close::status::value code )
        {
            status_ = CLOSING;

            std::error_code ec;
            wsclient_.close( wshandle_, code, {}, ec );
            if ( ec ) {
                std::cerr << "WARN: Closing connection normally failed, close forced: " << ec.message() << "\n";
                forceClose();
            }
        }

        void Client::forceClose()
        {
            status_ = CLOSED;

            std::error_code ec;
            wsclient_.close( wshandle_, websocketpp::close::status::force_tcp_drop, {}, ec );
        }

        void Client::propagateError( std::error_code ec )
        {
            if ( status_ == CONNECTING ) {
                connectCallback_( ec );
            }
            else if ( status_ == CONNECTED ) {
                std::for_each( actionHandlers_.begin(), actionHandlers_.end(),
                               [ec]( auto const& entry ) { entry.second( {}, ec ); } );
                actionHandlers_.clear();
            }
        }

        Action Client::action( char const* name )
        {
            return Action( this, name );
        }

        void Client::send( Json::Value& request, Handler&& handler )
        {
            auto callbackId = ++nextCallbackId_;
            request[ Json::StaticString( "callback_id" ) ] = callbackId;

            auto message = jsonContext_.toString( request );
            std::cerr << ">>> " << message << "\n";

            auto connection = wsclient_.get_con_from_hdl( wshandle_ );
            connection->send( message, websocketpp::frame::opcode::text );
            actionHandlers_.emplace( callbackId, std::move( handler ) );
        }

    } // namespace repetier
} // namespace gcu