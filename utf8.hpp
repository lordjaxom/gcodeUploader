#ifndef GCODEUPLOADER_UTF8_HPP
#define GCODEUPLOADER_UTF8_HPP

#include <string>

namespace gcu {
    namespace utf8 {

        std::string fromUtf8( std::string const& utf8 );
        std::string toUtf8( std::string const& local );

    } // namespace utf8
} // namespace gcu

#endif // GCODEUPLOADER_UTF8_HPP
