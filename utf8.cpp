#include <ostream>
#include <sstream>

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

        std::string fromUtf8( std::string const& utf8 )
        {
            std::ostringstream os;
            detail::fromUtf8( os, utf8.data(), utf8.size() );
            return os.str();
        }

        std::string toUtf8( std::string const& local )
        {
            std::ostringstream os;
            detail::toUtf8( os, local.data(), local.size() );
            return os.str();
        }

    } // namespace utf8
} // namespace gcu