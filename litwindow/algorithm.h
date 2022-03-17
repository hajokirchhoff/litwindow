/* 
 * Copyright 2004-2007, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 */
#ifndef _LW_ALGORITHM_
#define _LW_ALGORITHM_

#pragma once

#include <functional>
#include <boost/function.hpp>
#include <boost/utility.hpp>

namespace litwindow {

	// Use with for_each to delete pointers in a container.
	template <class V>
	struct delete_ptr
	{
		void operator()(V *v)
		{
			delete v;
		}
	};

	/// This helper class calls a functor when the object is being destroyed.
	struct scope_guard:boost::noncopyable
	{
		scope_guard(boost::function<void()> g):m_guard(g) {}
		~scope_guard() { m_guard(); }
		boost::function<void()> m_guard;
	};


	/** visitor design pattern - template class to be used with for_each.
	This function object 'visits' each element in 'for_each'.
	*/
	template <class _Visitor, class _Arg=typename _Visitor::argument_type, class _Ret=typename _Visitor::result_type>
	struct visitor_fun_t:public std::unary_function<_Arg, _Ret>
	{
		visitor_fun_t(_Visitor &_v)
			:visitor(&_v)
		{
		}

		typedef _Arg argument_type;
		typedef _Ret result_type;
		_Visitor *visitor;

		result_type operator()(const _Arg &v)
		{
			return (*visitor)(v);
		}
	};

	template <class _Visitor>
	visitor_fun_t<_Visitor> visitor_fun(_Visitor &_v)
	{
		return visitor_fun_t<_Visitor>(_v);
	}

#if 0
	/** visitor design pattern - template class to be used with for_each */
	template <class _Visitor, class _Arg, class _Ret>
	struct visitor_mem_fun_t:public std::unary_function<_Arg, _Ret>
	{
		typedef result_type (typename _Visitor::* _VisitorFunc)(const argument_type &);
		_Visitor *visitor;
		_VisitorFunc visitor_func;

		visitor_mem_fun_t(_Visitor &_v, _VisitorFunc f)
			:visitor(&_v)
			,visitor_func(f)
		{

		}

		result_type operator()(const argument_type &v)
		{
			return (visitor->*visitor_func)(v);
		}
	};

	template <class _Visitor, class _Arg, class _Ret>
	visitor_mem_fun_t<_Visitor, _Arg, _Ret> visitor_mem_fun(_Visitor &_v, _Ret (_Visitor::*f)(const _Arg &))
	{
		return visitor_mem_fun_t<_Visitor, _Arg, _Ret>(_v, f);
	}
#endif

}
#endif
