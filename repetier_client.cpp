#include <utility>

#include "repetier_action.hpp"
#include "repetier_client.hpp"
#include "repetier_connection.hpp"

namespace gcu {
    namespace repetier {

        Client::Client( std::string hostname, std::uint16_t port, std::string apikey,
                        ConnectCallback connectCallback, CloseCallback closeCallback, ErrorCallback errorCallback )
            : Client()
        {
            client_.init_asio();
            connect( std::move( hostname ), port, std::move( apikey ), std::move( connectCallback ),
                     std::move( closeCallback ), std::move( errorCallback ) );
            io_thread_ = std::thread( [this] {
                std::cerr << "starting io thread\n";
                client_.run();
                std::cerr << "stopping io thread\n";
            } );
        }

        Client::Client( asio::io_service& io_service, std::string hostname, std::uint16_t port, std::string apikey,
                        ConnectCallback connectCallback, CloseCallback closeCallback, ErrorCallback errorCallback )
            : Client()
        {
            client_.init_asio( &io_service );
            connect( std::move( hostname ), port, std::move( apikey ), std::move( connectCallback ),
                     std::move( closeCallback ), std::move( errorCallback ) );
        }

        Client::Client()
        {
            client_.set_access_channels( websocketpp::log::alevel::none );
            client_.set_error_channels( websocketpp::log::elevel::none );
        }

        Client::~Client()
        {
            if ( connection_ ) {
                std::cerr << "forcibly closing connection\n";
                connection_.reset();
            }

            std::cerr << "joining thread\n";
            io_thread_.join();
        }

        void Client::connect( std::string hostname, std::uint16_t port, std::string apikey,
                              ConnectCallback connectCallback, CloseCallback closeCallback, ErrorCallback errorCallback )
        {
            if ( connection_ && connection_->connected() ) {
                throw std::invalid_argument( "connection already established" );
            }

            hostname_ = std::move( hostname );
            port_ = port;
            apikey_ = std::move( apikey );

            connection_.reset(
                    new Connection( client_, hostname_, port_, apikey_, std::move( connectCallback ),
                                    std::move( closeCallback ), std::move( errorCallback ) ) );
        }

        void Client::takeAction( std::unique_ptr< Action > action )
        {
            if ( !connection_ ) {
                throw std::invalid_argument( "connection not established" );
            }

            connection_->takeAction( std::move( action ) );
        }

    } // namespace repetier
} // namespace gcu