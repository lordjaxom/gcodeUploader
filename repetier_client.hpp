#ifndef GCODEUPLOADER_REPETIER_CLIENT_HPP
#define GCODEUPLOADER_REPETIER_CLIENT_HPP

#include <memory>

#include <boost/thread.hpp>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

namespace gcu {

    class RepetierClient
    {
        using wsclient = websocketpp::client< websocketpp::config::asio_client >;

    public:
        RepetierClient();
        explicit RepetierClient( asio::io_service& service );

    private:
        void init();

        wsclient client_;
        std::unique_ptr< boost::thread > thread_;
    };

} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_CLIENT_HPP
