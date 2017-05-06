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

        inline auto fromUtf8( std::string const& utf8 )
        {
            return [&]( std::ostream& os ) { detail::fromUtf8( os, utf8.data(), utf8.size() ); };
        }

        inline auto toUtf8( std::string const& local )
        {
            return [&]( std::ostream& os ) { detail::toUtf8( os, local.data(), local.size() ); };
        }

    } // namespace utf8
} // namespace gcu

#endif // GCODEUPLOADER_UTF8_HPP
