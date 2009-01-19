/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: MyFrame.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdwx.h"
#include "myframe.h"
#include "mainpanel.h"

//TODO: remember to add this line to every cpp source file.
#define new DEBUG_NEW

BEGIN_EVENT_TABLE(MyFrame, Inherited)
//TODO: add more events here.
    EVT_MENU(IDM_SAY_HELLO, OnSayHello)
    EVT_MENU(wxID_EXIT, OnExit)
END_EVENT_TABLE()

BEGIN_RULES(mainPanelRules)
    TWOWAY(wxT("m_strRadioBox"), wxT("wnd::m_radioBox"))
    RULE(wxT("m_controls.Enabled"), !_e<bool>(_v("wnd::m_enable_controls")))
    RULE(wxT("m_rbSum.Value"), _e<int>(_v("wnd::m_radioBox.Value"))+_v("wnd::m_rbTwo.Value")+2)
END_RULES()

using namespace litwindow;

MyFrame::MyFrame(void)
    :wxFrame(0, -1, _("Your application frame title here"), wxDefaultPosition, wxSize(600,500))
{
    //TODO: create and set an application icon here
    //NOTE: To make Windows show the application icon for the exe file automatically,
    //      you must make it the first icon in the resource file. To do so, give it
    //      an alphanumerical name (don't assign it an integer ID) and ensure that the name
    //      is the smallest name when sorted alphabetically.
    wxIcon myFrameIcon(wxICON(APP_ICON));
    SetIcon(myFrameIcon);
    wxMenuBar *menuBar=new wxMenuBar;
    wxMenu *fileMenu=new wxMenu;
    fileMenu->Append(IDM_SAY_HELLO, _("Say hello"), _("Open a box saying... you guessed it."));
    fileMenu->Append(wxID_EXIT, _("E&xit"), _("Exit the program"));
    menuBar->Append(fileMenu, _("&File"));
    //TODO: add more menus to the menu bar
    SetMenuBar(menuBar);

    wxStatusBar *statusBar=new wxStatusBar(this);
    //TODO: initialize the status bar panes
    SetStatusBar(statusBar);

    MainPanel *main=new MainPanel(this);
    Show();
    m_data.m_radioBox=3;

    m_rapidUI << this << make_accessor(m_data) << mainPanelRules;
    m_rapidUI.Start();
}

    //-----------------------------------------------------------------------------------------------------------//
void MyFrame::OnSayHello(wxCommandEvent &evt)
{
    wxMessageBox(_("Yes, I know this is boring, but I couldn't think of anything else ;)"), _("Hello world!"), MB_OK);
    wxLogDebug(litwindow::as_debug(*static_cast<wxWindow*>(this)).c_str());
}

void MyFrame::OnExit(wxCommandEvent &evt)
{
    // calling close will exit the window.
    Close();
	evt.Skip();
}
