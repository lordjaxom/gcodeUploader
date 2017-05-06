#ifndef GCODEUPLOADER_CONVERSION_HPP
#define GCODEUPLOADER_CONVERSION_HPP

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <boost/convert.hpp>
#include <boost/convert/stream.hpp>

namespace boost {
    namespace cnv {
        struct by_default : boost::cnv::cstream {};
    } // namespace cnv
} // namespace boost

namespace gcu {
    namespace cnv {

        namespace detail {

            template< typename ...Args >
            void noOp( Args&&... args )
            {
            }

        } // namespace detail

        template< typename ...Args >
        auto toString( Args&&... args )
        {
            std::ostringstream os;
            int inOrder[] { ( os << std::forward< Args >( args ), 0 )... };
            (void) inOrder;
            return os.str();
        }

    } // namespace util
} // namespace cnv

template< typename Func >
auto operator<<( std::ostream& os, Func&& arg )
        -> decltype( std::forward< Func >( arg )( os ), std::forward< std::ostream& >( os ) )
{
    std::forward< Func >( arg )( os );
    return os;
}

#endif //GCODEUPLOADER_CONVERSION_HPP
