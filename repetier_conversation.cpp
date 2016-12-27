#include <iostream>
#include <sstream>

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include "repetier_conversation.hpp"

namespace gcu {
    namespace repetier {

        static std::unique_ptr< Json::CharReader > createJsonReader()
        {
            Json::CharReaderBuilder builder;
            return std::unique_ptr< Json::CharReader >( builder.newCharReader() );
        }

        static std::unique_ptr< Json::StreamWriter > createJsonWriter()
        {
            Json::StreamWriterBuilder builder;
            builder[ "indentation" ] = "";
            return std::unique_ptr< Json::StreamWriter >( builder.newStreamWriter() );
        }

        ConversationFactory::ConversationFactory()
            : jsonReader_( createJsonReader() )
            , jsonWriter_( createJsonWriter() )
        {
        }

        ConversationFactory::~ConversationFactory() = default;

        void ConversationFactory::handleMessage( std::string const& message )
        {
            Json::Value value;
            std::string errors;
            jsonReader_->parse( &*message.begin(), &*message.end(), &value, &errors );

            if ( value[ "callback_id" ].asLargestUInt() != -1 ) {
                handleResponse( value );
            }
            else if ( value[ "eventList" ].asBool() ) {
                handleEventList( value );
            }
            else {
                std::cout << "ERROR: unknown response\n";
            }
        }

        std::string ConversationFactory::toString( Action const* action ) const
        {
            Json::Value json;
            action->toJson( json );

            std::ostringstream os;
            jsonWriter_->write( json, &os );
            return os.str();
        }

        void ConversationFactory::handleResponse( Json::Value const& value )
        {
            auto callbackId = value[ "callback_id" ].asLargestInt();
            auto it = pendingActions_.find( callbackId );
            if ( it == pendingActions_.end() ) {
                std::cout << "ERROR: received response to unrequested callback id " << callbackId << "\n";
                return;
            }
            it->second->handle( value );
            pendingActions_.erase( it );
        }

        void ConversationFactory::handleEventList( Json::Value const& value )
        {
        }

    } // namespace repetier
} // namespace gcu