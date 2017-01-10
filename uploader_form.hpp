#ifndef GCODEUPLOADER_UPLOADER_FORM_HPP
#define GCODEUPLOADER_UPLOADER_FORM_HPP

#include <experimental/filesystem>

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
        UploaderForm( std::string const& gcodePath );

    private:
        void printerSelected();
        void modelGroupSelected();
        void newModelGroupClicked();
        void uploadClicked();

        bool handleError( std::error_code ec );

        void handleConnect( std::error_code ec );
        void handleListPrinter( std::vector< repetier::Printer >&& printers, std::error_code ec );
        void handleListModelGroups( std::vector< std::string >&& modelGroups, std::error_code ec );

        std::experimental::filesystem::path gcodePath_;
        RepetierClient client_;
        std::vector< repetier::Printer > printers_;
        std::vector< std::string > modelGroups_;

        nana::place place_ { *this };
        nana::label fileNameLabel_ { *this, "G-Code file:" };
        nana::textbox fileNameTextbox_ { *this };
        nana::checkbox deleteFileCheckbox_ { *this, "Delete file after upload?" };
        nana::label printerLabel_ { *this, "Printer:" };
        nana::combox printerCombox_ { *this };
        nana::label modelGroupLabel_ { *this, "Model group: " };
        nana::combox modelGroupCombox_ { *this };
        nana::button newModelGroupButton_ { *this, "+" };
        nana::label modelNameLabel_ { *this, "Model name:" };
        nana::textbox modelNameTextbox_ { *this };
        nana::button uploadButton_ { *this, "Upload" };
    };

} // namespace gcu

#endif //GCODEUPLOADER_UPLOADER_FORM_HPP