#include <functional>
#include <memory>
#include <ostream>
#include <utility>

#include <asio/connect.hpp>
#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/placeholders.hpp>
#include <asio/streambuf.hpp>

#include "repetier_upload.hpp"

using namespace asio::ip;

namespace gcu {
    namespace repetier {

        static std::string buildUploadUrl( std::string const& hostname, std::uint16_t port,
                                           std::string const& printer, std::string const& session,
                                           std::string const& name )
        {
            std::ostringstream os;
            os << "http://" << hostname << ':' << port << "/printer/model/" << printer << "?a=upload&sess="
               << session << "&name=" << name;
            return os.str();
        }

        namespace detail {

            class Uploader
                    : std::enable_shared_from_this< Uploader >
            {
                using Handler = std::function< void ( std::error_code ec ) >;

            public:
                Uploader( std::string&& hostname, std::uint16_t port, std::string&& session, std::string&& printer,
                          std::string&& name, std::string&& content, Handler&& handler )
                        : hostname_( std::move( hostname ) )
                        , port_( port )
                        , session_( std::move( session ) )
                        , printer_( std::move( printer ) )
                        , name_( std::move( name ) )
                        , content_( std::move( content ) )
                        , handler_( std::move( handler ) )
                {
                }

                void start()
                {
                    using namespace std::placeholders;
                    tcp::resolver::query query( hostname_, std::to_string( port_ ) );
                    resolver_.async_resolve( query, std::bind( &Uploader::handleResolve, shared_from_this(), _1, _2 ) );
                }

            private:
                void handleResolve( std::error_code ec, tcp::resolver::iterator it )
                {
                    if ( handleError( ec ) ) {
                        return;
                    }

                    using namespace std::placeholders;
                    asio::async_connect( socket_, it, std::bind( &Uploader::handleConnect, shared_from_this(), _1 ) );
                }

                void handleConnect( std::error_code ec )
                {
                    std::ostream os( &request_ );
                    os << "POST /printer/model/" << printer_ << "?a=upload&sess=" << session_ << "&name=" << name_ << " HTTP/1.1\r\n"
                       << "Host: " << hostname_ << "\r\n"
                       << "Content-Type: "
                       << "Connection: close\r\n\r\n"

                       << content_
                }

                bool handleError( std::error_code ec )
                {
                    if ( ec ) {
                        handler_( ec );
                        return true;
                    }
                    return false;
                }

                std::string hostname_;
                std::uint16_t port_;
                std::string session_;
                std::string printer_;
                std::string name_;
                std::string content_;
                Handler handler_;
                asio::io_service service_;
                tcp::resolver resolver_ { service_ };
                tcp::socket socket_ { service_ };
                asio::streambuf request_;
            };

            Uploader::Uploader() = default;

        } // namespace detail

    } // namespace repetier
} // namespace gcu
