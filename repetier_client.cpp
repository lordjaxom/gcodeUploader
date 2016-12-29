#include <utility>

#include "repetier_action.hpp"
#include "repetier_client.hpp"
#include "repetier_connection.hpp"

namespace gcu {
    namespace repetier {

        Client::Client( std::string hostname, std::uint16_t port, std::string apikey, repetier::StatusCallback callback )
                    : Client()
        {
            client_.init_asio();
            connect( std::move( hostname ), std::move( port ), std::move( apikey ), std::move( callback ) );
            io_thread_ = std::thread( [this] { client_.run(); } );
        }

        Client::Client( asio::io_service& io_service, std::string hostname, std::uint16_t port, std::string apikey, repetier::StatusCallback callback )
                : Client()
        {
            client_.init_asio( &io_service );
            connect( std::move( hostname ), std::move( port ), std::move( apikey ), std::move( callback ) );
        }

        Client::Client()
        {
            client_.set_access_channels( websocketpp::log::alevel::none );
            client_.set_error_channels( websocketpp::log::elevel::none );
        }

        Client::~Client() = default;

        void Client::connect( std::string hostname, std::uint16_t port, std::string apikey, repetier::StatusCallback callback )
        {
            connection_.reset( new Connection( std::move( hostname ), std::move( port ), std::move( apikey ), std::move( callback ), client_ ));
        }

        void Client::takeAction( std::unique_ptr< Action > action )
        {
            if ( !connection_ ) {
                throw std::runtime_error( "connection not established" );
            }

            connection_->takeAction( std::move( action ) );
        }

    } // namespace repetier
} // namespace gcu