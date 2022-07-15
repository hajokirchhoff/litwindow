/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: internals.cpp,v 1.3 2006/06/27 11:23:54 Hajo Kirchhoff Exp $
*/
#include "stdafx.h"
#include "litwindow/odbc/internals.h"
#include <sqlext.h>
#include <litwindow/dataadapter.h>
#include <litwindow/logging.h>
#include <boost/bind/bind.hpp>
#include "litwindow/odbc/lwodbc.h"
#include "litwindow/odbc/statement.h"

#define new DEBUG_NEW

namespace litwindow {

namespace odbc {;

scope null_scope;

bool scope::operator <(const scope &s) const
{
	return    catalog<s.catalog ||
		( catalog==s.catalog && (schema<s.schema ||
		( schema==s.schema && table<s.table)
		) );
}

tstring quote(const tstring &name,  TCHAR quote_char)
{
	if (is_null(name) || name.length()==0) return tstring();
	tstring rc;
	if (quote_char!=0)
		rc=quote_char;
	rc+=name;
	if (quote_char!=0)
		rc+=quote_char;
	return rc;
}

tstring scope::make_name(TCHAR sep_char, TCHAR quote_char) const
{
	tstring rc;
	rc=quote(catalog, quote_char);
	if (rc.length())
		rc+=sep_char;
	rc+=quote(schema, quote_char);
	if (rc.length())
		rc+=sep_char;
	rc+=quote(table, quote_char);
	if (rc.length())
		rc+=sep_char;
	return rc;
}

};

};
