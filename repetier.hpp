#ifndef GCODEUPLOADER_REPETIER_HPP
#define GCODEUPLOADER_REPETIER_HPP

#include <cstdint>
#include <memory>
#include <string>

#include "repetier_definitions.hpp"

namespace asio {
    class io_service;
} // namespace asio

namespace gcu {

    namespace repetier {
        class Client;
    } // namespace repetier

    class RepetierClient
    {
    public:
        RepetierClient();
        RepetierClient( RepetierClient const& ) = delete;
        ~RepetierClient();

        void connect(
                std::string hostname, std::uint16_t port, std::string apikey, repetier::ConnectCallback callback );
        //void listModelGroups( std::string printer, repetier::ListModelGroupsAction::Callback callback );
        //void listPrinter( repetier::ListPrinterAction::Callback callback );

    private:
        std::unique_ptr< repetier::Client > client_;
    };

} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_HPP
