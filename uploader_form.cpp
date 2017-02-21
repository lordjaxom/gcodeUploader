#include <cctype>
#include <cstddef>
#include <cstdio>
#include <algorithm>
#include <locale>
#include <sstream>

#include <nana/gui/timer.hpp>
#include <nana/gui/wvl.hpp>

#include "printer_service.hpp"
#include "uploader_form.hpp"

namespace gcu {

    static std::string const forbiddenPunctuationChars = "\\/:*?\"<>|";

    static std::wstring const forbiddenChars = L"\\/:*?\"<>|";

    static bool isAllowedChar( char ch )
    {
        return ch == ' ' || std::isalnum( ch, std::locale::classic() ) ||
                ( std::ispunct( ch, std::locale::classic() ) && forbiddenChars.find( ch ) == std::string::npos );
    }

    static char const* surrogateChar( int ch )
    {
        switch ( ch ) {
            case 'ä': return "ae";
            case 'Ä': return "Ae";
            case 'ö': return "oe";
            case 'Ö': return "Oe";
            case 'ü': return "ue";
            case 'Ü': return "Ue";
            case 'ß': return "ss";
            default: return "_";
        }
    }

    std::string generateModelName( std::string const& fileName )
    {
        std::ostringstream os;
        for ( auto ch : fileName ) {
            if ( ch == ' ' || std::isalnum( ch ) ||
                    ( std::ispunct( ch ) && forbiddenPunctuationChars.find( ch ) == std::string::npos ) ) {
                os << ch;
            }
            else {
                os << surrogateChar( ch );
            }
        }
        return os.str();
    }

    UploaderForm::UploaderForm(
            PrinterService& printerService, std::string const& gcodePath, std::string const& printer, bool deleteFile )
            : form( nana::API::make_center( 400, 250 ), nana::appear::decorate<>() )
            , printerService_( printerService )
            , gcodePath_( gcodePath )
            , selectedPrinter_( printer )
    {
        caption( "G-Code Uploader" );

        fileNameLabel_.text_align( nana::align::left, nana::align_v::center );
        fileNameTextbox_.enabled( false );
        fileNameTextbox_.reset( gcodePath_.filename().string() );
        deleteFileCheckbox_.check( deleteFile );
        printerLabel_.text_align( nana::align::left, nana::align_v::center );
        printerCombox_.enabled( false );
        modelGroupLabel_.text_align( nana::align::left, nana::align_v::center );
        modelGroupCombox_.enabled( false );
        newModelGroupButton_.enabled( false );
        modelNameLabel_.text_align( nana::align::left, nana::align_v::center );
        modelNameTextbox_.reset( generateModelName( gcodePath_.stem().string() ) );
        uploadButton_.enabled( false );

        printerCombox_.events().selected.connect( [this] { printerSelected(); } );
        modelGroupCombox_.events().selected.connect( [this] { modelGroupSelected(); } );
        newModelGroupButton_.events().click.connect( [this] { newModelGroupClicked(); } );
        modelNameTextbox_.events().text_changed.connect( [this] { modelNameTextChanged(); } );
        uploadButton_.events().click.connect( [this] { uploadClicked(); } );

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
                            "< weight=25 <> < buttons weight=100 > >"
                            "< weight=5 >"
                            "< weight=23 info >");
        place_[ "fileName" ] << fileNameLabel_ << fileNameTextbox_;
        place_[ "deleteFile" ] << deleteFileCheckbox_;
        place_[ "printer" ] << printerLabel_ << printerCombox_;
        place_[ "modelGroup" ] << modelGroupLabel_ << modelGroupCombox_ << newModelGroupButton_;
        place_[ "modelName" ] << modelNameLabel_ << modelNameTextbox_;
        place_[ "buttons" ] << uploadButton_;
        place_[ "info" ] << infoLabel_;
        place_.collocate();

