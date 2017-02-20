#include <cstring>
#include <algorithm>
#include <iterator>

#include <getopt.h>

#include <boost/convert.hpp>
#include <boost/convert/spirit.hpp>

#include "commandline.hpp"
#include "utility.hpp"

namespace gcu {

    struct Usage
    {
        Usage( char const* pgmname )
                : pgmname_( pgmname )
        {
        }

        template< typename Stream >
        decltype( auto ) friend operator<<( Stream&& stream, Usage const& value )
        {
            return std::forward< Stream >( stream )
                    << "Usage: " << value.pgmname_ << " [OPTIONS] GCODEPATH\n"
                       "Possible OPTIONS:\n"
                       "    --host HOST     (required) Hostname of the printer server\n"
                       "    --port PORT     (required) Port of the printer server\n"
                       "    --apikey KEY    (required) API key for unrestricted access to the print server\n"
                       "    --printer SLUG  (optional) Printer to initially select\n"
                       "    --delete        (optional) Whether the GCODEPATH is to be deleted after uploading\n"
                       "    GCODEPATH       (required) Path to the G-Code file to upload";
        }

    private:
        char const* pgmname_;
    };

    static char const* mapShortToLong( int shortopt )
    {
        switch ( shortopt ) {
            case 'H':
                return "host";
            case 'P':
                return "port";
            case 'a':
                return "apikey";
            case 'p':
                return "printer";
            case 'd':
                return "delete";
            default:
                throw std::invalid_argument( "cmdLineMapShortToLong" );
        }
    }

    CommandLine::CommandLine( char* const* argv, int argc )
    {
        struct option options[] {
                { nullptr, required_argument, nullptr, 'H' },
                { nullptr, required_argument, nullptr, 'P' },
                { nullptr, required_argument, nullptr, 'a' },
                { nullptr, required_argument, nullptr, 'p' },
                { nullptr, no_argument,       nullptr, 'd' },
                {}
        };

        std::for_each(
                std::begin( options ), std::prev( std::end( options ), 1 ),
                []( auto& option ) { option.name = mapShortToLong( option.val ); } );

        opterr = 0;
        int optionIndex = 0;
        int optionChar;
        while ( ( optionChar = getopt_long( argc, argv, ":H:P:p:", options, &optionIndex ) ) != -1 ) {
            switch ( optionChar ) {
                case ':':
                    throw std::runtime_error( util::str(
                            "Missing argument to --", mapShortToLong( optopt ), " (-", (char) optopt, ")\n\n",
                            Usage( argv[ 0 ] ) ) );
                case '?':
                    throw std::runtime_error( util::str(
                            "Invalid option -", (char) optopt, "\n\n", Usage( argv[ 0 ] ) ) );
                case 'H':
                    hostname_ = optarg;
                    break;
                case 'P':
                    port_ = boost::convert< std::uint16_t >( optarg, boost::cnv::spirit() ).value_or( 0 );
                    if ( port_ == 0 ) {
                        throw std::runtime_error( util::str(
                                "Argument to --port (-P) must be a valid port number\n\n", Usage( argv[ 0 ] ) ) );
                    }
                    break;
                case 'a':
                    apikey_ = optarg;
                case 'p':
                    printer_ = optarg;
                    break;
                case 'd':
                    deleteFile_ = true;
                    break;
                default:
                    throw std::invalid_argument( "getopt_long" );
            }
        }

        if ( hostname_.empty() ) {
            throw std::runtime_error( util::str( "Missing required option --host (-h)\n\n", Usage( argv[ 0 ] ) ) );
        }
        if ( port_ == 0 ) {
            throw std::runtime_error( util::str( "Missing required option --port (-p)\n\n", Usage( argv[ 0 ] ) ) );
        }
        if ( apikey_.empty() ) {
            throw std::runtime_error( util::str( "Missing required option --apikey (-a)\n\n", Usage( argv[ 0 ] ) ) );
        }

        if ( optind == argc ) {
            throw std::runtime_error( util::str( "Missing G-Code file path after last option\n\n", Usage( argv[ 0 ] ) ) );
        }
        gcodeFile_ = argv[ optind++ ];

        if ( optind < argc ) {
            throw std::runtime_error( util::str( "excess options" ) );
        }
    }

} // namespace gcu
