#ifndef GCODEUPLOADER_UPLOADER_FORM_HPP
#define GCODEUPLOADER_UPLOADER_FORM_HPP

#include <limits>

#include "std_filesystem.hpp"

#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>

#include "repetier.hpp"

namespace gcu {

    class PrinterService;

    class UploaderForm
        : public nana::form
    {
        static constexpr unsigned invalidId = std::numeric_limits< unsigned >::max();

    public:
        UploaderForm(
                PrinterService& printerService, std::string const& gcodePath, std::string const& printer,
                bool deleteFile );

    private:
        void printerSelected();
        void modelGroupSelected();
        void newModelGroupClicked();
        void modelNameTextChanged();
        void uploadClicked();

        void checkModelName();
        void performUpload(
                std::string const& printer, std::string const& modelName, std::string const& modelGroup,
                bool deleteFile );

        void handleConnectionLost( std::error_code ec );
        void handlePrintersChanged( std::vector< repetier::Printer > const& printers );
        void handleModelGroupsChanged(
                std::string const& printer, std::vector< repetier::ModelGroup > const& modelGroups );
        void handleModelsChanged( std::string const& printer, std::vector< repetier::Model > const& models );

        unsigned existingModelId( std::string const& modelName, std::string const& modelGroup ) const;

        PrinterService& printerService_;
        std::filesystem::path gcodePath_;
        std::string selectedPrinter_;
        std::string selectedModelGroup_;
        std::string enteredModelName_;
        std::vector< repetier::Model > models_;

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
        nana::label infoLabel_ { *this };
    };

} // namespace gcu

#endif //GCODEUPLOADER_UPLOADER_FORM_HPP