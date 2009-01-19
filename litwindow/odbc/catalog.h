/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: catalog.h,v 1.5 2006/10/10 14:13:10 Hajo Kirchhoff Exp $
*/

#ifndef __LWODBC_CATALOG_H
#define __LWODBC_CATALOG_H

#include <litwindow/dataadapter.h>
#include <boost/utility.hpp>
#include <set>
#include "./statement.h"

#pragma warning(push, 4)
// now make intellisense work again by undefining LWODBC_API for intellisense
#ifdef INTELLISENSE_DOES_NOT_WORK_PSEUDO_DEFINE
#undef LWODBC_API
#define LWODBC_API
#endif

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

namespace litwindow {

namespace odbc {

#pragma warning(disable: 4511 4512)

struct full_column_information:public column_descriptor
{
	full_column_information():column_descriptor(), m_pk_name(0), m_fk_key_seq(-1), m_pk_key_seq(-1) {}
	tstring	m_table, m_schema, m_catalog, m_remarks;
	SQLSMALLINT m_pk_key_seq;
	const TCHAR *m_pk_name;
	SQLSMALLINT m_fk_key_seq;
	column_name get_column_name() const { return column_name(m_catalog, m_schema, m_table, m_name); }
};

struct table_information
{
	typedef vector<full_column_information> columns_t;
	columns_t m_columns;
	typedef scope_index<SQLSMALLINT> scopes_t;
	scopes_t m_column_index;
	std::set<tstring> m_pk_names;
	/// true if the driver returned information about the primary keys.
	bool m_primary_keys_valid;
	/// true if the driver returned information about the foreign keys.
	bool m_foreign_keys_valid;
};

struct sql_table_privileges_result_set
{
	tstring TABLE_CAT;
	tstring TABLE_SCHEM;
	tstring TABLE_NAME;
	tstring GRANTOR;
	tstring GRANTEE;
	tstring PRIVILEGE;
	tstring IS_GRANTABLE;
};

struct sql_column_privileges_result_set
{
	tstring	TABLE_CAT, 
			TABLE_SCHEM, 
			TABLE_NAME,
			COLUMN_NAME,
			GRANTOR,
			GRANTEE,
			PRIVILEGE,
			IS_GRANTABLE;
};

struct sql_foreign_keys_result_set
{
	tstring			PKTABLE_CAT, PKTABLE_SCHEM, PKTABLE_NAME, PKTABLE_COLUMN,
					FKTABLE_CAT, FKTABLE_SCHEM, FKTABLE_NAME, FKTABLE_COLUMN;
	SQLSMALLINT KEY_SEQ,
					UPDATE_RULE,
					DELETE_RULE;
	tstring			FK_NAME, PK_NAME;
	SQLSMALLINT	DEFERRABILITY;
};

struct sql_tables_result_set
{
	tstring			TABLE_CAT, TABLE_SCHEM, TABLE_NAME, TABLE_TYPE;
};

enum privilege_enum {
	no_privilege = 0,
	select_privilege = 1,
	update_privilege = 2,
	delete_privilege = 4,
	insert_privilege = 8,
	references_privilege = 10
};

class catalog:public statement
{
public:
	catalog(connection &c):statement(c) {}
	catalog():statement(_T("")) {}
	catalog(const tstring &named_connection):statement(_T(""), named_connection) {}

	bool LWODBC_API has_table_privileges(const tstring &grantee, int privilege_bitmask, const tstring &table, const tstring &schema=odbc::default_string, const tstring &catalog=odbc::default_string) throw();


	sqlreturn using_schema_catalog(const tstring &schema, const tstring &catalog, TCHAR * &use_schema, SQLSMALLINT &use_schema_length, TCHAR * &use_catalog, SQLSMALLINT &use_catalog_length);

	static const LWODBC_API tstring null;	///< a NULL string, which is different from "" meaning 'no input' for catalog functions
	///@catalog functions
	//@{
	/// execute SQLTables and bind to result set
	sqlreturn LWODBC_API execute_tables(sql_tables_result_set &rc);
	/// execute SQLTablePrivileges and bind to result set
	sqlreturn LWODBC_API execute_table_privileges(sql_table_privileges_result_set &rc, const tstring &table, const tstring &schema=odbc::default_string, const tstring &catalog=odbc::default_string);
	sqlreturn LWODBC_API execute_column_privileges(sql_column_privileges_result_set &rc, const tstring &column, const tstring &table, const tstring &schema=odbc::default_string, const tstring &catalog=odbc::default_string);
	sqlreturn LWODBC_API execute_foreign_keys(sql_foreign_keys_result_set &rc, const tstring &pk_catalog, const tstring &pk_schema, const tstring &pk_table, const tstring &fk_catalog, const tstring &fk_schema, const tstring &fk_table);
	/// get the list of columns in a table
	sqlreturn LWODBC_API get_table_information(table_information &rc, const tstring &table);
	sqlreturn LWODBC_API get_table_information(table_information &rc, const tstring &table, const tstring &schema, const tstring &catalog);

	const sqlreturn &get_type_information(sql_type_info_result_set &r, SQLSMALLINT DataType);
	const sqlreturn &get_type_information(sql_type_info_result_set &r) { return get_type_information(r, SQL_ALL_TYPES); }
protected:
	sqlreturn bind_foreign_keys_result_set(sql_foreign_keys_result_set &rc);
	sqlreturn bind_by_position(const aggregate &ag);
	typedef SQLRETURN (SQL_API  *catalog_function_4)(SQLHSTMT, SQLTCHAR*, SQLSMALLINT, SQLTCHAR*, SQLSMALLINT, SQLTCHAR*, SQLSMALLINT, SQLTCHAR*, SQLSMALLINT);
	sqlreturn execute_special(catalog_function_4, const tstring &column, const tstring &table, const tstring &schema, const tstring &catalog);

	const sqlreturn &get_type_information(SQLSMALLINT DataType);
	const sqlreturn &get_type_information() { return get_type_information(SQL_ALL_TYPES); }
	const sqlreturn &bind_type_info_result_set(sql_type_info_result_set &r);
	//@}
};

};

};
#pragma warning(pop)

#endif

