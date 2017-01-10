#ifndef GCODEUPLOADER_REPETIER_CLIENT_HPP
#define GCODEUPLOADER_REPETIER_CLIENT_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include "json.hpp"
#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

        class Client
        {
            using wsclient = websocketpp::client<websocketpp::config::asio_client>;

            using ActionHandler = std::function< void( Json::Value&& data, std::error_code ec ) >;

            enum Status
            {
                CONNECTING,
                CONNECTED,
                CLOSING,
                CLOSED
            };

        public:
            Client( std::string&& hostname, std::uint16_t port, std::string&& apikey, Callback&& callback );
            Client( Client const& ) = delete;
            ~Client();

            bool connected() const { return status_ == CONNECTED; }

            void sendActionRequest( std::string const& action, std::string const& printer, Json::Value&& data,
                                    ActionHandler&& handler );

        private:
            void handleOpen();
            void handleFail();
            void handleClose();
            void handleMessage( wsclient::message_ptr message );

            void close( websocketpp::close::status::value code );
            void forceClose();

            void handleActionResponse( std::intmax_t callbackId, Json::Value&& response );

            std::string hostname_;
            std::uint16_t port_;
            std::string apikey_;
            Callback callback_;
            wsclient client_;
            websocketpp::connection_hdl handle_;
            std::thread thread_;
            std::unordered_map< std::intmax_t, ActionHandler > actionHandlers_;
            JsonContext jsonContext_;

            Status status_ { CONNECTING };
            std::intmax_t nextCallbackId_ {};
        };

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_CLIENT_HPP
