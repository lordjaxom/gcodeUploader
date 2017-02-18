#include <cstring>
#include <algorithm>
#include <iterator>

#include <getopt.h>

#include <boost/convert.hpp>
#include <boost/convert/spirit.hpp>

#include "commandline.hpp"
#include "utility.hpp"

namespace gcu {

    template< typename Streamable >
    static decltype( auto ) usage( Streamable&& streamable, char const* pgmname )
    {
        util::stream(
                std::forward< Streamable >( streamable ),
                "Usage: ", pgmname, " [OPTIONS] [ARGUMENTS]\n"
                "    --host     Hostname of the printer server\n"
                "    --port     Port of the print server\n"
                "    --apikey   API key for unrestricted access to the print server\n" );
        return std::forward< Streamable >( streamable );
    }

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
                    throw std::runtime_error(
                            util::str( "Missing argument to --", mapShortToLong( optopt ), " (-", (char) optopt ) );
                case '?':
                    throw std::runtime_error(
                            util::str( "unknown option -", (char) optopt ) );
                case 'H':
                    hostname_ = optarg;
                    break;
                case 'P':
                    port_ = boost::convert< std::uint16_t >( optarg, boost::cnv::spirit() ).value_or( 0 );
                    if ( port_ == 0 ) {
                        throw std::runtime_error( util::str( "argument to --port (-P) requires a numeric argument" ) );
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
            throw std::runtime_error( "missing option --host (-h)" );
        }
        if ( port_ == 0 ) {
            throw std::runtime_error( "missing option --port (-p)" );
        }
        if ( apikey_.empty() ) {
            throw std::runtime_error( "missing option --apikey (-a)" );
        }

        if ( optind == argc ) {
            throw std::runtime_error( "missing filename after options" );
        }
        gcodeFile_ = argv[ optind++ ];

        if ( optind < argc ) {
            throw std::runtime_error( util::str( "excess options" ) );
        }
    }

} // namespace gcu
