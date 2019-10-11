#include "stdafx.h"
/////////////////////////////////////////////////////////////////////////////
// Name:        odbclistmediatortest.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     09/10/2019 16:36:09
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "odbclistmediatortest.hpp"
#include "odbc_list_mediatorapp.hpp"

////@begin XPM images
////@end XPM images


/*
 * ODBClistmediatortest type definition
 */

IMPLEMENT_CLASS( ODBClistmediatortest, wxFrame )


/*
 * ODBClistmediatortest event table definition
 */

BEGIN_EVENT_TABLE( ODBClistmediatortest, wxFrame )

////@begin ODBClistmediatortest event table entries
    EVT_BUTTON( XRCID("ID_REFRESH_BUTTON"), ODBClistmediatortest::OnRefreshButtonClick )
    EVT_BUTTON( XRCID("ID_SQL_EXECUTE"), ODBClistmediatortest::OnSqlExecuteClick )
    EVT_MENU( XRCID("ID_SQL_EXECUTE"), ODBClistmediatortest::OnSqlExecuteClick )
    EVT_MENU( wxID_EXIT, ODBClistmediatortest::OnExitClick )
////@end ODBClistmediatortest event table entries

END_EVENT_TABLE()


/*
 * ODBClistmediatortest constructors
 */

ODBClistmediatortest::ODBClistmediatortest()
{
    Init();
}

ODBClistmediatortest::ODBClistmediatortest( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*
 * ODBClistmediatortest creator
 */

bool ODBClistmediatortest::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ODBClistmediatortest creation
    SetParent(parent);
    CreateControls();
    Centre();
////@end ODBClistmediatortest creation

	m_static_mediator.columns().
		add(L"one", 100, &three_columns::one)(L"two", 100, &three_columns::two)(L"three", -1, &three_columns::three);
	m_static_mediator.set(m_static_container).set(m_listctrl);

	for (int i = 0; i < 100; ++i) {
		m_static_container.emplace_back(three_columns{ boost::lexical_cast<std::wstring>(i), i, i + 0.5 });
	}
	m_static_mediator.refresh();


	odbc_mediator::columns_type::column_descriptor_type::text_renderer_type fnc;
	ui::odbc_column c("one");
	//fnc = boost::bind(&ui::odbc_column::operator(), c, _1, _2);
	m_odbc_mediator.columns().add(L"one", 100, c);
	m_odbc_mediator.columns().add(L"two", 100, c);
	m_odbc_mediator.set(m_odbc_list).set(m_odbc_container);
	m_odbc_mediator.refresh();

    return true;
}


/*
 * ODBClistmediatortest destructor
 */

ODBClistmediatortest::~ODBClistmediatortest()
{
////@begin ODBClistmediatortest destruction
////@end ODBClistmediatortest destruction
}


/*
 * Member initialisation
 */

void ODBClistmediatortest::Init()
{
////@begin ODBClistmediatortest member initialisation
    m_listctrl = NULL;
    m_sql_statement = NULL;
    m_odbc_list = NULL;
////@end ODBClistmediatortest member initialisation
}


/*
 * Control creation for ODBClistmediatortest
 */

void ODBClistmediatortest::CreateControls()
{    
////@begin ODBClistmediatortest content construction
    if (!wxXmlResource::Get()->LoadFrame(this, GetParent(), wxT("ID_ODBCLISTMEDIATORTEST")))
        wxLogError(wxT("Missing wxXmlResource::Get()->Load() in OnInit()?"));
    m_listctrl = XRCCTRL(*this, "ID_LISTCTRL", VirtualListCtrl);
    m_sql_statement = XRCCTRL(*this, "ID_SQL_STATEMENT", wxTextCtrl);
    m_odbc_list = XRCCTRL(*this, "ID_ODBC_LIST", VirtualListCtrl);
////@end ODBClistmediatortest content construction

    // Create custom windows not generated automatically here.
////@begin ODBClistmediatortest content initialisation
////@end ODBClistmediatortest content initialisation
}


/*
 * Should we show tooltips?
 */

bool ODBClistmediatortest::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap ODBClistmediatortest::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ODBClistmediatortest bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end ODBClistmediatortest bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon ODBClistmediatortest::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ODBClistmediatortest icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ODBClistmediatortest icon retrieval
}


/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
 */

void ODBClistmediatortest::OnExitClick( wxCommandEvent& event )
{
	wxGetApp().Exit();
}




/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REFRESH_BUTTON
 */

void ODBClistmediatortest::OnRefreshButtonClick( wxCommandEvent& event )
{
	m_odbc_mediator.refresh();
	m_static_mediator.refresh();
}


/*
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SQL_EXECUTE
 */

void ODBClistmediatortest::OnSqlExecuteClick( wxCommandEvent& event )
{
	odbc::sqlreturn rc;
	m_odbc_container.set_connection(*odbc::default_connection());
	rc = m_odbc_container.set_statement(m_sql_statement->GetValue().ToStdWstring());
	rc = m_odbc_container.execute();
	SQLLEN rowcount;
	rc = m_odbc_container.get_row_count(rowcount);
	int count = 0;
	while (m_odbc_container.fetch()) {
		++count;
	}
	rc = m_odbc_container.get_row_count(rowcount);
}

