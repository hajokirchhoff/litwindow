/////////////////////////////////////////////////////////////////////////////
// Name:        list_mediatorapp.h
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     07/09/2009 17:10:52
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _LIST_MEDIATORAPP_H_
#define _LIST_MEDIATORAPP_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "generic_list_sample.h"
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
////@end control identifiers

/*!
 * List_mediatorApp class declaration
 */

class List_mediatorApp: public wxApp
{    
    DECLARE_CLASS( List_mediatorApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    List_mediatorApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin List_mediatorApp event handler declarations

////@end List_mediatorApp event handler declarations

////@begin List_mediatorApp member function declarations

////@end List_mediatorApp member function declarations

////@begin List_mediatorApp member variables
////@end List_mediatorApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(List_mediatorApp)
////@end declare app

#endif
    // _LIST_MEDIATORAPP_H_
