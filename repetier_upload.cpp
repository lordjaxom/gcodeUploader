#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <thread>
#include <utility>

#include <asio/connect.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>

#include "repetier_upload.hpp"

namespace gcu {
    namespace repetier {

        Upload::Upload( asio::io_service& service, std::string const& hostname, std::uint16_t port,
                        std::string const& session )
                : resolver_( service )
                ,
        {

        }
        namespace detail {

            class Uploader
                    : public std::enable_shared_from_this< Uploader >
            {
            public:
                Uploader(
                        asio::io_service& service, std::string&& hostname, std::uint16_t port, std::string&& session,
                        std::string&& printer, std::string&& name, std::string&& content, Callback<>&& callback )
                        : hostname_( std::move( hostname ) )
                        , port_( port )
                        , session_( std::move( session ) )
                        , printer_( std::move( printer ) )
                        , name_( std::move( name ) )
                        , content_( std::move( content ) )
                        , callback_( std::move( callback ) )
                        , resolver_( service )
                        , socket_( service )
                {
                }

                ~Uploader()
                {
                    std::cerr << "DEBUG: Closing connection and joining thread\n";

                    std::error_code ec;
                    socket_.shutdown( asio::ip::tcp::socket::shutdown_both, ec );
                    socket_.close( ec );
                    // destructor runs in thread, so don't join... thread will "die" automatically
                }

                void start()
                {
                    std::cerr << "INFO: Uploading G-Code to " << hostname_ << ":" << port_ << "\n";

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

                    std::cerr << "DEBUG: Resolved " << hostname_ << ", connecting\n";

                    using namespace std::placeholders;
                    asio::async_connect( socket_, it, std::bind( &Uploader::handleConnect, shared_from_this(), _1 ) );
                }

                void handleConnect( std::error_code ec )
                {
                    if ( handleError( ec ) ) {
                        return;
                    }

                    std::cerr << "DEBUG: Connected to " << hostname_ << ", sending file (" << content_.size() << " bytes)\n";

                    std::ostringstream cts;
                    cts << "---------------------------9051914041544843365972754266\r\n"
                        << "Content-Disposition: form-data; name=\"filename\"; filename=\"" << name_ << "\"\r\n"
                        << "Content-Type: application/octet-stream\r\n\r\n"
                        << content_ << "\r\n"
                        << "---------------------------9051914041544843365972754266--"
                        << std::flush;
                    std::string content = cts.str();

                    std::ostream os( &request_ );
                    os << "POST /printer/model/" << printer_ << "?a=upload&sess=" << session_ << "&name=" << name_ << " HTTP/1.1\r\n"
                       << "Host: " << hostname_ << ":" << port_ << "\r\n"
                       << "Content-Type: multipart/form-data; boundary=---------------------------9051914041544843365972754266" << "\r\n"
                       << "Content-Length: " << content.size() << "\r\n"
                       << "Connection: close\r\n\r\n"
                       << content
                       << std::flush;

                    std::cerr << ">>> " << std::string( asio::buffers_begin( request_.data() ), asio::buffers_end( request_.data() ) ) << "\n";

                    using namespace std::placeholders;
                    asio::async_write( socket_, request_, std::bind( &Uploader::handleWrite, shared_from_this(), _1 ) );
                }

                void handleWrite( std::error_code ec )
                {
                    if ( handleError( ec ) ) {
                        return;
                    }

                    std::cerr << "DEBUG: Request sent, reading response\n";

                    using namespace std::placeholders;
                    asio::async_read_until(
                            socket_, response_, "\n", std::bind( &Uploader::handleReadStatus, shared_from_this(), _1 ) );
                }

                void handleReadStatus( std::error_code ec )
                {
                    if ( handleError( ec ) ) {
                        return;
                    }

                    std::cerr << "DEBUG: Response read\n";

                    std::cerr << "<<< " << std::string( asio::buffers_begin( response_.data() ), asio::buffers_end( response_.data() ) ) << "\n";
                    callback_( ec );
                }

                bool handleError( std::error_code ec )
                {
                    if ( ec ) {
                        callback_( ec );
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
                Callback<> callback_;

                tcp::resolver resolver_;
                tcp::socket socket_;
                asio::streambuf request_;
                asio::streambuf response_;
            };

        } // namespace detail

        void upload(
                asio::io_service& service, std::string hostname, std::uint16_t port, std::string session,
                std::string printer, std::string name, std::string content, Callback<> callback )
        {
            std::make_shared< detail::Uploader >(
                    service, std::move( hostname ), port, std::move( session ), std::move( printer ), std::move( name ),
                    std::move( content ), std::move( callback ) )->start();
        }

    } // namespace repetier
} // namespace gcu
