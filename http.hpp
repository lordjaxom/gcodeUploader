#ifndef GCODEUPLOADER_URL_HPP
#define GCODEUPLOADER_URL_HPP

#include <cstdint>
#include <iosfwd>
#include <string>

namespace gcu {

    class Url
    {
    public:
        Url( std::string protocol, std::string host, std::uint16_t port, std::string path = {} );

        void operator()( std::ostream& os );

    private:
        std::string protocol_;
        std::string host_;
        std::uint16_t port_;
        std::string path_;
    };

} // namespace gcu

#endif // GCODEUPLOADER_URL_HPP
