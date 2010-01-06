/////////////////////////////////////////////////////////////////////////////
// Name:        generic_list_sample.h
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     08/09/2009 09:06:44
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _GENERIC_LIST_SAMPLE_H_
#define _GENERIC_LIST_SAMPLE_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/notebook.h"
#include "wx/listctrl.h"
#include "wx/grid.h"
////@end includes

#include <litwindow/wx/list_mediator.hpp>

using litwindow::wx::VirtualListCtrl;

/*!
 * Forward declarations
 */

////@begin forward declarations
class VirtualListCtrl;
class wxGrid;
class wxDataViewCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_GENERIC_LIST_SAMPLE 10000
#define ID_NOTEBOOK 10006
#define ID_PANEL_GENERIC 10008
#define ID_VIRTUALLISTCTRL_GENERIC 10009
#define ID_LISTBOX_GENERIC 10010
#define ID_CHOICE_GENERIC 10011
#define ID_GRID_GENERIC 10012
#define ID_DATAVIEWCTRL_GENERIC 10013
#define ID_PANEL 10007
#define ID_LISTCTRL 10003
#define ID_LISTBOX 10001
#define ID_CHOICE 10002
#define ID_GRID 10004
#define ID_DATAVIEW 10005
#define SYMBOL_GENERIC_LIST_SAMPLE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_GENERIC_LIST_SAMPLE_TITLE _("generic_list_sample")
#define SYMBOL_GENERIC_LIST_SAMPLE_IDNAME ID_GENERIC_LIST_SAMPLE
#define SYMBOL_GENERIC_LIST_SAMPLE_SIZE wxSize(400, 400)
#define SYMBOL_GENERIC_LIST_SAMPLE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * generic_list_sample class declaration
 */

class generic_list_sample: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( generic_list_sample )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    generic_list_sample();
    generic_list_sample( wxWindow* parent, wxWindowID id = SYMBOL_GENERIC_LIST_SAMPLE_IDNAME, const wxString& caption = SYMBOL_GENERIC_LIST_SAMPLE_TITLE, const wxPoint& pos = SYMBOL_GENERIC_LIST_SAMPLE_POSITION, const wxSize& size = SYMBOL_GENERIC_LIST_SAMPLE_SIZE, long style = SYMBOL_GENERIC_LIST_SAMPLE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_GENERIC_LIST_SAMPLE_IDNAME, const wxString& caption = SYMBOL_GENERIC_LIST_SAMPLE_TITLE, const wxPoint& pos = SYMBOL_GENERIC_LIST_SAMPLE_POSITION, const wxSize& size = SYMBOL_GENERIC_LIST_SAMPLE_SIZE, long style = SYMBOL_GENERIC_LIST_SAMPLE_STYLE );

    /// Destructor
    ~generic_list_sample();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin generic_list_sample event handler declarations

////@end generic_list_sample event handler declarations

////@begin generic_list_sample member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end generic_list_sample member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    void setup_lists();

    void setup_sample_listctrl( );
    void setup_sample_list_adapter();
    void setup_sample_columns_adapter();
	void setup_template_lists();
	void setup_generic_lists();
	////@begin generic_list_sample member variables
    VirtualListCtrl* m_listctrl_generic;
    wxListBox* m_listbox_generic;
    wxChoice* m_choice_generic;
    wxGrid* m_grid_generic;
    wxDataViewCtrl* m_dataview_generic;
    VirtualListCtrl* m_listctrl;
    wxListBox* m_listbox;
    wxChoice* m_choice;
    wxGrid* m_grid;
    wxDataViewCtrl* m_dataview;
////@end generic_list_sample member variables
};

#endif
    // _GENERIC_LIST_SAMPLE_H_
