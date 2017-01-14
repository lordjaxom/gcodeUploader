#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

#include <json/value.h>

#include "repetier.hpp"
#include "repetier_action.hpp"
#include "repetier_client.hpp"

namespace gcu {

    template< typename Client, typename... Args >
    static void sendActionRequest( Client& client, Args&&... args )
    {
        if ( !client || !client->connected() ) {
            throw std::invalid_argument( "connection not established" );
        }
        client->sendActionRequest( std::forward< Args >( args )... );
    }

    RepetierClient::RepetierClient() = default;
    RepetierClient::~RepetierClient() = default;

    void RepetierClient::connect(
            std::string hostname, uint16_t port, std::string apikey, repetier::Callback callback )
    {
        if ( client_ ) {
            throw std::invalid_argument( "connection already in progress" );
        }

        client_.reset(
                new repetier::Client( std::move( hostname ), port, std::move( apikey ), std::move( callback ) ) );
    }

    void RepetierClient::listPrinter( repetier::ListPrinterCallback callback )
    {
        using namespace repetier::action;
        client_->action( "listPrinter" )
                .handle( transform< repetier::Printer >( []( auto&& printer ) {
                    return repetier::Printer( printer[Json::StaticString( "active" )].asBool(),
                                              printer[Json::StaticString( "name" )].asString(),
                                              printer[Json::StaticString( "slug" )].asString());
                } ) )
                .send( std::move( callback ) );
    }

    void RepetierClient::listModelGroups( std::string const& printer, repetier::ListModelGroupsCallback callback )
    {
        using namespace repetier::action;
        client_->action( "listModelGroups" )
                .printer( printer.c_str() )
                .handle( checkOkFlag() )
                .handle( resolveKey( "groupNames" ) )
                .handle( transform< std::string >( []( auto&& groupName ) { return groupName.asString(); } ) )
                .send( std::move( callback ) );
    }

    void RepetierClient::addModelGroup( std::string const& printer, std::string const& modelGroup,
                                        repetier::Callback callback )
    {
        using namespace repetier::action;
        client_->action( "addModelGroup" )
                .printer( printer.c_str() )
                .arg( "groupName", modelGroup.c_str() )
                .handle( checkOkFlag() )
                .send( [callback]( auto&&, std::error_code ec ) { callback( ec ); } ); // TODO
    }

} // namespace gcu
