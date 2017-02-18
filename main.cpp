#include <exception>
#include <iostream>

#include <getopt.h>

#include <nana/gui/wvl.hpp>

#include "commandline.hpp"
#include "printer_service.hpp"
#include "uploader_form.hpp"
#include "utility.hpp"

namespace gcu {

    template< typename ...Args >
    auto exitWithError( Args&& ... args )
    {
        nana::msgbox msgbox( nullptr, "Error", nana::msgbox::ok );
        msgbox.icon( nana::msgbox::icon_error );
        util::stream( msgbox, std::forward< Args >( args )... );
        msgbox.show();
        return EXIT_FAILURE;
    }

} // namespace gcu

int main( int argc, char* const argv[] )
{
    try {
        gcu::CommandLine commandLine( argv, argc );

        gcu::PrinterService printerService( commandLine.hostname(), commandLine.port(), commandLine.apikey() );

        gcu::UploaderForm form(
                printerService, commandLine.gcodeFile(), commandLine.printer(), commandLine.deleteFile() );
        form.show();
        nana::exec();

        std::cerr << "INFO: Exiting regularly\n";
    }
    catch ( std::exception const& e ) {
        return gcu::exitWithError( e.what() );
    }
}