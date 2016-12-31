#include <algorithm>
#include <iterator>
#include <sstream>
#include <utility>

#include <json/value.h>
#include <json/writer.h>

#include "repetier_action.hpp"

namespace gcu {
    namespace repetier {

        Action::Action( char const* name )
            : name_( name )
        {
        }

        Action::~Action() = default;

        Json::Value Action::createOutgoing() const
        {
            Json::Value outgoing( Json::objectValue );
            outgoing[ "action" ] = Json::StaticString( name_ );
            fillOutgoingData( outgoing["data"] = Json::objectValue );
            return outgoing;
        }

        void Action::handleResponse( Json::Value const& response ) const
        {
            handleResponseData( response[ "data" ] );
        }

        LoginAction::LoginAction( std::string apikey, Callback callback )
            : Action( "login" )
            , apikey_( std::move( apikey ) )
            , callback_( std::move( callback ) )
        {
        }

        void LoginAction::fillOutgoingData( Json::Value& data ) const
        {
            data[ "apikey" ] = Json::StaticString( apikey_.c_str() );
        }

        void LoginAction::handleResponseData( Json::Value const& data ) const
        {
            callback_();
        }

        ListPrinterAction::ListPrinterAction( ListPrinterAction::Callback callback )
                : Action( "listPrinter" )
                , callback_( std::move( callback ) )
        {
        }

        void ListPrinterAction::handleResponseData( Json::Value const& data ) const
        {
            std::vector< Printer > result;
            std::transform(
                    data.begin(), data.end(), std::back_inserter( result ),
                    []( auto const& value ) {
                        return Printer( value[ "active" ].asBool(), value[ "name" ].asString(), value[ "slug" ].asString() );
                    } );
            callback_( result );
        }

        PrinterAction::PrinterAction( char const* name, std::string printer )
                : Action( name )
                , printer_( std::move( printer ) )
        {
        }

        Json::Value PrinterAction::createOutgoing() const
        {
            auto outgoing = Action::createOutgoing();
            outgoing[ "printer" ] = Json::StaticString( printer_.c_str() );
            return outgoing;
        }

        ListModelGroupsAction::ListModelGroupsAction( std::string printer, Callback callback )
                : PrinterAction( "listModelGroups", std::move( printer ) )
                , callback_( std::move( callback ) )
        {
        }

        void ListModelGroupsAction::handleResponseData( Json::Value const& data ) const
        {
            auto const& groupNames = data[ "groupNames" ];
            std::vector< std::string > result( groupNames.size() );
            std::transform(
                    groupNames.begin(), groupNames.end(), result.begin(),
                    []( auto const& value ) { return value.asString(); } );
            callback_( std::move( result ) );
        }

    } // namespace repetier
} // namespace gcu


