/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: genericuimainwindow.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
/////////////////////////////////////////////////////////////////////////////
// Name:        genericuimainwindow.h
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     05/14/04 09:09:51
// RCS-ID:      
// Copyright:   Copyright 2004, Hajo Kirchhoff, Lit Window Productions
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _GENERICUIMAINWINDOW_H_
#define _GENERICUIMAINWINDOW_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "genericuimainwindow.cpp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/xrc/xmlres.h"
#include "wx/frame.h"
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
#define ID_FRAME 10000
#define SYMBOL_GENERICUIMAINWINDOW_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU
#define SYMBOL_GENERICUIMAINWINDOW_TITLE _("generic UI sample")
#define SYMBOL_GENERICUIMAINWINDOW_IDNAME ID_FRAME
#define SYMBOL_GENERICUIMAINWINDOW_SIZE wxSize(600, 500)
#define SYMBOL_GENERICUIMAINWINDOW_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * GenericUIMainWindow class declaration
 */

class GenericUIMainWindow: public wxFrame
{    
    DECLARE_CLASS( GenericUIMainWindow )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    GenericUIMainWindow( );
    GenericUIMainWindow( wxWindow* parent, wxWindowID id = SYMBOL_GENERICUIMAINWINDOW_IDNAME, const wxString& caption = SYMBOL_GENERICUIMAINWINDOW_TITLE, const wxPoint& pos = SYMBOL_GENERICUIMAINWINDOW_POSITION, const wxSize& size = SYMBOL_GENERICUIMAINWINDOW_SIZE, long style = SYMBOL_GENERICUIMAINWINDOW_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_GENERICUIMAINWINDOW_IDNAME, const wxString& caption = SYMBOL_GENERICUIMAINWINDOW_TITLE, const wxPoint& pos = SYMBOL_GENERICUIMAINWINDOW_POSITION, const wxSize& size = SYMBOL_GENERICUIMAINWINDOW_SIZE, long style = SYMBOL_GENERICUIMAINWINDOW_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin GenericUIMainWindow event handler declarations

////@end GenericUIMainWindow event handler declarations

////@begin GenericUIMainWindow member function declarations

////@end GenericUIMainWindow member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin GenericUIMainWindow member variables
////@end GenericUIMainWindow member variables
};

#endif
    // _GENERICUIMAINWINDOW_H_
