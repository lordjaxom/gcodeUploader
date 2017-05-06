#include <ostream>
#include <utility>

#include "http.hpp"

namespace gcu {

    namespace url {

        std::tuple< char const*, std::uint16_t > http( std::uint16_t port )
        {
            return { "http", port };
        }

        std::tuple< char const*, std::uint16_t > ws( std::uint16_t port )
        {
            return { "ws", port };
        }

    } // namespace url

    Url::Url( url::Protocol protocol, String host )
            : Url( std::move( protocol ), std::move( host ), std::vector< String > {} )
    {

    }

    Url::Url( url::Protocol&& protocol, String&& host, std::vector< String >&& path )
            : protocol_( std::move( protocol ) ),
              host_( std::move( host ) ),
              path_( std::move( path ) )
    {
    }

    std::ostream& operator<<( std::ostream& os, Url const& value )
    {
        os << std::get< 0 >( value.protocol_ ) << "://" << value.host_ << ':' << std::get< 1 >( value.protocol_ );

        bool slash = false;
        for ( auto const& component : value.path_ ) {
            if ( component.empty() ) {
                continue;
            }

            if ( !slash && component.front() != '/' ) {
                os << '/';
            }
            os << component;
            slash = component.back() == '/';
        }
    }

} // namespace gcu