/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: mainpanel.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
/////////////////////////////////////////////////////////////////////////////
// Name:        mainpanel.h
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     05/14/04 09:10:45
// RCS-ID:      
// Copyright:   Copyright 2004, Hajo Kirchhoff, Lit Window Productions
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _MAINPANEL_H_
#define _MAINPANEL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "mainpanel.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/xrc/xmlres.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10002
#define SYMBOL_MAINPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_MAINPANEL_TITLE _("mainpanel")
#define SYMBOL_MAINPANEL_IDNAME ID_DIALOG
#define SYMBOL_MAINPANEL_SIZE wxSize(600, 500)
#define SYMBOL_MAINPANEL_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * MainPanel class declaration
 */

class MainPanel: public wxPanel
{    
    DECLARE_CLASS( MainPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    MainPanel( );
    MainPanel( wxWindow* parent, wxWindowID id = SYMBOL_MAINPANEL_IDNAME, const wxString& caption = SYMBOL_MAINPANEL_TITLE, const wxPoint& pos = SYMBOL_MAINPANEL_POSITION, const wxSize& size = SYMBOL_MAINPANEL_SIZE, long style = SYMBOL_MAINPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MAINPANEL_IDNAME, const wxString& caption = SYMBOL_MAINPANEL_TITLE, const wxPoint& pos = SYMBOL_MAINPANEL_POSITION, const wxSize& size = SYMBOL_MAINPANEL_SIZE, long style = SYMBOL_MAINPANEL_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin MainPanel event handler declarations

////@end MainPanel event handler declarations

////@begin MainPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end MainPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin MainPanel member variables
////@end MainPanel member variables
};

#endif
    // _MAINPANEL_H_
