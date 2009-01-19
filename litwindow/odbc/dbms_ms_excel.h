/** \file
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: dbms_ms_excel.h,v 1.1 2006/10/10 14:13:10 Hajo Kirchhoff Exp $
*/

#ifndef __LWODBC_DBMS_MS_EXCEL_H
#define __LWODBC_DBMS_MS_EXCEL_H

#pragma once

#include "./dbms.h"

#pragma warning(push, 4)

// now make intellisense work again by undefining LWODBC_API for intellisense
#ifdef INTELLISENSE_DOES_NOT_WORK_PSEUDO_DEFINE
#undef LWODBC_API
#define LWODBC_API
#endif

namespace litwindow {

	namespace odbc {

		using namespace std;

#if defined(_WIN32) || defined(DOXYGEN_INVOKED)

#define DBMS_EXCEL_DEFINED

		/** This class exists only under Windows */
		/** MS-Access or MS-Jet ODBC class */
		class dbms_excel:public dbms_generic
		{
		public:
			dbms_excel(const tstring &odbcConnection=tstring()):dbms_generic(odbcConnection)
			{
				init_macros();
			}

			virtual tstring get_driver_name() const { return _T("Microsoft Excel Driver (*.xls)"); }

			bool has_capability(capabilities c) const
			{
				if (c==has_user_accounts) return false;
				if (c==has_get_current_sequence_value)
					return false;
				return false;
			}

			static dbms_base *construct(const tstring &odbc_connection) { return new dbms_excel(odbc_connection); }
			static bool can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string);

			tstring get_dbms_name() const { return _T("MS-Jet"); }
		protected:
			void init_macros();
		};
#endif

	};

};

#pragma warning (pop)
#endif
