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
            using websocketclient = websocketpp::client< websocketpp::config::asio_client >;

            using Handler = std::function< void ( Json::Value&& response, std::error_code ec ) >;

            enum Status
            {
                CONNECTING,
                CONNECTED,
                CLOSING,
                CLOSED
            };

        public:
            Client( std::string const& hostname, std::uint16_t port, std::string&& apikey, Callback< void >&& callback );
            Client( Client const& ) = delete;
            ~Client();

            bool connected() const { return status_ == CONNECTED; }

            Action action( char const* name );
            void send( Json::Value& request, Handler&& handler );

        private:
            void handleFail();
            void handleClose();
            void handleMessage( websocketclient::message_ptr message );

            void handleActionResponse( std::intmax_t callbackId, Json::Value&& response );

            void login( std::string const& apikey, Callback< void > const& callback );
            void close( websocketpp::close::status::value code );
            void forceClose();

            void propagateError( std::error_code ec );

            websocketclient wsclient_;
            websocketpp::connection_hdl wshandle_;
            std::thread wsthread_;

            JsonContext jsonContext_;

            Status status_ { CONNECTING };
            std::string session_;
            std::intmax_t nextCallbackId_ {};
            std::unordered_map< std::intmax_t, Handler > actionHandlers_;
        };

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_CLIENT_HPP
