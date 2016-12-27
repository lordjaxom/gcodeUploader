#include <functional>
#include <system_error>

#include <json/json.h>

#include "repetier_action.hpp"
#include "repetier_connection.hpp"

using namespace std::placeholders;

namespace gcu {
    namespace repetier {

        Connection::Connection( std::string const& url, std::string const& apikey, wsclient& client )
            : client_( client )
            , apikey_( apikey )
        {
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
            send( factory_.createAction< LoginAction >( apikey_, []( char const* session ) { std::cout << "YAY! got session: " << session << "\n"; } ) );
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
            // std::cout << "<<< " << message->get_payload() << std::endl;
            factory_.handleMessage( message->get_payload() );
        }

        void Connection::send( Action const* action )
        {
            auto connection = client_.get_con_from_hdl( handle_ );
            auto payload = factory_.toString( action );
            // std::cout << ">>> " << payload << "\n";
            connection->send( payload, websocketpp::frame::opcode::text );
        }

    } // namespace repetier
} // namespace gcu
