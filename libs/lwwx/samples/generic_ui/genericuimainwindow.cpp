/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: genericuimainwindow.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdwx.h"
/////////////////////////////////////////////////////////////////////////////
// Name:        genericuimainwindow.cpp
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     05/14/04 09:09:51
// RCS-ID:      
// Copyright:   Copyright 2004, Hajo Kirchhoff, Lit Window Productions
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "genericuimainwindow.h"
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

#include "genericuimainwindow.h"

////@begin XPM images
////@end XPM images

/*!
 * GenericUIMainWindow type definition
 */

IMPLEMENT_CLASS( GenericUIMainWindow, wxFrame )

/*!
 * GenericUIMainWindow event table definition
 */

BEGIN_EVENT_TABLE( GenericUIMainWindow, wxFrame )

////@begin GenericUIMainWindow event table entries
////@end GenericUIMainWindow event table entries

END_EVENT_TABLE()

/*!
 * GenericUIMainWindow constructors
 */

GenericUIMainWindow::GenericUIMainWindow( )
{
}

GenericUIMainWindow::GenericUIMainWindow( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create( parent, id, caption, pos, size, style );
}

/*!
 * GenericUIMainWindow creator
 */

bool GenericUIMainWindow::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin GenericUIMainWindow member initialisation
////@end GenericUIMainWindow member initialisation

////@begin GenericUIMainWindow creation
    SetParent(parent);
    CreateControls();
    Centre();
////@end GenericUIMainWindow creation
    return TRUE;
}

/*!
 * Control creation for GenericUIMainWindow
 */

void GenericUIMainWindow::CreateControls()
{    
////@begin GenericUIMainWindow content construction

    wxXmlResource::Get()->LoadFrame(this, GetParent(), _T("ID_FRAME"));
////@end GenericUIMainWindow content construction
}

/*!
 * Should we show tooltips?
 */

bool GenericUIMainWindow::ShowToolTips()
{
    return TRUE;
}
