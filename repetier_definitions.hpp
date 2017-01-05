#ifndef GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
#define GCODEUPLOADER_REPETIER_DEFINITIONS_HPP

#include <functional>
#include <string>
#include <system_error>
#include <vector>

namespace gcu {
    namespace repetier {

        class Printer
        {
        public:
            Printer( bool active, std::string name, std::string slug );

            bool active() const { return active_; }
            std::string const& name() const { return name_; }
            std::string const& slug() const { return slug_; }

        private:
            bool active_;
            std::string name_;
            std::string slug_;
        };

        using ConnectCallback = std::function< void ( std::error_code ec ) >;
        using ListPrinterCallback = std::function< void ( std::vector< Printer >&& printers, std::error_code ec ) >;
        using ListModelGroupsCallback = std::function< void ( std::vector< std::string >&& modelGroups, std::error_code ec ) >;

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
