#include "repetier_definitions.hpp"

namespace gcu {
    namespace repetier {

        Printer::Printer( bool active, std::string name, std::string slug )
                : active_( active )
                , name_( std::move( name ) )
                , slug_( std::move( slug ) )
        {
        }

        Model::Model(
                std::size_t id, std::string name, std::string modelGroup, std::time_t created, std::size_t length,
                std::size_t layers, std::size_t lines, std::chrono::microseconds printTime )
                : id_( id )
                , name_( std::move( name ) )
                , modelGroup_( std::move( modelGroup ) )
                , created_( created )
                , length_( length )
                , layers_( layers )
                , lines_( lines )
                , printTime_( printTime )
        {
        }

        bool ModelGroup::defaultGroup( std::string const& name )
        {
            return name == "#";
        }

        ModelGroup::ModelGroup( std::string name )
                : name_( std::move( name ) )
        {
        }

    } // namespace repetier
} // namespace gcu
