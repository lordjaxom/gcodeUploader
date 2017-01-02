#include <functional>
#include <sstream>
#include <system_error>
#include <utility>

#include <json/json.h>

#include "repetier_action.hpp"
#include "repetier_connection.hpp"

using namespace std::placeholders;

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

        Connection::Connection(
                wsclient& client, std::string const& hostname, std::uint16_t port, std::string const& apikey,
                ConnectCallback connectCallback, CloseCallback closeCallback, ErrorCallback errorCallback )
            : client_( client )
            , apikey_( std::move( apikey ) )
            , connectCallback_( std::move( connectCallback ) )
            , closeCallback_( std::move( closeCallback ) )
            , errorCallback_( std::move( errorCallback ) )
        {
            std::string url = buildUrl( hostname, port, "/socket" );
            std::cerr << "INFO: Connecting to " << url << "\n";

            std::error_code ec;
            auto connection = client_.get_connection( url, ec );
            if ( ec ) {
                errorCallback_( ec );
                return;
            }
            handle_ = connection->get_handle();

            connection->set_open_handler( std::bind( &Connection::handleOpen, this ) );
            connection->set_fail_handler( std::bind( &Connection::handleFail, this ) );
            connection->set_close_handler( std::bind( &Connection::handleClose, this ) );
            connection->set_message_handler( std::bind( &Connection::handleMessage, this, _2 ) );

            client_.connect( connection );
        }

        Connection::~Connection()
        {
            std::error_code ec;
            std::cerr << "sending websocket close\n";
            client_.close( handle_, websocketpp::close::status::going_away, "", ec );
        }

        void Connection::handleOpen()
        {
            takeAction( std::make_unique< LoginAction >( apikey_, [this] {
                connected_ = true;
                connectCallback_();
                std::cerr << "INFO: Connection established\n";
            } ), false );
        }

        void Connection::handleFail()
        {
            auto connection = client_.get_con_from_hdl( handle_ );
            errorCallback_( connection->get_ec() );
            std::cerr << "ERROR: Connection has failed: " << connection->get_ec().message() << "\n";
        }

        void Connection::handleClose()
        {
            auto connection = client_.get_con_from_hdl( handle_ );
            closeCallback_( connection->get_remote_close_reason() );
            std::cerr << "ERROR: Connection closed by server: code "
                      << websocketpp::close::status::get_string( connection->get_remote_close_code() )
                      << ", reason: " << connection->get_remote_close_reason() << "\n";
        }

        void Connection::handleMessage( wsclient::message_ptr message )
        {
            auto incoming = jsonContext_.toJson( message->get_payload() );
            if ( incoming[ "callback_id" ].asLargestInt() != -1 ) {
                std::cerr << "<<< " << message->get_payload() << "\n";
            }
            collator_.handleIncoming( incoming );
        }

        void Connection::takeAction( std::unique_ptr< Action > action )
        {
            takeAction( std::move( action ), true );
        }

        void Connection::takeAction( std::unique_ptr< Action > action, bool connectedOnly )
        {
            if ( connectedOnly && !connected_ ) {
                throw std::invalid_argument( "connection not ready" );
            }

            auto outgoing = action->createOutgoing();
            collator_.trackOutgoing( outgoing, std::move( action ));

            auto payload = jsonContext_.toString( outgoing );
            std::cerr << ">>> " << payload << "\n";
            auto connection = client_.get_con_from_hdl( handle_ );
            connection->send( payload, websocketpp::frame::opcode::text );
        }

    } // namespace repetier
} // namespace gcu
