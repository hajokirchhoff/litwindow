/////////////////////////////////////////////////////////////////////////////
// Name:        extended_error_dialog.cpp
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     26/11/2008 09:49:00
// RCS-ID:      
// Copyright:   Copyright Hajo Kirchhoff IT-Consulting, Lit Window Productions
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#include "stdwx.h"
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "litwindow/wx/extended_error_dialog.h"
#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/phoenix1.hpp>

////@begin XPM images
////@end XPM images

using namespace litwindow::wx;

/*!
* extended_error_dialog type definition
*/

IMPLEMENT_DYNAMIC_CLASS( extended_error_dialog, wxDialog )


/*!
* extended_error_dialog event table definition
*/

BEGIN_EVENT_TABLE( extended_error_dialog, wxDialog )

////@begin extended_error_dialog event table entries
////@end extended_error_dialog event table entries

END_EVENT_TABLE()

namespace litwindow { namespace wx {

	/*!
	* extended_error_dialog constructors
	*/

	extended_error_dialog::extended_error_dialog()
	{
		Init();
	}

	extended_error_dialog::extended_error_dialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
	{
		Init();
		Create(parent, id, caption, pos, size, style);
	}


	/*!
	* extended_error_dialog creator
	*/

	bool extended_error_dialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
	{
		////@begin extended_error_dialog creation
		SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
		wxDialog::Create( parent, id, caption, pos, size, style );

		CreateControls();
		if (GetSizer())
		{
			GetSizer()->SetSizeHints(this);
		}
		Centre();
		////@end extended_error_dialog creation
		return true;
	}


	/*!
	* extended_error_dialog destructor
	*/

	extended_error_dialog::~extended_error_dialog()
	{
		////@begin extended_error_dialog destruction
		////@end extended_error_dialog destruction
	}


	/*!
	* Member initialisation
	*/

	void extended_error_dialog::Init()
	{
		////@begin extended_error_dialog member initialisation
		m_message = NULL;
		m_detail = NULL;
		////@end extended_error_dialog member initialisation
	}


	/*!
	* Control creation for extended_error_dialog
	*/

	void extended_error_dialog::CreateControls()
	{    
		////@begin extended_error_dialog content construction
		extended_error_dialog* itemDialog1 = this;

		wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
		itemDialog1->SetSizer(itemBoxSizer2);

		m_message = new wxHtmlWindow( itemDialog1, ID_MESSAGE, wxDefaultPosition, wxSize(300, 100), wxHW_SCROLLBAR_AUTO|wxNO_BORDER|wxHSCROLL|wxVSCROLL );
		itemBoxSizer2->Add(m_message, 1, wxGROW|wxALL, 5);

		wxCollapsiblePane* itemCollapsiblePane4 = new wxCollapsiblePane( itemDialog1, ID_COLLAPSIBLEPANE, _("More Info..."), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE );
		itemBoxSizer2->Add(itemCollapsiblePane4, 0, wxGROW, 5);

		wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
		itemCollapsiblePane4->GetPane()->SetSizer(itemBoxSizer5);

		m_detail = new wxHtmlWindow( itemCollapsiblePane4->GetPane(), ID_DETAIL, wxDefaultPosition, wxSize(-1, 150), wxHW_SCROLLBAR_AUTO|wxNO_BORDER|wxHSCROLL|wxVSCROLL );
		itemBoxSizer5->Add(m_detail, 1, wxGROW|wxALL, 5);

		wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
		itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

		wxButton* itemButton8 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
		itemBoxSizer7->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

		wxButton* itemButton9 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
		itemBoxSizer7->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

		wxButton* itemButton10 = new wxButton( itemDialog1, wxID_STOP, _("&Stop"), wxDefaultPosition, wxDefaultSize, 0 );
		itemBoxSizer7->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

		////@end extended_error_dialog content construction
	}


	/*!
	* Should we show tooltips?
	*/

	bool extended_error_dialog::ShowToolTips()
	{
		return true;
	}

	/*!
	* Get bitmap resources
	*/

	wxBitmap extended_error_dialog::GetBitmapResource( const wxString& name )
	{
		// Bitmap retrieval
		////@begin extended_error_dialog bitmap retrieval
		wxUnusedVar(name);
		return wxNullBitmap;
		////@end extended_error_dialog bitmap retrieval
	}

	/*!
	* Get icon resources
	*/

	wxIcon extended_error_dialog::GetIconResource( const wxString& name )
	{
		// Icon retrieval
		////@begin extended_error_dialog icon retrieval
		wxUnusedVar(name);
		return wxNullIcon;
		////@end extended_error_dialog icon retrieval
	}

	int extended_error_dialog::show_error_dialog( const wxString& msg, const wxString& detail )
	{
		extended_error_dialog dlg(0);
		dlg.SetDetails(detail);
		dlg.SetMessage(msg);
		int rc=dlg.ShowModal();
		return rc;
	}

    int extended_error_dialog::show_error_dialog( const extended_error<char> &e )
    {
        int rc=show_error_dialog(e.message(), e.details());
        e.was_shown();
        return rc;
    }

	void extended_error_dialog::SetMessage( const wxString &message )
	{
		m_message->SetPage(MakeHtml(message));
	}

	void extended_error_dialog::SetDetails( const wxString &details )
	{
		m_detail->SetPage(details);
	}

#pragma warning(disable: 4503)
	wxString extended_error_dialog::MakeHtml(const wxString &input)
	{
        using namespace boost::spirit::classic;
		using namespace phoenix;
		wxString rc;
		parse_info<const wchar_t*> info
			=parse(input.wc_str(), (
			!((alpha_p >> +digit_p >> ch_p(_T(':')))[var(rc)=wxString(_T("<b><span style=\"color: Blue\">"))+construct_<wxString>(arg1, arg2)+_T("</span></b>")])
			>> !(+(print_p - (ch_p(_T('.')) | eol_p | end_p | confix_p(_T('<'), ~ch_p(_T('>')), ch_p('>'))) ) >> (ch_p(_T('.')) | eol_p))[var(rc)=var(rc)+_T("<b>")+arg1+_T("</b>")]
		) );
		if (info.full==false)
			rc=input;
		return rc;
	}

} /*namespace wx*/ } /*namespace litwindow*/
