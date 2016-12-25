#include <sstream>
#include <utility>

#include "repetier_client.hpp"
#include "repetier_connection.hpp"

namespace gcu {
/*
    RepetierClient::RepetierClient()
        : RepetierClient( DISCONNECTED )
    {
    }

    RepetierClient::RepetierClient( std::string url )
        : RepetierClient( CONNECTING )
    {
        connect( std::move( url ) );
    }

    RepetierClient::RepetierClient( RepetierClient::Status status )
        : status_( status )
        , pingTimer_( io_service_ )
    {
        client_.set_access_channels( websocketpp::log::alevel::all );
        client_.set_error_channels( websocketpp::log::elevel::all );

        client_.init_asio( &io_service_ );
        client_.set_user_agent( "websocket-sharp/1.0" );
        client_.start_perpetual();

        io_thread_ = std::thread( [this] {
            std::cout << "service thread starting\n";
            io_service_.run();
            std::cout << "service thread stopped\n";
        } );
    }

    RepetierClient::~RepetierClient()
    {
        client_.stop_perpetual();
        close();
        io_thread_.join();
    }

    void RepetierClient::connect( std::string url )
    {
        std::error_code ec;

        url_ = std::move( url );

        std::cout << "connecting to " << url_ << "\n";

        auto connection = client_.get_connection( url_, ec );
        if ( ec ) {
            throw std::system_error( ec, "couldn't initialize connection: " + ec.message() );
        }
        handle_ = connection->get_handle();

        connection->set_open_handler( [this]( websocketpp::connection_hdl h ) {
            auto c = client_.get_con_from_hdl( h );
            std::cout << "connection to " << c->get_response_header( "Server" ) << " established\n";
            c->send( std::string( "{action:\"login\",data:{apikey:\"7f77558d-75e1-45e1-b424-74c5c81b6b47\"}}" ), websocketpp::frame::opcode::text );
        } );
        connection->set_fail_handler( [this] ( websocketpp::connection_hdl h ) {
            auto c = client_.get_con_from_hdl( h );
            std::cout << "operation on " << c->get_response_header( "Server" ) << " failed:"
                      << c->get_ec().message() << "\n";
        } );
        connection->set_close_handler( [this] ( websocketpp::connection_hdl h ) {
            auto c = client_.get_con_from_hdl( h );
            std::cout << "connection closed\n";
        } );
        connection->set_message_handler( [this] ( websocketpp::connection_hdl h, wsclient::message_ptr message ) {
            if ( message->get_opcode() == websocketpp::frame::opcode::text ) {
                std::cout << "received text: " << message->get_payload() << "\n";
            }
            else {
                std::cout << "received binary\n";
            }
        } );

        client_.connect( connection );
    }

    void RepetierClient::close()
    {
        if ( status_ == OPEN ) {
            client_.close( handle_, websocketpp::close::status::going_away, "" );
        }
    }
*/
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

        Client::Client( std::string const& hostname, std::uint16_t port, std::string const& apikey )
                    : Client()
        {
            client_.init_asio();
            connect( hostname, port, apikey );
            io_thread_ = std::thread( [this] { client_.run(); } );
        }

        Client::Client( std::string const& hostname, std::uint16_t port, std::string const& apikey, asio::io_service* io_service )
                : Client()
        {
            client_.init_asio( io_service );
            connect( hostname, port, apikey );
        }

        Client::Client()
        {
            client_.set_access_channels( websocketpp::log::alevel::all );
            client_.set_error_channels( websocketpp::log::elevel::all );
        }

        Client::~Client()
        {
        }

        void Client::connect( std::string const& hostname, std::uint16_t port, std::string const& apikey )
        {
            connection_.reset( new Connection( buildUrl( hostname, port, "/socket/" ), apikey, client_ ));
        }

    } // namespace repetier
} // namespace gcu