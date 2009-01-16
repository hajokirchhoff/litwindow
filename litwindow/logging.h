/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: logging.h,v 1.5 2006/11/22 16:21:15 Hajo Kirchhoff Exp $
*/
/** @file
    This file declares classes for the Lit Window logging mechanism.
    */
#ifndef _LW_LOGGING_H
#define _LW_LOGGING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sstream>	
#include <string>
#include <memory>
#include "./lwbase.hpp"
#include "./lw_pragma_import.h"
#include "./tstring.hpp"

namespace litwindow {

/** A specialized version of a strstreambuf that wraps around instead of increasing the storage size
    when the end of the buffer has been reached.
    */

class wrapping_tstreambuf:public litwindow::tstringbuf
{
	typedef litwindow::tstringbuf inherited_t;
public:
    /// Copies the entire buffer to the destination.
    LWBASE_API std::streamsize copy(litwindow::tstring &destination) const;
    /// Writes the entire buffer to the destination.
    LWBASE_API std::streamsize write(litwindow::tofstream &destination) const;
    /// Clears the entire buffer.
    LWBASE_API void clear_buffer();

    struct dumper_t
    {
        virtual ~dumper_t() { }
        virtual litwindow::tostream &get() = 0;
    };
    typedef dumper_t *get_dumper_t;

    /// Set the dumper function.
    LWBASE_API void set_dumper(get_dumper_t new_dumper)
    {
        m_dump_function=std::auto_ptr<dumper_t>(new_dumper);
    }
    LWBASE_API wrapping_tstreambuf(std::streamsize _Count=0x10000, get_dumper_t dumper=0);
    LWBASE_API ~wrapping_tstreambuf();
protected:
    virtual int_type overflow(int_type _Meta);
    bool m_has_wrapped;
    /** The dump function is called by the destructor and gives
        applications a chance to dump the contents of the
        wrap buffer to a file or debug channel.
        */
    std::auto_ptr<dumper_t> m_dump_function;
};

struct wrapping_base
{
	wrapping_base(std::streamsize _Count, wrapping_tstreambuf::get_dumper_t dumper)
		:m_buf(_Count, dumper)
	{
	}
	wrapping_tstreambuf &_GetMysb() { return m_buf; }
protected:
	wrapping_tstreambuf m_buf;
};

/** An output stream writing data into a wraparound buffer located in memory.
    */
class wrapping_tostrstream:private wrapping_base,public tostream
{
public:
	/// Construct a wrapping ostrstream.
	/// @p _Count is the size of the wraparound buffer.
	LWBASE_API wrapping_tostrstream(std::streamsize _Count=0x10000, wrapping_tstreambuf::get_dumper_t dumper=0);
	const wrapping_tstreambuf *rdbuf() const
	{
		return static_cast<const wrapping_tstreambuf*>(&_Mysb);
	}
	/// Clears the entire buffer.
	LWBASE_API void clear_buffer()
	{
		_Mysb.clear_buffer();
	}
	/// Copies the entire buffer to the destination.
	LWBASE_API std::streamsize copy(litwindow::tstring &destination) const;
	/// Writes the entire buffer to the destination.
	LWBASE_API std::streamsize write(litwindow::tofstream &destination) const;
	/// set the dumper function
	LWBASE_API void set_dumper(wrapping_tstreambuf::get_dumper_t new_dump_function)
	{
		_Mysb.set_dumper(new_dump_function);
	}
protected:
	wrapping_tstreambuf &_Mysb;
};

/// Returns the global logging stream.
extern LWBASE_API wrapping_tostrstream& lw_log();
/// Returns the global error stream.
extern LWBASE_API wrapping_tostrstream& lw_err();

/** This template class can be inserted into the logging stream to route
    the log output to a @p Printer. The Printer class is a function object
    that takes a const char* parameter as input and returns void.
    The following options determine the behaviour:
    -   @p linewise
        If true (default), waits until EOL or EOF has been written to the stream before sending
        the output to Printer.
        If false, calls Printer for every character written to the stream.
    -   @p passthrough
        If true (default), sends all characters to the original stream as well.
        If false, sends characters to Printer only.
        */
template <class Printer>
class template_redirect_tstreambuf:public tstreambuf
{
public:
    typedef Printer printer_t;
    template_redirect_tstreambuf(printer_t _printer)
        :m_printer(_printer)
        ,m_original_stream(0)
        ,m_redirected_ios(0)
        ,m_linewise(true)
        ,m_passthrough(true)
    {
    }
    template_redirect_tstreambuf(tostream &where, printer_t printer)
        :m_printer(printer)
        ,m_original_stream(0)
        ,m_redirected_ios(0)
        ,m_linewise(true)
        ,m_passthrough(true)
    {
        insert(where);
    }
    virtual ~template_redirect_tstreambuf()
    {
        remove();
    }
    /// Insert the redirector into a stream.
    void insert(litwindow::tostream &where);
    /// Remove the redirector from the stream.
    void remove()
    {
        if (m_original_stream && m_redirected_ios) {
            m_redirected_ios->rdbuf(m_original_stream);
        }
        m_original_stream=0;
        m_redirected_ios=0;
    }
    /// Set or clear the linewise flag.
    void    set_linewise(bool enabled=true) { m_linewise = enabled; }
    /// Set or clear the passthrough flag.
    void    set_passthrough(bool enabled=true) { m_passthrough = enabled; }
protected:
    virtual int_type overflow(int_type _Meta)
    {
        if (m_linewise) {
		if (_Meta==traits_type::eof() || _Meta==_T('\n')) {
                	m_printer(m_line.c_str());
                	m_line.erase();
            	} else if (_Meta!=_T('\r'))
                	m_line.append(1, _Meta);
        } else {
            if (_Meta!=_T('\r')) {
                TCHAR line[2];
                line[0]=_Meta;
                line[1]=0;
                m_printer(line);
            }
        }
        return m_passthrough ? m_original_stream->sputc(_Meta) : _Meta;
    }

    printer_t       m_printer;
    tstreambuf *m_original_stream;
    litwindow::tostream   *m_redirected_ios;
    bool            m_linewise;
    bool            m_passthrough;
    litwindow::tstring    m_line;
};

template <class Printer>
void template_redirect_tstreambuf<Printer>::insert(litwindow::tostream &where)
{
    remove();
    m_original_stream=where.rdbuf(this);
    m_redirected_ios=&where;
}

/// Standard specialization for void (*)(const char*) Printer functions.
typedef template_redirect_tstreambuf<void (*)(const TCHAR*)> static_redirect_tstreambuf;

};

#endif
