/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: lwodbc_def.h,v 1.3 2006/06/27 11:23:54 Hajo Kirchhoff Exp $
*/

#ifndef _LWODBC_DEF_
#define _LWODBC_DEF_

#pragma once

#define LWODBC_LIB_PREFIX "lwodbc"
#define LWODBC_LIB_NAME LWODBC_LIB_PREFIX _LWL_LIB_THREAD _LWL_LIB_DLL _LWL_LIB_UNICODE _LWL_LIB_DEBUG

#if defined(LWODBC_EXPORTS)
	#define LWODBC_API _declspec(dllexport)
#elif defined(_USRDLL) || defined(USING_DLL)
	#define LWODBC_API _declspec(dllimport)
#else
    #define LWODBC_API
#endif

#ifndef LWODBC_EXPORTS
#pragma comment(lib, LWODBC_LIB_NAME)
#endif

#ifndef _
#define _(a) _T(a)
#endif

/// major version number of the lwbase part of the library
#define LWODBC_VERSION_MAJOR LWBASE_VERSION_MAJOR

/// minor version number of the lwbase part of the library
#define LWODBC_VERSION_MINOR LWBASE_VERSION_MINOR

/// build number of the lwbase part of the library
#define LWODBC_VERSION_BUILD LWBASE_VERSION_BUILD

#define LWODBC_VERSION ((LWODBC_VERSION_MAJOR << 16) | LWODBC_VERSION_MINOR)

#ifdef _DEBUG
#define SQLRETURN_GLOBAL_LOG_DEFAULT true
#else
#define SQLRETURN_GLOBAL_LOG_DEFAULT false
#endif

#endif
