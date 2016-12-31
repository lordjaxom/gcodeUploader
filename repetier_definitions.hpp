#ifndef GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
#define GCODEUPLOADER_REPETIER_DEFINITIONS_HPP

#include <functional>
#include <string>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

namespace gcu {
    namespace repetier {

        using ConnectCallback = std::function< void () >;
        using CloseCallback = std::function< void ( std::string const& ) >;
        using ErrorCallback = std::function< void ( std::error_code ) >;

        class Printer
        {
        public:
            Printer( bool active, std::string name, std::string slug );

            bool active() const { return active_; }
            std::string const& name() const { return name_; }
            std::string const& slug() const { return slug_; }

        private:
            bool active_;
            std::string name_;
            std::string slug_;
        };

        using wsclient = websocketpp::client<websocketpp::config::asio_client>;

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
