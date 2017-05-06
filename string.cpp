#include <cstring>
#include <algorithm>
#include <ostream>

#include "string.hpp"

namespace gcu {

    namespace detail {

        class StringBuffer
        {
        public:
            StringBuffer() = default;

            explicit StringBuffer( std::size_t size )
                    : size_( size ),
                      memory_( new char[ size_ + 1 ] )
            {
                memory_[ size_ ] = '\0';
            }

            std::size_t size() const { return size_; }

            char const* data() const { return memory_.get(); }
            char* data() { return memory_.get(); }

        private:
            std::size_t size_;
            std::unique_ptr< char[] > memory_;
        };

    } // namespace detail

    String::String( string::Literal, char const* str, size_t size )
            : str_( str ),
              size_( size != string::npos ? size : std::strlen( str ) ),
              terminated_( size == string::npos )
    {
    }

    String::String( string::Foreign, char const* str, size_t size )
            : memory_( std::make_shared< detail::StringBuffer >( size != string::npos ? size : std::strlen( str ) ) ),
              str_( memory_->data() ),
              size_( memory_->size() ),
              terminated_( true )
    {
        std::copy_n( str, size, memory_->data() );
    }

    String::String( std::string const& str )
            : String( string::foreign, str.data(), str.size() )
    {
    }

    String operator "" _c( char const* str, std::size_t size )
    {
        return String( string::literal, str, size );
    }

    std::ostream& operator<<( std::ostream& os, String const& value )
    {
        os.write( value.str_, value.size() );
    }

} // namespace gcu
