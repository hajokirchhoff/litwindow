/////////////////////////////////////////////////////////////////////////////
// Name:        odbc_list_mediatorapp.hpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     08/10/2019 11:45:16
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _ODBC_LIST_MEDIATORAPP_H_
#define _ODBC_LIST_MEDIATORAPP_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/xrc/xmlres.h"
#include "wx/image.h"
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
 * Odbc_list_mediatorApp class declaration
 */

class Odbc_list_mediatorApp: public wxApp
{    
    DECLARE_CLASS( Odbc_list_mediatorApp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructor
    Odbc_list_mediatorApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

////@begin Odbc_list_mediatorApp event handler declarations

////@end Odbc_list_mediatorApp event handler declarations

////@begin Odbc_list_mediatorApp member function declarations

////@end Odbc_list_mediatorApp member function declarations

////@begin Odbc_list_mediatorApp member variables
////@end Odbc_list_mediatorApp member variables
};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(Odbc_list_mediatorApp)
////@end declare app

#endif
    // _ODBC_LIST_MEDIATORAPP_H_
