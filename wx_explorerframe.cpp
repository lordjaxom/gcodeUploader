#include <iomanip>
#include <utility>

#include <wx/msgdlg.h>

#include "printer_service.hpp"
#include "utility.hpp"
#include <wx/msw/winundef.h>

#include "wx_clientptr.hpp"
#include "wx_explorerframe.hpp"

namespace gct {

    static constexpr std::size_t kilobyte = 1024;
    static constexpr std::size_t megabyte = kilobyte * 1024;

    static std::string formatFileSize( std::size_t size )
    {
        if ( size >= megabyte ) {
            return gcu::util::str( std::fixed, std::setprecision( 2 ), (double) size / megabyte, " MiB" );
        }
        if ( size >= kilobyte ) {
            return gcu::util::str( std::fixed, std::setprecision( 2 ), (double) size / kilobyte, " kiB" );
        }
        return gcu::util::str( size, " B" );
    }

    ExplorerFrame::ExplorerFrame( std::shared_ptr< gcu::PrinterService > printerService )
            : ExplorerFrameBase( nullptr )
            , printerService_( std::move( printerService ) )
    {
        modelsListCtrl_->AppendColumn( _( "Model" ), wxLIST_FORMAT_LEFT, 200 );
        modelsListCtrl_->AppendColumn( _( "Uploaded" ), wxLIST_FORMAT_LEFT, 100 );
        modelsListCtrl_->AppendColumn( _( "Size" ), wxLIST_FORMAT_LEFT, 100 );
        modelsListCtrl_->AppendColumn( _( "Lines" ), wxLIST_FORMAT_LEFT, 50 );
        modelsListCtrl_->AppendColumn( _( "Layers" ), wxLIST_FORMAT_LEFT, 50 );

        printerChoice_->Bind( wxEVT_CHOICE, [this]( auto& ) { this->OnPrinterSelected(); } );
        modelGroupChoice_->Bind( wxEVT_CHOICE, [this]( auto& ) { this->OnModelGroupSelected(); } );
        modelsListCtrl_->Bind( wxEVT_LIST_ITEM_SELECTED, [this]( auto& ) { this->OnModelsListItemSelected(); } );
        modelsListCtrl_->Bind( wxEVT_CONTEXT_MENU, [this]( auto& ) { this->OnModelsListContextMenu(); } );
        toolBar_->Bind( wxEVT_TOOL, [this]( auto& ) { this->OnToolBarRemove(); }, gctID_REMOVE );

        OnModelsListItemSelected();

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

    void ExplorerFrame::OnPrinterSelected()
    {
        int selection = printerChoice_->GetSelection();
        if ( selection != wxNOT_FOUND ) {
            selectedPrinter_ = wxClientPtrCast< gcu::repetier::Printer >(
                    printerChoice_->GetClientObject( (unsigned) selection ) ).slug();
            std::cerr << "selected printer is " << selectedPrinter_ << "\n";
            printerService_->requestModelGroups( selectedPrinter_ );
        }
    }

    void ExplorerFrame::OnModelGroupSelected()
    {
        int selection = modelGroupChoice_->GetSelection();
        if ( selection != wxNOT_FOUND ) {
            selectedModelGroup_ = wxClientPtrCast< gcu::repetier::ModelGroup >(
                    modelGroupChoice_->GetClientObject( (unsigned) selection ) ).name();
            printerService_->requestModels( selectedPrinter_ );
        }
    }

    void ExplorerFrame::OnModelsListItemSelected()
    {
        selectedModels_.clear();

        long index = -1;
        while ( ( index = modelsListCtrl_->GetNextItem( index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) ) != -1 ) {
            selectedModels_.insert( models_.find( index )->second.id() );
        }

        bool selection = !selectedModels_.empty();
        toolBar_->EnableTool( gctID_REMOVE, selection );
    }

    void ExplorerFrame::OnModelsListContextMenu()
    {
    }

    void ExplorerFrame::OnToolBarRemove()
    {
        if ( wxMessageBox(
                gcu::util::str( "Really remove ", selectedModels_.size(), " models?" ), _( "Question" ),
                wxYES_NO | wxICON_QUESTION, this ) == wxYES ) {
            std::for_each( selectedModels_.begin(), selectedModels_.end(), [this]( auto id ) {
                printerService_->removeModel( selectedPrinter_, id );
            } );
        }
    }

    void ExplorerFrame::OnConnectionLost( std::error_code ec )
    {
        wxMessageBox( ec.message(), "Error", wxOK | wxICON_ERROR, this );
        Close();
    }

    void ExplorerFrame::OnPrintersChanged( std::vector< gcu::repetier::Printer >&& printers )
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

    void ExplorerFrame::OnModelGroupsChanged(
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
        }
    }

    void ExplorerFrame::OnModelsChanged( std::string const& printer, std::vector< gcu::repetier::Model >&& models )
    {
        if ( printer == selectedPrinter_ ) {
            modelsListCtrl_->DeleteAllItems();
            models_.clear();

            for ( auto&& model : models ) {
                if ( model.modelGroup() == selectedModelGroup_ ) {
                    long index = modelsListCtrl_->InsertItem( modelsListCtrl_->GetItemCount(), model.name() );
                    modelsListCtrl_->SetItem(
                            index, 1, gcu::util::str( std::put_time( std::localtime( &model.created() ), "%c" ) ) );
                    modelsListCtrl_->SetItem( index, 2, formatFileSize( model.length() ) );
                    modelsListCtrl_->SetItem( index, 3, std::to_string( model.id() ) );
                    if ( selectedModels_.find( model.id() ) != selectedModels_.end() ) {
                        modelsListCtrl_->SetItemState( index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
                    }
                    models_.emplace( index, std::move( model ) );
                }
            }
            modelsListCtrl_->Enable( true );
            OnModelsListItemSelected();
        }
    }

} // namespace gct