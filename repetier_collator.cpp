#include <iostream>
#include <sstream>

#include <json/value.h>

#include "repetier_action.hpp"
#include "repetier_collator.hpp"

namespace gcu {
    namespace repetier {

        Collator::Collator() = default;
        Collator::~Collator() = default;

        void Collator::trackOutgoing( Json::Value& outgoing, std::unique_ptr< Action > action )
        {
            std::intmax_t callbackId = ++nextCallbackId_;
            pendingActions_.emplace( callbackId, std::move( action ) );
            outgoing[ "callback_id" ] = callbackId;
        }

        void Collator::handleIncoming( Json::Value const& incoming )
        {
            if ( incoming[ "callback_id" ].asLargestInt() != -1 ) {
                handleResponse( incoming );
            }
            else if ( incoming[ "eventList" ].asBool() ) {
                handleEventList( incoming );
            }
            else {
                std::cerr << "ERROR: unknown response\n";
            }
        }

        void Collator::handleResponse( Json::Value const& response )
        {
            auto callbackId = response[ "callback_id" ].asLargestInt();
            auto it = pendingActions_.find( callbackId );
            if ( it == pendingActions_.end() ) {
                std::cerr << "ERROR: received response to unrequested callback id " << callbackId << "\n";
                return;
            }
            it->second->handleResponse( response );
            pendingActions_.erase( it );
        }

        void Collator::handleEventList( Json::Value const& json )
        {
        }

    } // namespace repetier
} // namespace gcu