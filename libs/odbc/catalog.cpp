/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: catalog.cpp,v 1.5 2006/10/10 14:13:10 Hajo Kirchhoff Exp $
*/

#include "stdafx.h"
#include <malloc.h>
#include <litwindow/logging.h>
#include "catalog.h"

#define new DEBUG_NEW

#pragma warning(push, 4)
//#pragma warning(disable: 4312 4267)

namespace litwindow {

namespace odbc {;

struct sql_columns_result_set
{
	tstring TABLE_CAT;
	tstring TABLE_SCHEM;
	tstring TABLE_NAME;
	tstring COLUMN_NAME;
	SQLSMALLINT	DATA_TYPE;
	tstring TYPE_NAME;
	SQLINTEGER COLUMN_SIZE;
	SQLINTEGER BUFFER_LENGTH;

	SQLSMALLINT DECIMAL_DIGITS;
	SQLSMALLINT NUM_PREC_RADIX;
	SQLSMALLINT NULLABLE;
	tstring	REMARKS;
	SQLINTEGER	CHAR_OCTET_LENGTH;
	SQLINTEGER ORDINAL_POSITION;
};

struct sql_primary_keys_result_set
{
	tstring TABLE_CAT;
	tstring TABLE_SCHEM;
	tstring TABLE_NAME;
	tstring COLUMN_NAME;
	SQLSMALLINT KEY_SEQ;
	tstring PK_NAME;
};

inline SQLTCHAR *to_sql_char_p(const tstring &v)
{
	return v.length()==0 ? 0 : reinterpret_cast<SQLTCHAR*>(const_cast<TCHAR*>(v.c_str()));
}

sqlreturn catalog::using_schema_catalog(const tstring &schema, const tstring &catalog, TCHAR * &use_schema, SQLSMALLINT &use_schema_length, TCHAR * &use_catalog, SQLSMALLINT &use_catalog_length)
{
	tstring const *which_schema;
	tstring const *which_catalog;
	tstring empty;
	if (is_default(schema)) {
		which_schema=&null_string;
	} else {
		which_schema=&schema;
	}
	if (is_default(catalog)) {
		which_catalog=&null_string;
	} else {
		which_catalog=&catalog;
	}
	use_schema=is_null(*which_schema) ? 0 : const_cast<TCHAR*>(which_schema->c_str());
	use_schema_length=use_schema==0 ? 0 : (SQLSMALLINT)which_schema->length();
	use_catalog=is_null(*which_catalog) ? 0 : const_cast<TCHAR*>(which_catalog->c_str());
	use_catalog_length=use_catalog==0 ? 0 : (SQLSMALLINT)which_catalog->length();
	if (get_connection().has_schema()==false) {
		use_schema=use_catalog=0;
		use_schema_length=use_catalog_length=0;
		if (is_null(*which_schema)==false && which_schema->length()>0  ||  is_null(*which_catalog)==false && which_catalog->length()>0) {
			return sqlreturn(_("catalog::columns: Catalog/Schema is not supported by this DBMS"), odbc::err_not_supported_by_dbms);
		}
	}
	return sqlreturn(SQL_SUCCESS);
}

sqlreturn catalog::execute_special(catalog_function_4 f, const tstring &columns, const tstring &table, const tstring &schema, const tstring &catalog)
{
	clear();
	set_attr(SQL_ATTR_METADATA_ID, SQL_FALSE);
	if (last_error().ok() || last_error().optional_feature_not_implemented()) {
		TCHAR *uschema, *ucatalog;
		SQLSMALLINT uschema_length, ucatalog_length;
		if (using_schema_catalog(schema, catalog, uschema, uschema_length, ucatalog, ucatalog_length).ok()) {
			m_last_error=(*f)(handle(), (SQLTCHAR*)ucatalog, ucatalog_length,
				(SQLTCHAR*)uschema, uschema_length,
				!is_any(table) ? (SQLTCHAR*)(table.c_str()) : 0, (SQLSMALLINT)table.length(), 
				!is_any(columns) ? (SQLTCHAR*)(columns.c_str()) : 0, (SQLSMALLINT)columns.length());
		}
	}
	if (m_last_error.ok())
		m_state=statement::executed;
	return m_last_error;
}

sqlreturn catalog::execute_tables(sql_tables_result_set &rc)
{
	m_last_error=SQLTables(handle(), NULL, 0, NULL, 0, NULL, 0, NULL, 0);
	if (m_last_error) {
		m_state=statement::executed;
		bind(1, rc.TABLE_CAT) && bind(2, rc.TABLE_SCHEM) && bind(3, rc.TABLE_NAME) && bind(4, rc.TABLE_TYPE);
		if (m_last_error.fail())
			m_last_error.append_diag(sqldiag(_T("LWODB"), err_logic_error, _("error while binding result columns for SQLTables")));
	}
	return m_last_error;
}

sqlreturn catalog::execute_table_privileges(sql_table_privileges_result_set &r, const tstring &table, const tstring &schema, const tstring &catalog)
{
	TCHAR *uschema, *ucatalog, *utable;
	SQLSMALLINT uschema_length, ucatalog_length, utable_length;
	if (using_schema_catalog(schema, catalog, uschema, uschema_length, ucatalog, ucatalog_length).fail())
		return m_last_error;
	if (is_any(table)) {
		utable=0; utable_length=0;
	} else {
		utable=const_cast<TCHAR*>(table.c_str());
		utable_length=(SQLSMALLINT)table.length();
	}
	clear();
	SQLUINTEGER schema_views;
	if (get_connection().get_info(SQL_INFO_SCHEMA_VIEWS, schema_views) && (schema_views & SQL_ISV_TABLE_PRIVILEGES)) {
		schema_views=0;
	}
	set_attr(SQL_ATTR_METADATA_ID, SQL_FALSE);
	if (m_last_error.fail() && m_last_error.optional_feature_not_implemented()==false) {
		return m_last_error;
	}
	m_last_error=SQLTablePrivileges(handle(), (SQLTCHAR*)ucatalog, ucatalog_length, (SQLTCHAR*)uschema, uschema_length, (SQLTCHAR*)utable, utable_length);
	if (m_last_error) {
		m_state=statement::executed;
		bind(1, r.TABLE_CAT) && bind(2, r.TABLE_SCHEM) && bind(3, r.TABLE_NAME) && bind(4, r.GRANTOR) && bind(5, r.GRANTEE) && bind(6, r.PRIVILEGE) && bind(7, r.IS_GRANTABLE);
		if (m_last_error.fail())
			m_last_error.append_diag(sqldiag(_T("LWODB"), err_logic_error, _("error while binding result columns for SQLTablePrivileges")));
	}
	return m_last_error;
}

const tstring catalog::null;

sqlreturn catalog::execute_foreign_keys(sql_foreign_keys_result_set &r, const tstring &pk_catalog, const tstring &pk_schema, const tstring &pk_table,
												const tstring &fk_catalog, const tstring &fk_schema, const tstring &fk_table)
{
#define CATALOG_STRING_PARAMETER(p) (&p==&null ? NULL : to_sql_char_p(p)), (&p==&null ? 0 : (SQLSMALLINT)p.length())
	m_last_error=SQLForeignKeys(handle(), 
			CATALOG_STRING_PARAMETER(pk_catalog),
			CATALOG_STRING_PARAMETER(pk_schema),
			CATALOG_STRING_PARAMETER(pk_table),
			CATALOG_STRING_PARAMETER(fk_catalog),
			CATALOG_STRING_PARAMETER(fk_schema),
			CATALOG_STRING_PARAMETER(fk_table)
			);
	if (m_last_error) {
		m_state=statement::executed;
		m_last_error=bind_foreign_keys_result_set(r);
	}
	return m_last_error;
}

const sqlreturn &catalog::get_type_information(SQLSMALLINT DataType)
{
	m_last_error=SQLGetTypeInfo(handle(), DataType);
	if (m_last_error) {
		m_state=statement::executed;
	}
	return m_last_error;
}

const sqlreturn &catalog::get_type_information(sql_type_info_result_set &r, SQLSMALLINT DataType)
{
	if (get_type_information(DataType)) {
		bind_type_info_result_set(r);
	}
	return m_last_error;
}

const sqlreturn &catalog::bind_type_info_result_set(sql_type_info_result_set &r)
{
	m_last_error=bind_by_position(make_aggregate(r));
	return m_last_error;
}

sqlreturn catalog::bind_foreign_keys_result_set(sql_foreign_keys_result_set &r)
{
	m_last_error=bind_by_position(make_aggregate(r));
	return m_last_error;
}

sqlreturn catalog::bind_by_position(const aggregate &ag)
{
	SQLSMALLINT pos=0;
	for (aggregate::iterator i=ag.begin(); i!=ag.end() && m_last_error; ++i) {
		m_last_error=bind_column(++pos, *i);
	}
	return m_last_error;
}

sqlreturn catalog::execute_column_privileges(sql_column_privileges_result_set &r, const tstring &column, const tstring &table, const tstring &schema, const tstring &catalog)
{
	TCHAR *uschema, *ucatalog, *utable, *ucolumn;
	SQLSMALLINT uschema_length, ucatalog_length, utable_length, ucolumn_length;
	if (using_schema_catalog(schema, catalog, uschema, uschema_length, ucatalog, ucatalog_length).fail())
		return m_last_error;
	if (is_any(table)) {
		utable=0; utable_length=0;
	} else {
		utable=const_cast<TCHAR*>(table.c_str());
		utable_length=(SQLSMALLINT)table.length();
	}
	if (is_any(column)) {
		ucolumn=0; ucolumn_length=0;
	} else {
		ucolumn=const_cast<TCHAR*>(column.c_str());
		ucolumn_length=(SQLSMALLINT)column.length();
	}
	clear();
	SQLUINTEGER schema_views;
	if (get_connection().get_info(SQL_INFO_SCHEMA_VIEWS, schema_views) && (schema_views & SQL_INFO_SCHEMA_VIEWS)) {
		schema_views=0;
	}
	set_attr(SQL_ATTR_METADATA_ID, SQL_FALSE);
	if (m_last_error.fail() && m_last_error.optional_feature_not_implemented()==false) {
		return m_last_error;
	}
	m_last_error=SQLColumnPrivileges(handle(), (SQLTCHAR*)ucatalog, ucatalog_length, (SQLTCHAR*)uschema, uschema_length, (SQLTCHAR*)utable, utable_length, (SQLTCHAR*)ucolumn, ucolumn_length);
	if (m_last_error) {
		m_state=statement::executed;
		bind(1, r.TABLE_CAT) && bind(2, r.TABLE_SCHEM) && bind(3, r.TABLE_NAME) && bind(4, r.COLUMN_NAME) && bind(5, r.GRANTOR) && bind(6, r.GRANTEE) && bind(7, r.PRIVILEGE) && bind(8, r.IS_GRANTABLE);
		if (m_last_error.fail())
			m_last_error.append_diag(sqldiag(_T("LWODB"), err_logic_error, _("error while binding result columns for SQLColumnPrivileges")));
	}
	return m_last_error;
}

bool catalog::has_table_privileges(const tstring &grantee, int privilege_bitmask, const tstring &table, const tstring &schema, const tstring &catalog)
{
	sql_table_privileges_result_set rset;
	if (execute_table_privileges(rset, table, schema, catalog).fail())
		return false;
	int privileges_set=0;
	while (fetch()) {
		if (grantee==rset.GRANTEE) {
			static TCHAR *privs[]={_T("SELECT"), _T("UPDATE"), _T("DELETE"), _T("INSERT"), _T("REFERENCES")};
			size_t i;
			int mask=1;
			for (i=0; i<sizeof(privs)/sizeof(*privs) && ((privilege_bitmask & mask)==0 || rset.PRIVILEGE!=privs[i]); ++i, mask<<=1)
				;
			privileges_set|=mask;
			if ((privileges_set & privilege_bitmask) == privilege_bitmask)
				return true;	// all required privileges have been found
		}
	}
	return false;
}

sqlreturn catalog::get_table_information(table_information &rc, const tstring &table)
{
	table_name name(table);
	return get_table_information(rc, name.table(), name.schema(), name.catalog());
}
sqlreturn catalog::get_table_information(table_information &rc, const tstring &table, const tstring &schema, const tstring &catalog)
{
	TCHAR *uschema, *ucatalog, *utable;
	SQLSMALLINT uschema_length, ucatalog_length, utable_length;
	if (using_schema_catalog(schema, catalog, uschema, uschema_length, ucatalog, ucatalog_length).log_errors())
		return m_last_error;
	if (is_any(table)) {
		utable=0; utable_length=0;
	} else {
		utable=const_cast<TCHAR*>(table.c_str());
		utable_length=(SQLSMALLINT)table.length();
	}
	clear();
	m_last_error.ignore_once(_T("HYC00"));
	set_attr(SQL_ATTR_METADATA_ID, SQL_FALSE);
	if (last_error().ok()==false && last_error().optional_feature_not_implemented()==false) {
		return m_last_error;
	}
	//TODO: escape search pattern '_' and '%' and the like in table and schema name!!!! See ms-help://MS.VSCC.2003/MS.MSDNQTR.2003FEB.1033/odbc/htm/odch07pr_6.htm
	m_last_error=SQLColumns(handle(), (SQLTCHAR*)ucatalog, ucatalog_length, (SQLTCHAR*)uschema, uschema_length, (SQLTCHAR*)utable, utable_length, 0, 0);
	if (m_last_error.ok()) {
		m_state=statement::executed;
		sql_columns_result_set result;
		// don't 'bind' as usual. The column names returned by this statement differ from ODBC 2.x to 3.x and are not translated by the driver manager
		// use bind by ordinal instead
		//bind(result);
		if (
			(bind(1, result.TABLE_CAT) && bind(2, result.TABLE_SCHEM) && bind(3, result.TABLE_NAME) &&
			bind(4, result.COLUMN_NAME) && bind(5, result.DATA_TYPE) && bind(6, result.TYPE_NAME) &&
			bind(7, result.COLUMN_SIZE) && 
			bind(8, result.BUFFER_LENGTH) &&
			bind(9, result.DECIMAL_DIGITS) &&
			bind(10, result.NUM_PREC_RADIX) &&
			bind(11, result.NULLABLE) &&
			bind(12, result.REMARKS) &&
			bind(16, result.CHAR_OCTET_LENGTH) &&
			bind(17, result.ORDINAL_POSITION))==false) 
		{
			m_last_error.append_diag(sqldiag(_T("LWODB"), odbc::err_logic_error, _("error while binding result columns from SQLColumns")));
			return m_last_error;
		}
		//bind_column(1, result.TABLE_SCHEM);
		SQLINTEGER row_count;
		get_row_count(row_count);
		rc.m_columns.reserve(row_count > 0 ? row_count+1 : 32);
		row_count=0;
		//lw_log() << _T("Table information: ") << endl;
		while (fetch().ok()) {
			++row_count;
			full_column_information d;
			d.m_sql_type=result.DATA_TYPE;
			d.m_column_size=result.COLUMN_SIZE;
			d.m_octet_length=result.BUFFER_LENGTH;
			d.m_unsigned = SQL_FALSE;						//TODO: determine actual signed/unsigned value
			d.m_decimal=result.NUM_PREC_RADIX;
			d.m_nullable=result.NULLABLE;
			d.m_remarks=result.REMARKS;
			d.m_name=result.COLUMN_NAME;
			// ODBC 2.x drivers do not return ORDINAL_POSITION
			// simulate this here with the 'row_count' value
			d.m_position=is_column_null(17) ? (SQLSMALLINT)row_count : (SQLSMALLINT)result.ORDINAL_POSITION;
			// ODBC 2.x drivers also do not return CHAR_OCTET_LENGTH and others
			d.m_char_octet_length= is_column_null(16) ? -1 : result.CHAR_OCTET_LENGTH;
			d.m_table=is_column_null(3) ? null_string : result.TABLE_NAME;
			d.m_schema=is_column_null(2) ? null_string :  result.TABLE_SCHEM;
			d.m_catalog=is_column_null(1) ? null_string : result.TABLE_CAT;

			if ((SQLSMALLINT)rc.m_columns.size()<d.m_position+1)
				rc.m_columns.resize(d.m_position+1);
			rc.m_columns[d.m_position]=d;
			rc.m_column_index.add(d.m_name, scope(d.m_table, d.m_schema, d.m_catalog), d.m_position);
			//lw_log() << row_count << _T(": ") << as_debug(result) << endl;
		}
		if (rc.m_columns.size()>0 && m_last_error.no_data()) {
			// now get primary keys
			clear();
			m_last_error.ignore_once(_T("IM001"));
			m_last_error=SQLPrimaryKeys(handle(), (SQLTCHAR*)ucatalog, ucatalog_length, (SQLTCHAR*)uschema, uschema_length, (SQLTCHAR*)utable, utable_length);
			m_state=statement::executed;
			if (last_error().ok()) {
				rc.m_primary_keys_valid=true;
				sql_primary_keys_result_set result;
				//bind(result);
				if ((bind(1, result.TABLE_CAT).ok() && bind(2, result.TABLE_SCHEM).ok() && bind(3, result.TABLE_NAME).ok() &&
					bind(4, result.COLUMN_NAME).ok() && bind(5, result.KEY_SEQ).ok() && bind(6, result.PK_NAME).ok())==false) 
				{
					m_last_error.append_diag(sqldiag(_T("LWODB"), odbc::err_logic_error, _("error while binding result columns from SQLPrimaryKeys")));
					return m_last_error;
				}
				while (fetch().ok()) {
					scope sc(result.TABLE_NAME);
					sc.schema= is_column_null(2) ? null_string : result.TABLE_SCHEM;
					sc.catalog= is_column_null(1) ? null_string : result.TABLE_CAT;
					SQLSMALLINT pos=rc.m_column_index.find(result.COLUMN_NAME, sc, -1);
					if (pos==-1) {
						if (is_column_null(2))
							sc.schema=any_string;
						if (is_column_null(1))
							sc.catalog=any_string;
						pos=rc.m_column_index.find(result.COLUMN_NAME, sc, -1);
					}
					if (pos==-1)
						return sqlreturn(_("FATAL: SQLPrimaryKeys returns column that was not also returned by SQLColumns"), odbc::err_unknown_error);
					const tstring pk_name = is_column_null(6) ? null_string : result.PK_NAME;
					const tstring *p=& *rc.m_pk_names.insert(pk_name).first;
					rc.m_columns[pos].m_pk_key_seq=result.KEY_SEQ;
					rc.m_columns[pos].m_pk_name=p->c_str();
				}
				m_last_error.clear();
			} else if (last_error().is_state(_T("IM001"))) {
				rc.m_primary_keys_valid=false;
				// Driver does not support this function. Does not matter. Fail silently.
				m_last_error.clear();
			}
			// now get foreign keys
			if (last_error().ok()) {

			}
		}
	}
	return m_last_error;
}

};

};

