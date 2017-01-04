#ifndef GCODEUPLOADER_UPLOADER_FORM_HPP
#define GCODEUPLOADER_UPLOADER_FORM_HPP

#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>

#include "repetier.hpp"

namespace gcu {

    class UploaderForm
        : public nana::form
    {
    public:
        UploaderForm();

    private:
        RepetierClient client_;

        nana::place place_ { *this };
        nana::label fileNameLabel_ { *this, "G-Code file:" };
        nana::textbox fileNameTextbox_ { *this };
        nana::label modelNameLabel_ { *this, "Model name:" };
        nana::textbox modelNameTextbox_ { *this };
        nana::label modelGroupLabel_ { *this, "Model group: " };
        nana::combox modelGroupCombox_ { *this, "Default" };
        nana::checkbox deleteFileCheckbox_ { *this, "Delete file after upload?" };
        nana::button uploadButton_ { *this, "Upload" };
    };

} // namespace gcu

#endif //GCODEUPLOADER_UPLOADER_FORM_HPP