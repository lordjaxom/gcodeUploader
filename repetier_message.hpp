#ifndef GCODEUPLOADER_REPETIER_MESSAGE_HPP
#define GCODEUPLOADER_REPETIER_MESSAGE_HPP

#include <cstddef>
#include <string>

#include <json/value.h>
#include <json/writer.h>

namespace gcu {
    namespace repetier {

        class Message
        {
        public:
            Message( std::size_t callbackId );
            virtual ~Message() = 0;

            std::string toString( Json::StreamWriter& writer ) const;

        protected:
            Json::Value value_;
        };

        class ActionMessage
                : public Message
        {
        public:
            ActionMessage( std::size_t callbackId, std::string action );

            std::string const& action() const { return action_; }

        protected:
            std::string const action_;
            Json::Value& data_;
        };

        class LoginMessage
                : public ActionMessage
        {
        public:
            LoginMessage( std::size_t callbackId, std::string action, std::string apikey );

        protected:
            std::string const apikey_;
        };

    } // namespace repetier
} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_MESSAGE_HPP
