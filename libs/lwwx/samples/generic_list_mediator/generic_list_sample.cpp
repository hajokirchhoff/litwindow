#include "stdwx.h"
/////////////////////////////////////////////////////////////////////////////
// Name:        generic_list_sample.cpp
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     08/09/2009 09:06:44
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

#include <wx/dataview.h>
#include <litwindow/ui/list_mediator.hpp>
#include <litwindow/wx/list_mediator.hpp>
#include <litwindow/dataadapter.h>
#include "generic_list_sample.h"

using namespace litwindow;

////@begin XPM images
////@end XPM images

struct test_data
{
    wxString    name;
    int         age;
    test_data(const wxString &n, int a)
        :name(n), age(a)
    {
    }
};
typedef std::list<test_data> test_list_t;

test_list_t g_sample;

wxString age_formatter(const test_data &t)
{
    return wxString::Format(L"%d", t.age);
}


typedef ui::basic_column_descriptor<test_data> sample_column_descriptor;
typedef ui::basic_columns_adapter<sample_column_descriptor> sample_columns_adapter;
typedef ui::stl_container_dataset_accessor<test_list_t> sample_dataset_adapter;

static sample_columns_adapter g_columns;
static sample_dataset_adapter g_test_list_adapter;

void generic_list_sample::setup_lists()
{
	extern void compile_tests();
	compile_tests();
	setup_template_lists();
	setup_generic_lists();
}

void generic_list_sample::setup_template_lists()
{
    // setup some example records
    g_sample.push_back(test_data(L"Thatcher", 91));
    g_sample.push_back(test_data(L"Obama", 48));
    g_sample.push_back(test_data(L"Merkel", 54));
	g_sample.push_back(test_data(L"Melanchton", 54));
    setup_sample_columns_adapter();

    setup_sample_list_adapter();

    setup_sample_listctrl();

    //=======================================================================
    // setup listbox
    static wx::wxListBox_list_adapter g_listbox_adapter;
    g_listbox_adapter.set_control(m_listbox);

	static ui::basic_list_mediator<sample_dataset_adapter, wx::wxListBox_list_adapter, sample_columns_adapter> g_listbox_mediator;
    g_listbox_mediator.set_ui_adapter(g_listbox_adapter);
    g_listbox_mediator.set_dataset_adapter(g_test_list_adapter);
	g_listbox_mediator.set_columns_adapter(g_columns);
    g_listbox_mediator.refresh();

    //=======================================================================
    // setup choicebox
    static ui::basic_list_mediator<sample_dataset_adapter, wx::wxChoiceBox_list_adapter, sample_columns_adapter> g_choicebox_mediator;
    g_choicebox_mediator.set_ui(m_choice);
    g_choicebox_mediator.set_dataset(g_sample, g_columns);
    g_choicebox_mediator.refresh();

    //=======================================================================
    // setup dataviewctrl
    static wx::wxDataViewCtrl_list_adapter g_dataviewctrl_adapter;
    g_dataviewctrl_adapter.set_control(m_dataview);

    static ui::basic_list_mediator<sample_dataset_adapter, wx::wxDataViewCtrl_list_adapter, sample_columns_adapter> g_dataview_mediator;
    g_dataview_mediator.set_ui_adapter(g_dataviewctrl_adapter);
    g_dataview_mediator.set_dataset(g_sample, g_columns);
    g_dataview_mediator.refresh();

}

/*
 * generic_list_sample type definition
 */

IMPLEMENT_DYNAMIC_CLASS( generic_list_sample, wxDialog )


/*
 * generic_list_sample event table definition
 */

BEGIN_EVENT_TABLE( generic_list_sample, wxDialog )

////@begin generic_list_sample event table entries
////@end generic_list_sample event table entries

END_EVENT_TABLE()


/*
 * generic_list_sample constructors
 */

generic_list_sample::generic_list_sample()
{
    Init();
}

generic_list_sample::generic_list_sample( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * generic_list_sample creator
 */

bool generic_list_sample::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin generic_list_sample creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end generic_list_sample creation
    return true;
}


/*
 * generic_list_sample destructor
 */

generic_list_sample::~generic_list_sample()
{
////@begin generic_list_sample destruction
////@end generic_list_sample destruction
}


/*
 * Member initialisation
 */

void generic_list_sample::Init()
{
////@begin generic_list_sample member initialisation
    m_listctrl_generic = NULL;
    m_listbox_generic = NULL;
    m_choice_generic = NULL;
    m_grid_generic = NULL;
    m_dataview_generic = NULL;
    m_listctrl = NULL;
    m_listbox = NULL;
    m_choice = NULL;
    m_grid = NULL;
    m_dataview = NULL;
////@end generic_list_sample member initialisation
}


