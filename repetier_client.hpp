#ifndef GCODEUPLOADER_REPETIER_CLIENT_HPP
#define GCODEUPLOADER_REPETIER_CLIENT_HPP

#include <cstdint>
#include <memory>
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

        class Action;

        class Client
        {
            using wsclient = websocketpp::client<websocketpp::config::asio_client>;

            using Handler = std::function< void ( Json::Value&& response, std::error_code ec ) >;

            enum Status
            {
                CONNECTING,
                CONNECTED,
                CLOSING,
                CLOSED
            };

        public:
            Client( std::string&& hostname, std::uint16_t port, std::string&& apikey, Callback< void >&& callback );
            Client( Client const& ) = delete;
            ~Client();

            bool connected() const { return status_ == CONNECTED; }

            Action action( char const* name );
            void upload( std::string const& printer, std::string const& modelGroup, Callback< void >&& callback );

            void send( Json::Value& request, Handler&& handler );

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
            Callback< void > callback_;
            wsclient client_;
            websocketpp::connection_hdl handle_;
            std::thread thread_;
            std::string session_;
            std::unordered_map< std::intmax_t, Handler > actionHandlers_;
            JsonContext jsonContext_;

            Status status_ { CONNECTING };
            std::intmax_t nextCallbackId_ {};
        };

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_CLIENT_HPP
