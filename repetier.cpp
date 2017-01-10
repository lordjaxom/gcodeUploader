#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

#include <json/value.h>

#include "repetier.hpp"
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
        sendActionRequest( client_, "listPrinter", "", Json::objectValue, [callback]( auto&& data ) {
            std::vector< repetier::Printer > result;
            std::transform( data.begin(), data.end(), std::back_inserter( result ),
                            []( auto const& printer ) {
                                return repetier::Printer( printer[ Json::StaticString( "active" ) ].asBool(),
                                                          printer[ Json::StaticString( "name" ) ].asString(),
                                                          printer[ Json::StaticString( "slug" ) ].asString() );
                            } );
            callback( std::move( result ), {} );
        } );
    }

    void RepetierClient::listModelGroups( std::string const& printer, repetier::ListModelGroupsCallback callback )
    {
        sendActionRequest( client_, "listModelGroups", printer, Json::objectValue, [callback]( auto&& data ) {
            std::error_code ec; // TODO

            std::vector< std::string > result;
            if ( data[ Json::StaticString( "ok" ) ].asBool() ) {
                auto&& modelGroups = data[ Json::StaticString( "groupNames" ) ];
                std::transform( modelGroups.begin(), modelGroups.end(), std::back_inserter( result ),
                                []( auto const& modelGroup ) { return modelGroup.asString(); } );
            }
            else {
                ec = std::make_error_code( std::errc::invalid_argument ); // TODO
            }
            callback( std::move( result ), ec );
        } );
    }

    void RepetierClient::addModelGroup( std::string const& printer, std::string const& modelGroup,
                                        repetier::Callback callback )
    {
        Json::Value data = Json::objectValue;
        data[ Json::StaticString( "groupName" ) ] = Json::StaticString( modelGroup.c_str() );
        sendActionRequest( client_, "addModelGroup", printer, std::move( data ), [callback]( auto&& data ) {
            std::error_code ec; // TODO
            if ( !data[ Json::StaticString( "ok" ) ].asBool() ) {
                ec = std::make_error_code( std::errc::invalid_argument ); // TODO
            }
            callback( ec );
        } );
    }

} // namespace gcu
