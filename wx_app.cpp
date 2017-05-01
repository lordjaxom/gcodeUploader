#include <memory>

#include <boost/convert.hpp>
#include <boost/convert/spirit.hpp>

#include <wx/cmdline.h>
#include <wx/msgdlg.h>

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
            { wxCMD_LINE_OPTION, _( "m" ), _( "modelname" ), _( "Suggestion for model name" ),
                    wxCMD_LINE_VAL_STRING },
            { wxCMD_LINE_SWITCH, _( "d" ), _( "delete" ), _( "Whether the G-Code file is to be deleted after uploading" ) },
            { wxCMD_LINE_PARAM, nullptr, nullptr, _( "Commands and parameters" ),
                    wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE },
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

        wxFrame* frame;
        switch ( command_ ) {
            case UPLOAD:
                frame = new UploadFrame( printerService, gcodePath_.ToStdString(), printer_, modelName_, deleteFile_ );
                break;
            case EXPLORE:
                frame = new ExplorerFrame( printerService );
                break;
        }
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
        parser.Found( _( "m" ), &modelName_ );
        deleteFile_ = parser.Found( _( "d" ) );

        long port;
        parser.Found( _( "P" ), &port );
        if ( port < std::numeric_limits< std::uint16_t >::min() ||
                port > std::numeric_limits< std::uint16_t >::max() ) {
            // TODO
            return false;
        }
        port_ = (std::uint16_t) port;

        if ( parser.GetParamCount() > 0 ) {
            wxString command = parser.GetParam( 0 );
            if ( command == _( "upload" ) ) {
                if ( parser.GetParamCount() != 2 ) {
                    wxMessageBox( _( "The command upload requires a filename" ), _( "Error" ), wxOK | wxICON_ERROR );
                    return false;
                }
                command_ = UPLOAD;
                gcodePath_ = parser.GetParam( 1 );
            }
            else if ( command == _( "explore" ) ) {
                command_ = EXPLORE;
            }
            else {
                wxMessageBox( _( "Unknown command " ) + command, _( "Error" ), wxOK | wxICON_ERROR );
                return false;
            }
        }
        else {
            command_ = EXPLORE;
        }
        return true;
    }

} // namespace gct

wxIMPLEMENT_APP( gct::GctApp );
