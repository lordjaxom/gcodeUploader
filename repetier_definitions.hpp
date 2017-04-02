#ifndef GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
#define GCODEUPLOADER_REPETIER_DEFINITIONS_HPP

#include <ctime>
#include <chrono>
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

        class Model
        {
        public:
            Model(
                    std::size_t id, std::string name, std::string modelGroup, std::time_t created, std::size_t length,
                    std::size_t layers, std::size_t lines, std::chrono::microseconds printTime );

            std::size_t id() const { return id_; }
            std::string const& name() const { return name_; }
            std::string const& modelGroup() const { return modelGroup_; }
            std::time_t const& created() const { return created_; }
            std::size_t length() const { return length_; }
            std::size_t layers() const { return layers_; }
            std::size_t lines() const { return lines_; }
            std::chrono::microseconds printTime() const { return printTime_; }

        private:
            std::size_t id_;
            std::string name_;
            std::string modelGroup_;
            std::time_t created_;
            std::size_t length_;
            std::size_t layers_;
            std::size_t lines_;
            std::chrono::microseconds printTime_;
        };

        class ModelGroup
        {
        public:
            static bool defaultGroup( std::string const& name );

            ModelGroup( std::string name );

            std::string const& name() const { return name_; }
            bool defaultGroup() const { return defaultGroup( name_ ); }

        private:
            std::string name_;
        };

        template< typename ...Args >
        using Callback = std::function< void ( Args..., std::error_code ) >;

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
