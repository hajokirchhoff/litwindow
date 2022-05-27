/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: check.hpp,v 1.5 2006/11/28 13:44:03 Hajo Kirchhoff Exp $
 */
#ifndef _CHECK_HPP
#define _CHECK_HPP

#ifdef _MSC_VER
#pragma once
#endif

#include <exception>
#include <stdexcept>
#include <map>
#include <string>
#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif
#include "lwbase.hpp"
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

namespace litwindow {

	namespace checks {

#ifdef __cpp_lib_uncaught_exceptions
		inline bool has_uncaught_exceptions()
		{
			return std::uncaught_exceptions() > 0;
		}
#else
		inline bool has_uncaught_exceptions()
		{
			return std::uncaught_exception();
		}
#endif

		class LWBASE_API __c_ContextObject
		{
		public:
			typedef std::map<std::string, std::string> attributes_t;
			__c_ContextObject(const char* text);
			~__c_ContextObject();
			std::string &operator[](const char *name)
			{
				if (m_attributes==0) m_attributes=new attributes_t;
				return (*m_attributes)[name];
			}
			const char *context;
			attributes_t *m_attributes;
			static __c_ContextObject *get_top() { return top; }
		protected:
			__c_ContextObject *previous;
			static __c_ContextObject *top;			
		};

		inline __c_ContextObject& Context() { return *__c_ContextObject::get_top(); }
		inline void AddContext(const char* name, const char *value) { Context()[name]=value; }

		typedef __c_ContextObject context_t;

		#define  LWL_CONTEXT(a) litwindow::checks::__c_ContextObject BOOST_PP_CAT(__c_obj, __LINE__)(a)
#define LWLCHECK litwindow::checks::__c_ContextObject BOOST_PP_CAT(__c_obj, __LINE__)(__FILE__ ":" BOOST_PP_STRINGIZE(__LINE__)"(" __FUNCTION__ ")")
		//#define CONTEXT(a) __c_ContextObject __c_obj(a)

#define AssertRange(idx, vec, msg) Verify((idx)>=0 && (idx)<sizeof (vec)/sizeof (*(vec)), msg)

		class assertion_failed : public std::runtime_error {
		public:
			assertion_failed(const std::string& message):std::runtime_error(message) {}
		};

		class precondition_violated : public std::logic_error {
		public:
			precondition_violated(const std::string& message):std::logic_error(message) {}
		};

		class invariant_violated : public std::logic_error {
		public:
			invariant_violated(const std::string& message):std::logic_error(message) {}
		};

		class abort_on : public std::runtime_error {
		public:
			abort_on(const std::string& message):std::runtime_error(message) {}
		};

		//////////////////////////////////////////////////////////////////////////
		inline void AbortOn(bool condition, const char* text)
		{
			if (condition)
				throw abort_on(text);
		}

		inline void Verify(bool condition, /*const char* str_condition, */const char* text="")
		{
			if (!condition) {
				throw assertion_failed(text);
			}
		}

		inline void Precondition(bool condition, /*const char* str_condition, */const char* text)
		{
			if (!condition) {
				// throw an error
				throw precondition_violated(text);
			}
		}

		inline void Invariant(bool condition, const char* text="Invariant violated")
		{
			if (!condition) {
				throw invariant_violated(text);
			}
		}

		inline void Fail(const char* text)
		{
			// throw an error
			throw std::runtime_error(text);
		}

		//////////////////////////////////////////////////////////////////////////

		//#define Precondition(a, b) _Precondition(a, #a, b)
		//#define Verify(a, b) _Assure(a, #a, b)

		std::string LWBASE_API GetErrorMessage(std::exception &e);
		std::string LWBASE_API GetErrorMessage(const char *);
		std::string LWBASE_API GetExceptionContext(bool resetContext=true);

		class try_op_base_t
		{
		public:
			virtual void operator() () = 0;
		};

		template <class Result, class T>
		class try_mem_fun_t:public try_op_base_t
		{
		public:
			try_mem_fun_t(T* pThis, Result (T::*pMemFun)()):thisPtr(pThis), memFunPtr(pMemFun) {}
			void operator() ()
			{
				rc=(thisPtr->*memFunPtr)();
			}
			T* thisPtr;
			Result (T::*memFunPtr)();
			Result rc;
		};

		template <class Result, class T>
		try_mem_fun_t<Result, T> try_mem_fun(T* pThis, Result (T::*pMemFun)())
		{
			return try_mem_fun_t<Result, T>(pThis, pMemFun);
		}

		int LWBASE_API TryOperation(try_op_base_t& tryOperation);

		template <class Result, class T>
		std::pair<int, Result> TryOperation(try_mem_fun_t<Result, T>& tryOperation)
		{
			int rc=TryOperation(static_cast<try_op_base_t&>(tryOperation));
			return make_pair(rc, tryOperation.rc);
		}
	};
	using namespace checks;
    //namespace check = checks;

#define MemFun(a) try_mem_fun(this, a)

}

#endif
