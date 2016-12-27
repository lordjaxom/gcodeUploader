#ifndef GCODEUPLOADER_REPETIER_CONNECTION_HPP
#define GCODEUPLOADER_REPETIER_CONNECTION_HPP

#include <cstddef>
#include <memory>
#include <string>

#include "repetier_conversation.hpp"
#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

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
            Connection( Connection const& ) = delete;
            ~Connection();

        private:
            void handleOpen();
            void handleFail();
            void handleClose();
            void handleMessage( wsclient::message_ptr message );

            void send( Action const* action );

            std::string const& apikey_;
            wsclient& client_;
            websocketpp::connection_hdl handle_;
            ConversationFactory factory_;

            Status status_ { CONNECTING };
        };

    } // namespace repetier
} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_CONNECTION_HPP
