/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: rssmainframe.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
*/
#include "stdwx.h"
#include "data.h"
/////////////////////////////////////////////////////////////////////////////
// Name:        rssmainframe.cpp
// Purpose:     
// Author:      Hajo Kirchhoff - Lit Window Productions
// Modified by: 
// Created:     08/10/04 10:13:41
// RCS-ID:      
// Copyright:   Copyright 2004 - Hajo Kirchhoff - Lit Window Productions
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "rssmainframe.h"
#endif

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

#include "rssmainframe.h"
using namespace litwindow;

#define new DEBUG_NEW
////@begin XPM images
////@end XPM images

/*!
* RssMainFrame type definition
*/

IMPLEMENT_CLASS( RssMainFrame, wxFrame )

/*!
* RssMainFrame event table definition
*/

BEGIN_EVENT_TABLE( RssMainFrame, wxFrame )

////@begin RssMainFrame event table entries
EVT_MENU( wxID_EXIT, RssMainFrame::OnExitClick )

////@end RssMainFrame event table entries

END_EVENT_TABLE()

/*!
* RssMainFrame constructors
*/

RssMainFrame::RssMainFrame( )
{
}

RssMainFrame::RssMainFrame( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create( parent, id, caption, pos, size, style );
}

/*!
* RssMainFrame creator
*/

BEGIN_RULES(g_rules)
    RULE(wxT("m_channelsList.Items"), make_expr<accessor>("m_channels"))         // listbox uses elements from m_channels
    RULE(wxT("m_channelsList.Column"), make_const(tstring(wxT("m_title"))))            // listbox displays "Channel::m_title"
    RULE(wxT("ID_FRAME.Title"), make_const<wxString>(wxT("Headline: "))+make_expr<wxString>("m_channelsList.Current.m_title"))
    // the next rule connects the headlines listbox with the m_headlines
    // member of the currently selected channel
    RULE(wxT("m_headlinesList.Items"), make_expr<accessor>("m_channelsList.Current.m_headlines"))
    // the following rule connects the content of the html window m_newsItem with
    // the body of the currently selected headline
    RULE(wxT("m_newsItem.Page"), make_expr<wxString>("m_headlinesList.Current.m_body"))
END_RULES()

bool RssMainFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    ////@begin RssMainFrame member initialisation
    ////@end RssMainFrame member initialisation

    ////@begin RssMainFrame creation
    SetParent(parent);
    CreateControls();
    Centre();
    ////@end RssMainFrame creation
    m_rapidUI.AddWindow(this);                  // Add the main frame to RapidUI
    m_rapidUI.AddData(make_accessor(g_data));   // Add the list of channels
    m_rapidUI.AddRules(g_rules);                // Add rules
    m_rapidUI.Start();                          // Start the RapidUI mediator mechanism
    return TRUE;
}

/*!
* Control creation for RssMainFrame
*/

void RssMainFrame::CreateControls()
{    
    ////@begin RssMainFrame content construction

    wxXmlResource::Get()->LoadFrame(this, GetParent(), _T("ID_FRAME"));
    ////@end RssMainFrame content construction

    // Create custom windows not generated automatically here.

    ////@begin RssMainFrame content initialisation

    ////@end RssMainFrame content initialisation
}

/*!
* Should we show tooltips?
*/

bool RssMainFrame::ShowToolTips()
{
    return TRUE;
}
/*!
* wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
*/

void RssMainFrame::OnExitClick( wxCommandEvent& event )
{
    // Insert custom code here
    Close();
    event.Skip();
}


