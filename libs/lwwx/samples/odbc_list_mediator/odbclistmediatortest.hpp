/////////////////////////////////////////////////////////////////////////////
// Name:        odbclistmediatortest.hpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     09/10/2019 16:36:09
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _ODBCLISTMEDIATORTEST_H_
#define _ODBCLISTMEDIATORTEST_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/xrc/xmlres.h"
#include "wx/frame.h"
#include "wx/listctrl.h"
////@end includes
#include <vector>
#include "litwindow/wx/list_mediator.hpp"
#include "litwindow/ui/odbc_list_mediator.hpp"
/*!
 * Forward declarations
 */

using litwindow::wx::VirtualListCtrl;
using namespace litwindow;

////@begin forward declarations
class VirtualListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ODBCLISTMEDIATORTEST 10000
#define SYMBOL_ODBCLISTMEDIATORTEST_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_ODBCLISTMEDIATORTEST_TITLE _("ODBC list mediator test")
#define SYMBOL_ODBCLISTMEDIATORTEST_IDNAME ID_ODBCLISTMEDIATORTEST
#define SYMBOL_ODBCLISTMEDIATORTEST_SIZE wxSize(400, 300)
#define SYMBOL_ODBCLISTMEDIATORTEST_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * ODBClistmediatortest class declaration
 */

class ODBClistmediatortest: public wxFrame
{    
    DECLARE_CLASS( ODBClistmediatortest )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ODBClistmediatortest();
    ODBClistmediatortest( wxWindow* parent, wxWindowID id = SYMBOL_ODBCLISTMEDIATORTEST_IDNAME, const wxString& caption = SYMBOL_ODBCLISTMEDIATORTEST_TITLE, const wxPoint& pos = SYMBOL_ODBCLISTMEDIATORTEST_POSITION, const wxSize& size = SYMBOL_ODBCLISTMEDIATORTEST_SIZE, long style = SYMBOL_ODBCLISTMEDIATORTEST_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ODBCLISTMEDIATORTEST_IDNAME, const wxString& caption = SYMBOL_ODBCLISTMEDIATORTEST_TITLE, const wxPoint& pos = SYMBOL_ODBCLISTMEDIATORTEST_POSITION, const wxSize& size = SYMBOL_ODBCLISTMEDIATORTEST_SIZE, long style = SYMBOL_ODBCLISTMEDIATORTEST_STYLE );

    /// Destructor
    ~ODBClistmediatortest();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin ODBClistmediatortest event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_REFRESH_BUTTON
    void OnRefreshButtonClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SQL_EXECUTE
    void OnSqlExecuteClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
    void OnExitClick( wxCommandEvent& event );

////@end ODBClistmediatortest event handler declarations

////@begin ODBClistmediatortest member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ODBClistmediatortest member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin ODBClistmediatortest member variables
    VirtualListCtrl* m_listctrl;
    wxTextCtrl* m_sql_statement;
    VirtualListCtrl* m_odbc_list;
////@end ODBClistmediatortest member variables
////@
	struct three_columns
	{
		std::wstring one;
		int two;
		double three;
	};
	using test_container=std::vector<three_columns>;

	litwindow::wx::list_mediator<test_container, VirtualListCtrl> m_static_mediator;
	using odbc_mediator = litwindow::wx::list_mediator<ui::odbc_container, VirtualListCtrl>;
	odbc_mediator m_odbc_mediator;
	test_container m_static_container;
	ui::odbc_container m_odbc_container;
};

#endif
    // _ODBCLISTMEDIATORTEST_H_
