#include <utility>

#include "wx_uploadframe.hpp"

namespace gct {

    UploadFrame::UploadFrame( std::shared_ptr< gcu::PrinterService > printerService )
            : UploadFrameBase( nullptr )
            , printerService_( std::move( printerService ) )
    {
    }

} // namespace gct
