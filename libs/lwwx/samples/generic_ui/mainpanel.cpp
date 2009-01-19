/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: mainpanel.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdwx.h"
/////////////////////////////////////////////////////////////////////////////
// Name:        mainpanel.cpp
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     05/14/04 09:10:45
// RCS-ID:      
// Copyright:   Copyright 2004, Hajo Kirchhoff, Lit Window Productions
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "mainpanel.h"
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

#include "mainpanel.h"

////@begin XPM images
////@end XPM images

/*!
 * MainPanel type definition
 */

IMPLEMENT_CLASS( MainPanel, wxPanel )

/*!
 * MainPanel event table definition
 */

BEGIN_EVENT_TABLE( MainPanel, wxPanel )

////@begin MainPanel event table entries
////@end MainPanel event table entries

END_EVENT_TABLE()

/*!
 * MainPanel constructors
 */

MainPanel::MainPanel( )
{
}

MainPanel::MainPanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * MainPanel creator
 */

bool MainPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin MainPanel member initialisation
////@end MainPanel member initialisation

////@begin MainPanel creation
    SetParent(parent);
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
////@end MainPanel creation
    return TRUE;
}

/*!
 * Control creation for MainPanel
 */

void MainPanel::CreateControls()
{    
////@begin MainPanel content construction

    wxXmlResource::Get()->LoadPanel(this, GetParent(), _T("ID_DIALOG"));
////@end MainPanel content construction
}

/*!
 * Should we show tooltips?
 */

bool MainPanel::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap MainPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin MainPanel bitmap retrieval
    return wxNullBitmap;
////@end MainPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon MainPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin MainPanel icon retrieval
    return wxNullIcon;
////@end MainPanel icon retrieval
}
