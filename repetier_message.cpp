#include <sstream>
#include <utility>

#include "repetier_message.hpp"

namespace gcu {
    namespace repetier {

        Message::Message( std::size_t callbackId )
            : value_( Json::objectValue )
        {
            value_[ "callbackId" ] = callbackId;
        }

        Message::~Message() = default;

        std::string Message::toString( Json::StreamWriter& writer ) const
        {
            std::ostringstream os;
            writer.write( value_, &os );
            return os.str();
        }

        ActionMessage::ActionMessage( std::size_t callbackId, std::string action )
                : Message( callbackId )
                , action_( std::move( action ) )
                , data_( value_[ "data" ] = Json::Value( Json::objectValue ) )
        {
            value_[ "action" ] = Json::StaticString( action_.c_str() );
        }

        LoginMessage::LoginMessage( std::size_t callbackId, std::string action, std::string apikey )
                : ActionMessage( callbackId, std::move( action ) )
                , apikey_( std::move( apikey ) )
        {
            data_[ "apikey" ] = Json::StaticString( apikey_.c_str() );
        }

    } // namespace repetier
} // namespace gcu
