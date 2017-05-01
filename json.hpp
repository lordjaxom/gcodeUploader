#ifndef GCODEUPLOADER_JSON_HPP
#define GCODEUPLOADER_JSON_HPP

#include <stdexcept>
#include <string>

#include <json/value.h>

namespace gcu {

    class JsonException
        : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    namespace json {

        Json::Value toJson( std::string const& string );
        std::string fromJson( Json::Value const& value );

    } // namespace json

} // namespace gcu

#endif // GCODEUPLOADER_JSON_HPP