/*
 * Control creation for generic_list_sample
 */

void generic_list_sample::CreateControls()
{    
////@begin generic_list_sample content construction
    generic_list_sample* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxNotebook* itemNotebook3 = new wxNotebook( itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    wxPanel* itemPanel4 = new wxPanel( itemNotebook3, ID_PANEL_GENERIC, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
    itemFlexGridSizer5->AddGrowableRow(1);
    itemFlexGridSizer5->AddGrowableRow(3);
    itemFlexGridSizer5->AddGrowableCol(0);
    itemFlexGridSizer5->AddGrowableCol(1);
    itemFlexGridSizer5->AddGrowableCol(2);
    itemPanel4->SetSizer(itemFlexGridSizer5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel4, wxID_STATIC, _("wxListCtrl"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel4, wxID_STATIC, _("wxListBox"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel4, wxID_STATIC, _("wxChoice"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    m_listctrl_generic = new VirtualListCtrl( itemPanel4, ID_VIRTUALLISTCTRL_GENERIC, wxDefaultPosition, wxSize(100, 100), wxLC_REPORT );
    itemFlexGridSizer5->Add(m_listctrl_generic, 0, wxGROW|wxGROW|wxALL, 5);

    wxArrayString m_listbox_genericStrings;
    m_listbox_generic = new wxListBox( itemPanel4, ID_LISTBOX_GENERIC, wxDefaultPosition, wxDefaultSize, m_listbox_genericStrings, wxLB_SINGLE );
    itemFlexGridSizer5->Add(m_listbox_generic, 0, wxGROW|wxGROW|wxALL, 5);

    wxArrayString m_choice_genericStrings;
    m_choice_generic = new wxChoice( itemPanel4, ID_CHOICE_GENERIC, wxDefaultPosition, wxDefaultSize, m_choice_genericStrings, 0 );
    itemFlexGridSizer5->Add(m_choice_generic, 0, wxGROW|wxALIGN_TOP|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel4, wxID_STATIC, _("wxGrid"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel4, wxID_STATIC, _("wxDataViewCtrl"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel4, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText14, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    m_grid_generic = new wxGrid( itemPanel4, ID_GRID_GENERIC, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    m_grid_generic->SetDefaultColSize(50);
    m_grid_generic->SetDefaultRowSize(25);
    m_grid_generic->SetColLabelSize(25);
    m_grid_generic->SetRowLabelSize(50);
    m_grid_generic->CreateGrid(5, 5, wxGrid::wxGridSelectCells);
    itemFlexGridSizer5->Add(m_grid_generic, 0, wxGROW|wxGROW|wxALL, 5);

    m_dataview_generic = new wxDataViewCtrl( itemPanel4, ID_DATAVIEWCTRL_GENERIC, wxDefaultPosition, wxSize(200, 100), wxSIMPLE_BORDER );
    itemFlexGridSizer5->Add(m_dataview_generic, 0, wxGROW|wxGROW|wxALL, 5);

    itemNotebook3->AddPage(itemPanel4, _("Generic versions"));

    wxPanel* itemPanel18 = new wxPanel( itemNotebook3, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    wxFlexGridSizer* itemFlexGridSizer19 = new wxFlexGridSizer(0, 3, 0, 0);
    itemFlexGridSizer19->AddGrowableRow(1);
    itemFlexGridSizer19->AddGrowableRow(3);
    itemFlexGridSizer19->AddGrowableCol(0);
    itemFlexGridSizer19->AddGrowableCol(1);
    itemFlexGridSizer19->AddGrowableCol(2);
    itemPanel18->SetSizer(itemFlexGridSizer19);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel18, wxID_STATIC, _("wxListCtrl"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText20, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText21 = new wxStaticText( itemPanel18, wxID_STATIC, _("wxListBox"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText21, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText22 = new wxStaticText( itemPanel18, wxID_STATIC, _("wxChoice"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText22, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    m_listctrl = new VirtualListCtrl( itemPanel18, ID_LISTCTRL, wxDefaultPosition, wxSize(100, 100), wxLC_REPORT );
    itemFlexGridSizer19->Add(m_listctrl, 0, wxGROW|wxGROW|wxALL, 5);

    wxArrayString m_listboxStrings;
    m_listbox = new wxListBox( itemPanel18, ID_LISTBOX, wxDefaultPosition, wxDefaultSize, m_listboxStrings, wxLB_SINGLE );
    itemFlexGridSizer19->Add(m_listbox, 0, wxGROW|wxGROW|wxALL, 5);

    wxArrayString m_choiceStrings;
    m_choice = new wxChoice( itemPanel18, ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_choiceStrings, 0 );
    itemFlexGridSizer19->Add(m_choice, 0, wxGROW|wxALIGN_TOP|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText( itemPanel18, wxID_STATIC, _("wxGrid"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText26, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText27 = new wxStaticText( itemPanel18, wxID_STATIC, _("wxDataViewCtrl"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText27, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText28 = new wxStaticText( itemPanel18, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer19->Add(itemStaticText28, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    m_grid = new wxGrid( itemPanel18, ID_GRID, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    m_grid->SetDefaultColSize(50);
    m_grid->SetDefaultRowSize(25);
    m_grid->SetColLabelSize(25);
    m_grid->SetRowLabelSize(50);
    m_grid->CreateGrid(5, 5, wxGrid::wxGridSelectCells);
    itemFlexGridSizer19->Add(m_grid, 0, wxGROW|wxGROW|wxALL, 5);

    m_dataview = new wxDataViewCtrl( itemPanel18, ID_DATAVIEW, wxDefaultPosition, wxSize(200, 100), wxSIMPLE_BORDER );
    itemFlexGridSizer19->Add(m_dataview, 0, wxGROW|wxGROW|wxALL, 5);

    itemNotebook3->AddPage(itemPanel18, _("Template based"));

    itemBoxSizer2->Add(itemNotebook3, 1, wxGROW|wxALL, 5);

////@end generic_list_sample content construction
    setup_lists();
}


/*
 * Should we show tooltips?
 */

bool generic_list_sample::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap generic_list_sample::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin generic_list_sample bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end generic_list_sample bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon generic_list_sample::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin generic_list_sample icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end generic_list_sample icon retrieval
}

void generic_list_sample::setup_sample_columns_adapter()
{
    boost::function<void(const test_data &t, tstring &c)> fnc;
    fnc=boost::bind(&litwindow::ui::to_string<int>, 
        boost::bind(&litwindow::ui::to_member<test_data, int>, _1, &test_data::age),
        _2
        );
    g_columns.add(L"Name", 120, &test_data::name)(L"Age", 80, fnc);
}

void generic_list_sample::setup_sample_list_adapter()
{
    g_test_list_adapter.set_container(g_sample);
    //g_test_list_adapter.set_columns_adapter(g_columns);
}

void generic_list_sample::setup_sample_listctrl( )
{
    static wx::wxListCtrl_list_adapter g_listctrl_adapter;
    g_listctrl_adapter.set_control(m_listctrl);

    static ui::basic_list_mediator<sample_dataset_adapter, wx::wxListCtrl_list_adapter, sample_columns_adapter> g_listctrl_mediator;
    g_listctrl_mediator.set_ui_adapter(g_listctrl_adapter);
    g_listctrl_mediator.set_dataset_adapter(g_test_list_adapter);
	g_listctrl_mediator.set_columns_adapter(g_columns);
    g_listctrl_mediator.refresh();
}

void generic_list_sample::setup_generic_lists()
{
	using litwindow::wx::uicontrol_policies;
	static ui::list_mediator<test_list_t, VirtualListCtrl> g_generic_mediator;
	g_generic_mediator.set_ui(m_listctrl_generic);
	g_generic_mediator.set_container(g_sample);
	ui::list_mediator<test_list_t, VirtualListCtrl>::columns_type &ca(g_generic_mediator.columns());
	ca.add(L"Name", 120, &test_data::name)(L"TestColumn", 140, &test_data::age);
	g_generic_mediator.refresh();

	static ui::list_mediator<test_list_t, wxListBox> g_listbox_mediator(g_sample, m_listbox_generic);
	g_listbox_mediator.columns().add(L"Name", 120, &test_data::name);
	g_listbox_mediator.refresh();

	static ui::list_mediator<test_list_t, wxChoice> g_combobox_mediator(g_sample, m_choice_generic);
	g_combobox_mediator.columns(g_listbox_mediator.columns());
	g_combobox_mediator.refresh();

	static ui::list_mediator<test_list_t, wxDataViewCtrl> g_dataviewctrl_mediator(g_sample, m_dataview_generic);
	g_dataviewctrl_mediator.columns(g_generic_mediator.columns());
	g_dataviewctrl_mediator.refresh();
}
