// stdafx.cpp : source file that includes just the standard includes
// wxWindowsTemplate.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdwx.h"

#ifndef __WIN95__
#error An error in wxWindows version 2.4.0 and 2.4.1 require that you set WINVER=0x400 in the project settings!
        // To set WINVER, open the project properties page, then
        // locate C++ | Preprocessor | Preprocessor Definitions
        // and add ;WINVER=0x400 to the line.
#endif

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#include "litwindow/wx/wxwidgetimport.h"
