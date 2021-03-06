///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx_generated.h"

///////////////////////////////////////////////////////////////////////////
using namespace gct;

UploadFrameBase::UploadFrameBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 5, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	gcodeFileLabel_ = new wxStaticText( this, wxID_ANY, wxT("G-Code file:"), wxDefaultPosition, wxDefaultSize, 0 );
	gcodeFileLabel_->Wrap( -1 );
	fgSizer1->Add( gcodeFileLabel_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	gcodeFileText_ = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	fgSizer1->Add( gcodeFileText_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	
	deleteFileCheckbox_ = new wxCheckBox( this, wxID_ANY, wxT("Delete file after uploading?"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( deleteFileCheckbox_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	printerLabel_ = new wxStaticText( this, wxID_ANY, wxT("Printer:"), wxDefaultPosition, wxDefaultSize, 0 );
	printerLabel_->Wrap( -1 );
	fgSizer1->Add( printerLabel_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString printerChoice_Choices;
	printerChoice_ = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, printerChoice_Choices, 0 );
	printerChoice_->SetSelection( 0 );
	printerChoice_->Enable( false );
	
	fgSizer1->Add( printerChoice_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	modelGroupLabel_ = new wxStaticText( this, wxID_ANY, wxT("Model group:"), wxDefaultPosition, wxDefaultSize, 0 );
	modelGroupLabel_->Wrap( -1 );
	fgSizer1->Add( modelGroupLabel_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxArrayString modelGroupChoice_Choices;
	modelGroupChoice_ = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, modelGroupChoice_Choices, 0 );
	modelGroupChoice_->SetSelection( 0 );
	modelGroupChoice_->Enable( false );
	
	fgSizer2->Add( modelGroupChoice_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	addModelGroupButton_ = new wxButton( this, wxID_ANY, wxT("+"), wxDefaultPosition, wxDefaultSize, 0 );
	addModelGroupButton_->Enable( false );
	
	fgSizer2->Add( addModelGroupButton_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	fgSizer1->Add( fgSizer2, 1, wxEXPAND, 5 );
	
	modelNameLabel_ = new wxStaticText( this, wxID_ANY, wxT("Model name:"), wxDefaultPosition, wxDefaultSize, 0 );
	modelNameLabel_->Wrap( -1 );
	fgSizer1->Add( modelNameLabel_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	modelNameText_ = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( modelNameText_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	
	bSizer2->Add( fgSizer1, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer5->AddGrowableCol( 0 );
	fgSizer5->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	infoLabel_ = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	infoLabel_->Wrap( -1 );
	infoLabel_->SetForegroundColour( wxColour( 128, 0, 0 ) );
	
	fgSizer5->Add( infoLabel_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	uploadButton_ = new wxButton( this, wxID_ANY, wxT("Upload"), wxDefaultPosition, wxDefaultSize, 0 );
	uploadButton_->Enable( false );
	
	fgSizer5->Add( uploadButton_, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	
	bSizer2->Add( fgSizer5, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer2 );
	this->Layout();
	toolBar_ = this->CreateToolBar( wxTB_HORIZONTAL|wxTB_HORZ_TEXT|wxTB_NOICONS, wxID_ANY ); 
	exploreTool_ = toolBar_->AddTool( gctID_EXPLORE, wxT("Explore..."), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL ); 
	
	toolBar_->Realize(); 
	
	
	this->Centre( wxBOTH );
}

UploadFrameBase::~UploadFrameBase()
{
}

ExplorerFrameBase::ExplorerFrameBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	
	wxGridBagSizer* gbSizer1;
	gbSizer1 = new wxGridBagSizer( 0, 0 );
	gbSizer1->SetFlexibleDirection( wxBOTH );
	gbSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	modelGroupLabel_ = new wxStaticText( this, wxID_ANY, wxT("Model group:"), wxDefaultPosition, wxDefaultSize, 0 );
	modelGroupLabel_->Wrap( -1 );
	gbSizer1->Add( modelGroupLabel_, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString modelGroupChoice_Choices;
	modelGroupChoice_ = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, modelGroupChoice_Choices, 0 );
	modelGroupChoice_->SetSelection( 0 );
	modelGroupChoice_->Enable( false );
	
	gbSizer1->Add( modelGroupChoice_, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	modelsListCtrl_ = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	modelsListCtrl_->Enable( false );
	
	gbSizer1->Add( modelsListCtrl_, wxGBPosition( 2, 0 ), wxGBSpan( 1, 2 ), wxALL|wxEXPAND, 5 );
	
	printerLabel_ = new wxStaticText( this, wxID_ANY, wxT("Printer:"), wxDefaultPosition, wxDefaultSize, 0 );
	printerLabel_->Wrap( -1 );
	gbSizer1->Add( printerLabel_, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString printerChoice_Choices;
	printerChoice_ = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, printerChoice_Choices, 0 );
	printerChoice_->SetSelection( 0 );
	printerChoice_->Enable( false );
	
	gbSizer1->Add( printerChoice_, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	
	gbSizer1->AddGrowableCol( 1 );
	gbSizer1->AddGrowableRow( 2 );
	
	this->SetSizer( gbSizer1 );
	this->Layout();
	toolBar_ = this->CreateToolBar( wxTB_HORIZONTAL|wxTB_HORZ_TEXT|wxTB_NOICONS, wxID_ANY ); 
	removeModelsTool_ = toolBar_->AddTool( gctID_REMOVE_MODELS, wxT("Remove models"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL ); 
	
	newGroupTool_ = toolBar_->AddTool( gctID_NEW_GROUP, wxT("New group"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL ); 
	
	removeGroupTool_ = toolBar_->AddTool( gctID_REMOVE_GROUP, wxT("Remove group"), wxNullBitmap, wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString, NULL ); 
	
	toolBar_->Realize(); 
	
	
	this->Centre( wxBOTH );
}

ExplorerFrameBase::~ExplorerFrameBase()
{
}
