#ifndef GCODEUPLOADER_JSON_HPP
#define GCODEUPLOADER_JSON_HPP

#include <memory>
#include <stdexcept>
#include <string>

#include <json/value.h>

namespace Json {
    class CharReader;
    class StreamWriter;
} // namespace Json

namespace gcu {

    class JsonException
        : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    class JsonContext
    {
    public:
        JsonContext();
        JsonContext( JsonContext const& ) = delete;
        ~JsonContext();

        Json::Value toJson( std::string const& string ) const;
        std::string toString( Json::Value const& value ) const;

    private:
        std::unique_ptr< Json::CharReader > reader_;
        std::unique_ptr< Json::StreamWriter > writer_;
    };

} // namespace gcu

#endif // GCODEUPLOADER_JSON_HPP
