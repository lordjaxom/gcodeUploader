#include <stdexcept>
#include <utility>

#include "repetier.hpp"
#include "repetier_client.hpp"

namespace gcu {

    RepetierApi::RepetierApi()
        : io_service_()
    {
     }

    RepetierApi::RepetierApi( asio::io_service& io_service )
        : io_service_( &io_service )
    {
    }

    RepetierApi::RepetierApi( std::string url, unsigned short port, std::string apikey )
        : hostname_( std::move( url ) )
        , port_( port )
        , apikey_( std::move( apikey ) )
        , io_service_()
    {
    }

    RepetierApi::RepetierApi( std::string hostname, unsigned short port, std::string apikey, asio::io_service& io_service )
        : hostname_( std::move( hostname ) )
        , port_( port )
        , apikey_( std::move( apikey ) )
        , io_service_( &io_service )
    {
        connect();
    }

    RepetierApi::~RepetierApi() = default;

    void RepetierApi::connect( std::string hostname, unsigned short port, std::string apikey )
    {
        if ( client_ ) {
            throw std::invalid_argument( "connection already in progress" );
        }

        hostname_ = std::move( hostname );
        port_ = port;
        apikey_ = std::move( apikey );

        connect();
    }

    void RepetierApi::connect()
    {
        client_.reset( io_service_ ? new repetier::Client( hostname_, port_, apikey_, io_service_ )
                                   : new repetier::Client( hostname_, port_, apikey_ ) );
    }

} // namespace gcu
