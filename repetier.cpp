#include <functional>
#include <future>
#include <utility>

#include "repetier.hpp"
#include "repetier_client.hpp"

namespace gcu {

    template< typename T, typename... Args >
    static void performAction( repetier::Client& client, Args&&... args )
    {
        client.takeAction( std::make_unique< T >( std::forward< Args >( args )... ) );
    }

    template< typename Result, typename Func >
    static Result waitForAsyncCall( Func&& func )
    {
        std::promise< Result > promise;
        auto future = promise.get_future();
        std::forward< Func >( func )( [&]( Result result ) {
            promise.set_value( result );
        } );
        return future.get();
    }

    RepetierApi::RepetierApi()
        : io_service_()
    {
    }

    RepetierApi::RepetierApi( asio::io_service& io_service )
        : io_service_( &io_service )
    {
    }

    RepetierApi::~RepetierApi() = default;

    void RepetierApi::connect( std::string hostname, uint16_t port, std::string apikey, bool wait )
    {
        if ( wait ) {
            throw std::invalid_argument( "not implemented" );
        }

        using namespace std::placeholders;

        if ( client_ ) {
            throw std::invalid_argument( "connection already in progress" );
        }

        client_.reset( io_service_
               ? new repetier::Client( *io_service_, std::move( hostname ), port, std::move( apikey ),
                                       std::bind( &RepetierApi::handleConnect, this ),
                                       std::bind( &RepetierApi::handleClose, this, _1 ),
                                       std::bind( &RepetierApi::handleError, this, _1 ) )
               : new repetier::Client( std::move( hostname ), port, std::move( apikey ),
                                       std::bind( &RepetierApi::handleConnect, this ),
                                       std::bind( &RepetierApi::handleClose, this, _1 ),
                                       std::bind( &RepetierApi::handleError, this, _1 ) ) );
    }

    void RepetierApi::listModelGroups( std::string printer, repetier::ListModelGroupsAction::Callback callback )
    {
        performAction< repetier::ListModelGroupsAction >( *client_, std::move( printer ), callback );
    }

    std::vector< std::string > RepetierApi::listModelGroups( std::string printer )
    {
        std::promise< std::vector< std::string > > promise;
        auto future = promise.get_future();
        listModelGroups( std::move( printer ), [&promise]( std::vector< std::string > groups ) {
                    promise.set_value( std::move( groups ));
                } );
        auto x = future.get();
        return x;
    }

    void RepetierApi::listPrinter( repetier::ListPrinterAction::Callback callback )
    {
        performAction< repetier::ListPrinterAction >( *client_, callback );
    }

    std::vector< repetier::Printer > RepetierApi::listPrinter()
    {
        return std::vector< repetier::Printer >();
    }

    void RepetierApi::handleConnect()
    {
        if ( connectCallback_ ) {
            connectCallback_();
        }
    }

    void RepetierApi::handleClose( std::string reason )
    {

    }

    void RepetierApi::handleError( std::error_code ec )
    {

    }

} // namespace gcu
