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

#include <cstring>
#include <cwchar>
#include <strings.h>

#ifdef _UNICODE

#ifndef TCHAR
typedef wchar_t TCHAR;
#endif
#ifndef _T
#define _T(a) L ## a
#endif
#define _tcscpy(a,b) wcscpy(a,b)
#define _tsetlocale(a,b) wsetlocale(a,b)
#define _tcscmp(a,b) wcscmp(a,b)
#define _tcsicmp(a,b) wcscasecmp(a,b)
#define _tcslen(a) wcslen(a)

#else

#ifndef TCHAR
// unixODBC's sqltypes.h also typedefs TCHAR (to char in narrow/non-UNICODE
// mode). Using a typedef here - rather than a #define macro - avoids blindly
// substituting the TCHAR token inside sqltypes.h's own typedef, which would
// otherwise corrupt it into "typedef char char;". Duplicate identical
// typedefs are legal in C++, so this coexists safely regardless of include
// order.
typedef char TCHAR;
#endif
#ifndef _T
#define _T(a) a
#endif
#define _tcscpy(a,b) strcpy(a,b)
#define _tsetlocale(a,b) setlocale(a,b)
#define _tcscmp(a,b) strcmp(a,b)
#define _tcsicmp(a,b) strcasecmp(a,b)
#define _tcslen(a) strlen(a)
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
