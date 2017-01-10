#include <cctype>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <locale>
#include <sstream>

#include <iostream>

#include <nana/gui/wvl.hpp>

#include "uploader_form.hpp"

namespace gcu {

    static std::string const forbiddenPunctuationChars = "\\/:*?\"<>|";

    static std::wstring const forbiddenChars = L"\\/:*?\"<>|";

    static bool isAllowedChar( char ch )
    {
        std::wcout << "check for " << ch << " (" << std::hex << (unsigned) ch << ")\n";
        return std::isalnum( ch, std::locale::classic() ) ||
                ( std::ispunct( ch, std::locale::classic() ) && forbiddenChars.find( ch ) == std::string::npos );
    }

    static char const* surrogateChar( int ch )
    {
        // std::cerr << "CHAR: " << ch << ", INT: " << ch << "\n";
        switch ( ch ) {
            case '�': return "ae";
            case '�': return "Ae";
            case '�': return "oe";
            case '�': return "Oe";
            case '�': return "ue";
            case '�': return "Ue";
            case '�': return "ss";
            default: return "_";
        }
    }

    std::string generateModelName( std::string const& fileName )
    {
        std::ostringstream os;
        for ( auto ch : fileName ) {
            if ( std::isalnum( ch ) ||
                    ( std::ispunct( ch ) && forbiddenPunctuationChars.find( ch ) == std::string::npos ) ) {
                os << ch;
            }
            else {
                os << surrogateChar( ch );
            }
        }
        return os.str();
    }

    UploaderForm::UploaderForm( std::string const& gcodePath )
        : form( nana::API::make_center( 400, 250 ), nana::appear::decorate<>() )
        , gcodePath_( gcodePath )
    {
        caption( "G-Code Uploader" );

        fileNameLabel_.text_align( nana::align::left, nana::align_v::center );
        fileNameTextbox_.enabled( false );
        fileNameTextbox_.reset( gcodePath_.filename().string() );
        printerLabel_.text_align( nana::align::left, nana::align_v::center );
        printerCombox_.enabled( false );
        modelGroupLabel_.text_align( nana::align::left, nana::align_v::center );
        modelGroupCombox_.enabled( false );
        newModelGroupButton_.enabled( false );
        modelNameLabel_.text_align( nana::align::left, nana::align_v::center );
        modelNameTextbox_.reset( generateModelName( gcodePath_.stem().string() ) );
        uploadButton_.enabled( false );

        printerCombox_.events().selected.connect( std::bind( &UploaderForm::printerSelected, this ) );
        modelGroupCombox_.events().selected.connect( std::bind( &UploaderForm::modelGroupSelected, this ) );
        newModelGroupButton_.events().click.connect( std::bind( &UploaderForm::newModelGroupClicked, this ) );
        uploadButton_.events().click.connect( std::bind( &UploaderForm::uploadClicked, this ) );

        place_.div( "vertical margin=10"
                            "< weight=23 arrange=[100,variable] fileName >"
                            "< weight=5 >"
                            "< weight=23 < weight=100 > < deleteFile > >"
                            "< weight=15 >"
                            "< weight=23 arrange=[100,variable] printer >"
                            "< weight=5 >"
                            "< weight=23 arrange=[100,variable,23] gap=[0,5] modelGroup >"
                            "< weight=5 >"
                            "< weight=23 arrange=[100,variable] modelName >"
                            "< weight=15 >"
                            "< weight=25 <> < buttons weight=100 > >" );
        place_[ "fileName" ] << fileNameLabel_ << fileNameTextbox_;
        place_[ "deleteFile" ] << deleteFileCheckbox_;
        place_[ "printer" ] << printerLabel_ << printerCombox_;
        place_[ "modelGroup" ] << modelGroupLabel_ << modelGroupCombox_ << newModelGroupButton_;
        place_[ "modelName" ] << modelNameLabel_ << modelNameTextbox_;
        place_[ "buttons" ] << uploadButton_;
        place_.collocate();

        using namespace std::placeholders;
        client_.connect( "192.168.178.70", 3344, "7f77558d-75e1-45e1-b424-74c5c81b6b47",
                         std::bind( &UploaderForm::handleConnect, this, _1 ) );
    }

    void UploaderForm::printerSelected()
    {
        using namespace std::placeholders;
        client_.listModelGroups( printers_[ printerCombox_.option() ].slug(),
                                 std::bind( &UploaderForm::handleListModelGroups, this, _1, _2 ) );
    }

    void UploaderForm::modelGroupSelected()
    {
        uploadButton_.enabled( true );
    }

    void UploaderForm::newModelGroupClicked()
    {
        nana::inputbox inputbox( *this, "Please enter the name of the new model group", "Input" );
        nana::inputbox::text text( "Model group:" );
        inputbox.verify( [&]( nana::window ) {
            auto value = text.value();
            return !value.empty() && std::find_if_not( value.begin(), value.end(), &isAllowedChar ) == value.end();
        } );
        if ( inputbox.show_modal( text ) ) {
            std::string value = text.value();
            client_.addModelGroup( printers_[ printerCombox_.option() ].slug(), value,
                                   [=]( std::error_code ec ) {
                                       if ( handleError( ec ) ) {
                                           return;
                                       }
                                       modelGroups_.emplace_back( std::move( value ) );
                                       modelGroupCombox_.push_back( modelGroups_.back() );
                                       //modelGroupCombox_.option( modelGroups_.size() - 1 );
                                   } );
        }
    }

    void UploaderForm::uploadClicked()
    {

    }

    bool UploaderForm::handleError( std::error_code ec )
    {
        if ( !ec ) {
            return false;
        }

        printerCombox_.enabled( false );
        modelGroupCombox_.enabled( false );
        newModelGroupButton_.enabled( false );
        modelNameTextbox_.enabled( false );
        uploadButton_.enabled( false );
        return true;
    }

    void UploaderForm::handleConnect( std::error_code ec )
    {
        if ( handleError( ec ) ) {
            return;
        }

        using namespace std::placeholders;
        client_.listPrinter( std::bind( &UploaderForm::handleListPrinter, this, _1, _2 ) );
    }

    void UploaderForm::handleListPrinter( std::vector< repetier::Printer >&& printers, std::error_code ec )
    {
        if ( handleError( ec ) ) {
            return;
        }

        printers_ = std::move( printers );

        printerCombox_.clear();
        for ( auto const& printer : printers_ ) {
            printerCombox_.push_back( printer.name() );
        }
        if ( !printers_.empty() ) {
            printerCombox_.enabled( true );
            printerCombox_.option( 0 );
        }
        else {
            printerCombox_.enabled( false );
        }
    }

    void UploaderForm::handleListModelGroups( std::vector< std::string >&& modelGroups, std::error_code ec )
    {
        if ( handleError( ec ) ) {
            return;
        }

        modelGroups_ = std::move( modelGroups );

        modelGroupCombox_.clear();
        for ( auto const& modelGroup : modelGroups_ ) {
            modelGroupCombox_.push_back( modelGroup == "#" ? "Default" : modelGroup );
        }
        modelGroupCombox_.enabled( true );
        modelGroupCombox_.option( 0 );
        newModelGroupButton_.enabled( true );
    }

} // namespace gcu