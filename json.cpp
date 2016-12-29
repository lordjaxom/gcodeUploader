#include <sstream>
#include <utility>

#include <json/reader.h>
#include <json/writer.h>

#include "json.hpp"

namespace gcu {

    static std::unique_ptr< Json::CharReader > createJsonReader()
    {
        Json::CharReaderBuilder builder;
        return std::unique_ptr< Json::CharReader >( builder.newCharReader() );
    }

    static std::unique_ptr< Json::StreamWriter > createJsonWriter()
    {
        Json::StreamWriterBuilder builder;
        builder[ "indentation" ] = "";
        return std::unique_ptr< Json::StreamWriter >( builder.newStreamWriter() );
    }

    JsonContext::JsonContext()
        : reader_( createJsonReader() )
        , writer_( createJsonWriter() )
    {
    }

    JsonContext::~JsonContext() = default;

    Json::Value JsonContext::toJson( std::string const& string ) const
    {
        Json::Value value;
        std::string errors;
        if ( !reader_->parse( &string[ 0 ], &string[ string.size() ], &value, &errors ) ) {
            throw JsonException( errors );
        }
        return value;
    }

    std::string JsonContext::toString( Json::Value const& value ) const
    {
        std::ostringstream os;
        writer_->write( value, &os );
        return os.str();
    }

} // namespace gcu
