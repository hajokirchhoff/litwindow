/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: check.cpp,v 1.5 2006/11/28 13:44:03 Hajo Kirchhoff Exp $
*/
#include "stdafx.h"
#include "litwindow/lwbase.hpp"
#include "litwindow/logging.h"
#include "litwindow/check.hpp"
#include "litwindow/result.hpp"
#include <stdexcept>
#include <cstring>

namespace litwindow {
	namespace checks {
		using namespace std;

		struct ExceptionContext
		{
			char exceptionContextBuffer[8192];
			unsigned nextFreeChar = 0;
			unsigned nextFreePointer = sizeof exceptionContextBuffer - sizeof(unsigned);
			void trace(char c)
			{
				if (nextFreeChar + sizeof(c) < nextFreePointer)
					exceptionContextBuffer[nextFreeChar++] = c;
			}

			void trace(const char* context)
			{
				lw_log() << _T("   context-> ") << context << endl;
				size_t length = strlen(context);
				if (nextFreeChar + length < nextFreePointer) {
					strncpy(exceptionContextBuffer + nextFreeChar, context, length);
					nextFreeChar += (unsigned)length;
				}
				// else overflow
			}

			void push_pointer(unsigned ptr)
			{
				if (nextFreePointer - sizeof(unsigned) > nextFreeChar) {
					*(unsigned*)(exceptionContextBuffer + nextFreePointer) = ptr;
					nextFreePointer -= sizeof(unsigned);
				}
				else
					lw_log() << _T("exception context - not enough room to store all context") << endl;
			}

			void AddToContextStack(const char* context)
			{
				push_pointer(nextFreeChar);
				trace(context);
				trace('\0');
			}

			void AddToContextStack(__c_ContextObject::attributes_t* attributes)
			{
				__c_ContextObject::attributes_t::reverse_iterator i;
				for (i = attributes->rbegin(); i != attributes->rend(); ++i) {
					string c = string(" *  ") + i->first + string("=") + i->second;
					AddToContextStack(c.c_str());
				}
			}

			inline void ResetExceptionContext()
			{
				nextFreeChar = 0;
				nextFreePointer = sizeof exceptionContextBuffer - sizeof(unsigned);
			}

			string GetExceptionContext(bool resetContext = true)
			{
				string rc;
				auto* ptr = (unsigned*)(exceptionContextBuffer + nextFreePointer);
				while (++ptr < (unsigned*)(exceptionContextBuffer + sizeof(exceptionContextBuffer))) {
					rc.append(exceptionContextBuffer + *ptr);
					rc.append("\n ");
				}
				if (resetContext) {
					ResetExceptionContext();
				}
				return rc;
			}

		};

		static thread_local ExceptionContext theContext;

#ifndef DOXYGEN_INVOKED
		__c_ContextObject::~__c_ContextObject()
		{
			set_top(previous);
			if (has_uncaught_exceptions()) {
				if (m_attributes) theContext.AddToContextStack(m_attributes);
				theContext.AddToContextStack(context);
			}
			delete m_attributes;
		}

		__c_ContextObject::top_ptr& __c_ContextObject::get_top_object()
		{
			static thread_local top_ptr top = nullptr;
			return top;
		}

		__c_ContextObject::top_ptr __c_ContextObject::get_top()
		{
			return get_top_object();
		}

		void __c_ContextObject::set_top(top_ptr new_top)
		{
			get_top_object() = new_top;
		}

		__c_ContextObject::__c_ContextObject(const char* text)
			:context(text), m_attributes(nullptr), previous(get_top())
		{ 
			if (has_uncaught_exceptions() == false)
				theContext.ResetExceptionContext();
			set_top(this); 
		}
#endif

		string LWBASE_API GetExceptionContext(bool resetContext/*=true*/)
		{
			return theContext.GetExceptionContext(resetContext);
		}

		string GetErrorMessage(const char *e)
		{
			string rc("the error is \"");
			rc.append(e).append("\"\n").append(theContext.GetExceptionContext());
			return rc;
		}

		string GetErrorMessage(exception &e)
		{
			return GetErrorMessage(e.what());
		}
	}
}
