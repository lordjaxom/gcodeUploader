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

        class Model
        {
        public:
            Model( unsigned id, std::string name, std::string modelGroup );

            unsigned id() const { return id_; }
            std::string const& name() const { return name_; }
            std::string const& modelGroup() const { return modelGroup_; }

        private:
            unsigned id_;
            std::string name_;
            std::string modelGroup_;
        };

        class ModelGroup
        {
        public:
            ModelGroup( std::string name );

            std::string const& name() const { return name_; }
            bool defaultGroup() const { return defaultGroup_; }

        private:
            std::string name_;
            bool defaultGroup_;
        };

        template< typename ...Args >
        using Callback = std::function< void ( Args..., std::error_code ) >;

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
