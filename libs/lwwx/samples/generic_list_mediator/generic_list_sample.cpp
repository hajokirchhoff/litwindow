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
#include <litwindow/wx/list_mediator.hpp>
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
typedef ui::stl_container_dataset_accessor<test_list_t, sample_columns_adapter > sample_dataset_adapter;

void generic_list_sample::setup_lists()
{
    // setup some example records
    g_sample.push_back(test_data(L"Thatcher", 90));
    g_sample.push_back(test_data(L"Obama", 48));
    g_sample.push_back(test_data(L"Merkel", 54));

    static sample_columns_adapter g_columns;
    boost::function<void(const test_data &t, tstring &c)> fnc;
    fnc=boost::bind(&litwindow::ui::to_string<int>, 
        boost::bind(&litwindow::ui::to_member<test_data, int>, _1, &test_data::age),
        _2
        );
    g_columns.add(L"Name", 120, &test_data::name)(L"Age", 80, fnc);

    static sample_dataset_adapter g_test_list_adapter;
    g_test_list_adapter.set_container(g_sample);
    g_test_list_adapter.set_columns_adapter(g_columns);

    static wx::wxListCtrl_list_adapter g_listctrl_adapter;
    g_listctrl_adapter.set_control(m_listctrl);

    static ui::basic_list_mediator<sample_dataset_adapter, wx::wxListCtrl_list_adapter> g_listctrl_mediator;
    g_listctrl_mediator.set_ui_adapter(g_listctrl_adapter);
    g_listctrl_mediator.set_dataset_adapter(g_test_list_adapter);
    g_listctrl_mediator.refresh();


    static wx::wxListBox_list_adapter g_listbox_adapter;
    g_listbox_adapter.set_control(m_listbox);

    static ui::basic_list_mediator<sample_dataset_adapter, wx::wxListBox_list_adapter> g_listbox_mediator;
    g_listbox_mediator.set_ui_adapter(g_listbox_adapter);
    g_listbox_mediator.set_dataset_adapter(g_test_list_adapter);
    g_listbox_mediator.refresh();


    static ui::basic_list_mediator<sample_dataset_adapter, wx::wxChoiceBox_list_adapter> g_choicebox_mediator;
    g_choicebox_mediator.set_ui(m_choice);
    g_choicebox_mediator.set_dataset(g_sample, g_columns);
    g_choicebox_mediator.refresh();

    static wx::wxDataViewCtrl_list_adapter g_dataviewctrl_adapter;
    g_dataviewctrl_adapter.set_control(m_dataview);

    static ui::basic_list_mediator<sample_dataset_adapter, wx::wxDataViewCtrl_list_adapter> g_dataview_mediator;
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

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    itemFlexGridSizer3->AddGrowableRow(1);
    itemFlexGridSizer3->AddGrowableRow(3);
    itemFlexGridSizer3->AddGrowableCol(0);
    itemFlexGridSizer3->AddGrowableCol(1);
    itemFlexGridSizer3->AddGrowableCol(2);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("wxListCtrl"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("wxListBox"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("wxChoice"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    m_listctrl = new VirtualListCtrl( itemDialog1, ID_LISTCTRL, wxDefaultPosition, wxSize(100, 100), wxLC_REPORT );
    itemFlexGridSizer3->Add(m_listctrl, 0, wxGROW|wxGROW|wxALL, 5);

    wxArrayString m_listboxStrings;
    m_listbox = new wxListBox( itemDialog1, ID_LISTBOX, wxDefaultPosition, wxDefaultSize, m_listboxStrings, wxLB_SINGLE );
    itemFlexGridSizer3->Add(m_listbox, 0, wxGROW|wxGROW|wxALL, 5);

    wxArrayString m_choiceStrings;
    m_choice = new wxChoice( itemDialog1, ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_choiceStrings, 0 );
    itemFlexGridSizer3->Add(m_choice, 0, wxGROW|wxALIGN_TOP|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _("wxGrid"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("wxDataViewCtrl"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    m_grid = new wxGrid( itemDialog1, ID_GRID, wxDefaultPosition, wxSize(200, 150), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    m_grid->SetDefaultColSize(50);
    m_grid->SetDefaultRowSize(25);
    m_grid->SetColLabelSize(25);
    m_grid->SetRowLabelSize(50);
    m_grid->CreateGrid(5, 5, wxGrid::wxGridSelectCells);
    itemFlexGridSizer3->Add(m_grid, 0, wxGROW|wxGROW|wxALL, 5);

    m_dataview = new wxDataViewCtrl( itemDialog1, ID_DATAVIEW, wxDefaultPosition, wxSize(200, 100), wxSIMPLE_BORDER );
    itemFlexGridSizer3->Add(m_dataview, 0, wxGROW|wxGROW|wxALL, 5);



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
