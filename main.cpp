#include <nana/gui/wvl.hpp>

#include "uploader_form.hpp"

int main( int argc, char const* const argv[] )
{
    using namespace gcu;

    UploaderForm form;
    form.show();

    nana::exec();
}