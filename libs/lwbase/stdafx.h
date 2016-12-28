/* 
 * Copyright 2004-2014, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: stdafx.h,v 1.4 2006/09/13 19:07:44 Hajo Kirchhoff Exp $
 */
#ifndef _STDAFX_H
#define _STDAFX_H

#ifdef _MSC_VER
#pragma warning(disable: 4786)  // debug symbol length truncated
#pragma once

// tell MS VC8 to stop issuing tons of (well meaning, but stupid) warnings
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#define NOMINMAX
#endif

//// mingw
//#ifndef _GLIBCPP_USE_WCHAR_T
//#define _GLIBCPP_USE_WCHAR_T	1
//#endif

#include "./targetver.h"

#ifdef wxUSE_GUI
#include <wx/wxprec.h>
#endif

#include <algorithm>
#include <map>
#include <vector>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#include <crtdbg.h>
#else
#define DEBUG_NEW new
#endif

// TODO: reference additional headers your program requires here
#endif
