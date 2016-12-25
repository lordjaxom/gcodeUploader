#ifndef GCODEUPLOADER_REPETIER_CONNECTION_HPP
#define GCODEUPLOADER_REPETIER_CONNECTION_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

#include <json/writer.h>

#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

        class Message;

        class Connection
        {
            enum Status
            {
                CONNECTING,
                OPEN,
                FAILED,
                CLOSED
            };

        public:
            Connection( std::string const& url, std::string const& apikey, wsclient& client );
            ~Connection();

        private:
            void handleOpen();
            void handleFail();
            void handleClose();
            void handleMessage( wsclient::message_ptr message );

            template< typename T, typename... Args >
            void sendMessage( Args&&... args );

            std::string const& apikey_;
            std::unique_ptr< Json::StreamWriter > jsonWriter_;
            wsclient& client_;
            websocketpp::connection_hdl handle_;
            std::unordered_map< std::size_t, std::unique_ptr< Message > > pending_;

            Status status_ { CONNECTING };
            std::size_t nextCallbackId_ {};
        };

    } // namespace repetier
} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_CONNECTION_HPP