LWL_BEGIN_AGGREGATE(litwindow::odbc::sql_columns_result_set)
PROP(TABLE_CAT)
PROP(TABLE_SCHEM)
PROP(TABLE_NAME)
PROP(COLUMN_NAME)
PROP(DATA_TYPE)
PROP(TYPE_NAME)
PROP(COLUMN_SIZE)
PROP(DECIMAL_DIGITS)
PROP(NUM_PREC_RADIX)
PROP(NULLABLE)
PROP(REMARKS)
PROP(ORDINAL_POSITION)
LWL_END_AGGREGATE()

LWL_BEGIN_AGGREGATE(litwindow::odbc::sql_primary_keys_result_set)
PROP(TABLE_CAT)
PROP(TABLE_SCHEM)
PROP(TABLE_NAME)
PROP(COLUMN_NAME)
PROP(KEY_SEQ)
PROP(PK_NAME)
LWL_END_AGGREGATE()

LWL_BEGIN_AGGREGATE(litwindow::odbc::sql_column_privileges_result_set)
PROP(TABLE_CAT)
PROP(TABLE_SCHEM)
PROP(TABLE_NAME)
PROP(COLUMN_NAME)
PROP(GRANTOR)
PROP(GRANTEE)
PROP(PRIVILEGE)
PROP(IS_GRANTABLE)
LWL_END_AGGREGATE()

LWL_BEGIN_AGGREGATE(litwindow::odbc::sql_foreign_keys_result_set)
	PROP(PKTABLE_CAT)
	PROP(PKTABLE_SCHEM)
	PROP(PKTABLE_NAME)
	PROP(PKTABLE_COLUMN)
	PROP(FKTABLE_CAT)
	PROP(FKTABLE_SCHEM)
	PROP(FKTABLE_NAME)
	PROP(FKTABLE_COLUMN)
	PROP(KEY_SEQ)
	PROP(UPDATE_RULE)
	PROP(DELETE_RULE)
	PROP(FK_NAME)
	PROP(PK_NAME)
	PROP(DEFERRABILITY)
LWL_END_AGGREGATE()
