/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: MyFrame.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#ifndef _MYFRAME_
#define _MYFRAME_

#include "litwindow/wx/rapidui.h"
using namespace litwindow;

#include "sample_data.h"

#pragma once

class MyFrame : public wxFrame
{
    typedef wxFrame Inherited;
public:
    MyFrame(void);

protected:
    enum MenuIds {
            // user defined ids should start above wxID_HIGHEST
        IDM_SAY_HELLO = wxID_HIGHEST + 1
            //TODO: add more menu ids here
    };
    void OnSayHello(wxCommandEvent &evt);
    void OnExit(wxCommandEvent &evt);

    RapidUI m_rapidUI;
    MyFrameData m_data;

    DECLARE_EVENT_TABLE()
};
#endif
