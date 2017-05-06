#ifndef GCODEUPLOADER_URL_HPP
#define GCODEUPLOADER_URL_HPP

#include <cstdint>
#include <iosfwd>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "string.hpp"

namespace gcu {

    namespace url {

        using Protocol = std::tuple< char const*, std::uint16_t >;

        std::tuple< char const*, std::uint16_t > http( std::uint16_t port = 80 );
        std::tuple< char const*, std::uint16_t > ws( std::uint16_t port = 80 );

    } // namespace url

    class Url
    {
    public:
        Url( url::Protocol protocol, String host );

        template< typename ...Args >
        Url( url::Protocol protocol, String host, Args&&... args )
                : Url( std::move( protocol ), std::move( host ),
                       std::vector< String > { std::forward< Args >( args )... } )
        {
        }

        friend std::ostream& operator<<( std::ostream& os, Url const& value );

    private:
        Url( url::Protocol&& protocol, String&& host, std::vector< String >&& path );

        url::Protocol protocol_;
        String host_;
        std::vector< String > path_;
    };

} // namespace gcu

#endif // GCODEUPLOADER_URL_HPP
