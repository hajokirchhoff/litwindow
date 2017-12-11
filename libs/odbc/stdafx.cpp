// stdafx.cpp : source file that includes just the standard includes
// lwodbc_dll.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#if _MSC_VER >= 1911
// Required for linking against odbc.lib
// This was a breaking change in VS2015, see also
// https://social.msdn.microsoft.com/Forums/vstudio/en-US/b63a5ba5-71a9-4758-a6ea-a419836c3285/vs-2015-unresolved-external-when-linking-to-odbc-libs?forum=vcgeneral
// or https://www.google.com/search?q=legacy_stdio_definitions+odbc
#pragma comment(lib, "legacy_stdio_definitions")#endif

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

/**@todo
    - move connection_script_parser to statement::
    - write the named_connection_pool
    - write unittests for creating the database
    - test the dbms classes

    */

