/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: result.hpp,v 1.2 2006/01/17 08:55:22 hajo Exp $
*/
#ifndef _RESULT_HPP
#define _RESULT_HPP

#ifdef _MSC_VER
#pragma once
#endif

namespace litwindow {

#define ASSERT assert

struct result_base_t
{
};

template <class ResultCode>
class result_t:public result_base_t
{
public:
	result_t()
	{
		init();
	}
	result_t(const ResultCode &_hr)
	{
		init();
		set_result_code(_hr);
	}
	result_t(const result_t<ResultCode> &_hr)
	{
		init();
		set_result_code(_hr);
		_hr.m_was_matched=true;
	}
	~result_t(void)
	{
		if (!m_was_matched && !std::uncaught_exception())
			raise_exception_on_failure();
	}
	result_t& operator=(ResultCode _hr)
	{
		set_result_code(_hr);
		return *this;
	}
	result_t& operator=(const result_t<ResultCode> &_hr)
	{
		set_result_code(_hr);
		_hr.m_was_matched=true;
		return *this;
	}
	bool operator==(ResultCode _hr) const
	{
		bool r= (hr==_hr);
		m_was_matched= (m_was_matched || r);
		return r;
	}
	bool operator!=(ResultCode _hr) const
	{
		return hr!=_hr;
	}
	bool operator!() const
	{
		return failed();
	}
	void clear() // reset error code
	{
		init();
	}

	void set_result_code(ResultCode _hr)
	{
		if (!m_was_matched)
			raise_exception_on_failure();
		hr=_hr;
		if (!success())
			trace_error();
		m_was_matched=false;
	}
	ResultCode get_result_code() const
	{
		m_was_matched=true;
		return hr;
	}
	bool failed() const
	{
		m_was_matched=true;
		return is_failure();
	}
	bool success() const
	{
		m_was_matched=true;
		return !is_failure();
	}
	void assert_success() const
	{
		raise_exception_on_failure();
	}
	operator ResultCode() const
	{
		return hr;
	}
	void trace_error() const
	{
		ASSERT("Not implemented"!=0);
	}
	result_t<ResultCode>& ignore()		// call this to explicitly ignore this error
	{
		m_was_matched=true;
		return *this;
	}
protected:
	bool is_failure() const
	{
		return hr!=0;
	}
	void raise_exception_on_failure() const
	{
		if (!success()) {
			trace_error();
			ASSERT(success());
			throw *this;
		}
	}
	void init()
	{
		hr=0;
		m_was_matched=true;
	}
	mutable bool m_was_matched;
	ResultCode hr;
};

    //-----------------------------------------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------------------------------------//

    ///@name Commonly used WIN32 specializations
    //@{
#if defined(_WIN32_WINDOWS)
#ifdef _ATL_VER
template <>
inline void result_t<HRESULT>::trace_error() const
{
	AtlTraceErrorRecords(hr);
}

template <> inline bool result_t<HRESULT>::is_failure() const
{
	return FAILED(hr);
}
#endif

template<> inline bool result_t<BOOL>::is_failure() const
{
	return !hr;
}

template<> inline bool result_t<UINT_PTR>::is_failure() const
{
	return hr==0;
}

template<> inline void result_t<BOOL>::trace_error() const
{
//	AtlTrace("result_t<BOOL>::trace_error: The result returned from the function was FALSE! LastError was %08lx.\n", ::GetLastError());
}

template<> inline void result_t<BOOL>::init()
{
	hr=TRUE; m_was_matched=true;
}

template<> inline void result_t<UINT_PTR>::trace_error() const
{
//	AtlTrace("result_t<UINT_PTR>::trace_error: The function call returned 0. LastError was %08lx.\n", ::GetLastError());
}

template<> inline void result_t<UINT_PTR>::init()
{
	hr=0; m_was_matched=true;
}

typedef result_t<HRESULT> HResult;
typedef result_t<BOOL> BResult;
#endif
//@}

template <> inline bool result_t<bool>::is_failure() const
{
    return !hr;   
}

typedef result_t<bool> bool_result;
typedef result_t<int> int_result;

template <> inline result_t<bool>::operator bool() const
{
    m_was_matched=true; return hr;
}

} // end namespace

#endif
