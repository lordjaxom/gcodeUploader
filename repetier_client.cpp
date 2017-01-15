#include <functional>
#include <sstream>
#include <utility>

#include <boost/network/include/http/client.hpp>

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
                , callback_( std::move( callback ) )
        {
            client_.clear_access_channels( websocketpp::log::alevel::all );
            client_.clear_error_channels( websocketpp::log::elevel::all );

            client_.init_asio();

            std::error_code ec;
            auto connection = client_.get_connection( buildSocketUrl( hostname_, port_, "/socket" ), ec );
            if ( ec ) {
                throw std::system_error( ec );
            }
            handle_ = connection->get_handle();

            connection->set_open_handler( std::bind( &Client::handleOpen, this ) );
            connection->set_fail_handler( std::bind( &Client::handleFail, this ) );
            connection->set_close_handler( std::bind( &Client::handleClose, this ) );
            connection->set_message_handler( std::bind( &Client::handleMessage, this, std::placeholders::_2 ) );

            std::cerr << "INFO: Connecting to " << connection->get_uri()->str() << "\n";

            client_.connect( connection );

            thread_ = std::thread( [this] { client_.run(); } );
        }

        Client::~Client()
        {
            close( websocketpp::close::status::normal );
            thread_.join();
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
                            callback_( std::make_error_code( std::errc::invalid_argument ) ); // TODO
                        }
                        else {
                            std::cerr << "INFO: Login successful, connection completed\n";
                            status_ = CONNECTED;
                            callback_( {} );
                        }
                    } );
        }

        void Client::handleFail()
        {
            auto connection = client_.get_con_from_hdl( handle_ );
            std::cerr << "ERROR: Connection has failed: " << connection->get_ec().message() << "\n";
            callback_( connection->get_ec() );
            forceClose();
        }

        void Client::handleClose()
        {
            if ( status_ != CLOSING ) {
                auto connection = client_.get_con_from_hdl( handle_ );
                std::cerr << "ERROR: Connection closed by server: code "
                          << websocketpp::close::status::get_string( connection->get_remote_close_code())
                          << ", reason: " << connection->get_remote_close_reason() << "\n";
                callback_( std::make_error_code( std::errc::invalid_argument ) ); // TODO
            }
            status_ = CLOSED;
        }

        void Client::handleMessage( wsclient::message_ptr message )
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
            client_.close( handle_, code, {}, ec );
            if ( ec ) {
                std::cerr << "WARN: Closing connection normally failed, close forced: " << ec.message() << "\n";
                forceClose();
            }
        }

        void Client::forceClose()
        {
            status_ = CLOSED;

            std::error_code ec;
            client_.close( handle_, websocketpp::close::status::force_tcp_drop, {}, ec );
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

            auto connection = client_.get_con_from_hdl( handle_ );
            connection->send( message, websocketpp::frame::opcode::text );
            actionHandlers_.emplace( callbackId, std::move( handler ) );
        }

        void Client::upload( std::string const& printer, std::string const& modelGroup, Callback< void >&& callback )
        {
            using namespace boost::network;
            using client = http::basic_client< http::tags::http_async_8bit_udp_resolve, 1, 1 >;

            client cl;
            client::request request( buildUploadUrl( hostname_, port_, printer, session_, "Test" ) );
            request << header( "Connection", "close" );
            auto response = cl.post( request, "M28 S0" );

        }

    } // namespace repetier
} // namespace gcu