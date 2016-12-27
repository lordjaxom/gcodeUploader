#include <sstream>
#include <utility>

#include "repetier_client.hpp"
#include "repetier_connection.hpp"

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
            // client_.set_access_channels( websocketpp::log::alevel::all );
            // client_.set_error_channels( websocketpp::log::elevel::all );
        }

        Client::~Client() = default;

        void Client::connect( std::string const& hostname, std::uint16_t port, std::string const& apikey )
        {
            connection_.reset( new Connection( buildUrl( hostname, port, "/socket/" ), apikey, client_ ));
        }

    } // namespace repetier
} // namespace gcu