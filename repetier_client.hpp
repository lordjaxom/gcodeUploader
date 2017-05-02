#ifndef GCODEUPLOADER_REPETIER_CLIENT_HPP
#define GCODEUPLOADER_REPETIER_CLIENT_HPP

#include <cstdint>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include <asio/io_service.hpp>

#include <boost/signals2/signal.hpp>

#include <json.hpp>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include "std_optional.hpp"

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
            using ActionHandler = std::function< void ( nlohmann::json&& response, std::error_code ec ) >;

            enum Status
            {
                CLOSED,
                CONNECTING,
                CONNECTED,
                CLOSING
            };

            struct Action
            {
                Action( std::intmax_t callbackId, nlohmann::json&& request, ActionHandler&& handler )
                        : callbackId( callbackId )
                        , request( std::move( request ) )
                        , handler( std::move( handler ) )
                {
                    this->request[ "callback_id" ] = callbackId;
                }

                std::intmax_t callbackId;
                nlohmann::json request;
                ActionHandler handler;
                bool pending {};
            };

        public:
            Client( asio::io_service& service );
            Client( Client const& ) = delete;
            ~Client();

            bool closed() const { return status_ == CLOSED; }
            bool connected() const { return status_ == CONNECTED; }

            void retry( std::size_t retryCount ) { retryCount_ = retryCount; }

            void connect(
                    std::string const& hostname, std::uint16_t port, std::string const& apikey,
                    ConnectHandler&& handler );
            void close();
            void send( nlohmann::json&& request, ActionHandler&& handler );

            ClientEvents& events() { return events_; }

        private:
            void handleOpen();
            void handleFail();
            void handleClose();
            void handleMessage( websocketclient::message_ptr message );
            void handleActionResponse( std::intmax_t callbackId, nlohmann::json&& response );
            void handleEvent( nlohmann::json&& event );

            bool reconnect();
            void connect();
            void sendIfReady();
            void close( bool checked );
            void forceClose();

            void propagateError( std::error_code ec );

            std::size_t retryCount_;
            std::size_t errorCount_;
            websocketclient wsclient_;
            websocketpp::connection_hdl wshandle_;
            Status status_ { CLOSED };
            std::string const* hostname_;
            std::uint16_t port_;
            std::string const* apikey_;
            ConnectHandler connectHandler_;
            std::intmax_t loginCallbackId_;
            std::intmax_t nextCallbackId_ {};
            std::list< Action > actionQueue_;
            std::recursive_mutex actionMutex_;
            ClientEvents events_;
        };

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_CLIENT_HPP
