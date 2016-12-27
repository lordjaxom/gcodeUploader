#ifndef GCODEUPLOADER_REPETIER_HPP
#define GCODEUPLOADER_REPETIER_HPP

#include <cstdint>
#include <memory>
#include <string>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include "repetier_definitions.hpp"

namespace asio {
    class io_service;
} // namespace asio

namespace gcu {

    namespace repetier {
        class Client;
    } // namespace repetier

    class RepetierApi
    {
    public:
        RepetierApi();
        RepetierApi( RepetierApi const& ) = delete;
        explicit RepetierApi( asio::io_service& io_service );
        RepetierApi( std::string hostname, std::uint16_t port, std::string apikey );
        RepetierApi( std::string hostname, std::uint16_t port, std::string apikey, asio::io_service& io_service );
        ~RepetierApi();

        void connect( std::string hostname, std::uint16_t port, std::string apikey );

    private:
        void connect();

        std::string hostname_;
        std::uint16_t port_;
        std::string apikey_;
        asio::io_service* io_service_;
        std::unique_ptr< repetier::Client > client_;
    };

} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_HPP
