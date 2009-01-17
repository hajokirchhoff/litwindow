/*
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library
 * distribution, file LICENCE.TXT
 * $Id: lw_pragma_import.h,v 1.7 2006/07/17 06:30:57 Hajo Kirchhoff Exp $
 */
#ifndef _LITWINDOW_IMPORT_
#define _LITWINDOW_IMPORT_

#ifdef _MSC_VER // microsoft compiler

#include <tchar.h>

#ifndef LWL_VERBOSE_BUILD
#define LWL_VERBOSE_BUILD
#endif

#if defined(_USRDLL) || defined(USING_DLL) || defined(LWBASE_EXPORTS)
#define LWL_USING_DLL
#endif

#if defined(LWL_USING_DLL)
#define LWBASE_DLL_EXPORT _declspec(dllexport)
#define LWBASE_DLL_IMPORT _declspec(dllimport)
#define _FORCE_DLL_EXPORT _declspec(dllexport)

#ifdef LWBASE_EXPORTS
#define LWBASE_API LWBASE_DLL_EXPORT
#define LWBASE_EXTERN

#ifdef LWL_VERBOSE_BUILD
#pragma message(" >> LWBASE_API - export")
#endif

#else
#define LWBASE_API LWBASE_DLL_IMPORT
#define LWBASE_EXTERN extern
#pragma warning(disable:4231) // nonstandard extension: 'extern' before template explicit instantiation

#ifdef LWL_VERBOSE_BUILD
#pragma message(" >> LWBASE_API - import")
#endif

#endif
#pragma warning(disable:4273)   // inconsistent dll linkage
#else
/* defining the macros as '_declspec()' is a workaround for a MS VC6.0 compiler bug. It crashes if the macros are empty. */
#define LWBASE_DLL_EXPORT _declspec()
#define LWBASE_DLL_IMPORT
#define _FORCE_DLL_EXPORT _declspec()
#define LWBASE_API /**/
#define LWBASE_EXTERN

#ifdef LWL_VERBOSE_BUILD
#pragma message(" >> LWBASE_API - none")
#endif

#endif

#ifndef _LWL_LIB_PREFIX
#define _LWL_LIB_PREFIX "lwbase"
#endif

#if defined(LWL_USING_DLL)
#define _LWL_LIB_DLL ""
#define _LWL_LIB_THREAD ""
#else

#ifdef _DLL // using dynamic runtime libraries ??
#define _LWL_LIB_DLL ""
#else
#define _LWL_LIB_DLL "s"
#endif

#ifdef _MT  // using multithreaded runtime libraries ??
#define _LWL_LIB_THREAD "_m"
#else
#define _LWL_LIB_THREAD "_s"
#endif

#endif

#ifdef _DEBUG   // using debug version ??
#define _LWL_LIB_DEBUG "d"
#else
#define _LWL_LIB_DEBUG ""
#endif

#ifdef _UNICODE // using unicode version ??
#define _LWL_LIB_UNICODE "u"
#else
#define _LWL_LIB_UNICODE ""
#endif

#define LWL_MAKE_LWL_LIB_VERSION(m,n,b) "_" #m "_" #n "_" #b
#define _LWL_LIB_VERSION LWL_MAKE_LWL_LIB_VERSION(LWBASE_VERSION_MAJOR, LWBASE_VERSION_MINOR, LWBASE_VERSION_BUILD)

    //-----------------------------------------------------------------------------------------------------------//
#define _LWL_LIB_NAME _LWL_LIB_PREFIX _LWL_LIB_THREAD _LWL_LIB_DLL _LWL_LIB_UNICODE _LWL_LIB_DEBUG "-" LWBASE_VERSION_STRING

#ifdef LWL_VERBOSE_BUILD
#define SHOW_LIB 1
#else
#define SHOW_LIB 0
#endif

#ifndef LWBASE_EXPORTS
#if SHOW_LIB
#pragma message("using litwindow library: " _LWL_LIB_NAME)
#endif

#if defined(LWL_ALL_NO_LIB) && LWL_ALL_NO_LIB==1
#define LWL_NO_AUTO_LINK 1
#endif

#if !defined(LWL_NO_AUTO_LINK) || LWL_NO_AUTO_LINK==0
#pragma comment(lib, _LWL_LIB_NAME)
#endif

#else
#if SHOW_LIB
#pragma message("create import library: " _LWL_LIB_NAME)
#endif
#endif

#define MSG_SHOWN

#define STL_EXPORT_VECTOR(type) \
    LWBASE_EXTERN template class LWBASE_API std::allocator<type>; \
    LWBASE_EXTERN template class LWBASE_API std::vector<type>


#if _MSC_VER >= 1300
#define MEMBER_TEMPLATES_SUPPORTED 1
#endif

#else // end microsoft compiler

#define STL_EXPORT_VECTOR(type)

#define LWBASE_API
#define LWBASE_DLL_IMPORT
#define LWBASE_DLL_EXPORT
#define _FORCE_DLL_EXPORT

#endif

#endif
