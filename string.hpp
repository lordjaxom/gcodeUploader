#ifndef GCODEUPLOADER_STRING_HPP
#define GCODEUPLOADER_STRING_HPP

#include <iosfwd>
#include <limits>
#include <memory>
#include <string>

namespace gcu {

    namespace string {

        struct Literal {};
        struct Foreign {};

        static constexpr Literal literal {};
        static constexpr Foreign foreign {};

        static constexpr std::size_t npos = std::numeric_limits< std::size_t >::max();

    } // namespace string

    namespace detail {
        class StringBuffer;
        using StringBufferPtr = std::shared_ptr< StringBuffer >;
    } // namespace detail

    class String
    {
    public:
        String( string::Literal, char const* str, std::size_t size = string::npos );
        String( string::Foreign, char const* str, std::size_t size = string::npos );

        __attribute__(( deprecated ))
        String( std::string const& str );

        bool empty() const { return size_ == 0; }
        std::size_t size() const { return size_; }
        
        char front() const { return str_[ 0 ]; }
        char back() const { return str_[ size_ - 1 ]; }
        
        friend std::ostream& operator<<( std::ostream& os, String const& value );

    private:
        detail::StringBufferPtr memory_;
        char const* str_;
        std::size_t size_;
        bool terminated_;
    };

    String operator "" _c( char const* str, std::size_t size );

} // namespace gcu

#endif // GCODEUPLOADER_STRING_HPP
