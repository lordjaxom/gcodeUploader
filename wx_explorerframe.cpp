#include <iomanip>
#include <utility>

#include <wx/msgdlg.h>
#include <wx/textdlg.h>

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

    static std::string formatDuration( std::chrono::microseconds duration )
    {
        std::ostringstream os;
        bool output = false;
        auto hours = std::chrono::duration_cast< std::chrono::hours >( duration );
        if ( hours.count() > 0 ) {
            os << hours.count() << 'h';
            output = true;
        }
        duration -= hours;
        auto minutes = std::chrono::duration_cast< std::chrono::minutes >( duration );
        if ( minutes.count() > 0 || output ) {
            if ( output ) os << ' ';
            os << minutes.count() << 'm';
            output = true;
        }
        duration -= minutes;
        auto seconds = std::chrono::duration_cast< std::chrono::seconds >( duration );
        if ( output ) os << ' ';
        os << seconds.count() << 's';
        return os.str();
    }

    ExplorerFrame::ExplorerFrame( wxWindow* parent, std::shared_ptr< gcu::PrinterService > printerService )
            : ExplorerFrameBase( parent )
            , printerService_( std::move( printerService ) )
    {
        modelsListCtrl_->AppendColumn( _( "Model" ), wxLIST_FORMAT_LEFT, 300 );
        modelsListCtrl_->AppendColumn( _( "Uploaded" ), wxLIST_FORMAT_LEFT, 150 );
        modelsListCtrl_->AppendColumn( _( "Size" ), wxLIST_FORMAT_LEFT, 100 );
        modelsListCtrl_->AppendColumn( _( "Lines" ), wxLIST_FORMAT_LEFT, 50 );
        modelsListCtrl_->AppendColumn( _( "Time" ), wxLIST_FORMAT_LEFT, 100 );
        modelsListCtrl_->AppendColumn( _( "Layers" ), wxLIST_FORMAT_LEFT, 50 );

        printerChoice_->Bind( wxEVT_CHOICE, [this]( auto& ) { this->OnPrinterSelected(); } );
        modelGroupChoice_->Bind( wxEVT_CHOICE, [this]( auto& ) { this->OnModelGroupSelected(); } );
        modelsListCtrl_->Bind( wxEVT_LIST_ITEM_SELECTED, [this]( auto& ) { this->OnModelsListItemSelected(); } );
        modelsListCtrl_->Bind( wxEVT_CONTEXT_MENU, [this]( auto& ) { this->OnModelsListContextMenu(); } );
        toolBar_->Bind( wxEVT_TOOL, [this]( auto& ) { this->OnToolBarRemoveModels(); }, gctID_REMOVE_MODELS );
        toolBar_->Bind( wxEVT_TOOL, [this]( auto& ) { this->OnToolBarNewGroup(); }, gctID_NEW_GROUP );
        toolBar_->Bind( wxEVT_TOOL, [this]( auto& ) { this->OnToolBarRemoveGroup(); }, gctID_REMOVE_GROUP );

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

    ExplorerFrame::ExplorerFrame( std::shared_ptr< gcu::PrinterService > printerService )
            : ExplorerFrame( nullptr, std::move( printerService ) )
    {
    }

    void ExplorerFrame::RefreshControlStates()
    {
        toolBar_->EnableTool( gctID_REMOVE_MODELS, !selectedModels_.empty() );
        toolBar_->EnableTool(
                gctID_REMOVE_GROUP,
                !selectedModelGroup_.empty() && !gcu::repetier::ModelGroup::defaultGroup( selectedModelGroup_ ) );
    }

    void ExplorerFrame::InvalidateModelGroup()
    {
        modelGroupChoice_->Clear();
        selectedModelGroup_ = {};
    }

    void ExplorerFrame::InvalidateModels()
    {
        modelsListCtrl_->DeleteAllItems();
        models_.clear();
        selectedModels_.clear();
    }

    void ExplorerFrame::OnPrinterSelected()
    {
        int selection = printerChoice_->GetSelection();
        if ( selection != wxNOT_FOUND ) {
            selectedPrinter_ = wxClientPtrCast< gcu::repetier::Printer >(
                    printerChoice_->GetClientObject( (unsigned) selection ) ).slug();

            InvalidateModelGroup();
            InvalidateModels();
            RefreshControlStates();

            printerService_->requestModelGroups( selectedPrinter_ );
        }
    }

    void ExplorerFrame::OnModelGroupSelected()
    {
        int selection = modelGroupChoice_->GetSelection();
        if ( selection != wxNOT_FOUND ) {
            selectedModelGroup_ = wxClientPtrCast< gcu::repetier::ModelGroup >(
                    modelGroupChoice_->GetClientObject( (unsigned) selection ) ).name();

            InvalidateModels();
            RefreshControlStates();

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

        RefreshControlStates();
    }

    void ExplorerFrame::OnModelsListContextMenu()
    {
    }

    void ExplorerFrame::OnToolBarRemoveModels()
    {
        if ( wxMessageBox(
                gcu::util::str( "Really remove ", selectedModels_.size(), " models?" ), _( "Question" ),
                wxYES_NO | wxICON_QUESTION, this ) == wxYES ) {
            std::for_each( selectedModels_.begin(), selectedModels_.end(), [this]( auto id ) {
                printerService_->removeModel( selectedPrinter_, id );
            } );
        }
    }

    void ExplorerFrame::OnToolBarNewGroup()
    {
        wxTextEntryDialog dialog( this, _( "Please enter the name of the new model group" ) );
        // TODO: input validation
        if ( dialog.ShowModal() == wxID_OK ) {
            InvalidateModelGroup();
            InvalidateModels();
            RefreshControlStates();

            selectedModelGroup_ = dialog.GetValue().ToStdString();
            printerService_->addModelGroup( selectedPrinter_, selectedModelGroup_ );
        }
    }

    void ExplorerFrame::OnToolBarRemoveGroup()
    {
        if ( !models_.empty() ) {
            if ( wxMessageBox(
                    gcu::util::str( "Really remove group containing ", models_.size(), " models?" ), _( "Question" ),
                    wxYES_NO | wxICON_QUESTION, this ) == wxNO ) {
                return;
            }
        }

        printerService_->delModelGroup( selectedPrinter_, selectedModelGroup_, true );
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
                    modelsListCtrl_->SetItem( index, 3, std::to_string( model.lines() ) );
                    modelsListCtrl_->SetItem( index, 4, formatDuration( model.printTime() ) );
                    modelsListCtrl_->SetItem( index, 5, std::to_string( model.layers() ) );
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