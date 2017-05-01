#include <algorithm>
#include <locale>
#include <utility>

#include <wx/msgdlg.h>
#include <wx/textdlg.h>

#include "printer_service.hpp"
#include <wx/msw/winundef.h>

#include "wx_clientptr.hpp"
#include "wx_uploadframe.hpp"
#include "wx_explorerframe.hpp"

namespace gct {

    UploadFrame::UploadFrame(
            std::shared_ptr< gcu::PrinterService > printerService, std::filesystem::path gcodePath,
            wxString printer, wxString modelName, bool deleteFile )
            : UploadFrameBase( nullptr )
            , printerService_( std::move( printerService ) )
            , gcodePath_( std::move( gcodePath ) )
            , selectedPrinter_( std::move( printer ) )
            , enteredModelName_( !modelName.empty() ? std::move( modelName ) : gcodePath_.stem().string() )
    {
        gcodeFileText_->SetValue( gcodePath_.filename().string() );
        deleteFileCheckbox_->SetValue( deleteFile );
        modelNameText_->SetValue( enteredModelName_ );

        printerChoice_->Bind( wxEVT_CHOICE, [this]( auto& ) { this->OnPrinterSelected(); } );
        modelGroupChoice_->Bind( wxEVT_CHOICE, [this]( auto& ) { this->OnModelGroupSelected(); } );
        addModelGroupButton_->Bind( wxEVT_BUTTON, [this]( auto& ) { this->OnAddModelGroupClicked(); } );
        modelNameText_->Bind( wxEVT_TEXT, [this]( auto& ) { this->OnModelNameChanged(); } );
        uploadButton_->Bind( wxEVT_BUTTON, [this]( auto& ) { this->OnUploadClicked(); } );
        toolBar_->Bind( wxEVT_TOOL, [this]( auto& ) { this->OnToolBarExplore(); }, gctID_EXPLORE );

        printerService_->connectionLost.connect( [this]( auto ec ) {
            this->CallAfter( [=] {
                this->OnConnectionLost( ec );
            } );
        } );
        printerService_->printersChanged.connect( [this]( auto const& printers ) {
            this->CallAfter( [=, printers = printers]() mutable {
                this->OnPrintersChanged( std::move( printers ) );
            } );
        } );
        printerService_->modelGroupsChanged.connect( [this]( auto const& printer, auto const& modelGroups ) {
            this->CallAfter( [=, modelGroups = modelGroups]() mutable {
                this->OnModelGroupsChanged( printer, std::move( modelGroups ) );
            } );
        } );
        printerService_->modelsChanged.connect( [this]( auto const& printer, auto const& models ) {
            this->CallAfter( [=, models = models]() mutable {
                this->OnModelsChanged( printer, std::move( models ) );
            } );
        } );
        printerService_->requestPrinters();
    }

    void UploadFrame::CheckModelNameExists()
    {
        infoLabel_->SetLabel(
                FindSelectedModelId() != MODEL_NOT_FOUND ? _( "Existing G-Code file will be overwritten!" ) : _( "" ) );
    }

    std::size_t UploadFrame::FindSelectedModelId()
    {
        auto it = std::find_if( models_.begin(), models_.end(), [this]( auto const& item ) {
            return item.name() == enteredModelName_.ToUTF8().data() && item.modelGroup() == selectedModelGroup_;
        } );
        return it != models_.end() ? it->id() : MODEL_NOT_FOUND;
    }

    void UploadFrame::PerformUpload(
            wxString const& printer, wxString const& modelName, wxString const& modelGroup, bool deleteFile )
    {
        printerService_->upload(
                printer.ToStdString(), modelName.ToUTF8().data(), modelGroup.ToStdString(), gcodePath_,
                [this, deleteFile] {
                    if ( deleteFile ) {
                        std::remove( gcodePath_.string().c_str() );
                    }
                    Close();
                } );
    }