        printerService_.connectionLost.connect( [this]( auto ec ) {
            this->handleConnectionLost( ec );
        } );
        printerService_.printersChanged.connect( [this]( auto const& printers ) {
            this->handlePrintersChanged( std::move( printers ) );
        } );
        printerService_.modelGroupsChanged.connect( [this]( auto const& printer, auto const& modelGroups ) {
            this->handleModelGroupsChanged( printer, std::move( modelGroups ) );
        } );
        printerService_.modelsChanged.connect( [this]( auto const& printer, auto const& models ) {
            this->handleModelsChanged( printer, std::move( models ) );
        } );
        printerService_.requestPrinters();
    }

    void UploaderForm::printerSelected()
    {
        selectedPrinter_ = printerCombox_.anyobj< repetier::Printer >( printerCombox_.option() )->slug();
        printerService_.requestModelGroups( selectedPrinter_ );
        printerService_.requestModels( selectedPrinter_ );
    }

    void UploaderForm::modelGroupSelected()
    {
        std::cerr << "modelGroupSelected: option = " << modelGroupCombox_.option() << "\n";
        selectedModelGroup_ = modelGroupCombox_.anyobj< repetier::ModelGroup >( modelGroupCombox_.option() )->name();
        uploadButton_.enabled( true );
        modelNameTextChanged();
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
            selectedModelGroup_ = text.value();
            printerService_.addModelGroup( selectedPrinter_, selectedModelGroup_ );
        }
    }

    void UploaderForm::modelNameTextChanged()
    {
        std::string modelName;
        modelNameTextbox_.getline( 0, enteredModelName_ );
        checkModelName();
    }

    void UploaderForm::uploadClicked()
    {
        deleteFileCheckbox_.enabled( false );
        printerCombox_.enabled( false );
        modelGroupCombox_.enabled( false );
        newModelGroupButton_.enabled( false );
        modelNameTextbox_.enabled( false );
        uploadButton_.enabled( false );

        auto deleteFile = deleteFileCheckbox_.checked();

        auto modelId = existingModelId( enteredModelName_, selectedModelGroup_ );
        if ( modelId != invalidId ) {
            printerService_.removeModel( selectedPrinter_, modelId, [this, deleteFile] {
                performUpload( selectedPrinter_, enteredModelName_, selectedModelGroup_, deleteFile );
            } );
        }
        else {
            performUpload( selectedPrinter_, enteredModelName_, selectedModelGroup_, deleteFile );
        }
    }

    void UploaderForm::checkModelName()
    {
        auto modelId = existingModelId( enteredModelName_, selectedModelGroup_ );
        infoLabel_.caption( modelId != invalidId ? "Existing G-Code file will be overwritten!" : "" );
    }

    void UploaderForm::performUpload(
            std::string const& printer, std::string const& modelName, std::string const& modelGroup, bool deleteFile )
    {
        printerService_.upload( printer, modelName, modelGroup, gcodePath_, [this, deleteFile] {
            if ( deleteFile ) {
                std::remove( gcodePath_.string().c_str() );
            }
            close();
        } );
    }

    void UploaderForm::handleConnectionLost( std::error_code ec )
    {
        nana::msgbox mb( *this, "Error" );
        mb << "Connection lost: " << ec.message();
        mb.show();

        close();
    }

    void UploaderForm::handlePrintersChanged( std::vector< repetier::Printer > const& printers )
    {
        printerCombox_.clear();

        std::size_t selected = 0;
        for ( auto const& printer : printers ) {
            std::size_t index = printerCombox_.the_number_of_options();
            if ( printer.slug() == selectedPrinter_ ) {
                selected = index;
            }
            printerCombox_.push_back( printer.name() );
            printerCombox_.anyobj( index, printer );
        }
        if ( !printers.empty() ) {
            printerCombox_.enabled( true );
            printerCombox_.option( selected );
        }
        else {
            printerCombox_.enabled( false );
        }
    }

    void UploaderForm::handleModelGroupsChanged(
            std::string const& printer, std::vector< repetier::ModelGroup > const& modelGroups )
    {
        std::cerr << "handleModelGroupsChanged( " << printer << " )\n";
        if ( selectedPrinter_ == printer ) {
            modelGroupCombox_.clear();

            std::size_t selected = 0;
            for ( auto const& modelGroup : modelGroups ) {
                std::size_t index = modelGroupCombox_.the_number_of_options();
                if ( modelGroup.name() == selectedModelGroup_ ) {
                    selected = index;
                }
                modelGroupCombox_.push_back( modelGroup.defaultGroup() ? "Default" : modelGroup.name() );
                modelGroupCombox_.anyobj( index, modelGroup );
            }
            modelGroupCombox_.enabled( true );
            modelGroupCombox_.option( selected );
            newModelGroupButton_.enabled( true );
        }
    }

    void UploaderForm::handleModelsChanged( std::string const& printer, std::vector< repetier::Model > const& models )
    {
        if ( selectedPrinter_ == printer ) {
            models_ = std::move( models );
            checkModelName();
        }
    }

    unsigned UploaderForm::existingModelId( std::string const& modelName, std::string const& modelGroup ) const
    {
        auto it = std::find_if( models_.begin(), models_.end(), [&]( auto const& item ) {
            return item.name() == modelName && item.modelGroup() == modelGroup;
        } );
        return it != models_.end() ? it->id() : invalidId;
    }

} // namespace gcu