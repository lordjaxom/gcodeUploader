#ifndef GCODEUPLOADER_REPETIER_HPP
#define GCODEUPLOADER_REPETIER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include <experimental/optional>

#include <asio/io_service.hpp>

#include "repetier_client.hpp"
#include "repetier_definitions.hpp"

namespace gcu {

    class RepetierClient
    {
    public:
        RepetierClient();
        RepetierClient( RepetierClient const& ) = delete;
        ~RepetierClient();

        std::string session() const;

        void connect( std::string hostname, std::uint16_t port, std::string apikey, repetier::Callback<> callback );
        void listPrinter( repetier::Callback< std::vector< repetier::Printer > > callback );
        void listModelGroups( std::string const& printer, repetier::Callback< std::vector< std::string > > callback );
        void addModelGroup( std::string const& printer, std::string const& modelGroup, repetier::Callback<> callback );

        void upload(
                std::string const& printer, std::string const& name, std::string const& content,
                repetier::Callback<> callback );

    private:
        std::string hostname_;
        std::uint16_t port_;
        std::string apikey_;

        asio::io_service service_;
        std::experimental::optional< repetier::Client > client_ { std::experimental::in_place, service_ };
        std::experimental::optinal< repetier::Upload > upload_;
        std::thread thread_ { [this] { service_.run(); }};
    };

} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_HPP
