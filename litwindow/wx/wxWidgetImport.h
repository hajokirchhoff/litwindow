/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: wxWidgetImport.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#ifndef _WXWIDGETIMPORT_
#define _WXWIDGETIMPORT_

#if wxVERSION_NUMBER < 2600

#ifdef VERBOSE_BUILD
#pragma message("using wx 2.4.x")
#endif

#pragma comment(lib, "rpcrt4")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "wsock32")
#pragma comment(lib, "wxmsw" libDebug)
#pragma comment(lib, "wxxrc" libDebug)

#endif

#endif
