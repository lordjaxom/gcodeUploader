#ifndef GCODEUPLOADER_REPETIER_CLIENT_HPP
#define GCODEUPLOADER_REPETIER_CLIENT_HPP

#include <cstdint>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include "json.hpp"
#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

        class Client
        {
            using websocketclient = websocketpp::client< websocketpp::config::asio_client >;

            using ConnectHandler = std::function< void ( std::error_code ec ) >;
            using ActionHandler = std::function< void ( Json::Value&& response, std::error_code ec ) >;

            enum Status
            {
                NONE,
                CONNECTING,
                CONNECTED,
                CLOSING,
                CLOSED
            };

        public:
            Client();
            Client( Client const& ) = delete;
            ~Client();

            bool connected() const { return status_ == CONNECTED; }

            void connect( std::string const& hostname, std::uint16_t port, std::string const& apikey,
                          ConnectHandler&& handler );
            void close();

            void sendActionRequest( Json::Value& request, ActionHandler&& handler );

        private:
            void handleOpen( std::string const& apikey, ConnectHandler const& handler );
            void handleFail( ConnectHandler const& handler );

            void handleClose();

            void handleMessage( websocketclient::message_ptr message );
            void handleActionResponse( std::intmax_t callbackId, Json::Value&& response );

            void forceClose();

            void propagateError( std::error_code ec );

            websocketclient wsclient_;
            std::thread wsthread_;
            websocketpp::connection_hdl wshandle_;
            Status status_ { NONE };
            std::intmax_t nextCallbackId_ {};
            std::string session_;
            std::unordered_map< std::intmax_t, ActionHandler > actionHandlers_;
            JsonContext jsonContext_;
        };

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_CLIENT_HPP
