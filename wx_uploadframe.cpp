#include <cstddef>
#include <algorithm>
#include <limits>
#include <string>
#include <system_error>
#include <utility>

#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/textdlg.h>

#include <3dprnet/repetier/frontend.hpp>
#include <3dprnet/repetier/types.hpp>

#include <wx/msw/winundef.h>

#include "wx_clientptr.hpp"
#include "wx_uploadframe.hpp"
#include "wx_generated.h"

using namespace prnet;
using namespace std;

namespace gct {

class UploadFrame
        : public UploadFrameBase
{
    static constexpr size_t MODEL_NOT_FOUND = numeric_limits< size_t >::max();

public:
    UploadFrame( rep::Frontend& frontend, filesystem::path&& gcodePath, wxString&& printer,
                 wxString&& modelName, bool deleteFile )
            : UploadFrameBase( nullptr )
            , frontend_( frontend )
            , gcodePath_( move( gcodePath ) )
            , selectedPrinter_( move( printer ) )
            , enteredModelName_( !modelName.empty() ? move( modelName ) : gcodePath_.stem().native() )
    {
        gcodeFileText_->SetValue( gcodePath_.filename().native() );
        deleteFileCheckbox_->SetValue( deleteFile );
        modelNameText_->SetValue( enteredModelName_ );

        printerChoice_->Bind( wxEVT_CHOICE, [this]( auto& ) { this->OnPrinterSelected(); } );
        modelGroupChoice_->Bind( wxEVT_CHOICE, [this]( auto& ) { this->OnModelGroupSelected(); } );
        addModelGroupButton_->Bind( wxEVT_BUTTON, [this]( auto& ) { this->OnAddModelGroupClicked(); } );
        modelNameText_->Bind( wxEVT_TEXT, [this]( auto& ) { this->OnModelNameChanged(); } );
        uploadButton_->Bind( wxEVT_BUTTON, [this]( auto& ) { this->OnUploadClicked(); } );
        toolBar_->Bind( wxEVT_TOOL, [this]( auto& ) { this->OnToolBarExplore(); }, gctID_EXPLORE );

        frontend_.on_disconnect( [this]( auto ec ) {
            this->CallAfter( [this, ec] {
                this->OnConnectionLost( ec );
            } );
        } );
        frontend_.on_printers( [this]( auto printers ) {
            this->CallAfter( [this, printers]() mutable {
                this->OnPrintersChanged( move( printers ) );
            } );
        } );
        frontend_.on_groups( [this]( auto printer, auto modelGroups ) {
            this->CallAfter( [this, printer, modelGroups]() mutable {
                this->OnModelGroupsChanged( printer, move( modelGroups ) );
            } );
        } );
        frontend_.on_models( [this]( auto printer, auto models ) {
            this->CallAfter( [this, printer, models]() mutable {
                this->OnModelsChanged( printer, move( models ) );
            } );
        } );
        frontend_.requestPrinters();
    }

private:
    void CheckModelNameExists()
    {
        infoLabel_->SetLabel(
                FindSelectedModelId() != MODEL_NOT_FOUND ? _( "Existing G-Code file will be overwritten!" ) : _( "" ) );
    }

    size_t FindSelectedModelId()
    {
        auto it = find_if( models_.begin(), models_.end(), [this]( auto const& item ) {
            return item.name() == enteredModelName_ && item.modelGroup() == selectedModelGroup_;
        } );
        return it != models_.end() ? it->id() : MODEL_NOT_FOUND;
    }

    void PerformUpload( wxString const& printer, wxString const& modelName, wxString const& modelGroup, bool deleteFile )
    {
        frontend_.upload( rep::model_ident( printer.ToStdString(), modelGroup.ToStdString(), modelName.ToStdString() ),
                          gcodePath_, [this, deleteFile]( auto ec ) {
                    if ( ec ) {
                        wxMessageBox( wxString::Format( _( "Upload failed: %s" ), ec.message().c_str() ) );
                    } else if ( deleteFile ) {
                        remove( prnet::filesystem::native_path( gcodePath_ ).c_str() );
                    }
                    this->Close();
                } );
    }

    void OnPrinterSelected()
    {
        int selection = printerChoice_->GetSelection();
        if ( selection != wxNOT_FOUND ) {
            selectedPrinter_ = wxClientPtrCast< rep::Printer >(
                    printerChoice_->GetClientObject( (unsigned) selection ) ).slug();
            frontend_.requestModelGroups( selectedPrinter_.ToStdString() );
            frontend_.requestModels( selectedPrinter_.ToStdString() );
        }
    }

    void OnModelGroupSelected()
    {
        int selection = modelGroupChoice_->GetSelection();
        if ( selection != wxNOT_FOUND ) {
            selectedModelGroup_ = wxClientPtrCast< rep::ModelGroup >(
                    modelGroupChoice_->GetClientObject( (unsigned) selection ) ).name();
            uploadButton_->Enable( true );
            CheckModelNameExists();
        }
    }

    void OnAddModelGroupClicked()
    {
        wxTextEntryDialog dialog( this, _( "Please enter the name of the new model group" ) );
        // TODO: input validation
        if ( dialog.ShowModal() == wxID_OK ) {
            selectedModelGroup_ = dialog.GetValue();
            frontend_.addModelGroup( selectedPrinter_.ToStdString(), selectedModelGroup_.ToStdString() );
        }
    }

    void OnModelNameChanged()
    {
        enteredModelName_ = modelNameText_->GetLineText( 0 ).ToStdString();
        CheckModelNameExists();
    }

    void OnUploadClicked()
    {
        Enable( false );

        auto modelId = FindSelectedModelId();
        if ( modelId != MODEL_NOT_FOUND ) {
            frontend_.removeModel( selectedPrinter_.ToStdString(), modelId,
                                   [this, printer = selectedPrinter_, modelName = enteredModelName_,
                                           modelGroup = selectedModelGroup_, deleteFile = deleteFileCheckbox_->GetValue()] {
                                       PerformUpload( printer, modelName, modelGroup, deleteFile );
                                   } );
        }
        else {
            PerformUpload( selectedPrinter_, enteredModelName_, selectedModelGroup_, deleteFileCheckbox_->GetValue() );
        }
    }

    void OnToolBarExplore()
    {
        // ExplorerFrame* frame = new ExplorerFrame( this, printerService_ );
        // frame->Show( true );
    }


    void OnConnectionLost( error_code ec )
    {
        wxMessageBox( wxString::Format( _( "Connection lost: %s" ), ec.message().c_str() ), "Error", wxOK | wxICON_ERROR, this );
        Close();
    }

    void OnPrintersChanged( vector< rep::Printer >&& printers )
    {
        printerChoice_->Clear();

        int selected = 0;
        for ( auto&& printer : printers ) {
            auto ptr = new wxClientPtr< rep::Printer >( move( printer ) );
            int index = printerChoice_->Append( ( *ptr )->name(), ptr );
            if ( ( *ptr )->slug() == selectedPrinter_ ) {
                selected = index;
            }
        }
        if ( !printers.empty() ) {
            printerChoice_->Enable( true );
            printerChoice_->Select( selected );
            OnPrinterSelected();
        }
        else {
            printerChoice_->Enable( false );
        }
    }

    void OnModelGroupsChanged( string const& printer, vector< rep::ModelGroup >&& modelGroups )
    {
        if ( printer == selectedPrinter_ ) {
            modelGroupChoice_->Clear();

            int selected = 0;
            for ( auto&& modelGroup : modelGroups ) {
                auto ptr = new wxClientPtr< rep::ModelGroup >( move( modelGroup ) );
                int index = modelGroupChoice_->Append(
                        ( *ptr )->defaultGroup() ? _( "Default" ) : ( *ptr )->name(), ptr );
                if ( ( *ptr )->name() == selectedModelGroup_ ) {
                    selected = index;
                }
            }
            modelGroupChoice_->Enable( true );
            modelGroupChoice_->Select( selected );
            OnModelGroupSelected();
            addModelGroupButton_->Enable( true );
        }
    }

    void OnModelsChanged( string const& printer, vector< rep::Model >&& models )
    {
        if ( printer == selectedPrinter_ ) {
            models_ = move( models );
            CheckModelNameExists();
        }
    }


    rep::Frontend& frontend_;
    filesystem::path gcodePath_;
    wxString selectedPrinter_;
    wxString selectedModelGroup_;
    wxString enteredModelName_;
    vector< rep::Model > models_;
};

wxFrame* makeUploadFrame( rep::Frontend& frontend, filesystem::path gcodePath, wxString printer,
                              wxString modelName, bool deleteFile )
{
    return new UploadFrame( frontend, move( gcodePath ), move( printer ), move( modelName ), deleteFile );
}

} // namespace gct
