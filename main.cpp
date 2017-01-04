#include <exception>
#include <iostream>

#include <nana/gui/wvl.hpp>

#include "uploader_form.hpp"

int main( int argc, char const* const argv[] )
{
    try {
        gcu::UploaderForm form;
        form.show();
        nana::exec();
        std::cerr << "left nana exec loop\n";
    }
    catch ( std::exception const& e ) {
        std::cerr << "unexpected exception: " << e.what() << "\n";
    }
    catch ( ... ) {
        std::cerr << "what?!\n";
    }
}