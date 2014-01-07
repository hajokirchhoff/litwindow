/* 
 * Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window ODBC Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window ODBC Library 
 * distribution, file LICENCE.TXT
 * $Id: utility.cpp,v 1.4 2006/07/29 11:49:19 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include "./utility.h"
#include "./connection.h"
#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_actor.hpp>

#define new DEBUG_NEW

namespace litwindow {

namespace odbc {;

using namespace boost::spirit::classic;

typedef scanner<const TCHAR*> scanner_t;
typedef rule<scanner_t> trule;

map<tstring, tstring> split_dsn(const tstring &dsn_connection_string)
{
	map<tstring, tstring> rc;
	tstring last_id;
	tstring last_value;
	trule id=(alpha_p >> *alnum_p)[assign_a(last_id)] >> *space_p;
	trule keyword_pair=(id >> _T('=') >> (lexeme_d[*(print_p-_T(';'))] || confix_p(_T('{'), lexeme_d[*print_p], _T('}')))[assign_a(last_value)]) >> *space_p;
	trule connection_string=*space_p >> !(keyword_pair[insert_at_a(rc, last_id, last_value)] % _T(';'));
	parse(dsn_connection_string.c_str(), connection_string);
	return rc;
}

sqlreturn get_drivers(drivers_list_t &driver_list)
{
    sqlreturn_auto_set_diagnostics rc;
    SQLTCHAR _buffer[1024];
    SQLTCHAR *buffer=_buffer;
    SQLSMALLINT buffer_size=sizeof(_buffer);
    SQLSMALLINT length;
    SQLTCHAR _attributes_buffer[4192];
    SQLTCHAR* attributes_buffer=_attributes_buffer;
    SQLSMALLINT attributes_buffer_size=sizeof(_attributes_buffer);
    SQLSMALLINT attributes_length;
    SQLUSMALLINT direction=SQL_FETCH_FIRST;
    bool some_data_was_truncated;

    do {
        direction=SQL_FETCH_FIRST;
        some_data_was_truncated=false;
	  rc.set_handles(SQL_HANDLE_ENV, get_default_environment()->handle());
        while ( (rc.set(SQLDrivers(get_default_environment()->handle(), direction, 
                    buffer, buffer_size, &length, 
                    attributes_buffer, attributes_buffer_size, &attributes_length))).ok() ) 
        {
            if (rc==SQL_SUCCESS_WITH_INFO && rc.is_state(_T("01004"))) {
                if (length>buffer_size) {
                    if (buffer!=_buffer)
                        delete [] buffer;
                    buffer = new SQLTCHAR[length+1];
                    buffer_size=(length+1)*sizeof(TCHAR);
                }
                if (attributes_length>attributes_buffer_size) {
                    if (attributes_buffer!=_attributes_buffer)
                        delete [] attributes_buffer;
                    attributes_buffer = new SQLTCHAR[attributes_length+1];
                    attributes_buffer_size= (attributes_length+1)*sizeof(TCHAR);
                }
                // but we cannot call SQLDrivers again immediately to retrieve the missing information
                // must restart for that, so set some_data_was_truncated=true and loop
                some_data_was_truncated=true;
            } else {
                attributes_list_t &the_list(driver_list[(TCHAR*)buffer]);
                if (the_list.size()==0) {
                    TCHAR *p=(TCHAR*)attributes_buffer;
                    while (*p) {
                        tstring name(p);
                        p+=name.length()+1;
                        tstring attribute(p);
                        p+=attribute.length()+1;
                        the_list[name]=attribute;
                    }
                } // else data was entered in the first pass already
            }
            direction=SQL_FETCH_NEXT;
        }
    } while (some_data_was_truncated);
    if (buffer!=_buffer)
        delete [] buffer;
    if (attributes_buffer!=_attributes_buffer)
        delete [] attributes_buffer;
    return rc.no_data() ? sqlreturn(SQL_SUCCESS) : (sqlreturn)rc;
}

sqlreturn get_datasources(datasource_list_t &data_sources, SQLSMALLINT direction, const tstring &only_for_driver)
{
    SQLTCHAR _dsn[1024];
    SQLTCHAR _driver[1024];
    SQLTCHAR *dsn=_dsn;
    SQLSMALLINT dsn_size=sizeof(_dsn);
    SQLTCHAR *driver=_driver;
    SQLSMALLINT driver_size=sizeof(_driver);
    SQLSMALLINT dsn_length, driver_length;
    sqlreturn_auto_set_diagnostics rc;

    rc.set_handles(SQL_HANDLE_ENV, get_default_environment()->handle());
    while ( (rc.set(SQLDataSources(get_default_environment()->handle(), direction, dsn, dsn_size, &dsn_length, driver, driver_size, &driver_length))).ok() ) {
        if (only_for_driver.length()==0 || only_for_driver==(TCHAR*)driver)
            data_sources.push_back(make_pair((TCHAR*)dsn, (TCHAR*)driver));
        direction=SQL_FETCH_NEXT;
    }
    return rc.no_data() ? sqlreturn(SQL_SUCCESS) : (sqlreturn)rc;
}

};

};
