/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: dbms_ms_excel.cpp,v 1.1 2006/10/10 14:13:10 Hajo Kirchhoff Exp $
*/

#include "stdafx.h"
#include <iomanip>
#include <odbcinst.h>
#include <litwindow/dataadapter.h>
#include <litwindow/check.hpp>
#include <io.h>
#include "litwindow/odbc/dbms_ms_excel.h"
#include "litwindow/odbc/statement.h"
#define new DEBUG_NEW

namespace litwindow {

	namespace odbc {

		namespace {
#include "./md5.cpp"
		};

		void dbms_excel::init_macros()
		{
			macros()[_T("BOOLEAN")]=_T("BIT");
			macros()[_T("TRUE")]=_T("TRUE");
			macros()[_T("FALSE")]=_T("FALSE");
		}

		bool dbms_excel::can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string)
		{
			return name==_T("Microsoft Excel Driver (*.xls)") || name==_T("EXCEL");
		}


		namespace {
			dbms_base::do_register ms_access_driver(dbms_excel::can_handle_, dbms_excel::construct);
		};

	};

};