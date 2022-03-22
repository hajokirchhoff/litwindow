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

namespace litwindow {
	namespace checks {
		using std::endl;

		static char exceptionContextBuffer[8192];
		static unsigned nextFreeChar=0;
		static unsigned nextFreePointer=sizeof exceptionContextBuffer-sizeof (unsigned);
		__c_ContextObject * __c_ContextObject::top=0;
		void trace(char c)
		{
			if (nextFreeChar+sizeof (c)<nextFreePointer)
				exceptionContextBuffer[nextFreeChar++]=c;
		}

		void trace(const char *context)
		{
			lw_log() << _T("   context-> ") << context << endl;
			size_t length=strlen(context);
			if (nextFreeChar+length<nextFreePointer) {
				strncpy(exceptionContextBuffer+nextFreeChar, context, length);
				nextFreeChar+=(unsigned)length;
			}
			// else overflow
		}

		void push_pointer(unsigned ptr)
		{
			if (nextFreePointer-sizeof (unsigned)>nextFreeChar) {
				*(unsigned*)(exceptionContextBuffer+nextFreePointer)=ptr;
				nextFreePointer-=sizeof (unsigned);
			} else
				lw_log() << _T("exception context - not enough room to store all context") << endl;
		}

		void AddToContextStack(const char *context)
		{
			push_pointer(nextFreeChar);
			trace(context);
			trace('\0');
		}

		void AddToContextStack(__c_ContextObject::attributes_t *attributes)
		{
			__c_ContextObject::attributes_t::reverse_iterator i;
			for (i=attributes->rbegin(); i!=attributes->rend(); ++i) {
				string c=string(" *  ")+i->first + string("=") + i->second;
				AddToContextStack(c.c_str());
			}
		}

		inline void ResetExceptionContext()
		{
			nextFreeChar=0;
			nextFreePointer=sizeof exceptionContextBuffer-sizeof (unsigned);
		}

#ifndef DOXYGEN_INVOKED
		__c_ContextObject::~__c_ContextObject()
		{
			top=previous;
			if (std::uncaught_exception()) {
				if (m_attributes) AddToContextStack(m_attributes);
				AddToContextStack(context);
			}
			delete m_attributes;
		}
		__c_ContextObject::__c_ContextObject(const char* text)
			:context(text), m_attributes(0),previous(get_top()) 
		{ 
			if (std::uncaught_exception()==false)
				ResetExceptionContext();
			top=this; 
		}
#endif

		string GetExceptionContext(bool resetContext)
		{
			string rc;
			unsigned *ptr=(unsigned*)(exceptionContextBuffer+nextFreePointer);
			while (++ptr<(unsigned*)(exceptionContextBuffer+sizeof (exceptionContextBuffer))) {
				rc.append(exceptionContextBuffer+*ptr);
				rc.append("\n ");
			}
			if (resetContext) {
				ResetExceptionContext();
			}
			return rc;
		}

		string GetErrorMessage(const char *e)
		{
			string rc("the error is \"");
			rc.append(e).append("\"\n").append(GetExceptionContext());
			return rc;
		}

		string GetErrorMessage(std::exception &e)
		{
			return GetErrorMessage(e.what());
		}

#ifdef USES_MFC
		int TryOperation(try_op_base_t& tryOperation)
		{
			try {
				tryOperation();
			}
			catch (CBResult &) {
				CString err("Operation failed - ");
				err.Append(check::GetExceptionContext().c_str());
				AfxMessageBox(err, MB_OK);
				return IDABORT;
			}
			catch (CHResult &hr) {
				hk::AtlMessageBoxErrorRecords(hr.GetResultCode(), "operation could not be completed. exiting now.");
				return IDABORT;
			}
			catch (std::exception &e) {
				ErrorMessageBox(e);
				return IDABORT;
			}
			catch (...) {
				AfxMessageBox("unspecified exception caught in CDeflApp::InitInstance", MB_OK);
				return IDABORT;
			}
			return IDOK;
		}

		int ErrorMessageBox(const char *e, int type)
		{
			CString err("Operation failed - \n");
			err.Append(check::GetErrorMessage(e).c_str());
			return AfxMessageBox(err, type);
		}

		int ErrorMessageBox(std::exception &e, int type)
		{
			return ErrorMessageBox(e.what(), type);
		}

#endif
	}
}
