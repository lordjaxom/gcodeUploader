#include <sstream>
#include <utility>

#include <json/value.h>
#include <json/writer.h>

#include "repetier_action.hpp"

namespace gcu {
    namespace repetier {

        Action::Action( std::intmax_t callbackId, char const* action )
            : callbackId_( callbackId )
            , action_( action )
        {
        }

        Action::~Action() = default;

        void Action::handle( Json::Value const& value ) const
        {
            doHandle( value );
        }

        void Action::toJson( Json::Value& value ) const
        {
            value = Json::objectValue;
            value[ "callback_id" ] = callbackId_;
            value[ "action" ] = Json::StaticString( action_ );
            doToJson( value[ "data" ] = Json::objectValue );
        }

        LoginAction::LoginAction( std::intmax_t callbackId, std::string apikey, Callback callback )
            : Action( callbackId, "login" )
            , apikey_( std::move( apikey ) )
            , callback_( std::move( callback ) )
        {
        }

        void LoginAction::doHandle( Json::Value const& value ) const
        {
            callback_( value[ "session" ].asCString() );
        }

        void LoginAction::doToJson( Json::Value& data ) const
        {
            data[ "apikey" ] = Json::StaticString( apikey_.c_str() );
        }

    } // namespace repetier
} // namespace gcu


