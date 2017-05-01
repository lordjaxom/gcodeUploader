#include <memory>
#include <sstream>

#include <json/reader.h>
#include <json/writer.h>

#include "json.hpp"

namespace gcu {

    namespace json {

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

        Json::Value toJson( std::string const& string )
        {
            static auto readerPtr = createJsonReader();

            Json::Value value;
            std::string errors;
            if ( !readerPtr->parse( &string[ 0 ], &string[ string.size() ], &value, &errors ) ) {
                throw JsonException( errors );
            }
            return value;
        }

        std::string fromJson( Json::Value const& json )
        {
            static auto writerPtr = createJsonWriter();

            std::ostringstream os;
            writerPtr->write( json, &os );
            return os.str();
        }

    } // namespace json

} // namespace gcu
