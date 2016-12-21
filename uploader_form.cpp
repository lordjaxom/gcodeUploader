#include <nana/gui/programming_interface.hpp>
#include <nana/gui/msgbox.hpp>

#include "uploader_form.hpp"

namespace gcu {

    UploaderForm::UploaderForm()
        : form( nana::API::make_center( 400, 150 ), nana::appear::decorate<>() )
    {
        caption( "G-Code Uploader" );

        fileNameLabel_.text_align( nana::align::left, nana::align_v::center );
        fileNameTextbox_.enabled(false);
        modelNameLabel_.text_align( nana::align::left, nana::align_v::center );

        uploadButton_.events().click( [this] {
            nana::msgbox message( *this, "Event", nana::msgbox::ok );
            message.icon( nana::msgbox::icon_information );
            message << "Upload clicked!";
            message();
        } );

        place_.div( "vertical margin=10"
                            "< weight=23 arrange=[100,variable] fileName>"
                            "< weight=5 >"
                            "< weight=23 arrange=[100,variable] modelName >"
                            "< weight=5 >"
                            "< weight=23 < weight=100 > < deleteFile > >"
                            "< weight=15 >"
                            "< weight=25 <> < buttons weight=100 > >" );
        place_[ "fileName" ] << fileNameLabel_ << fileNameTextbox_;
        place_[ "modelName" ] << modelNameLabel_ << modelNameTextbox_;
        place_[ "deleteFile" ] << deleteFileCheckbox_;
        place_[ "buttons" ] << uploadButton_;
        place_.collocate();
    }

} // namespace gcu