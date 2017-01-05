#include <exception>
#include <iostream>

#include <nana/gui/wvl.hpp>

#include "uploader_form.hpp"

int main( int argc, char const* const argv[] )
{
    try {
        if ( argc != 2 ) {
            nana::msgbox msgbox( nullptr, "Error", nana::msgbox::ok );
            msgbox.icon( nana::msgbox::icon_error );
            msgbox << "Usage: " << argv[ 0 ] << " FILE";
            msgbox.show();
            return EXIT_FAILURE;
        }

        gcu::UploaderForm form( argv[ 1 ] );
        form.show();
        nana::exec();

        std::cerr << "INFO: Exiting regularly\n";
    }
    catch ( std::exception const& e ) {
        std::cerr << "ERROR: unexpected exception: " << e.what() << "\n";
    }
    catch ( ... ) {
        std::cerr << "ERROR: what?!\n";
    }
}