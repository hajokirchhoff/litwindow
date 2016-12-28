/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: rssmainframe.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
/////////////////////////////////////////////////////////////////////////////
// Name:        rssmainframe.h
// Purpose:     
// Author:      Hajo Kirchhoff - Lit Window Productions
// Modified by: 
// Created:     08/10/04 09:47:23
// RCS-ID:      
// Copyright:   Copyright 2004 - Hajo Kirchhoff - Lit Window Productions
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _RSSMAINFRAME_H_
#define _RSSMAINFRAME_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "rssmainframe.cp"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/xrc/xmlres.h"
#include "wx/frame.h"
#include "wx/splitter.h"
#include "wx/html/htmlwin.h"
////@end includes

#include <litwindow/wx/rapidui.h>

using litwindow::RapidUI;

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
#define SYMBOL_RSSMAINFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU
#define SYMBOL_RSSMAINFRAME_TITLE _("Simple RSS Reader")
#define SYMBOL_RSSMAINFRAME_IDNAME ID_FRAME
#define SYMBOL_RSSMAINFRAME_SIZE wxSize(400, 300)
#define SYMBOL_RSSMAINFRAME_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * RssMainFrame class declaration
 */

class RssMainFrame: public wxFrame
{    
    DECLARE_CLASS( RssMainFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    RssMainFrame( );
    RssMainFrame( wxWindow* parent, wxWindowID id = SYMBOL_RSSMAINFRAME_IDNAME, const wxString& caption = SYMBOL_RSSMAINFRAME_TITLE, const wxPoint& pos = SYMBOL_RSSMAINFRAME_POSITION, const wxSize& size = SYMBOL_RSSMAINFRAME_SIZE, long style = SYMBOL_RSSMAINFRAME_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_RSSMAINFRAME_IDNAME, const wxString& caption = SYMBOL_RSSMAINFRAME_TITLE, const wxPoint& pos = SYMBOL_RSSMAINFRAME_POSITION, const wxSize& size = SYMBOL_RSSMAINFRAME_SIZE, long style = SYMBOL_RSSMAINFRAME_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin RssMainFrame event handler declarations

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
    void OnExitClick( wxCommandEvent& event );

////@end RssMainFrame event handler declarations

////@begin RssMainFrame member function declarations

////@end RssMainFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin RssMainFrame member variables
////@end RssMainFrame member variables
    RapidUI m_rapidUI;
};

#endif
    // _RSSMAINFRAME_H_
