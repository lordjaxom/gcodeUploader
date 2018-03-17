#ifndef GCODEUPLOADER_WX_APP_HPP
#define GCODEUPLOADER_WX_APP_HPP

#include <cstdint>
#include <memory>

#include <wx/app.h>
#include <wx/string.h>

#include "std/filesystem.hpp"

namespace gct {

struct FrontendContext;

class GctApp
        : public wxApp
{
    enum Command
    {
        UPLOAD,
        EXPLORE
    };

public:
    GctApp();
    GctApp( GctApp const& ) = delete;

    void OnInitCmdLine( wxCmdLineParser& parser ) override;
    bool OnCmdLineParsed( wxCmdLineParser& parser ) override;
    bool OnInit() override;

private:
    wxString hostname_;
    std::uint16_t port_;
    wxString apikey_;
    wxString printer_;
    wxString modelName_;
    bool deleteFile_;
    Command command_;
    std::filesystem::path gcodePath_;
    std::unique_ptr< FrontendContext > frontendContext_;
};

} // namespace gct

wxDECLARE_APP( gct::GctApp );

#endif // GCODEUPLOADER_WX_APP_HPP
