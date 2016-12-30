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
        std::forward< Func >( func )( [&promise]( Result result ) {
            promise.set_value( std::move( result ) );
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

    void RepetierApi::connect( std::string hostname, uint16_t port, std::string apikey, repetier::StatusCallback callback )
    {
        if ( client_ ) {
            throw std::invalid_argument( "connection already in progress" );
        }

        client_.reset( io_service_ ? new repetier::Client( *io_service_, std::move( hostname ), port, std::move( apikey ), std::move( callback ) )
                                   : new repetier::Client( std::move( hostname ), port, std::move( apikey ), std::move( callback ) ) );
    }

    repetier::Status RepetierApi::connect( std::string hostname, unsigned short port, std::string apikey )
    {

        waitForAsyncCall< repetier::Status >( [&]( repetier::StatusCallback callback ) {
            connect( hostname, port, apikey , std::move( callback ) );
        } );
        /*
        std::promise< repetier::Status > pr omise;
        auto future = promise.get_future();
        connect( std::move( hostname ), port, std::move( apikey ), [&promise]( repetier::Status status ) {
            if ( status == repetier::Status::CONNECTED ) {
                promise.set_value( status );
            }
        } );
        future.get();
        */
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

} // namespace gcu
