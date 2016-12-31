#ifndef GCODEUPLOADER_REPETIER_CONNECTION_HPP
#define GCODEUPLOADER_REPETIER_CONNECTION_HPP

#include <cstddef>
#include <memory>
#include <string>

#include "json.hpp"
#include "repetier_collator.hpp"
#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

        class Connection
        {
        public:
            Connection( wsclient& client, std::string const& hostname, std::uint16_t port, std::string const& apikey,
                        ConnectCallback connectCallback, CloseCallback closeCallback, ErrorCallback errorCallback );
            Connection( Connection const& ) = delete;
            ~Connection();

            bool connected() const { return connected_; }

            void takeAction( std::unique_ptr< Action > action );

        private:
            void handleOpen();
            void handleFail();
            void handleClose();
            void handleMessage( wsclient::message_ptr message );

            void takeAction( std::unique_ptr< Action > action, bool connectedOnly );

            wsclient& client_;
            std::string const& apikey_;
            ConnectCallback connectCallback_;
            CloseCallback closeCallback_;
            ErrorCallback errorCallback_;
            websocketpp::connection_hdl handle_;
            JsonContext jsonContext_;
            Collator collator_;

            bool connected_ {};
        };

    } // namespace repetier
} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_CONNECTION_HPP
