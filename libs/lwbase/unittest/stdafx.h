/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: stdafx.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#pragma warning(disable: 4786)  // debug symbol length truncated
#pragma once

// tell MS VC8 to stop issuing tons of (well meaning, but stupid) warnings
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#define BOOST_ALL_DYN_LINK

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#include <crtdbg.h>
#else
#define DEBUG_NEW new
#endif

//#include <cppunit/ui/text/TestRunner.h>
//#include <cppunit/extensions/HelperMacros.h>
#define BOOST_TEST_MAIN

#include <boost/test/auto_unit_test.hpp>
