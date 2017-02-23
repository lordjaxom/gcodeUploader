#include <memory>

#include <boost/convert.hpp>
#include <boost/convert/spirit.hpp>

#include <wx/cmdline.h>

#include "printer_service.hpp"
#include <wx/msw/winundef.h>

#include "wx_app.hpp"
#include "wx_explorerframe.hpp"
#include "wx_uploadframe.hpp"

namespace gct {

    static wxCmdLineEntryDesc cmdLineDesc[] {
            { wxCMD_LINE_OPTION, _( "H" ), _( "host" ), _( "Hostname of the printer server" ),
                    wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
            { wxCMD_LINE_OPTION, _( "P" ), _( "port" ), _( "Port of the printer server" ),
                    wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_OPTION_MANDATORY },
            { wxCMD_LINE_OPTION, _( "a" ), _( "apikey" ), _( "API key for unrestricted access to the print server" ),
                    wxCMD_LINE_VAL_STRING, wxCMD_LINE_OPTION_MANDATORY },
            { wxCMD_LINE_OPTION, _( "p" ), _( "printer" ), _( "Printer that gets selected initially" ),
                    wxCMD_LINE_VAL_STRING },
            { wxCMD_LINE_SWITCH, _( "d" ), _( "delete" ), _( "Whether the G-Code file is to be deleted after uploading" ) },
            { wxCMD_LINE_PARAM, nullptr, nullptr, _( "Path to the G-Code file to upload" ) },
            { wxCMD_LINE_NONE }
    };

    GctApp::GctApp() = default;

    bool GctApp::OnInit()
    {
        if ( !wxApp::OnInit() ) {
            return false;
        }

        auto printerService = std::make_shared< gcu::PrinterService >(
                hostname_.ToStdString(), port_, apikey_.ToStdString() );

        auto frame = new UploadFrame( printerService, gcodePath_.ToStdString(), printer_.ToStdString(), deleteFile_ );
        //auto frame = new ExplorerFrame( printerService );
        frame->Show( true );
        return true;
    }

    void GctApp::OnInitCmdLine( wxCmdLineParser& parser )
    {
        parser.SetDesc( cmdLineDesc );
        parser.SetSwitchChars( _( "-" ) );
    }

    bool GctApp::OnCmdLineParsed( wxCmdLineParser& parser )
    {
        parser.Found( _( "H" ), &hostname_ );
        parser.Found( _( "a" ), &apikey_ );
        parser.Found( _( "p" ), &printer_ );
        deleteFile_ = parser.Found( _( "d" ) );

        long port;
        parser.Found( _( "P" ), &port );
        if ( port < std::numeric_limits< std::uint16_t >::min() ||
                port > std::numeric_limits< std::uint16_t >::max() ) {
            // TODO
            return false;
        }
        port_ = (std::uint16_t) port;

        gcodePath_ = parser.GetParam( 0 );

        return true;
    }

} // namespace gct

wxIMPLEMENT_APP( gct::GctApp );
