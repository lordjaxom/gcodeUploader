#ifndef GCODEUPLOADER_REPETIER_CONVERSATION_HPP
#define GCODEUPLOADER_REPETIER_CONVERSATION_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "repetier_action.hpp"

namespace Json {
    class CharReader;
    class StreamWriter;
} // namespace Json

namespace gcu {
    namespace repetier {

        class ConversationFactory
        {
        public:
            ConversationFactory();
            ConversationFactory( ConversationFactory const& ) = delete;
            ~ConversationFactory();

            template< typename T, typename... Args >
            T* createAction( Args&&... args )
            {
                std::intmax_t callbackId = ++nextCallbackId_;
                T* action = new T( callbackId, std::forward< Args >( args )... );
                pendingActions_.emplace( callbackId, std::unique_ptr< Action >( action ) );
                return action;
            }

            void handleMessage( std::string const& message );

            std::string toString( Action const* action ) const;

        private:
            void handleResponse( Json::Value const& value );
            void handleEventList( Json::Value const& value );

            std::unordered_map< std::intmax_t, std::unique_ptr< Action > > pendingActions_;
            std::unique_ptr< Json::CharReader > jsonReader_;
            std::unique_ptr< Json::StreamWriter > jsonWriter_;

            std::intmax_t nextCallbackId_ {};
        };

    } // namespace repetier
} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_CONVERSATION_HPP
