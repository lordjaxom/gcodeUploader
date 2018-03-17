#ifndef GCODEUPLOADER_WX_UPLOADFRAME_HPP
#define GCODEUPLOADER_WX_UPLOADFRAME_HPP

#include <wx/frame.h>
#include <wx/string.h>

#include <repetier/forward.hpp>

#include "std/filesystem.hpp"

namespace gct {

wxFrame* makeUploadFrame( prnet::rep::Frontend& frontend, std::filesystem::path gcodePath, wxString printer,
                          wxString modelName, bool deleteFile );

} // namespace gct

#endif // GCODEUPLOADER_WX_UPLOADFRAME_HPP
