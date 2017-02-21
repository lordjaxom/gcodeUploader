///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WX_GENERATED_H__
#define __WX_GENERATED_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

namespace gct
{
	
	///////////////////////////////////////////////////////////////////////////////
	/// Class UploadFrameBase
	///////////////////////////////////////////////////////////////////////////////
	class UploadFrameBase : public wxFrame 
	{
		private:
		
		protected:
			wxStaticText* gcodeFileLabel_;
			wxTextCtrl* gcodeFileText_;
			wxCheckBox* deleteFileCheckbox_;
			wxStaticText* printerLabel_;
			wxChoice* printerChoice_;
			wxStaticText* modelGroupLabel_;
			wxChoice* modelGroupChoice_;
			wxButton* addModelGroupButton_;
			wxStaticText* modelNameLabel_;
			wxTextCtrl* modelNameText_;
			wxStaticText* infoLabel_;
			wxButton* uploadButton_;
		
		public:
			
			UploadFrameBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Upload G-Code"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,266 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
			
			~UploadFrameBase();
		
	};
	
} // namespace gct

#endif //__WX_GENERATED_H__
