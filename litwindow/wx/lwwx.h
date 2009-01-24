/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: lwwx.h,v 1.3 2006/04/11 13:16:31 Hajo Kirchhoff Exp $
 */
#ifndef _LWWX_H
#define _LWWX_H

#pragma once

#include "litwindow/lwbase.hpp"

/// major version number of the lwwx part of the library
#define LWWX_VERSION_MAJOR LWBASE_VERSION_MAJOR

/// minor version number of the lwwx part of the library
#define LWWX_VERSION_MINOR LWBASE_VERSION_MINOR

/// build number of the lwwx part of the library
#define LWWX_VERSION_BUILD LWBASE_VERSION_BUILD

#define LWWX_VERSION ((LWWX_VERSION_MAJOR << 16) | LWWX_VERSION_MINOR)

#ifdef LWWX_EXPORTS
#define LWWX_API _declspec(dllexport)
#define LWWX_TEMPLATES_EXTERN
#elif defined(LWL_USING_DLL) || defined(LWWX_DYN_LIB)
#define LWWX_API _declspec(dllimport)
#define LWWX_TEMPLATES_EXTERN extern
#ifndef WXUSINGDLL
#error Using lwwx as a DLL requires using wxWidgets as a DLL. Please #define WXUSINGDLL in your project!
#endif
#ifndef LWWX_DYN_LIB
#define LWWX_DYN_LIB
#endif
#else
#define LWWX_API
#endif

#if defined(LWWX_EXPORTS) || defined(LWWX_DYN_LIB)

#define LWWX_STL_VECTOR_EXPORT(type) \
    LWWX_TEMPLATES_EXTERN template class LWWX_API std::allocator<type>; \
    LWWX_TEMPLATES_EXTERN template class LWWX_API std::vector<type>
#else

#define LWWX_STL_VECTOR_EXPORT(type)

#endif

#define LWWX_LIB_NAME "lwwx" _LWL_LIB_THREAD _LWL_LIB_DLL _LWL_LIB_UNICODE _LWL_LIB_DEBUG

#ifndef LWWX_EXPORTS
#ifdef LWL_VERBOSE_BUILD
#pragma message("using library: " LWWX_LIB_NAME)
#endif
//#pragma comment(lib, LWWX_LIB_NAME)
#else
#ifdef LWL_VERBOSE_BUILD
#pragma message("creating library: " LWWX_LIB_NAME)
#endif
#endif

    //-----------------------------------------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------------------------------------//
namespace litwindow {

/// If enabled == true, output from wx_log() gets sent to wxLogDebug as well.
extern void LWWX_API enable_log_to_wxdebug(bool enabled=true);

};

#if !defined(LWWX_EXPORTS) && !defined(LITWINDOW_LWWX_NO_LIB) && !defined(LITWINDOW_ALL_NO_LIB)
#define BOOST_LIB_NAME 
#ifdef LWWX_DYN_LIB
#define BOOST_DYN_LIB LWWX_LIB_NAME
#endif
#ifdef LWL_VERBOSE_BUILD
#define BOOST_LIB_DIAGNOSTIC
#endif
#include <boost/config/auto_link.hpp>
#endif


#endif
