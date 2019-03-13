/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: tstring.hpp,v 1.2 2006/04/04 09:21:33 Hajo Kirchhoff Exp $
*/
#ifndef _LITWINDOW_TSTRING_H
#define _LITWINDOW_TSTRING_H

/** @file tstring.hpp
Declare tstring, tstringstream and other macros that automatically switch between char and wchar_t types.
*/

#include "./lwbase.hpp"
#include <string>
#include <locale>
#include <wchar.h>
#include <map>
#include <vector>

#ifdef _UNICODE
#define tcout			wcout
#define tcerr			wcerr
#else
#define tcout			cout
#define tcerr			cerr
#endif

namespace litwindow {
	extern std::string LWBASE_API w2sstring(const std::wstring &w);
	extern std::wstring LWBASE_API s2wstring(const std::string &s);

	template <typename DestChar, typename SrcChar>
	std::basic_string<DestChar> to_string(const SrcChar *s);

	template<>
	inline std::basic_string<char> to_string(const char *s) { return std::basic_string<char>(s); }
	template<>
	inline std::basic_string<wchar_t> to_string(const char *s) { return s2wstring(std::string(s)); }

	template <typename DestChar, typename SrcChar>
	std::basic_string<DestChar> to_string(const std::basic_string<SrcChar> &s) { return s; }
	template <>
	inline std::basic_string<char> to_string(const std::basic_string<wchar_t> &s) { return w2sstring(s); }
	template <>
	inline std::basic_string<wchar_t> to_string(const std::basic_string<char> &s) { return s2wstring(s); }

#ifdef _MBCS
	typedef std::string tstring;
	typedef std::ofstream tofstream;
	typedef std::ifstream tifstream;
	typedef std::ostream tostream;
	typedef std::stringbuf tstringbuf;
	typedef std::stringstream tstringstream;
	typedef std::ostringstream tostringstream;
	typedef std::istringstream tistringstream;
	typedef std::streambuf tstreambuf;
	inline const std::string &t2string(const std::string &a) { return a; }
	inline const std::string &s2tstring(const std::string &a) { return a; }
	inline std::wstring t2wstring(const std::string &a) { return s2wstring(a); }
	inline std::string w2tstring(const std::wstring &a) { return w2sstring(a); }
#elif defined(_UNICODE)
	typedef std::wstring tstring;
	typedef std::wofstream tofstream;
	typedef std::wifstream tifstream;
	typedef std::wostream tostream;
	typedef std::wstringbuf tstringbuf;
	typedef std::wstringstream tstringstream;
	typedef std::wostringstream tostringstream;
	typedef std::wistringstream tistringstream;
	typedef std::wostream tostream;
	typedef std::wstreambuf tstreambuf;
	typedef std::wstringbuf tstringbuf;
	typedef std::codecvt<wchar_t, char, mbstate_t> _t_w_cvt;
	using std::codecvt;
	using std::locale;
	using std::string;
	using std::wstring;
	using std::use_facet;

	inline std::string t2string(const std::wstring &a) { return w2sstring(a); }
	inline std::wstring s2tstring(const std::string &a) { return s2wstring(a); }
	inline const std::wstring &t2wstring(const std::wstring &a) { return a; }
	inline const std::wstring &w2tstring(const std::wstring &a) { return a; }
#endif

	inline tstring &toupper(tstring &t)
	{
		tstring::iterator tou;
		for (tou=t.begin(); tou!=t.end(); ++tou)
			*tou=std::toupper(*tou, locale());
		return t;
	}

	//struct splitter_call_base
	//{
	//	virtual void operator()(const tstring &left, const tstring &right) = 0;
	//};

	//template <typename Splitter>
	//struct splitter_call
	//{
	//	splitter_call(Splitter s):m_splitter(s) {}
	//	virtual void operator()(const tstring &left, const tstring &right)
	//	{
	//		m_splitter(left, right);
	//	}
	//	Splitter m_splitter;
	//};

	std::map<tstring, tstring> LWBASE_API split_string(const tstring &in, const tstring &delimiters=_T(", "), const tstring &quotes=_T("\"'{["), const tstring &equal_sign=_T("="));
	void LWBASE_API split_string(std::vector<tstring> &rc, const tstring &in, const tstring &delimiters=_T(",. "), const tstring &quotes=_T("\"'{["));
};

#endif

#ifdef _MSC_VER
#pragma once
#endif

