#ifndef GCODEUPLOADER_REPETIER_CLIENT_HPP
#define GCODEUPLOADER_REPETIER_CLIENT_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

        class Connection;

        class Client
        {
        public:
            Client( std::string const& hostname, std::uint16_t port, std::string const& apikey );
            Client( std::string const& hostname, std::uint16_t port, std::string const& apikey, asio::io_service* io_service );
            Client( Client const& ) = delete;
            ~Client();

        private:
            Client();

            void connect( std::string const& hostname, std::uint16_t port, std::string const& apikey );

            std::thread io_thread_;
            wsclient client_;
            std::unique_ptr< Connection > connection_;
        };

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_CLIENT_HPP
