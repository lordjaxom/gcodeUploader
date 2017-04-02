#ifndef GCODEUPLOADER_REPETIER_HPP
#define GCODEUPLOADER_REPETIER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include "std_filesystem.hpp"
#include "std_optional.hpp"

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

        bool connected() const;

        repetier::ClientEvents& events();

        void connect( std::string hostname, std::uint16_t port, std::string apikey, repetier::Callback<> callback );
        void listPrinter( repetier::Callback< std::vector< repetier::Printer > > callback );
        void listModels( std::string const& printer, repetier::Callback< std::vector< repetier::Model > > callback );
        void listModelGroups( std::string const& printer, repetier::Callback< std::vector< repetier::ModelGroup > > callback );
        void addModelGroup( std::string const& printer, std::string const& modelGroup, repetier::Callback<> callback );
        void delModelGroup(
                std::string const& printer, std::string const& modelGroup, bool deleteModels,
                repetier::Callback<> callback );
        void removeModel( std::string const& printer, std::size_t id, repetier::Callback<> callback );
        void moveModelFileToGroup(
                std::string const& printer, unsigned id, std::string const& modelGroup,
                repetier::Callback<> callback );

        void upload(
                std::string const& printer, std::string const& modelName, std::string const& modelGroup,
                std::filesystem::path const& gcodePath, repetier::Callback<> callback );

    private:
        std::string hostname_;
        std::uint16_t port_;
        std::string apikey_;

        asio::io_service service_;
        std::optional< repetier::Client > client_ { std::in_place, service_ };
        std::thread thread_ { [this] { service_.run(); }};
    };

} // namespace gcu

#endif // GCODEUPLOADER_REPETIER_HPP
