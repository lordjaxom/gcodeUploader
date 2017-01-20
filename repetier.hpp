#ifndef GCODEUPLOADER_REPETIER_HPP
#define GCODEUPLOADER_REPETIER_HPP

#include <cstdint>
#include <memory>
#include <string>

#include "repetier_definitions.hpp"

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

        void connect( std::string hostname, std::uint16_t port, std::string apikey, repetier::Callback< void > callback );
        void listPrinter( repetier::Callback< std::vector< repetier::Printer > > callback );
        void listModelGroups( std::string const& printer, repetier::Callback< std::vector< std::string > > callback );
        void addModelGroup( std::string const& printer, std::string const& modelGroup, repetier::Callback< void > callback );
        void upload( std::string const& printer, std::string const& modelGroup, std::string const& name,
                     std::string gcode, repetier::Callback< void > callback );

    private:
        std::string hostname_;
        std::string port_;
        std::unique_ptr< repetier::Client > client_;
    };

} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_HPP
