#include <functional>
#include <system_error>

#include <json/json.h>

#include "repetier_connection.hpp"
#include "repetier_message.hpp"

using namespace std::placeholders;

namespace gcu {
    namespace repetier {

        Connection::Connection( std::string const& url, std::string const& apikey, wsclient& client )
            : client_( client )
            , apikey_( apikey )
        {
            Json::StreamWriterBuilder builder;
            builder[ "indentation" ] = "";
            jsonWriter_.reset( builder.newStreamWriter() );

            std::cout << "connecting to RepetierServer at " << url << "\n";

            std::error_code ec;
            auto connection = client_.get_connection( url, ec );
            if ( ec ) {
                throw std::system_error( ec, "couldn't initialize connection: " + ec.message() );
            }
            handle_ = connection->get_handle();

            connection->set_open_handler( std::bind( &Connection::handleOpen, this ) );
            connection->set_fail_handler( std::bind( &Connection::handleFail, this ) );
            connection->set_close_handler( std::bind( &Connection::handleClose, this ) );
            connection->set_message_handler( std::bind( &Connection::handleMessage, this, _2 ) );

            client_.connect( connection );
        }

        Connection::~Connection() = default;

        void Connection::handleOpen()
        {
            status_ = OPEN;

            sendMessage< LoginMessage >( "login", apikey_ );
        }

        void Connection::handleFail()
        {
            status_ = FAILED;
        }

        void Connection::handleClose()
        {
            status_ = CLOSED;
        }

        void Connection::handleMessage( wsclient::message_ptr message )
        {

        }

        template< typename T, typename... Args >
        void Connection::sendMessage( Args&&... args )
        {
            std::size_t callbackId = ++nextCallbackId_;
            std::unique_ptr< T > message( new T( callbackId, std::forward< Args >( args )... ) );

            auto connection = client_.get_con_from_hdl( handle_ );
            auto payload = message->toString( *jsonWriter_ );
            std::cout << ">>> " << payload << "\n";
            connection->send( payload, websocketpp::frame::opcode::text );

            pending_.emplace( callbackId, std::move( message ) );
        }

    } // namespace repetier
} // namespace gcu
