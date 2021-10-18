#include "stdafx.h"
/////////////////////////////////////////////////////////////////////////////
// Name:        odbc_list_mediatorapp.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     08/10/2019 11:45:16
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
#include "wx/fs_zip.h"
////@end includes

#include "odbc_list_mediatorapp.hpp"
#include "odbclistmediatortest.hpp"
#include "wx/object.h"
#include "wx/xrc/xmlres.h"
#include "litwindow/wx/list_mediator.hpp"

////@begin XPM images
////@end XPM images


/*
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( Odbc_list_mediatorApp )
////@end implement app


/*
 * Odbc_list_mediatorApp type definition
 */

IMPLEMENT_CLASS( Odbc_list_mediatorApp, wxApp )


/*
 * Odbc_list_mediatorApp event table definition
 */

BEGIN_EVENT_TABLE( Odbc_list_mediatorApp, wxApp )

////@begin Odbc_list_mediatorApp event table entries
////@end Odbc_list_mediatorApp event table entries

END_EVENT_TABLE()


/*
 * Constructor for Odbc_list_mediatorApp
 */

Odbc_list_mediatorApp::Odbc_list_mediatorApp()
{
    Init();
}


/*
 * Member initialisation
 */

void Odbc_list_mediatorApp::Init()
{
////@begin Odbc_list_mediatorApp member initialisation
////@end Odbc_list_mediatorApp member initialisation
}

/////////////////////////////////////////////////////////////////////////////
// Name:        
// Purpose:     XML resource handler for VirtualListCtrl
// Author:      
// Created:     09/10/2019
// RCS-ID:      $Id: wbcustomcontroldlg.cpp,v 1.13 2007/11/15 06:53:22 anthemion Exp $
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

using litwindow::wx::VirtualListCtrl;

class VirtualListCtrlXmlHandler : public wxXmlResourceHandler
{
	DECLARE_DYNAMIC_CLASS(VirtualListCtrlXmlHandler)
public:
	VirtualListCtrlXmlHandler();
	virtual wxObject* DoCreateResource();
	virtual bool CanHandle(wxXmlNode* node);
};

IMPLEMENT_DYNAMIC_CLASS(VirtualListCtrlXmlHandler, wxXmlResourceHandler)

VirtualListCtrlXmlHandler::VirtualListCtrlXmlHandler()
	: wxXmlResourceHandler()
{
	AddWindowStyles();
	AddStyle("wxLC_REPORT", wxLC_REPORT);
}

wxObject* VirtualListCtrlXmlHandler::DoCreateResource()
{
	XRC_MAKE_INSTANCE(control, VirtualListCtrl);

	control->Create(m_parentAsWindow, GetID(), GetPosition(), GetSize(), GetStyle());

	SetupWindow(control);

	return control;
}

bool VirtualListCtrlXmlHandler::CanHandle(wxXmlNode* node)
{
	return IsOfClass(node, wxT("VirtualListCtrl"));
}

/*
 * Initialisation for Odbc_list_mediatorApp
 */

bool Odbc_list_mediatorApp::OnInit()
{    
////@begin Odbc_list_mediatorApp initialisation
	// Remove the comment markers above and below this block
	// to make permanent changes to the code.

	wxXmlResource::Get()->InitAllHandlers();
	wxFileSystem::AddHandler(new wxZipFSHandler);

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
////@end Odbc_list_mediatorApp initialisation

	wxXmlResource::Get()->AddHandler(new VirtualListCtrlXmlHandler);
	wxXmlResource::Get()->Load("odbc_list_mediator.zip");

	using namespace litwindow::odbc;
	connection::pool().set(L"Driver={SQLite3 ODBC Driver};Database=v4.db;stepapi=0;notxn=0;shortnames=0;longnames=0;nocreat=0;nowchar=0;fksupport=1;oemcp=0;bigint=0;jdconv=0");

	default_connection()->execute(LR"(
DROP TABLE IF EXISTS odbc_list_mediator;
CREATE TABLE odbc_list_mediator (
  id integer primary key,
  one text,
  two integer,
  three real
);
INSERT INTO odbc_list_mediator(one, two, three)
VALUES("zeile 1", 1, 11.1);
)");
	ODBClistmediatortest* mainWindow = new ODBClistmediatortest(NULL);
	mainWindow->Show(true);

    return true;
}


/*
 * Cleanup for Odbc_list_mediatorApp
 */

int Odbc_list_mediatorApp::OnExit()
{    
////@begin Odbc_list_mediatorApp cleanup
	return wxApp::OnExit();
////@end Odbc_list_mediatorApp cleanup
}

