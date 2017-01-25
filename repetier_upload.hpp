#ifndef GCODEUPLOADER_REPETIER_UPLOADER_HPP
#define GCODEUPLOADER_REPETIER_UPLOADER_HPP

#include <cstdint>
#include <string>

#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>

#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

        class Upload
        {
        public:
            Upload(
                    asio::io_service& service, std::string const& hostname, std::uint16_t port,
                    std::string const& session, std::string const& printer, std::string const& name,
                    std::string const& content, Callback<>&& callback );

        private:
            asio::ip::tcp::resolver resolver_;
            asio::ip::tcp::socket socket_;
            asio::streambuf request_;
            asio::streambuf response_;
        };

        void upload(
                asio::io_service& service, std::string hostname, std::uint16_t port, std::string session,
                std::string printer, std::string name, std::string content, Callback<> callback );

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_UPLOADER_HPP
