#ifndef GCODEUPLOADER_WX_APP_HPP
#define GCODEUPLOADER_WX_APP_HPP

#include <cstdint>

#include <wx/app.h>
#include <wx/string.h>

namespace gct {

    class GctApp
            : public wxApp
    {
    public:
        GctApp();
        GctApp( GctApp const& ) = delete;

        virtual bool OnInit() override;
        virtual void OnInitCmdLine( wxCmdLineParser& parser ) override;
        virtual bool OnCmdLineParsed( wxCmdLineParser& parser ) override;

    private:
        wxString hostname_;
        std::uint16_t port_;
        wxString apikey_;
        wxString printer_;
        bool deleteFile_;
    };

} // namespace gct

wxDECLARE_APP( gct::GctApp );

#endif // GCODEUPLOADER_WX_APP_HPP
