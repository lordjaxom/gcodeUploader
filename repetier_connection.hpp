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
            Connection( std::string hostname, std::uint16_t port, std::string apikey, repetier::StatusCallback callback, wsclient& client );
            Connection( Connection const& ) = delete;
            ~Connection();

            Status status() const { return status_; }

            void takeAction( std::unique_ptr< Action > action, Status allowed = Status::CONNECTED );

        private:
            void handleOpen();
            void handleFail();
            void handleClose();
            void handleMessage( wsclient::message_ptr message );

            wsclient& client_;
            std::string hostname_;
            std::uint16_t port_;
            std::string apikey_;
            StatusCallback callback_;
            websocketpp::connection_hdl handle_;
            JsonContext jsonContext_;
            Collator collator_;

            Status status_ { Status::CONNECTING };
        };

    } // namespace repetier
} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_CONNECTION_HPP
