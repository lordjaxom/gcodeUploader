#ifndef GCODEUPLOADER_PRINTER_SERVICE_HPP
#define GCODEUPLOADER_PRINTER_SERVICE_HPP

#include <functional>
#include <mutex>
#include <map>
#include <memory>
#include <system_error>

#include "std/optional.hpp"

#include <boost/signals2/signal.hpp>

#include "repetier.hpp"
#include "string.hpp"

namespace gcu {

    class PrinterService
    {
        enum State
        {
            CONNECTING,
            CONNECTED,
            CLOSED
        };

    public:
        PrinterService( std::string const& hostname, std::uint16_t port, std::string const& apikey );

        void requestPrinters();
        void requestModelGroups( std::string const& printer );
        void requestModels( std::string const& printer );

        void addModelGroup(
                std::string const& printer, std::string const& modelGroup, std::function< void () > callback = []{} );
        void delModelGroup(
                std::string const& printer, std::string const& modelGroup, bool deleteModels,
                std::function< void () > callback = []{} );
        void removeModel( std::string const& printer, std::size_t id, std::function< void () > callback = {} );
        void moveModelToGroup(
                std::string const& printer, unsigned modelId, std::string const& modelGroup,
                std::function< void () > callback = {} );
        void upload(
                std::string const& printer, std::string const& modelName, std::string const& modelGroup,
                std::filesystem::path const& gcodePath, std::filesystem::path const& executablePath,
                std::function< void () > callback = {} );

        boost::signals2::signal< void ( std::error_code ) > connectionLost;
        boost::signals2::signal< void ( std::vector< repetier::Printer > const& ) > printersChanged;
        boost::signals2::signal< void ( std::string const&, std::vector< repetier::ModelGroup > const& ) > modelGroupsChanged;
        boost::signals2::signal< void ( std::string const&, std::vector< repetier::Model > const& ) > modelsChanged;

    private:
        bool success( std::error_code ec );

        bool checkConnection();

        void listPrinters();
        void listModelsAndModelGroups();
        void listModelGroups( std::string const& printer );
        void listModels( std::string const& printer );

        RepetierClient client_;
        State state_ { CONNECTING };
        std::error_code errorCode_;
        std::optional< std::vector< repetier::Printer > > printers_;
        std::map< std::string, std::vector< repetier::ModelGroup > > modelGroups_;
        std::map< std::string, std::vector< repetier::Model > > models_;
        std::recursive_mutex mutex_;
    };

} // namespace gcu

#endif // GCODEUPLOADER_PRINTER_SERVICE_HPP
