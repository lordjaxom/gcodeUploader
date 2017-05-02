#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <iterator>
#include <thread>
#include <utility>

#include <json.hpp>

#include "repetier.hpp"
#include "repetier_action.hpp"
#include "utf8.hpp"
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
                    return repetier::Printer( printer[ "active" ], printer[ "name" ], printer[ "slug" ] );
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
                            model[ "id" ],
                            model[ "name" ],
                            model[ "group" ],
                            model[ "created" ].template get< std::size_t >() / 1000,
                            model[ "length" ],
                            model[ "layer" ],
                            model[ "lines" ],
                            std::chrono::milliseconds(
                                    (std::uint64_t) ( model[ "printTime" ].template get< double >() * 1000.0 ) ) );
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
                    return repetier::ModelGroup( groupName.template get< std::string >() );
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

    void RepetierClient::delModelGroup( std::string const& printer, std::string const& modelGroup, bool deleteModels,
                                        repetier::Callback<> callback )
    {
        using namespace repetier::action;
        repetier::makeAction( &*client_, "delModelGroup" )
                .printer( printer.c_str() )
                .arg( "groupName", modelGroup.c_str() )
                .arg( "delFiles", deleteModels )
                .handle( checkOkFlag() )
                .send( std::move( callback ) );
    }

    void RepetierClient::removeModel( std::string const& printer, std::size_t id, repetier::Callback<> callback )
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
                "-F \"a=upload\" -F \"name=", utf8::toUtf8( modelName ), "\" -F \"group=", utf8::toUtf8( modelGroup ),
                "\" -F \"filename=@", gcodePath.string(), "\" ", UploadUrl( hostname_, port_, printer ) );

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
