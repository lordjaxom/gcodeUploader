#include <functional>
#include <sstream>
#include <utility>
#include <json/json.h>

#include "repetier_client.hpp"

namespace gcu {
    namespace repetier {

        static std::string buildUrl( std::string const& hostname, std::uint16_t port, std::string const& resource )
        {
            std::ostringstream os;
            os << "ws://" << hostname << ':' << port;
            if ( !resource.empty() && resource.front() != '/' ) {
                os << '/';
            }
            os << resource;
            return os.str();
        }

        Client::Client( std::string&& hostname, std::uint16_t port, std::string&& apikey, ConnectCallback&& callback )
                : hostname_( std::move( hostname ) )
                , port_( port )
                , apikey_( std::move( apikey ) )
                , callback_( std::move( callback ) )
        {
            client_.clear_access_channels( websocketpp::log::alevel::all );
            client_.clear_error_channels( websocketpp::log::elevel::all );

            client_.init_asio();

            std::error_code ec;
            auto connection = client_.get_connection( buildUrl( hostname_, port_, "/socket" ), ec );
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
            std::error_code ec;
            client_.close( handle_, websocketpp::close::status::going_away, "", ec );
            thread_.join();
        }

        void Client::handleOpen()
        {
            std::cerr << "INFO: Connection established, logging in\n";

            Json::Value data = Json::objectValue;
            data[ Json::StaticString( "apikey" ) ] = Json::StaticString( apikey_.c_str() );
            sendActionRequest( "login", std::move( data ), []( Json::Value&& ) {

            } );
        }

        void Client::handleFail()
        {
            auto connection = client_.get_con_from_hdl( handle_ );
            std::cerr << "ERROR: Connection has failed: " << connection->get_ec().message() << "\n";
        }

        void Client::handleClose()
        {
            auto connection = client_.get_con_from_hdl( handle_ );
            std::cerr << "ERROR: Connection closed by server: code "
                      << websocketpp::close::status::get_string( connection->get_remote_close_code() )
                      << ", reason: " << connection->get_remote_close_reason() << "\n";
        }

        void Client::handleMessage( wsclient::message_ptr message )
        {
            auto incoming = jsonContext_.toJson( message->get_payload() );
            if ( incoming[ "callback_id" ].asLargestInt() != -1 ) {

                std::cerr << "<<< " << message->get_payload() << "\n";
            }
            // collator_.handleIncoming( incoming );
        }

        void Client::sendActionRequest( std::string&& action, Json::Value&& data, ActionHandler&& callback )
        {
            auto callbackId = ++nextCallbackId_;

            Json::Value request = Json::objectValue;
            request[ Json::StaticString( "action" ) ] = std::move( action );
            request[ Json::StaticString( "callback_id" ) ] = callbackId;
            request[ Json::StaticString( "data" ) ] = std::move( data );

            auto message = jsonContext_.toString( request );
            std::cerr << ">>> " << message << "\n";

            auto connection = client_.get_con_from_hdl( handle_ );
            connection->send( message, websocketpp::frame::opcode::text );
            actionHandlers_.emplace( callbackId, std::move( callback ) );
        }

    } // namespace repetier
} // namespace gcu