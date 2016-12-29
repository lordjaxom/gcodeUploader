#ifndef GCODEUPLOADER_REPETIER_CLIENT_HPP
#define GCODEUPLOADER_REPETIER_CLIENT_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

        class Action;
        class Connection;

        class Client
        {
        public:
            Client( std::string hostname, std::uint16_t port, std::string apikey, repetier::StatusCallback callback );
            Client( asio::io_service& io_service, std::string hostname, std::uint16_t port, std::string apikey, repetier::StatusCallback callback );
            Client( Client const& ) = delete;
            ~Client();

            void takeAction( std::unique_ptr< Action > action );

        private:
            Client();

            void connect( std::string hostname, std::uint16_t port, std::string apikey, repetier::StatusCallback callback );

            std::thread io_thread_;
            wsclient client_;
            std::unique_ptr< Connection > connection_;
        };

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_CLIENT_HPP
