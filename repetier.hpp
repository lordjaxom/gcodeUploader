#ifndef GCODEUPLOADER_REPETIER_HPP
#define GCODEUPLOADER_REPETIER_HPP

#include <memory>
#include <string>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

namespace asio {
    class io_service;
} // namespace asio

namespace gcu {

    namespace repetier {

        using wsclient = websocketpp::client< websocketpp::config::asio_client >;

        class Client;

    } // namespace repetier

    class RepetierApi
    {
    public:
        RepetierApi();
        ~RepetierApi();
        explicit RepetierApi( asio::io_service& io_service );
        RepetierApi( std::string hostname, unsigned short port, std::string apikey );
        RepetierApi( std::string hostname, unsigned short port, std::string apikey, asio::io_service& io_service );

        void connect( std::string hostname, unsigned short port, std::string apikey );

    private:
        void connect();

        std::string hostname_;
        unsigned short port_;
        std::string apikey_;
        asio::io_service* io_service_;
        std::unique_ptr< repetier::Client > client_;
    };

} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_HPP