    void UploadFrame::OnPrinterSelected()
    {
        int selection = printerChoice_->GetSelection();
        if ( selection != wxNOT_FOUND ) {
            selectedPrinter_ = wxClientPtrCast< gcu::repetier::Printer >(
                    printerChoice_->GetClientObject( (unsigned) selection ) ).slug();
            printerService_->requestModelGroups( selectedPrinter_.ToStdString() );
            printerService_->requestModels( selectedPrinter_.ToStdString() );
        }
    }

    void UploadFrame::OnModelGroupSelected()
    {
        int selection = modelGroupChoice_->GetSelection();
        if ( selection != wxNOT_FOUND ) {
            selectedModelGroup_ = wxClientPtrCast< gcu::repetier::ModelGroup >(
                    modelGroupChoice_->GetClientObject( (unsigned) selection ) ).name();
            uploadButton_->Enable( true );
            CheckModelNameExists();
        }
    }

    void UploadFrame::OnAddModelGroupClicked()
    {
        wxTextEntryDialog dialog( this, _( "Please enter the name of the new model group" ) );
        // TODO: input validation
        if ( dialog.ShowModal() == wxID_OK ) {
            selectedModelGroup_ = dialog.GetValue();
            printerService_->addModelGroup( selectedPrinter_.ToStdString(), selectedModelGroup_.ToStdString() );
        }
    }

    void UploadFrame::OnModelNameChanged()
    {
        enteredModelName_ = modelNameText_->GetLineText( 0 ).ToStdString();
        CheckModelNameExists();
    }

    void UploadFrame::OnUploadClicked()
    {
        Enable( false );

        auto modelId = FindSelectedModelId();
        if ( modelId != MODEL_NOT_FOUND ) {
            printerService_->removeModel(
                    selectedPrinter_.ToStdString(), modelId,
                    [this, printer = selectedPrinter_, modelName = enteredModelName_,
                            modelGroup = selectedModelGroup_, deleteFile = deleteFileCheckbox_->GetValue()] {
                        PerformUpload( printer, modelName, modelGroup, deleteFile );
                    } );
        }
        else {
            PerformUpload( selectedPrinter_, enteredModelName_, selectedModelGroup_, deleteFileCheckbox_->GetValue() );
        }
    }

    void UploadFrame::OnToolBarExplore()
    {
        ExplorerFrame* frame = new ExplorerFrame( this, printerService_ );
        frame->Show( true );
    }

    void UploadFrame::OnConnectionLost( std::error_code ec )
    {
        wxMessageBox( ec.message(), "Error", wxOK | wxICON_ERROR, this );
        Close();
    }

    void UploadFrame::OnPrintersChanged( std::vector< gcu::repetier::Printer >&& printers )
    {
        printerChoice_->Clear();

        int selected = 0;
        for ( auto&& printer : printers ) {
            auto ptr = new wxClientPtr< gcu::repetier::Printer >( std::move( printer ) );
            int index = printerChoice_->Append( ptr->GetValue().name(), ptr );
            if ( ptr->GetValue().slug() == selectedPrinter_ ) {
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

    void UploadFrame::OnModelGroupsChanged(
            std::string const& printer, std::vector< gcu::repetier::ModelGroup >&& modelGroups )
    {
        if ( printer == selectedPrinter_ ) {
            modelGroupChoice_->Clear();

            int selected = 0;
            for ( auto&& modelGroup : modelGroups ) {
                auto ptr = new wxClientPtr< gcu::repetier::ModelGroup >( std::move( modelGroup ) );
                int index = modelGroupChoice_->Append(
                        ptr->GetValue().defaultGroup() ? _( "Default" ) : ptr->GetValue().name(), ptr );
                if ( ptr->GetValue().name() == selectedModelGroup_ ) {
                    selected = index;
                }
            }
            modelGroupChoice_->Enable( true );
            modelGroupChoice_->Select( selected );
            OnModelGroupSelected();
            addModelGroupButton_->Enable( true );
        }
    }

    void UploadFrame::OnModelsChanged( std::string const& printer, std::vector< gcu::repetier::Model >&& models )
    {
        if ( printer == selectedPrinter_ ) {
            models_ = std::move( models );
            CheckModelNameExists();
        }
    }

} // namespace gct
