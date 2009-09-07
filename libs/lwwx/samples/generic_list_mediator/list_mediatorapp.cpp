#include "stdwx.h"
/////////////////////////////////////////////////////////////////////////////
// Name:        list_mediatorapp.cpp
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     07/09/2009 17:10:52
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
////@end includes

#include "list_mediatorapp.h"

////@begin XPM images
////@end XPM images


/*
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( List_mediatorApp )
////@end implement app


/*
 * List_mediatorApp type definition
 */

IMPLEMENT_CLASS( List_mediatorApp, wxApp )


/*
 * List_mediatorApp event table definition
 */

BEGIN_EVENT_TABLE( List_mediatorApp, wxApp )

////@begin List_mediatorApp event table entries
////@end List_mediatorApp event table entries

END_EVENT_TABLE()


/*
 * Constructor for List_mediatorApp
 */

List_mediatorApp::List_mediatorApp()
{
    Init();
}


/*
 * Member initialisation
 */

void List_mediatorApp::Init()
{
////@begin List_mediatorApp member initialisation
////@end List_mediatorApp member initialisation
}

/*
 * Initialisation for List_mediatorApp
 */

bool List_mediatorApp::OnInit()
{    
////@begin List_mediatorApp initialisation
	// Remove the comment markers above and below this block
	// to make permanent changes to the code.

#if wxUSE_XPM
	wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
	wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
	wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
	wxImage::AddHandler(new wxGIFHandler);
#endif
	generic_list_sample* mainWindow = new generic_list_sample(NULL);
	/* int returnValue = */ mainWindow->ShowModal();

	mainWindow->Destroy();
	// A modal dialog application should return false to terminate the app.
	return false;
////@end List_mediatorApp initialisation

    return true;
}


/*
 * Cleanup for List_mediatorApp
 */

int List_mediatorApp::OnExit()
{    
////@begin List_mediatorApp cleanup
	return wxApp::OnExit();
////@end List_mediatorApp cleanup
}

