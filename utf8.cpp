#include <wx/string.h>

#include "utf8.hpp"

namespace gcu {
    namespace utf8 {

        std::string fromUtf8( std::string const& utf8 )
        {
            return wxString::FromUTF8( utf8.data(), utf8.size() ).ToStdString();
        }

        std::string toUtf8( std::string const& local )
        {
            auto utf8 = wxString( local ).ToUTF8();
            return { utf8.data(), utf8.length() };
        }

    } // namespace utf8
} // namespace gcu