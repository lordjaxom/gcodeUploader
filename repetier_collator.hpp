#ifndef GCODEUPLOADER_REPETIER_COLLATOR_HPP
#define GCODEUPLOADER_REPETIER_COLLATOR_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace Json {
    class CharReader;
    class StreamWriter;
    class Value;
} // namespace Json

namespace gcu {
    namespace repetier {

        class Action;

        class Collator
        {
        public:
            Collator();
            Collator( Collator const& ) = delete;
            ~Collator();

            void trackOutgoing( Json::Value& outgoing, std::unique_ptr< Action > action );
            void handleIncoming( Json::Value const& incoming );

        private:
            void handleResponse( Json::Value const& response );
            void handleEventList( Json::Value const& json );

        private:
            std::unordered_map< std::intmax_t, std::unique_ptr< Action > > pendingActions_;

            std::intmax_t nextCallbackId_ {};
        };

    } // namespace repetier
} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_COLLATOR_HPP
