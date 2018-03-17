#ifndef GCODEUPLOADER_UTF8_HPP
#define GCODEUPLOADER_UTF8_HPP

#include <cstddef>
#include <iosfwd>
#include <string>

namespace gcu {
    namespace utf8 {

        namespace detail {

            void fromUtf8( std::ostream& os, char const* data, std::size_t size );
            void toUtf8( std::ostream& os, char const* data, std::size_t size );

        } // namespace detail

        std::string fromUtf8( std::string const& utf8 );
        std::string toUtf8( std::string const& local );

    } // namespace utf8
} // namespace gcu

#endif // GCODEUPLOADER_UTF8_HPP
