#include <functional>
#include <future>
#include <utility>

#include "repetier.hpp"
#include "repetier_client.hpp"

namespace gcu {

    RepetierClient::RepetierClient() = default;
    RepetierClient::~RepetierClient() = default;

    void RepetierClient::connect(
            std::string hostname, uint16_t port, std::string apikey, repetier::ConnectCallback callback )
    {
        if ( client_ ) {
            throw std::invalid_argument( "connection already in progress" );
        }

        client_.reset(
                new repetier::Client( std::move( hostname ), port, std::move( apikey ), std::move( callback ) ) );
    }

    /*
    void RepetierClient::listModelGroups( std::string printer, repetier::ListModelGroupsAction::Callback callback )
    {
        performAction< repetier::ListModelGroupsAction >( *client_, std::move( printer ), callback );
    }

    std::vector< std::string > RepetierClient::listModelGroups( std::string printer )
    {
        std::promise< std::vector< std::string > > promise;
        auto future = promise.get_future();
        listModelGroups( std::move( printer ), [&promise]( std::vector< std::string > groups ) {
                    promise.set_value( std::move( groups ));
                } );
        auto x = future.get();
        return x;
    }

    void RepetierClient::listPrinter( repetier::ListPrinterAction::Callback callback )
    {
        performAction< repetier::ListPrinterAction >( *client_, callback );
    }

    std::vector< repetier::Printer > RepetierClient::listPrinter()
    {
        return std::vector< repetier::Printer >();
    }

    void RepetierClient::handleConnect()
    {
        if ( connectCallback_ ) {
            connectCallback_();
        }
    }

    void RepetierClient::handleClose( std::string reason )
    {

    }

    void RepetierClient::handleError( std::error_code ec )
    {

    }
*/
} // namespace gcu
