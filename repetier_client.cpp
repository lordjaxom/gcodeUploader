#include <functional>
#include <sstream>
#include <utility>

#include <asio/io_service.hpp>

#include <json/value.h>

#include "repetier_action.hpp"
#include "repetier_client.hpp"

namespace gcu {
    namespace repetier {

        static std::string buildSocketUrl( std::string const& hostname, std::uint16_t port,
                                           std::string const& resource )
        {
            std::ostringstream os;
            os << "ws://" << hostname << ':' << port;
            if ( !resource.empty() && resource.front() != '/' ) {
                os << '/';
            }
            os << resource;
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

        Client::Client( std::string&& hostname, std::uint16_t port, std::string&& apikey, Callback< void >&& callback )
                : hostname_( std::move( hostname ) )
                , port_( port )
                , apikey_( std::move( apikey ) )
                , connectCallback_( std::move( callback ) )
                , httpclient_( boost::network::http::client::options().io_service(
                        std::shared_ptr< asio::io_service >( &io_service_, []( void const* ) {} ) ) )
        {
            wsclient_.clear_access_channels( websocketpp::log::alevel::all );
            wsclient_.clear_error_channels( websocketpp::log::elevel::all );

            wsclient_.init_asio( &io_service_ );

            std::error_code ec;
            auto connection = wsclient_.get_connection( buildSocketUrl( hostname_, port_, "/socket" ), ec );
            if ( ec ) {
                throw std::system_error( ec );
            }
            wshandle_ = connection->get_handle();

            connection->set_open_handler( std::bind( &Client::handleOpen, this ) );
            connection->set_fail_handler( std::bind( &Client::handleFail, this ) );
            connection->set_close_handler( std::bind( &Client::handleClose, this ) );
            connection->set_message_handler( std::bind( &Client::handleMessage, this, std::placeholders::_2 ) );

            std::cerr << "INFO: Connecting to " << connection->get_uri()->str() << "\n";

            wsclient_.connect( connection );

            io_thread_ = std::thread( [this] { io_service_.run(); } );
        }

        Client::~Client()
        {
            close( websocketpp::close::status::normal );
            io_thread_.join();
        }

        void Client::handleOpen()
        {
            std::cerr << "INFO: Connection established, logging in\n";

            action( "login" )
                    .arg( "apikey", apikey_.c_str() )
                    .handle( action::checkOkFlag() )
                    .send( [this]( std::error_code ec ) {
                        if ( ec ) {
                            std::cerr << "ERROR: Login failed, closing connection\n";
                            this->close( websocketpp::close::status::going_away );
                            connectCallback_( std::make_error_code( std::errc::invalid_argument ) ); // TODO
                        }
                        else {
                            std::cerr << "INFO: Login successful, connection completed\n";
                            status_ = CONNECTED;
                            connectCallback_( {} );
                        }
                    } );
        }

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

        void Client::upload( std::string const& printer, std::string const& modelGroup, std::string const& name,
                             std::string&& gcode, Callback< void >&& callback )
        {
            using namespace boost::network;

            auto url = buildUploadUrl( hostname_, port_, printer, session_, name );

            std::cerr << "INFO: Uploading G-Code to " << url << "\n";

            http::client::request request( url );
            request << header( "Connection", "close" );

            http::client::response response = httpclient_.post( request, gcode );

            std::cerr << "DEBUG: Response body " << response.status() << "\n";
        }

    } // namespace repetier
} // namespace gcu