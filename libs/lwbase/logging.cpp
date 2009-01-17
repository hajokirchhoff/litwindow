/*
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library
* distribution, file LICENCE.TXT
* $Id: logging.cpp,v 1.5 2006/11/22 16:21:15 Hajo Kirchhoff Exp $
*/
#include "stdafx.h"
#include "litwindow/lwbase.hpp"
#include "litwindow/logging.h"
#include <strstream>
#include <iostream>
#include <fstream>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _WIN32
#ifdef _DEBUG
#include <windows.h>
namespace {
#if defined(_UNICODE) && !defined(UNICODE)
#error setup error - must define both _UNICODE and UNICODE
#endif
    void OutputDebug(const TCHAR *str)
    {
        OutputDebugString(_T("lw_log:  "));
        OutputDebugString(str); OutputDebugString(_T("\n"));
    }
};
#endif
#else
namespace {
    void OutputDebug(const TCHAR *str)
    {
        cout << str << endl;
    }
};
#pragma message("Modify 'OutputDebug' to send lw_log() messages to your debug device. Example: cout << str;")
#endif
namespace litwindow {

wrapping_tostrstream::wrapping_tostrstream(std::streamsize _Count, wrapping_tstreambuf::get_dumper_t dumper)
:wrapping_base(_Count, dumper)
,tostream(&_GetMysb())
,_Mysb(_GetMysb())
{
}

wrapping_tstreambuf::wrapping_tstreambuf(std::streamsize _Count, wrapping_tstreambuf::get_dumper_t dumper)
//TODO: Reintroduce _Count again
:tstringbuf(tstring())
,m_has_wrapped(false)
,m_dump_function(dumper)
{
	//str(tstring().resize(_Count));
	tstring s;
	s.resize(_Count);
	str(s);
}

wrapping_tstreambuf::~wrapping_tstreambuf()
{
    // if something has been written to the buffer and the dumper function has been set
    if (m_dump_function.get() && pptr()) {
        try {
            tostream &out=(*m_dump_function).get();
            if (m_has_wrapped)
                out.write(pptr(), streamsize(epptr()-pptr()));
            out.write(pbase(), streamsize(pptr()-pbase()));
        }
        catch (...) {
            // eat all exceptions: destructor must not throw exception
        }
    }
}

wrapping_tstreambuf::int_type wrapping_tstreambuf::overflow(wrapping_tstreambuf::int_type _Meta)
{
	if (_Meta==traits_type::eof())
        return 0;
    if (pptr()!=0 && pptr()==epptr()) {
        // lets wrap
        setp(pbase(), epptr());
        m_has_wrapped=true;
        return inherited_t::overflow(_Meta);
    } else
        return inherited_t::overflow(_Meta);
}

std::streamsize wrapping_tstreambuf::copy(litwindow::tstring &destination) const
{
	if (m_has_wrapped)
		destination.assign(pptr(), epptr()-pptr());
	else
		destination.erase();
	destination.append(pbase(), pptr()-pbase());
	return static_cast<streamsize>(destination.size());
}

std::streamsize wrapping_tstreambuf::write(litwindow::tofstream &destination) const
{
	if (m_has_wrapped)
		destination.write(pptr(), epptr()-pptr());
	destination.write(pbase(), pptr()-pbase());
	return destination.tellp();
}

void wrapping_tstreambuf::clear_buffer()
{
    setp(pbase(), epptr());
    m_has_wrapped=false;
}

std::streamsize wrapping_tostrstream::copy(litwindow::tstring &destination) const
{
    return rdbuf()->copy(destination);
}

std::streamsize wrapping_tostrstream::write(litwindow::tofstream &destination) const
{
	return rdbuf()->write(destination);
}

struct get_lw_log_dumper:public wrapping_tstreambuf::dumper_t
{
    tostream &get()
    {
        m_out.open("dump_lwlog.txt");
        return m_out;
    }
    tofstream m_out;
};

wrapping_tostrstream &lw_log()
{
#ifdef _DEBUG
    #define LW_LOG_DUMPER new get_lw_log_dumper
#else
    #define LW_LOG_DUMPER 0
#endif
    static wrapping_tostrstream _lw_log(0x10000, LW_LOG_DUMPER);
#ifdef _DEBUG
    static static_redirect_tstreambuf debug_printer(_lw_log, OutputDebug);
#endif
    return _lw_log;
}

struct get_lw_err_dumper:public wrapping_tstreambuf::dumper_t
{
    tostream &get()
    {
		std::tcerr << _T("--------------- lw_err ---------------") << std::endl;
        return std::tcerr;
    }
};

wrapping_tostrstream &lw_err()
{
    static wrapping_tostrstream _lw_err(8192, new get_lw_err_dumper);
    return _lw_err;
}

};
