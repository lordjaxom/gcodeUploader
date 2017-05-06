#include <ostream>
#include <utility>

#include "http.hpp"

namespace gcu {

    Url::Url( std::string protocol, std::string host, std::uint16_t port, std::string path )
        : protocol_( std::move( protocol ) ),
          host_( std::move( host ) ),
          port_( port ),
          path_( path )
    {
    }

    void Url::operator()( std::ostream& os )
    {
        os << protocol_ << "://" << host_ << ':' << port_ << '/' << path_;
    }

} // namespace gcu