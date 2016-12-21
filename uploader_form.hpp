#ifndef GCODEUPLOADER_UPLOADER_FORM_HPP
#define GCODEUPLOADER_UPLOADER_FORM_HPP

#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>

namespace gcu {

    class UploaderForm
        : public nana::form
    {
    public:
        UploaderForm();

    private:
        void onUploadClick();

        nana::place place_ { *this };
        nana::label fileNameLabel_ { *this, "G-Code file:" };
        nana::textbox fileNameTextbox_ { *this };
        nana::label modelNameLabel_ { *this, "Model name:" };
        nana::textbox modelNameTextbox_ { *this };
        nana::checkbox deleteFileCheckbox_ { *this, "Delete file after upload?" };
        nana::button uploadButton_ { *this, "Upload" };
    };

} // namespace gcu

#endif //GCODEUPLOADER_UPLOADER_FORM_HPP