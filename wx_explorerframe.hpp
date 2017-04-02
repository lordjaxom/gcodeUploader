#ifndef GCODEUPLOADER_WX_EXPLORERFRAME_HPP
#define GCODEUPLOADER_WX_EXPLORERFRAME_HPP

#include <memory>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

#include "wx_generated.h"

namespace gcu {
    class PrinterService;
} // namespace gcu

namespace gct {

    class ExplorerFrame
            : public ExplorerFrameBase
    {
    public:
        ExplorerFrame( std::shared_ptr< gcu::PrinterService > printerService );
        ExplorerFrame( ExplorerFrame const& ) = delete;

    private:
        void RefreshControlStates();
        void InvalidateModelGroup();
        void InvalidateModels();

        void OnPrinterSelected();
        void OnModelGroupSelected();
        void OnModelsListContextMenu();
        void OnModelsListItemSelected();
        void OnToolBarRemoveModels();
        void OnToolBarNewGroup();
        void OnToolBarRemoveGroup();

        void OnConnectionLost( std::error_code ec );
        void OnPrintersChanged( std::vector< gcu::repetier::Printer >&& printers );
        void OnModelGroupsChanged( std::string const& printer, std::vector< gcu::repetier::ModelGroup >&& modelGroups );
        void OnModelsChanged( std::string const& printer, std::vector< gcu::repetier::Model >&& models );

        std::shared_ptr< gcu::PrinterService > printerService_;
        std::string selectedPrinter_;
        std::string selectedModelGroup_;
        std::unordered_map< long, gcu::repetier::Model > models_;
        std::unordered_set< std::size_t > selectedModels_;

    };

} // namespace gct

#endif // GCODEUPLOADER_WX_EXPLORERFRAME_HPP
