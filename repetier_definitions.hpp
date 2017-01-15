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

        namespace detail {

            template< typename Result >
            struct Callback
            {
                using Type = std::function< void ( Result&&, std::error_code ) >;
            };

            template<>
            struct Callback< void >
            {
                using Type = std::function< void ( std::error_code ) >;
            };

        } // namespace detail

        template< typename Result >
        using Callback = typename detail::Callback< Result >::Type;

    } // namespace repetier
} // namespace gcu

#endif //GCODEUPLOADER_REPETIER_DEFINITIONS_HPP
