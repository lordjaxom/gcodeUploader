#include <thread>
#include <utility>

#include <boost/asio/io_context.hpp>

#include <wx/cmdline.h>
#include <wx/msgdlg.h>

#include <core/logging.hpp>
#include <repetier/frontend.hpp>
#include <repetier/types.hpp>

#include <wx/msw/winundef.h>

#include "wx_app.hpp"
// #include "wx_explorerframe.hpp"
#include "wx_uploadframe.hpp"

using namespace prnet;
using namespace std;

namespace asio = boost::asio;

namespace gct {

static logger logger( "gct::GctApp" );

struct FrontendContext
{
    explicit FrontendContext( rep::Endpoint&& endpoint )
            : frontend( context, move( endpoint ) )
            , thread( [this] { context.run(); } ) {}

    ~FrontendContext()
    {
        context.stop();
        thread.join();
    }

    asio::io_context context;
    rep::Frontend frontend;
    thread thread;
};

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

            auto gcodePath = parser.GetParam( 1 );
            gcodePath_ = gcodePath.ToStdString();
            if ( !std::filesystem::exists( gcodePath_ ) ) {
                wxMessageBox( wxString::Format( _( "G-Code file \"%s\" does not exist" ), gcodePath.c_str() ), _( "Error" ), wxOK | wxICON_ERROR );
                return false;
            }
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

bool GctApp::OnInit()
{
    if ( !wxApp::OnInit() ) {
        return false;
    }

    logger::threshold( logger::Debug );

    rep::Endpoint endpoint( hostname_.ToStdString(), to_string( port_ ), apikey_.ToStdString() );
    frontendContext_ = make_unique< FrontendContext >( move( endpoint ) );

    wxFrame* frame;
    switch ( command_ ) {
        case UPLOAD:
            frame = new UploadFrame( frontendContext_->frontend, gcodePath_, printer_, modelName_, deleteFile_ );
            break;
        case EXPLORE:
            // frame = new ExplorerFrame( printerService );
            break;
    }
    frame->Show( true );
    return true;
}

} // namespace gct

wxIMPLEMENT_APP( gct::GctApp );
