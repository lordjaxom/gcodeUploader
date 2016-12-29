#ifndef GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
#define GCODEUPLOADER_REPETIER_DEFINITIONS_HPP

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

namespace gcu {
    namespace repetier {

        enum class Status : unsigned
        {
            CONNECTING,
            AUTHORIZING,
            CONNECTED,
            CLOSED,
            FAILED
        };

        using StatusCallback = std::function< void ( Status ) >;

        using wsclient = websocketpp::client<websocketpp::config::asio_client>;

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
