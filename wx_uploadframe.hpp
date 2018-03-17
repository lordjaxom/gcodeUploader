#ifndef GCODEUPLOADER_WX_UPLOADFRAME_HPP
#define GCODEUPLOADER_WX_UPLOADFRAME_HPP

#include <limits>
#include <memory>
#include <string>
#include <system_error>

#include <repetier/forward.hpp>
#include <repetier/types.hpp>

#include "std/filesystem.hpp"
#include "wx_generated.h"

namespace gct {

    class UploadFrame
            : public UploadFrameBase
    {
        static constexpr std::size_t MODEL_NOT_FOUND = std::numeric_limits< std::size_t >::max();

    public:
        UploadFrame( prnet::rep::Frontend& frontend, std::filesystem::path gcodePath, wxString printer,
                     wxString modelName, bool deleteFile );
        UploadFrame( UploadFrame const& ) = delete;

    private:
        void CheckModelNameExists();
        std::size_t FindSelectedModelId();
        void PerformUpload(
                wxString const& printer, wxString const& modelName, wxString const& modelGroup,
                bool deleteFile );

        void OnPrinterSelected();
        void OnModelGroupSelected();
        void OnAddModelGroupClicked();
        void OnModelNameChanged();
        void OnUploadClicked();
        void OnToolBarExplore();

        void OnConnectionLost( std::error_code ec );
        void OnPrintersChanged( std::vector< prnet::rep::Printer >&& printers );
        void OnModelGroupsChanged( std::string const& printer, std::vector< prnet::rep::ModelGroup >&& modelGroups );
        void OnModelsChanged( std::string const& printer, std::vector< prnet::rep::Model >&& models );

        prnet::rep::Frontend& frontend_;
        std::filesystem::path gcodePath_;
        wxString selectedPrinter_;
        wxString selectedModelGroup_;
        wxString enteredModelName_;
        std::vector< prnet::rep::Model > models_;
    };

} // namespace gct

#endif // GCODEUPLOADER_WX_UPLOADFRAME_HPP
