#include "repetier_definitions.hpp"

namespace gcu {

    namespace repetier {

        Printer::Printer( bool active, std::string name, std::string slug )
                : active_( active )
                , name_( std::move( name ) )
                , slug_( std::move( slug ) )
        {
        }

        Model::Model( unsigned id, std::string name, std::string modelGroup )
                : id_( id )
                , name_( std::move( name ) )
                , modelGroup_( std::move( modelGroup ) )
        {
        }

        ModelGroup::ModelGroup( std::string name )
                : name_( std::move( name ) )
                , defaultGroup_( name_ == "#" )
        {
        }

    } // namespace repetier

} // namespace gcu
