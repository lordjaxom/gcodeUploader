#ifndef GCODEUPLOADER_COMMANDLINE_HPP
#define GCODEUPLOADER_COMMANDLINE_HPP

#include <cstdint>
#include <stdexcept>
#include <string>

namespace gcu {

    enum class Command
    {
        UPLOAD,
        LIST
    };

    class CommandLine
    {
    public:
        CommandLine( char* const* argv, int argc );

        std::string const& hostname() const { return hostname_; }
        std::uint16_t port() const { return port_; }
        std::string const& apikey() const { return apikey_; }
        std::string const& printer() const { return printer_; }
        bool deleteFile() const { return deleteFile_; }
        std::string const& gcodeFile() const { return gcodeFile_; }

    private:
        std::string hostname_;
        std::uint16_t port_ {};
        std::string apikey_;
        std::string printer_;
        bool deleteFile_ {};
        std::string gcodeFile_;
    };

} // namespace gcu

#endif // GCODEUPLOADER_COMMANDLINE_HPP
