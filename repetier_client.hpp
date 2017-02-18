#ifndef GCODEUPLOADER_REPETIER_CLIENT_HPP
#define GCODEUPLOADER_REPETIER_CLIENT_HPP

#include <cstdint>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>

#include <asio/io_service.hpp>

#include <boost/signals2/signal.hpp>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include "json.hpp"
#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

        struct ClientEvents
        {
            boost::signals2::signal< void () > printersChanged;
            boost::signals2::signal< void ( std::string const& printer ) > modelGroupsChanged;
            boost::signals2::signal< void ( std::string const& printer ) > modelsChanged;
        };

        class Client
        {
            using websocketclient = websocketpp::client< websocketpp::config::asio_client >;

            using ConnectHandler = std::function< void ( std::error_code ec ) >;
            using ActionHandler = std::function< void ( Json::Value&& response, std::error_code ec ) >;

            enum Status
            {
                CLOSED,
                CONNECTING,
                CONNECTED,
                CLOSING
            };

        public:
            Client( asio::io_service& service );
            Client( Client const& ) = delete;
            ~Client();

            bool closed() const { return status_ == CLOSED; }
            bool connected() const { return status_ == CONNECTED; }

            void connect( std::string const& hostname, std::uint16_t port, std::string const& apikey,
                          ConnectHandler&& handler );
            void close();
            void sendActionRequest( Json::Value& request, ActionHandler&& handler );

            ClientEvents& events() { return events_; }

        private:
            void handleOpen();
            void handleFail();
            void handleClose();
            void handleMessage( websocketclient::message_ptr message );
            void handleActionResponse( std::intmax_t callbackId, Json::Value&& response );
            void handleEvent( Json::Value&& event );

            void close( bool checked );
            void forceClose();

            void propagateError( std::error_code ec );

            websocketclient wsclient_;
            websocketpp::connection_hdl wshandle_;
            Status status_ { CLOSED };
            std::intmax_t nextCallbackId_ {};
            std::string const* apikey_;
            ConnectHandler connectHandler_;
            std::string session_;
            std::unordered_map< std::intmax_t, ActionHandler > actionHandlers_;
            ClientEvents events_;
            JsonContext jsonContext_;
        };

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_CLIENT_HPP
