#ifndef _LITWINDOW_TCHAR_H
#define _LITWINDOW_TCHAR_H

///@file
/// Defines TCHAR and other Unicode related macros.
/// To compile for Unicode, compile with /D_UNICODE

#if !defined(_UNICODE) && !defined(_MBCS)
#define _MBCS
#endif

#ifdef _MSC_VER
#include <tchar.h>
#ifdef LITWINDOW_VERBOSE_BUILD
#pragma message("Using Microsoft TCHAR.H")
#endif
#else

#ifdef LITWINDOW_VERBOSE_BUILD
#pragma message("Using internal TCHAR.H")
#endif

#ifdef _UNICODE

#ifndef TCHAR
#define TCHAR wchar_t
#endif
#ifndef _T
#define _T(a) L ## a
#endif
#define _tcscpy(a,b) wcscpy(a,b)
#define _tsetlocale(a,b) wsetlocale(a,b)

#else

#ifndef TCHAR
#define TCHAR char
#endif
#ifndef _T
#define _T(a) a
#endif
#define _tcscpy(a,b) strcpy(a,b)
#define _tsetlocale(a,b) setlocale(a,b)
#endif

#endif

#ifdef LITWINDOW_VERBOSE_BUILD
#ifdef _UNICODE
#pragma message("Unicode build")
#else
#pragma message("Non-unicode build")
#endif
#endif

#endif
