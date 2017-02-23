#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <iterator>
#include <thread>
#include <utility>

#include <json/value.h>

#include "repetier.hpp"
#include "repetier_action.hpp"
#include "utility.hpp"

namespace gcu {

    struct UploadUrl
    {
        UploadUrl( std::string const& hostname, std::uint16_t port, std::string const& printer )
                : hostname_( hostname )
                , port_( port )
                , printer_( printer )
        {
        }

        template< typename Stream >
        decltype( auto ) friend operator<<( Stream&& stream, UploadUrl const& value )
        {
            return stream << "http://" << value.hostname_ << ':' << value.port_ << "/printer/model/" << value.printer_;
        }

    private:
        std::string const& hostname_;
        std::uint16_t port_;
        std::string const& printer_;
    };

    RepetierClient::RepetierClient() = default;

    RepetierClient::~RepetierClient()
    {
        client_ = std::nullopt;
        thread_.join();
    }

    bool RepetierClient::connected() const
    {
        return client_ && client_->connected();
    }

    repetier::ClientEvents& RepetierClient::events()
    {
        return client_->events();
    }

    void RepetierClient::connect(
            std::string hostname, std::uint16_t port, std::string apikey, repetier::Callback<> callback )
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

    void RepetierClient::listModels( std::string const& printer,
                                     repetier::Callback< std::vector< gcu::repetier::Model > > callback )
    {
        using namespace repetier::action;
        repetier::makeAction( &*client_, "listModels" )
                .printer( printer.c_str() )
                .handle( resolveKey( "data" ) )
                .handle( transform< repetier::Model >( []( auto&& model ) {
                    return repetier::Model(
                            model[ Json::StaticString( "id") ].asLargestUInt(),
                            model[ Json::StaticString( "name" ) ].asString(),
                            model[ Json::StaticString( "group" ) ].asString(),
                            model[ Json::StaticString( "created" ) ].asLargestUInt() / 1000,
                            model[ Json::StaticString( "length" ) ].asLargestUInt() );
                } ) )
                .send( std::move( callback ) );
    }

    void RepetierClient::listModelGroups(
            std::string const& printer, repetier::Callback< std::vector< repetier::ModelGroup > > callback )
    {
        using namespace repetier::action;
        repetier::makeAction( &*client_, "listModelGroups" )
                .printer( printer.c_str() )
                .handle( checkOkFlag() )
                .handle( resolveKey( "groupNames" ) )
                .handle( transform< repetier::ModelGroup >( []( auto&& groupName ) {
                    return repetier::ModelGroup( groupName.asString() );
                } ) )
                .send( std::move( callback ) );
    }

    void RepetierClient::addModelGroup(
            std::string const& printer, std::string const& modelGroup, repetier::Callback<> callback )
    {
        using namespace repetier::action;
        repetier::makeAction( &*client_, "addModelGroup" )
                .printer( printer.c_str() )
                .arg( "groupName", modelGroup.c_str() )
                .handle( checkOkFlag() )
                .send( std::move( callback ) );
    }

    void RepetierClient::removeModel( std::string const& printer, unsigned id, repetier::Callback<> callback )
    {
        using namespace repetier::action;
        repetier::makeAction( &*client_, "removeModel" )
                .printer( printer.c_str() )
                .arg( "id", id )
                .send( std::move( callback ) );
    }

    void RepetierClient::moveModelFileToGroup(
            std::string const& printer, unsigned id, std::string const& modelGroup,
            repetier::Callback<> callback )
    {
        using namespace repetier::action;
        repetier::makeAction( &*client_, "moveModelFileToGroup" )
                .printer( printer.c_str() )
                .arg( "groupName", modelGroup.c_str() )
                .arg( "id", id )
                .handle( checkOkFlag() )
                .send( std::move( callback ) );
    }

    void RepetierClient::upload(
            std::string const& printer, std::string const& modelName, std::string const& modelGroup,
            std::filesystem::path const& gcodePath, repetier::Callback<> callback )
    {
        std::string command = util::str(
                "curl -s -X POST -H \"Content-Type: multipart/form-data\" -H \"x-api-key: ", apikey_, "\" ",
                "-F \"a=upload\" -F \"name=", modelName, "\" -F \"group=", modelGroup, "\" -F \"filename=@",
                gcodePath.string(), "\" ", UploadUrl( hostname_, port_, printer ) );

        std::thread worker(
                [command = std::move( command ), callback = std::move( callback )] {
                    auto pipe = popen( command.c_str(), "r" );
                    char buffer[ 8192 ];
                    std::size_t count;
                    do {
                        count = std::fread( buffer, 1, sizeof( buffer ), pipe );
                    } while ( count == sizeof( buffer ) );

                    std::error_code ec;
                    if ( ferror( pipe ) ) {
                        ec = std::make_error_code( std::errc::broken_pipe ); // TODO
                    }
                    int status = pclose( pipe );
                    if ( !ec && status != 0 ) {
                        ec = std::make_error_code( std::errc::invalid_argument ); // TODO
                    }
                    callback( ec );
                } );
        worker.detach();
    }

} // namespace gcu
