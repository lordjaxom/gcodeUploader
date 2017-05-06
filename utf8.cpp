#include <ostream>

#include <wx/string.h>

#include "utf8.hpp"

namespace gcu {
    namespace utf8 {
        namespace detail {

            void fromUtf8( std::ostream& os, char const* data, std::size_t size )
            {
                os << wxString::FromUTF8( data, size );
            }

            void toUtf8( std::ostream& os, char const* data, std::size_t size )
            {
                os << wxString( data, size ).ToUTF8();
            }

        } // namespace detail
    } // namespace utf8
} // namespace gcu