#ifndef GCODEUPLOADER_WX_UPLOADFRAME_HPP
#define GCODEUPLOADER_WX_UPLOADFRAME_HPP

#include <limits>
#include <memory>
#include <string>
#include <system_error>

#include "std_filesystem.hpp"

#include "repetier_definitions.hpp"
#include "wx_generated.h"

namespace gcu {
    class PrinterService;
} // namespace gcu

namespace gct {

    class UploadFrame
            : public UploadFrameBase
    {
        static constexpr unsigned MODEL_NOT_FOUND = std::numeric_limits< unsigned >::max();

    public:
        UploadFrame(
                std::shared_ptr< gcu::PrinterService > printerService, std::filesystem::path gcodePath,
                std::string printer, bool deleteFile );
        UploadFrame( UploadFrame const& ) = delete;

    private:
        void CheckModelNameExists();
        unsigned FindSelectedModelId();
        void PerformUpload(
                std::string const& printer, std::string const& modelName, std::string const& modelGroup,
                bool deleteFile );

        void OnPrinterSelected();
        void OnModelGroupSelected();
        void OnAddModelGroupClicked();
        void OnModelNameChanged();
        void OnUploadClicked();

        void OnConnectionLost( std::error_code ec );
        void OnPrintersChanged( std::vector< gcu::repetier::Printer >&& printers );
        void OnModelGroupsChanged( std::string const& printer, std::vector< gcu::repetier::ModelGroup >&& modelGroups );
        void OnModelsChanged( std::string const& printer, std::vector< gcu::repetier::Model >&& models );

        std::shared_ptr< gcu::PrinterService > printerService_;
        std::filesystem::path gcodePath_;
        std::string selectedPrinter_;
        std::string selectedModelGroup_;
        std::string enteredModelName_;
        std::vector< gcu::repetier::Model > models_;
    };

} // namespace gct

#endif // GCODEUPLOADER_WX_UPLOADFRAME_HPP
