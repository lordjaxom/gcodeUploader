#ifndef GCODEUPLOADER_WX_UPLOADFRAME_HPP
#define GCODEUPLOADER_WX_UPLOADFRAME_HPP

#include <wx/frame.h>
#include <wx/string.h>

#include <3dprnet/core/filesystem.hpp>
#include <3dprnet/repetier/forward.hpp>

namespace gct {

wxFrame* makeUploadFrame( prnet::rep::Frontend& frontend, prnet::filesystem::path gcodePath, wxString printer,
                          wxString modelName, bool deleteFile );

} // namespace gct

#endif // GCODEUPLOADER_WX_UPLOADFRAME_HPP
