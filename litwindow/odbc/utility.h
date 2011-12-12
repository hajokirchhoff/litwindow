/* 
 * Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window ODBC Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window ODBC Library 
 * distribution, file LICENCE.TXT
 * $Id: utility.h,v 1.4 2006/07/29 11:49:19 Hajo Kirchhoff Exp $
 */
#ifndef _LWODBC_UTILITY_
#define _LWODBC_UTILITY_

#include <litwindow/lwbase.hpp>
#include <litwindow/tstring.hpp>
#include <map>
#include <vector>
#include "lwodbc.h"

#pragma once

namespace litwindow {

namespace odbc {;

    typedef std::map<tstring, tstring> attributes_list_t;
    typedef std::map<tstring, attributes_list_t> drivers_list_t;

    extern sqlreturn LWODBC_API get_drivers(drivers_list_t &driver_list);

    const SQLSMALLINT get_user_dsn_only = SQL_FETCH_FIRST_USER;
    const SQLSMALLINT get_system_dsn_only = SQL_FETCH_FIRST_SYSTEM;
    const SQLSMALLINT get_all_dsn = SQL_FETCH_FIRST;

    typedef std::vector<pair<tstring, tstring> > datasource_list_t;

    extern sqlreturn LWODBC_API get_datasources(datasource_list_t &data_sources, SQLSMALLINT dsn_type=get_all_dsn, const tstring &only_for_driver=tstring());

    extern map<tstring, tstring> LWODBC_API split_dsn(const tstring &dsn_connection_string);

};

};

#endif
