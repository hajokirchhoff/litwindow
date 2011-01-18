/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: statement.cpp,v 1.17 2007/10/23 11:25:13 Hajo Kirchhoff Exp $
*/

#include "stdafx.h"
#include <malloc.h>
#include <litwindow/logging.h>
#include <boost/bind.hpp>
#include "statement.h"
#include "litwindow/check.hpp"

#define new DEBUG_NEW

#pragma warning(disable: 4312 4267)

namespace litwindow {

namespace odbc {;

tstring statement::get_macro_value(const tstring &name) const
{
	if (m_macros.find(name)!=m_macros.end())
		return m_macros[name];
	return m_the_connection ? m_the_connection->get_macro_value(name) : tstring();
}

const sqlreturn &statement::update_row()
{
	return m_last_error=SQLSetPos(handle(), 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE);
}

const sqlreturn &statement::delete_row()
{
	return m_last_error=SQLSetPos(handle(), 1, SQL_DELETE, SQL_LOCK_NO_CHANGE);
}

const sqlreturn &statement::insert_row()
{
	m_last_error.clear();
	(needs_bind_columns()==false || bind_columns()) &&
	(needs_put_columns()==false || put_columns()) &&
		(m_last_error=SQLBulkOperations(handle(), SQL_ADD));
	return m_last_error;
}

SQLSMALLINT statement::find_column(const tstring &name) const
{
	column_index_t::const_iterator i=m_column_index.find(name);
	if (i==m_column_index.end()) {
		tstring upper_name(name);
		i=m_column_index.find(litwindow::toupper(upper_name));
	}
	return i==m_column_index.end() ? -1 : i->second;
}

sqlreturn statement::describe_column(SQLSMALLINT col, column_descriptor &d)
{
	SQLTCHAR buffer[512];
	SQLTCHAR *name_buffer=buffer;
	SQLSMALLINT length;
	m_last_error=SQLDescribeCol(handle(), col, name_buffer, sizeof(buffer), &length, &d.m_sql_type, &d.m_column_size, &d.m_decimal, &d.m_nullable);
	if (m_last_error==SQL_SUCCESS_WITH_INFO && m_last_error.is_state(_T("01004"))) {
		name_buffer=(SQLTCHAR*)_alloca(length+sizeof(TCHAR));
		m_last_error=SQLDescribeCol(handle(), col, name_buffer, length+sizeof(TCHAR), &length, &d.m_sql_type, &d.m_column_size, &d.m_decimal, &d.m_nullable);
	}
	if (m_last_error.success())
		d.m_name=(TCHAR*)name_buffer;
	d.m_position=col;
	get_column_attr(col, SQL_DESC_CASE_SENSITIVE, d.m_case_sensitive).ok() &&
		get_column_attr(col, SQL_DESC_AUTO_UNIQUE_VALUE, d.m_auto_unique_value).ok() &&
		get_column_attr(col, SQL_DESC_UPDATABLE, d.m_updatable).ok() &&
		get_column_attr(col, SQL_DESC_SEARCHABLE, d.m_searchable).ok() &&
		get_column_attr(col, SQL_DESC_OCTET_LENGTH, d.m_octet_length) &&
		get_column_attr(col, SQL_DESC_UNSIGNED, d.m_unsigned);
	if (m_last_error.ok()==false)
		return m_last_error;
	return m_last_error;
}

/** Constructs a statement that uses the connection @p c. The connection must be open. */
statement::statement(connection &c)
{
	init(c);
}
/** Constructs a statement that uses the shared_connection @p c. The connection must be open. */
statement::statement(shared_connection &c)
{
	m_reference_counting_for_connection_pool=c;
	init(*c);
}
/** Constructs a statement that uses connection @p c. The connection must be open. Sets the SQL statement
to @p sql_statement */
statement::statement(const tstring &sql_statement, connection &c)
{
	init(c, sql_statement);
}

/** Constructs a statement that uses the connection named @p named_connection from the connection pool and sets the SQL statement
to @p sql_statement.
\note You must set the DSN or connection string for the @p named_connection before you construct a statement, calling
\code
connection::pool().set(dsn, uid, pwd, named_connection);
\endcode
or
\code
connection::pool().set(connection_string, named_connection);
\endcode
*/
statement::statement(const tstring &sql_statement, const tstring &named_connection)
:m_reference_counting_for_connection_pool(connection::pool().get(named_connection))
{
	init(*m_reference_counting_for_connection_pool, sql_statement);
}

/** This is the easiest way to use the statement class. The statement executes the SQL statement passed as a parameter and uses
the default connection in the connection pool. 
\note You must set the DSN or connection string for the default connection @b before you construct a statement. To do this, you'll
usually call either
\code
connection::pool().set(dsn, uid, pwd);
\endcode
or
\code
connection::pool().set(connection_string);
\endcode
usually as the first command in the startup sequence or after a login dialog of your application.
Example:
\code
struct invoices;    // your C++ structure mirroring the table 'invoices'
using litwindow::odbc;
void main()
{
// set the default connection
connection::pool().set("Some_DSN", "A_User_Name", "The_Password");

statement s("SELECT * FROM invoices");
invoices i;
s.bind(i);
s.execute();
while (s.fetch().ok()) {
printf("Invoices: %d" i.member);
}
}
*/
statement::statement(const tstring &sql_statement)
:m_reference_counting_for_connection_pool(connection::pool().open())
{
	init(*m_reference_counting_for_connection_pool, sql_statement);
}

statement::~statement()
{
	set_throw_on_error(false);	// important! Otherwise we might end up with a double-throw/unexpected exception
	free_handle();
}

void statement::set_connection(connection &c)
{
	clear();
	free_handle();
	m_reference_counting_for_connection_pool.reset();
	init(c, tstring());
}

void statement::set_connection(shared_connection &c)
{
	clear();
	free_handle();
	m_reference_counting_for_connection_pool=c;
	init(*m_reference_counting_for_connection_pool, tstring());
}

void statement::clear_connection()
{
	clear();
	free_handle();
	m_reference_counting_for_connection_pool.reset();
	init();
}

void statement::init(connection &c, const tstring &sql_statement)
{
	init();
	m_the_connection=&c;
	set_throw_on_error(c.m_throw_on_error_default);
	allocate_handle(c);
	if (sql_statement.length()>0)
		set_statement(sql_statement);
}

void statement::init()
{
	m_state=uninitialised;
	m_emulate_optimistic_concurrency=false;
	m_handle=0;
	m_sql_statement.clear();
	m_last_error.clear();
	m_is_prepared=false;
	m_state=uninitialised;
}

const sqlreturn &statement::get_cursor_name(tstring &cursor_name)
{
	TCHAR buffer[256];
	SQLSMALLINT length;
	m_last_error=SQLGetCursorName(handle(), (SQLTCHAR*)buffer, sizeof(buffer), &length);
	if (m_last_error.is_state(_T("01004"))) {
		TCHAR *b=(TCHAR*)_alloca(length+sizeof(TCHAR));
		m_last_error=SQLGetCursorName(handle(), (SQLTCHAR*)b, length+sizeof(TCHAR), &length);
		cursor_name=b;
	} else
		cursor_name=buffer;
	return m_last_error;
}

const sqlreturn &statement::set_concurrency(concurrency_enum cy)
{
	m_last_error=set_attr(SQL_ATTR_CONCURRENCY, cy);
	if (m_last_error==SQL_SUCCESS) {
		// make sure the driver did actually set the value. MySQL ODBC driver for one is broken.
		SQLUINTEGER value;
		get_attr(SQL_ATTR_CONCURRENCY, value);	// will change m_last_error !!!!
		if (m_last_error!=SQL_SUCCESS || value!=cy) {
			lw_err() << _("Broken ODBC driver changes SQL_ATTR_CONCURRENCY option without setting proper return code! ") << get_connection().get_dbms()->get_dbms_name() << endl;
			m_last_error=SQL_SUCCESS_WITH_INFO;
			m_last_error.append_diag(sqldiag(_T("01S02"), err_dbms_specific_error, _("Option value changed")));
		}
	}
	return m_last_error;
}

const sqlreturn &statement::get_concurrency(concurrency_enum &cy)
{
	SQLUINTEGER value;
	m_last_error=get_attr(SQL_ATTR_CONCURRENCY, value);
	cy=(concurrency_enum)value;
	return m_last_error;
}

const sqlreturn &statement::set_optimistic_locking()
{
	concurrency_enum actual;
	m_emulate_optimistic_concurrency=true;
	{
		throwing t_(*this, false, false);
		set_concurrency(rowver);
		if (m_last_error.ok()==false || m_last_error.option_value_changed()) {
			get_concurrency(actual);
			if (actual!=values) {
				set_concurrency(values);
				if (m_last_error.ok()==false || m_last_error.option_value_changed()) {
					get_concurrency(actual);
					set_no_locking();
					m_emulate_optimistic_concurrency=true;
					if (m_last_error.ok()==false)
						m_last_error=SQL_SUCCESS;
					// else leave 'option_value_changed'
				}
			}
		}
	}
	if (m_last_error.is_throw_on_error() && m_last_error==SQL_ERROR)
		throw m_last_error;
	return m_last_error;
}

const sqlreturn &statement::set_updatable(concurrency_enum cy)
{
	if (cy==optimistic)
		return set_optimistic_locking();
	if (cy==pessimistic)
		return set_pessimistic_locking();
	return set_concurrency(cy);
}

bool statement::is_updatable()
{
	concurrency_enum actual;
	return get_concurrency(actual) && (actual!=statement::none || m_emulate_optimistic_concurrency);
}

bool statement::is_scrollable()
{
	cursor_type_enum c;
	throwing t_(*this, false);
	return get_cursor_type(c) && c!=statement::forward_only_cursor;
}

const sqlreturn &statement::get_data_as_string(SQLUSMALLINT col, tstring &rc, SQLLEN *len_ind_p/* =0 */)
{
	rc.clear();
	SQLLEN len;
	TCHAR *buffer=0;
	if (get_data(col, SQL_C_TCHAR, buffer, 0, &len).fail()) {
		return m_last_error;
	}
	if (len_ind_p)
		*len_ind_p=len;
	if (len!=SQL_NULL_DATA) {
		if (len>=0x10000) {
			lw_err() << _("E92001: column length too large for get_data_as_string - truncated at 0x10000") << endl;
			len=0x10000;
		}
		buffer=(TCHAR*)_alloca(len*sizeof(TCHAR));
		if (get_data(col, SQL_C_TCHAR, buffer, len, len_ind_p))
			rc=tstring(buffer, len);
	}
	return m_last_error;
}

sqlreturn statement::get_scroll_options(SQLUINTEGER &options) const
{
	return get_connection().get_info(SQL_SCROLL_OPTIONS, options);
}

bool statement::supports_cursor(cursor_type_enum ct) const
{
	SQLUINTEGER value;
	bool rc=false;
	if (get_scroll_options(value)) {
		switch (ct) {
			case forward_only_cursor: rc=(value & so_forward_only)!=0; break;
			case dynamic_cursor: rc=(value & so_dynamic)!=0; break;
			case keyset_driven_cursor: rc=(value & so_keyset_driven)!=0; break;
			case static_cursor: rc=(value & so_static)!=0; break;
		}
	}
	return rc;
}

const sqlreturn &statement::set_scrollable(bool use_scrolling_cursor)
{
	{
		throwing t_(*this, false, false);
		m_last_error=set_attr(SQL_ATTR_CURSOR_SCROLLABLE, use_scrolling_cursor ? SQL_SCROLLABLE : SQL_NONSCROLLABLE);
		if (m_last_error.ok()==false && (m_last_error.is_state(_T("HY092")) || m_last_error.is_state(_T("HYC00")))) {
			// option not supported for this driver. simply choose a scrollable cursor instead
			m_last_error=set_cursor_type(keyset_driven_cursor);
		}
	}
	if (m_last_error.is_throw_on_error() && m_last_error==SQL_ERROR)
		throw m_last_error;
	return m_last_error;
}

sqlreturn statement::allocate_handle(const connection &c)
{
	if (handle()!=0)
		throw std::logic_error("must free odbc::statement::m_handle before allocating a new handle!");
	if (c.is_open()==false)
		throw std::logic_error("connection must be opened before allocating a new statement::handle");
	m_last_error=SQLAllocHandle(SQL_HANDLE_STMT, c.handle(), &m_handle);
	if (m_last_error.success())
		m_last_error.set_handles(SQL_HANDLE_STMT, handle());
	m_identifier_quote_char=c.m_identifier_quote_char;
	return m_last_error;
}

sqlreturn statement::free_handle()
{
	if (handle()!=0) {
		m_last_error.zero_handles();
		m_last_error=SQLCancel(handle());
		m_last_error=SQLFreeHandle(SQL_HANDLE_STMT, handle());
		m_handle=0;
		m_last_error.log_errors();
	}
	return m_last_error;
}

sqlreturn statement::set_statement(const tstring &sql_statement)
{
	close_cursor();
	clear_result_set();
	m_sql_statement=sql_statement; 
	m_last_error.clear();
	m_is_prepared=false;
	m_state=statement_set;
	parse_bindings();
	return m_last_error; 
}

void statement::clear_result_set()
{
	reset_column_bindings();
	clear_column_descriptors();
	m_binder.reset_column_bindings();
}

//const sqlreturn &statement::get_parameter_descriptors()
//{
//	if (has_been_prepared() || prepare()) {
//		SQLSMALLINT param_count;
//		m_last_error=SQLNumParams(handle(), &param_count);
//		if (m_last_error.fail()) return m_last_error;
//		SQLSMALLINT i;
//		m_parameter_list.resize(param_count+1);
//		for (i=1; i<=param_count && m_last_error; ++i) {
//			column_descriptor &c(m_parameter_list[i]);
//			m_last_error=SQLDescribeParam(handle(), i, &c.m_sql_type, &c.m_column_size, &c.m_decimal, &c.m_nullable);
//			SQLUINTEGER sz=c.m_column_size;
//		}
//	}
//	return m_last_error;
//}

const sqlreturn &statement::prepare()
{
	m_last_error=SQLPrepare(handle(), (SQLTCHAR*)m_sql_statement.c_str(), m_sql_statement.length());
	if (m_last_error)
		m_is_prepared=true;
	return m_last_error;
}

const sqlreturn &statement::execute()
{
	litwindow::context_t c("statement::execute");
	if (m_state==setting_statement) {
		if (set_statement(m_sql_statement).log_errors())
			return m_last_error;
	}

    // do NOT use this c["SQL_STATEMENT"] here, its verrrrrrry slow
	//c["SQL_STATEMENT"]=t2string(m_sql_statement);
	if (close_cursor().log_errors())
		return m_last_error;
	if (bind_parameters().fail() || put_parameters().fail())
		return m_last_error;

	/// some drivers, notably postgres, do not accept SQL_NTS in the len_ind field for character strings
	/// for these drivers, calculate the actual string length
	m_binder.fix_SQL_NTS_len_ind_parameter_workaround(binder::set_length);
	if (m_last_error) {
		if (m_is_prepared)
			m_last_error=SQLExecute(handle());
		else {
			m_last_error=SQLExecDirect(handle(), (SQLTCHAR*)m_sql_statement.c_str(), m_sql_statement.length());
		}
	}
	/// reset the len_ind field to SQL_NTS for all parameters where the actual length string was calculated as a workaround
	m_binder.fix_SQL_NTS_len_ind_parameter_workaround(binder::reset_length);

	if (m_last_error) {
		if (m_binder.needs_get_parameters()) {
			sqlreturn rc=m_binder.do_get_parameters(*this);	// do NOT overwrite m_last_error, because SQLExecDirect might return SQL_SUCCESS_WITH_INFO
			if (rc.log_errors())
				return m_last_error=rc;
		}
	}
	if (m_last_error || m_last_error.no_data())
		m_state=executed;
	if (m_last_error)	// if no_data() then there are no column_descriptors, so do not get them
		get_column_descriptors();
	return m_last_error;
}

SQLUINTEGER statement::current_cursor_attributes1()
{
	cursor_type_enum current_cursor;
	throwing t_(*this, false);
	return get_cursor_type(current_cursor) ? get_connection().get_cursor_attributes1(current_cursor) : 0;
}

sqlreturn statement::clear()
{
	if (handle()!=SQL_NULL_HANDLE) {
		close_cursor();
		clear_result_set();
		unbind().log_errors();
	}
	m_parameter_marker.clear();
	m_column_marker.clear();
	m_state=closed;
	m_is_prepared=false;
	m_continuous_sql_binder.clear();
	m_sql_statement.clear();
	return m_last_error;
}

const sqlreturn &statement::set_attr(SQLINTEGER attribute, SQLUINTEGER value)
{
	return m_last_error=SQLSetStmtAttr(handle(), attribute, (SQLPOINTER)value, 0);
}

const sqlreturn &statement::get_attr(SQLINTEGER attribute, SQLUINTEGER &value)
{
	return m_last_error=SQLGetStmtAttr(handle(), attribute, (SQLPOINTER)&value, SQL_IS_UINTEGER, 0);
}

sqlreturn statement::get_row_count(SQLINTEGER &rcount)
{
	rcount=-1;
	return m_last_error=SQLRowCount(handle(), &rcount);
}

const sqlreturn &statement::bind_columns()
{
	get_column_descriptors();
	if (m_last_error)
		m_last_error=m_binder.do_bind_columns(*this);
	return m_last_error;
}

const sqlreturn &statement::bind_parameters()
{
	if (needs_bind_parameters()) {
		reset_parameter_bindings();
		m_last_error=m_binder.do_bind_parameters(*this);
	} else
		m_last_error.clear();
	return m_last_error;
}

const sqlreturn &statement::put_parameters()
{
	if (needs_put_parameters()) {
		m_last_error=m_binder.do_put_parameters(*this);
	} else
		m_last_error.clear();
	return m_last_error;
}

sqlreturn statement::do_bind_parameter( SQLUSMALLINT pposition, SQLSMALLINT in_out, SQLSMALLINT c_type, SQLSMALLINT sql_type, 
					      SQLULEN column_size, SQLSMALLINT decimal_digits, SQLPOINTER buffer, SQLLEN length, SQLLEN *len_ind)
{
	if (column_size==0) {
		column_size=length;
		if (c_type==SQL_C_CHAR && column_size>1)
			--column_size;
		else if (c_type==SQL_C_WCHAR) {
			column_size/=2;
			if (column_size>1)
				--column_size;
		}
	}
	m_last_error=SQLBindParameter(handle(), pposition, in_out, c_type, sql_type, column_size, decimal_digits, buffer, length, len_ind);
	return m_last_error;
}

const sqlreturn &statement::put_columns() 
{
	return m_last_error=m_binder.do_put_columns();
}

sqlreturn statement::fetch()
{
	prefetch() && (m_last_error=SQLFetch(handle())) && postfetch();
	return m_last_error;
}

const sqlreturn &statement::fetch_scroll(fetch_scroll_orientation f, SQLINTEGER offset)
{ 
	prefetch() && (m_last_error=SQLFetchScroll(handle(), (SQLSMALLINT)f, offset)) && postfetch();
	return m_last_error;
}

const sqlreturn &statement::set_parameter_state(SQLUSMALLINT col, SQLLEN len_ind)
{
	if (needs_bind_parameters()==false || bind_parameters().success())
		m_last_error=m_binder.set_parameter_state(col, len_ind);
	return m_last_error;
}

const sqlreturn &statement::set_column_state(SQLUSMALLINT col, SQLLEN len_ind)
{
	return m_last_error=m_binder.set_column_state(col, len_ind);
}

const sqlreturn &statement::reset_column_states(SQLINTEGER len_ind)
{
	(needs_bind_columns()==false || bind_columns()) && (m_last_error=m_binder.reset_column_states(len_ind));
	if (m_last_error) {
		size_t i;
		for (i=1; i<m_column_list.size(); ++i) {
			column_descriptor &d(m_column_list[i]);
			if (m_column_list[i].m_updatable==SQL_ATTR_READONLY)
				set_column_state(i, SQL_IGNORE);
		}
	}
	return m_last_error;
}
const sqlreturn &statement::reset_parameter_states(SQLINTEGER len_ind)
{
	return m_last_error=m_binder.reset_parameter_states(len_ind);
}

SQLLEN statement::get_column_length(SQLSMALLINT pos) const
{
	if (m_state!=statement::fetched) {
		throw logic_error("Must fetch data first!");
	}
	SQLLEN value;
	if (m_binder.get_column_length(pos, value).ok()==false)
		throw logic_error("No such column or column not bound");
	return value;
}

bool	statement::parse_bindings()
{
	return do_parse_bindings();
}

parameter no_such_parameter;

SQLSMALLINT statement::find_parameter(const tstring &name) const
{
	parameters_t::const_iterator i=find(m_parameter_marker.begin(), m_parameter_marker.end(), name);
	if (i==m_parameter_marker.end()) {

	}
	return i==m_parameter_marker.end() ? -1 : i->m_position;
}
const parameter *statement::get_parameter(SQLSMALLINT pos) const
{
	return pos<(SQLSMALLINT)m_parameter_marker.size() ? & m_parameter_marker[pos] : 0;
}

sqlreturn statement::bind_column(const tstring &name, accessor a, SQLLEN *len_ind)
{
	return m_last_error=m_binder.bind_column(name, a, len_ind);
}

sqlreturn statement::bind_column(SQLSMALLINT col, accessor a, SQLLEN *len_ind)
{
	return m_last_error=m_binder.bind_column(col, a, len_ind);
}

sqlreturn statement::bind_column(aggregate a)
{
	return m_last_error=m_binder.bind_column(a);
}

const sqlreturn &statement::do_get_column_descriptors()
{
	if (!has_been_prepared())
		return m_last_error=sqlreturn(_T("Result columns not available before statement has been prepared or executed."), odbc::err_result_column_description_not_available);
	int i; 
	SQLSMALLINT result_column_count;
	m_last_error=SQLNumResultCols(handle(), &result_column_count);
	if (m_last_error.log_errors())
		return m_last_error;
	clear_column_descriptors();
	m_column_list.resize(result_column_count+1);
	for (i=1; i<=result_column_count && m_last_error.log_errors()==false; ++i) {
		column_descriptor c;
		m_last_error=describe_column(i, c);
		m_column_list[i]=c;
		m_column_index[c.m_name]=i;
		if (!c.m_case_sensitive) {
			tstring ics_name=c.m_name;
			m_column_index[litwindow::toupper(ics_name)]=i;
			//tstring cs_name(_T("\""));
			//cs_name+=c.m_name;
			//cs_name+=_T('"');
			//m_column_index[cs_name]=i;
		}
	}
	for (i=0; i<(SQLSMALLINT)m_column_marker.size(); ++i) {
		m_column_index[m_column_marker[i].m_parameter_name]=m_column_marker[i].m_position;
	}
	return m_last_error;
}

const sqlreturn &statement::get_column_attr(SQLSMALLINT pos, SQLSMALLINT field, tstring &value)
{
	TCHAR buffer[512];
	TCHAR *b=buffer;
	SQLSMALLINT length;
	while (true) {
		m_last_error=SQLColAttribute(handle(), pos, field, buffer, sizeof(buffer), &length, 0);
		if (m_last_error==SQL_SUCCESS_WITH_INFO && m_last_error.is_state(_T("01004"))) {
			b=(TCHAR*)_alloca(length+sizeof(TCHAR));
		} else
			break;
	};
	if (m_last_error.ok()) {
		if (length==SQL_NULL_DATA)
			value=null_string;
		else
			value=buffer;
	}
	return m_last_error;
}

const sqlreturn &statement::get_column_attr(SQLSMALLINT pos, SQLSMALLINT field, bool &value)
{
	SQLINTEGER v=SQL_FALSE;
	m_last_error=get_column_attr(pos, field, v);
	if (m_last_error.ok())
		value= v==SQL_TRUE;
	return m_last_error;
}

const sqlreturn &statement::get_column_attr(SQLSMALLINT pos, SQLSMALLINT field, SQLINTEGER &value)
{
	return m_last_error=SQLColAttribute(handle(), pos, field, 0, 0, 0, &value);
}

sqlreturn statement::get_column_size(SQLSMALLINT pos, SQLUINTEGER &column_size) const
{
	sqlreturn rc=is_column_valid(pos);
	if (rc.ok())
		column_size=m_column_list[pos].m_column_size;
	return rc;
}

sqlreturn statement::get_column_descriptor(SQLSMALLINT pos, column_descriptor &d) const
{
	sqlreturn rc=is_column_valid(pos);
	if (rc.ok())
		d=m_column_list[pos];
	return rc;
}

bool statement::g_use_SQLSetPos=true;
bool statement::g_use_SQLBulkOperations=true;

statement &statement::operator <<(const TCHAR *stmt)
{
	if (m_state!=reset && m_state!=setting_statement && m_state!=closed) {
		clear();
		m_sql_statement.clear();
	}
	m_state=setting_statement;
	m_continuous_sql_binder.m_last_was_parameter=false;
	m_sql_statement.append(stmt);
	return *this;
}

void statement::add_accessor(const litwindow::accessor &a)
{
#ifdef _NOT
	m_state=setting_statement;
	if (m_continuous_sql_binder.m_last_was_parameter)
		m_sql_statement.append(1, _T(','));
	m_sql_statement.append(1, _T('?'));
#endif // _NOT

	if (m_continuous_sql_binder.m_last_bind_type==bindto)
		m_last_error=bind_column(m_continuous_sql_binder.m_next_column++, a);
	else
		m_last_error=bind_parameter_accessor(m_continuous_sql_binder.m_next_parameter++, m_continuous_sql_binder.m_last_bind_type, a, NULL);
	m_continuous_sql_binder.m_last_was_parameter=true;
}

void statement::add_const_accessor(const litwindow::const_accessor &a)
{
    AbortOn(m_continuous_sql_binder.m_last_bind_type!=odbc::in, "const data must use odbc::in binding.");
    add_accessor(const_cast_accessor(a));
}

sqlreturn statement::feed_column_to_target_parameter(SQLSMALLINT col_position, statement *target, SQLSMALLINT par_pos) const
{
	Precondition(m_binder.m_columns.size()>0, "columns not bound!");
	Precondition(col_position<(SQLSMALLINT)m_binder.m_columns.size(), "invalid col_position");
	const bind_task &current_bind_task(m_binder.m_columns.get_bind_task(col_position));
	bind_task p;
	p.m_by_position=par_pos;
	p.m_in_out=odbc::in;
	p.m_bind_info=current_bind_task.m_bind_info;
	p.m_bind_info.m_target_ptr=0;
	p.m_bind_info.m_len_ind_p=0;
	return target->bind_parameter(p);
}

};

};
