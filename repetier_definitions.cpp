#include "repetier_definitions.hpp"

namespace gcu {

    namespace repetier {

        Printer::Printer( bool active, std::string name, std::string slug )
                : active_( active )
                , name_( std::move( name ) )
                , slug_( std::move( slug ) )
        {
        }

    } // namespace repetier

} // namespace gcu
