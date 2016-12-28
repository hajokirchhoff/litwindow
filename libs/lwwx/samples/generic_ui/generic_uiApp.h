/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: generic_uiApp.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#pragma once

class Cgeneric_uiApp:public wxApp
{
        /*! 
         *  This typedef creates an alias for the parent class which
         *  can then be used instead of it. Useful if you need
         *  to change the parent class and don't want to forget to
         *  update all instances where you used the class name.
         */
    typedef wxApp Inherited;
public:
        /*! 
         *  Called by wxWindows to inialize the application.
         *  @return true if initialization was successful. 
         */
    bool OnInit();
protected:
        /*! 
         *  Called by wxWindows to clean up initialization.
         *  @return exit code.
         */
    int OnExit();
        /// Called by wxWindows to initialize command line handling.
    void OnInitCmdLine(wxCmdLineParser &parser);
        /// Called by wxWindows to allow Cgeneric_uiApp to handle the command line.
    bool OnCmdLineParsed(wxCmdLineParser &parser);
};

DECLARE_APP(Cgeneric_uiApp)