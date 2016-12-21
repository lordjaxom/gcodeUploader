#include "repetier_client.hpp"

namespace gcu {

    RepetierClient::RepetierClient()
    {
        client_.init_asio();
        init();
    }

    RepetierClient::RepetierClient( asio::io_service& service )
    {
        client_.init_asio( &service );
        init();
    }

    void RepetierClient::init()
    {
        thread_.reset( new boost::thread( [this] { client_.run(); } ) );
    }

} // namespace gcu