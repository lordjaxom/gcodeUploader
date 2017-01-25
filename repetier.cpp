#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

#include <json/value.h>

#include "repetier.hpp"
#include "repetier_action.hpp"
#include "repetier_client.hpp"

namespace gcu {

    RepetierClient::RepetierClient() = default;

    RepetierClient::~RepetierClient()
    {
        client_ = std::experimental::nullopt;
        thread_.join();
    }

    std::string RepetierClient::session() const
    {
        return client_->session();
    }

    void RepetierClient::connect( std::string hostname, std::uint16_t port, std::string apikey,
                                  repetier::Callback<> callback )
    {
        if ( !client_->closed() ) {
            throw std::invalid_argument( "connect() called on an already connected client" );
        }

        hostname_ = std::move( hostname );
        port_ = port;
        apikey_ = std::move( apikey );

        client_->connect( hostname_, port_, apikey_, std::move( callback ) );
    }

    void RepetierClient::listPrinter( repetier::Callback< std::vector< repetier::Printer > > callback )
    {
        using namespace repetier::action;
        repetier::makeAction( &*client_, "listPrinter" )
                .handle( transform< repetier::Printer >( []( auto&& printer ) {
                    return repetier::Printer( printer[Json::StaticString( "active" )].asBool(),
                                              printer[Json::StaticString( "name" )].asString(),
                                              printer[Json::StaticString( "slug" )].asString());
                } ) )
                .send( std::move( callback ) );
    }

    void RepetierClient::listModelGroups( std::string const& printer,
                                          repetier::Callback< std::vector< std::string > > callback )
    {
        using namespace repetier::action;
        repetier::makeAction( &*client_, "listModelGroups" )
                .printer( printer.c_str() )
                .handle( checkOkFlag() )
                .handle( resolveKey( "groupNames" ) )
                .handle( transform< std::string >( []( auto&& groupName ) { return groupName.asString(); } ) )
                .send( std::move( callback ) );
    }

    void RepetierClient::addModelGroup( std::string const& printer, std::string const& modelGroup,
                                        repetier::Callback<> callback )
    {
        using namespace repetier::action;
        repetier::makeAction( &*client_, "addModelGroup" )
                .printer( printer.c_str() )
                .arg( "groupName", modelGroup.c_str() )
                .handle( checkOkFlag() )
                .send( std::move( callback ) );
    }

    void RepetierClient::upload( std::string const& printer, std::string const& name, std::string const& content,
                                 repetier::Callback<> callback )
    {

    }

} // namespace gcu
