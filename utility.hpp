#ifndef GCODEUPLOADER_UTILITY_HPP
#define GCODEUPLOADER_UTILITY_HPP

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace gcu {
    namespace util {

        template< typename Streamable >
        void stream( Streamable&& dest )
        {
        }

        template< typename Streamable, typename Arg0, typename ...Args >
        void stream( Streamable&& streamable, Arg0&& arg0, Args&&... args )
        {
            std::forward< Streamable >( streamable ) << std::forward< Arg0 >( arg0 );
            stream( std::forward< Streamable >( streamable ), std::forward< Args >( args )... );
        }

        template< typename ...Args >
        std::string str( Args&&... args )
        {
            std::ostringstream os;
            stream( os, std::forward< Args >( args )... );
            return os.str();
        }

    } // namespace util
} // namespace gcu

#endif // GCODEUPLOADER_UTILITY_HPP
