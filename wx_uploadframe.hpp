#ifndef GCODEUPLOADER_WX_UPLOADFRAME_HPP
#define GCODEUPLOADER_WX_UPLOADFRAME_HPP

#include <memory>

#include "wx_generated.h"

namespace gcu {
    class PrinterService;
} // namespace gcu

namespace gct {

    class UploadFrame
            : public UploadFrameBase
    {
    public:
        UploadFrame( std::shared_ptr< gcu::PrinterService > printerService );
        UploadFrame( UploadFrame const& ) = delete;

    private:
        std::shared_ptr< gcu::PrinterService > printerService_;
    };

} // namespace gct

#endif // GCODEUPLOADER_WX_UPLOADFRAME_HPP
