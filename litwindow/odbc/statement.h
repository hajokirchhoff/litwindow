/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: statement.h,v 1.26 2008/02/26 12:10:18 Merry\Hajo Kirchhoff Exp $
*/

#ifndef __LWODBC_STATEMENT_H
#define __LWODBC_STATEMENT_H

#include <litwindow/dataadapter.h>
#include <boost/utility.hpp>
#include "./lwodbc.h"
#include "./connection.h"
#include "./binder.h"
#include "./internals.h"
#include <list>

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

class my_symbols;

//-----------------------------------------------------------------------------------------------------------//

/// describes a single parameter of an SQL statement.
struct parameter {
	bind_type	m_bind_type;
	tstring		m_parameter_name;
	/// Position in the statement. Parameter(in,out,inout) and 'bindto' are counted seperately.
	SQLSMALLINT m_position;

	parameter(bind_type t, const tstring &name, SQLSMALLINT p)
		:m_bind_type(t), m_parameter_name(name), m_position(p)
	{}
	parameter():m_bind_type(unknown_bind_type),m_position(-1),m_parameter_name(_T("NOSUCHPARAMETERNAME__NOSUCHPARAMETERNAME__")) {}

	bool operator==(const tstring &name) const { return m_parameter_name==name; }
};

typedef vector<parameter> parameters_t;

extern parameter no_such_parameter;

/** SQL statement setter and binding helper class */
class sql_statement_setter:public boost::noncopyable
{
	friend class statement;
public:
	LWODBC_API sql_statement_setter(const TCHAR *sql_statement);

	sql_statement_setter LWODBC_API &bind(const accessor &a);
	sql_statement_setter LWODBC_API &as_sql(const TCHAR *statement);

	sql_statement_setter LWODBC_API &operator ,(const tstring &);
	template <typename Value>
	sql_statement_setter &operator << (Value &v)
	{
		return bind(make_accessor(v));
	}
	sql_statement_setter &operator << (const TCHAR *sql_statement)
	{
		return as_sql(sql_statement);
	}
private:
	void LWODBC_API set_and_bind(statement *to) const;
};

/** A @p statement binds parameters and columns to C++ variables, executes SQL statements and iterates over the result set.

Use this class to 
-	execute one or more SQL statements on a connection
-	bind parameters and columns to C++ variables
-	fetch results
-	query the properties of the columns returned by the statement

The typical order of events is
-#	Create and open a connection - or use connection::pool().set(...) to set the DSN and credentials for the default connection
-#	Instantiate a statement object, passing the connection and an SQL statement to the constructor
-#	Bind parameters to C++ variables
-#	Bind columns to C++ variables
-#	execute the statement
-#	fetch the results
-#	Optionally

*/
class statement:public boost::noncopyable {
public:
	///@name Constructors
	//@{
	/// Constructs an empty statement that is NOT initialised (has no handle allocated etc...). Use set_connection to initialize
	LWODBC_API		statement();
	/// Constructs a statement that uses connection @p c
	LWODBC_API explicit statement(connection &c);
	/// Constructs a statement that uses shared_connection @p c
	LWODBC_API explicit statement(shared_connection &c);
	/// Constructs a statement that uses connection @p c, executing @p sql_statement
	LWODBC_API          statement(const tstring &sql_statement, connection &c);
	/// Constructs a statement that use the @p named_connection from the connection::pool and @p sql_statement
	LWODBC_API          statement(const tstring &sql_statement, const tstring &named_connection);
	/// Constructs a statement that uses the default connection from the connection::pool and @p sql_statement
	LWODBC_API explicit statement(const tstring &sql_statement); // using "default" connection
	//@}

	LWODBC_API ~statement();

	/// for statements that have been constructed with the default constructor
	LWODBC_API void set_connection(connection &c);
	LWODBC_API void set_connection(shared_connection &c);
	/// disconnect the statement from a connection
	LWODBC_API void clear_connection();
    LWODBC_API void close() { clear_connection(); }

	statement LWODBC_API &operator <<(const TCHAR *stmt);
	statement LWODBC_API &operator <<(bind_type next_bind_type);
	template <typename Value>
	statement &operator << (Value &v)
	{
		add_accessor(make_accessor(v));
		return *this;
	}
    template <typename Value>
    statement &operator << (const Value &v)
    {
        add_const_accessor(make_const_accessor(v));
        return *this;
    }
	template <>
	statement &operator << (accessor &v)
	{
		add_accessor(v);
		return *this;
	}

	/// Get the SQLHANDLE of the statement.
	SQLHANDLE   handle() const { return m_handle; }

    /// return true if this statement has an active handle, i.o.w. is_open
    bool        is_open() const { return handle()!=0; }

	/// set an sql statement
	sqlreturn   LWODBC_API  set_statement(const tstring &sql_statement);
	const tstring LWODBC_API &get_statement() const { return m_sql_statement; }

	/// prepares the statement for execution
	const sqlreturn LWODBC_API &prepare();
	/// execute the statement on the connection
	const sqlreturn LWODBC_API &execute();

	/** \brief return the row count of the last executed statement 
	\note Not all drivers support this fully. The application must fail gracefully and
	use an alternative strategy if the row count returns -1
	*/
	sqlreturn LWODBC_API get_row_count(SQLINTEGER &rcount);

	/// fetch a row from the result set
	sqlreturn   LWODBC_API  fetch();

	enum fetch_scroll_orientation {
		sql_fetch_next = SQL_FETCH_NEXT,
		sql_fetch_prior = SQL_FETCH_PRIOR,
		sql_fetch_first = SQL_FETCH_FIRST,
		sql_fetch_last = SQL_FETCH_LAST,
		sql_fetch_absolute = SQL_FETCH_ABSOLUTE,
		sql_fetch_relative = SQL_FETCH_RELATIVE,
		sql_fetch_bookmark = SQL_FETCH_BOOKMARK
	};
	const sqlreturn LWODBC_API &fetch_scroll(fetch_scroll_orientation f, SQLINTEGER offset);
	const sqlreturn LWODBC_API &fetch_first() { return fetch_scroll(sql_fetch_first, 0); }
	const sqlreturn LWODBC_API &fetch_last() { return fetch_scroll(sql_fetch_last, 0); }
	const sqlreturn LWODBC_API &fetch_next() { return fetch_scroll(sql_fetch_next, 0); }
	const sqlreturn LWODBC_API &fetch_prior() { return fetch_scroll(sql_fetch_prior, 0); }
	const sqlreturn LWODBC_API &fetch_absolute(SQLINTEGER row) { return fetch_scroll(sql_fetch_absolute, row); }
	const sqlreturn LWODBC_API &fetch_relative(SQLINTEGER offset) { return fetch_scroll(sql_fetch_relative, offset); }
	//const sqlreturn LWODBC_API &fetch_bookmark( ... ) TODO

	enum column_states_enum {
		use_defaults = SQL_NTS,
		null_state = SQL_NULL_DATA,
		nts_length = SQL_NTS,
		ignore_state = SQL_IGNORE,
		default_state = SQL_DEFAULT
	};
	/// reset all column length indicators to an initial value
	const sqlreturn LWODBC_API &reset_column_states(SQLINTEGER len_ind=use_defaults);
	const sqlreturn LWODBC_API &reset_parameter_states(SQLINTEGER len_ind=use_defaults);
	/// set a column state/length indicator
	const sqlreturn LWODBC_API &set_column_state(SQLUSMALLINT col, SQLLEN len_ind);
	template <typename Value>
	const sqlreturn &set_length_for(const Value &v, SQLLEN len_ind)
	{
		SQLUSMALLINT pos=find_column_by_target(make_const_accessor(v));
		return set_column_state(pos, len_ind);
	}
	template <typename Value>
	const sqlreturn &set_column_state(const Value &v, column_states_enum state)
	{
		return set_length_for(v, (SQLLEN)state);
	}
	template <typename Value> 
	const sqlreturn &set_ignore_column(const Value &v)
	{
		return set_column_state(v, ignore_state);
	}
	template <typename Value>
	const sqlreturn &set_null_column(const Value &v, bool do_set_null=true)
	{
		return set_column_state(v, do_set_null ? null_state : use_defaults);
	}
	template <typename Value>
	const sqlreturn &set_default_column(const Value &v)
	{
		return set_column_state(v, default_state);
	}
	/// set a parameter state/length indicator. \todo: make this work - it currently does not
	const sqlreturn LWODBC_API &set_parameter_state(SQLUSMALLINT col, SQLLEN len_ind);
	const sqlreturn LWODBC_API &set_parameter_state(const tstring &name, SQLLEN len_ind)
	{
		return set_parameter_state(find_parameter(name), len_ind);
	}

	/// getting data
	const sqlreturn LWODBC_API &get_data(SQLUSMALLINT col, SQLSMALLINT c_type, SQLPOINTER buffer, SQLLEN buffer_length=0, SQLLEN *len_ind_p=0) 
	{ 
		return m_last_error=SQLGetData(handle(), col, c_type, buffer, buffer_length, len_ind_p);
	}
	const sqlreturn LWODBC_API &get_data_as_string(SQLUSMALLINT col, tstring &rc, SQLLEN *len_ind_p=0);
	///@name Setting cursor type and features
	/// Use these methods to query and set the cursortype and features for the statement.
	//@{
	enum scroll_options_enum {
		so_forward_only = SQL_SO_FORWARD_ONLY,
		so_static = SQL_SO_STATIC,
		so_keyset_driven = SQL_SO_KEYSET_DRIVEN,
		so_dynamic = SQL_SO_DYNAMIC,
		so_mixed = SQL_SO_MIXED
	};
	sqlreturn LWODBC_API get_scroll_options(SQLUINTEGER &options) const;
	enum cursor_type_enum {
		forward_only_cursor = SQL_CURSOR_FORWARD_ONLY,
		static_cursor = SQL_CURSOR_STATIC,
		dynamic_cursor = SQL_CURSOR_DYNAMIC,
		keyset_driven_cursor = SQL_CURSOR_KEYSET_DRIVEN
	};
	const sqlreturn LWODBC_API &set_cursor_type(cursor_type_enum ct) { return set_attr(SQL_ATTR_CURSOR_TYPE, ct); }
	const sqlreturn LWODBC_API &get_cursor_type(cursor_type_enum &ct) { return get_attr(SQL_ATTR_CURSOR_TYPE, (SQLUINTEGER&)ct); }
	bool LWODBC_API supports_cursor(cursor_type_enum ct) const;

	enum concurrency_enum {
		read_only = SQL_CONCUR_READ_ONLY,
		none = read_only,

		lock = SQL_CONCUR_LOCK,
		pessimistic = lock,

		rowver = SQL_CONCUR_ROWVER,
		timestamp = SQL_CONCUR_TIMESTAMP,
		values = SQL_CONCUR_VALUES,
		optimistic = 0x100		///< synonym for 'rowver', fall back to 'values' if not supported and fall back to lwodbc_emulated_optimistic if 'values' is not supported
	};
	const sqlreturn LWODBC_API &set_concurrency(concurrency_enum cy);
	const sqlreturn LWODBC_API &get_concurrency(concurrency_enum &cy);

	/// try to use rowversion or rowvalues for optimistic locking. emulate optimistic locking in the lwodbc library if driver does not support it
	const sqlreturn LWODBC_API &set_optimistic_locking();
	/// attempt to use pessimistic locking. Might not be supported by all drivers and will @b not be emulated if it isn't supported.
	const sqlreturn LWODBC_API &set_pessimistic_locking() { return set_concurrency(lock); }
	/// sets the statement to read only concurrency, no locking.
	const sqlreturn LWODBC_API &set_no_locking() { return set_concurrency(read_only); }

	const sqlreturn LWODBC_API &set_updatable(concurrency_enum cy);
	const sqlreturn LWODBC_API &set_updatable(bool yes=true) { return set_updatable(yes ? optimistic : none); }
	bool LWODBC_API is_updatable();

	SQLUINTEGER LWODBC_API current_cursor_attributes1() throw();
	bool LWODBC_API has_POS_UPDATE() throw()							{ return (current_cursor_attributes1() & SQL_CA1_POS_UPDATE)!=0; }
	bool LWODBC_API has_POS_DELETE() throw()							{ return (current_cursor_attributes1() & SQL_CA1_POS_DELETE)!=0; }
	bool LWODBC_API has_POSITIONED_UPDATE() throw()				{ return (current_cursor_attributes1() & SQL_CA1_POSITIONED_UPDATE)!=0; }
	bool LWODBC_API has_POSITIONED_DELETE() throw()				{ return (current_cursor_attributes1() & SQL_CA1_POSITIONED_DELETE)!=0; }
	bool LWODBC_API has_BULK_ADD() throw()							{ return (current_cursor_attributes1() & SQL_CA1_BULK_ADD)!=0; }

	///\ use a scrollable cursor.
	///\note if the driver does not support SQL_ATTR_SCROLL_OPTIONS, calling set_scrollable(true) will choose a keyset or dynamic cursor instead
	const sqlreturn LWODBC_API &set_scrollable(bool use_scrolling_cursor=true);
	/// test if the current cursor is scrollable
	bool LWODBC_API is_scrollable() throw();

	///\ cache values returned by fetch so that the statement can build a simulated update/delete statement later
	bool set_use_cache(bool yes=true) throw() { m_binder.set_use_cache(yes); return true; }
	bool has_cache() const throw() { return m_binder.has_cache(); }
	//@}

	static bool LWODBC_API g_use_SQLSetPos;
	static bool LWODBC_API g_use_SQLBulkOperations;
	/** \brief Check if the driver supports updating the row in the result set of this statement. */
	//TODO: remove 'return false' again
	bool has_update_row() throw() { return g_use_SQLSetPos && has_POS_UPDATE(); }
	bool has_delete_row() throw() { return g_use_SQLSetPos && has_POS_DELETE(); }
	bool has_insert_row() throw() { return g_use_SQLBulkOperations && has_BULK_ADD(); }
	/** \brief Update the current row - write it back to the SQL data source using SQLSetPos.
		This command attempts to update the values in the current row of the current
		SELECT statement. Not all drivers support this. Check has_update()  before calling this.
		odbc::table::update_row() can update tables even if odbc::statement::has_update() is false and is
		the preferred method of updating values. If statement::has_update() is true, statement::update_row()
		might be able to update joined tables.
	*/
	const sqlreturn LWODBC_API &update_row();
	const sqlreturn LWODBC_API &delete_row();
	const sqlreturn LWODBC_API &insert_row();

	/// get the name of the current cursor
	const sqlreturn LWODBC_API &get_cursor_name(tstring &cursor_name);

	/// move on to the next result set if the SQL statement returned more than one result set
	sqlreturn	LWODBC_API	more_results();

	/// close the current statement and unbind all columns/parameters.
	/// The statement can be reused with a different statement or the same statement and a different execution.
	sqlreturn   LWODBC_API  clear();

	/// close the current cursor, leave the bindings intact. The statement can be reused immediately with execute.
	sqlreturn				close_cursor() 		{ SQLCancel(handle()); return m_last_error=SQLFreeStmt(handle(), SQL_CLOSE); }

	const sqlreturn	LWODBC_API &set_attr(SQLINTEGER attribute, SQLUINTEGER value);
	const sqlreturn	LWODBC_API &get_attr(SQLINTEGER attribute, SQLUINTEGER &value);

	//-----------------------------------------------------------------------------------------------------------//
	///@name binding parameters
	/// Use these methods to bind parameters in a statement to C++ variables
	//@{
	/// bind a parameter by position to a C variable.
	sqlreturn   LWODBC_API  bind_parameter( SQLUSMALLINT pposition, SQLSMALLINT in_out, SQLSMALLINT c_type, SQLSMALLINT sql_type, 
		SQLULEN column_size, SQLSMALLINT decimal_digits, SQLPOINTER buffer, SQLLEN length, SQLLEN *len_ind);
	/** bind a parameter by name - this requires a parameter marker in the sql statement. 

	Example: SELECT * FROM my_table WHERE id=?<b><em>([in]variable)</em></b>
	*/
	sqlreturn	LWODBC_API	bind_parameter_accessor(const tstring &name, const accessor &a, SQLLEN *len_ind);
	sqlreturn	LWODBC_API	bind_parameter_accessor(SQLUSMALLINT pposition, SQLSMALLINT in_out, const accessor &a, SQLLEN *len_ind);
	//\brief template member function to bind parameter to a C++ variable.
	/** This template member functions binds a C++ variable to a named parameter. It tries to
	detect the correct C++ type automatically.

	Example:
	\code
	statement s;
	long id;
	s.bind_parameter("variable", id);
	\endcode
	\todo allow const values for Value &v for input parameters
	*/
	template <class Value>
		sqlreturn				bind_parameter(const tstring &name, Value &v, SQLLEN *len_ind=0)
	{
		const accessor a(make_accessor(v));
		return bind_parameter_accessor(name, a, len_ind);
	}
	template <typename Value>
		sqlreturn				bind_parameter(SQLUSMALLINT position, SQLSMALLINT in_out, Value &v, SQLLEN *len_ind=0)
	{
		const accessor a(make_accessor(v));
		return bind_parameter_accessor(position, in_out, a, len_ind);
	}
	sqlreturn	LWODBC_API bind_parameter( SQLUSMALLINT position, SQLSMALLINT in_out, accessor a, SQLLEN *len_ind)
	{
		return bind_parameter_accessor(position, in_out, a, len_ind);
	}

	/// Special bind_parameter version if you need greater control over the bind options. Accepts a bind_task object.
	sqlreturn				bind_parameter(const bind_task &task);

	sqlreturn reset_parameter_bindings()
	{
		return m_last_error=SQLFreeStmt(handle(), SQL_RESET_PARAMS);
	}
	/// unbind all parameters.
	sqlreturn               unbind_parameters()     
	{ 
		m_binder.unbind_parameters();
		return reset_parameter_bindings();
	}
	//@}

	//-----------------------------------------------------------------------------------------------------------//
	///@name binding columns
	/// Use these methods to bind C++ variables to columns in a result set.
	//@{
	SQLSMALLINT get_column_count() const { return (SQLSMALLINT)m_column_list.size()-1; }	///< number of columns in the result set not counting the bookmark

	//sqlreturn   LWODBC_API  bind_column(SQLSMALLINT col, const bind_descriptor &d) throw();
	sqlreturn   LWODBC_API  bind_column(SQLSMALLINT col, SQLSMALLINT c_type, SQLPOINTER target_ptr, SQLINTEGER size, SQLLEN* len_ind);
	/// bind an accessor to a column by position
	sqlreturn   LWODBC_API  bind_column(SQLSMALLINT col, accessor a, SQLLEN *len_ind=0);
#ifdef _NOT
	sqlreturn   LWODBC_API  bind_column(SQLSMALLINT col, accessor &a, SQLLEN *len_ind=0)
	{
		const accessor &a_(a);
		return bind_column(col, a_, len_ind);
	}
#endif // _NOT
	/// bind a value to a column by position
	template<typename Value>
	sqlreturn	bind_column(SQLSMALLINT col, Value &v, SQLLEN *len_ind=0)
	{
		return bind_column(col, make_accessor(v), len_ind);
	}
	/// bind an accessor adapter to a column by name
	sqlreturn	LWODBC_API	bind_column(const tstring &column_name, accessor a, SQLLEN *len_ind=0);
#ifdef _NOT
	//sqlreturn	LWODBC_API	bind_column(const tstring &column_name, accessor &a, SQLLEN *len_ind=0)
	//{
	//	const accessor &a_(a);
	//	return bind_column(column_name, a_, len_ind);
	//}
#endif // _NOT
	/// bind a value to a column by name
	template <typename Value>
	sqlreturn	bind_column(const tstring &column_name, Value &v, SQLLEN *len_ind=0)
	{
		return bind_column(column_name, make_accessor(v), len_ind);
	}
	/// bind an aggregate adapter to the result columns by name
	sqlreturn	LWODBC_API	bind_column(aggregate a);

	sqlreturn reset_column_bindings()
	{
		return m_last_error= SQLFreeStmt(handle(), SQL_UNBIND);
	}
	sqlreturn               unbind_columns()        
	{ 
		m_binder.unbind_columns();
		return reset_column_bindings();
	}

	/// bind an aggregate to columns by name
	template <class Value>
		sqlreturn				bind(Value &v)
	{
		return	bind_column(make_aggregate(v));
	}
	template <> 
		sqlreturn bind(const aggregate &a) { return bind_column(a); }
	template <>
		sqlreturn bind(aggregate &a) { return bind_column(a); }
	/// bind a single value to a column by name
	template <class Value>
		sqlreturn				bind(const tstring &name, Value &v)
	{
		return bind_column(name, make_accessor(v));
	}
	/// bind a single value to a column by position
	template <class Value>
		sqlreturn bind(SQLSMALLINT col, Value &v)
	{
		return bind_column(col, make_accessor(v));
	}
	template <class Value>
	sqlreturn bind(int col, Value &v)
	{
		return bind_column(static_cast<SQLSMALLINT>(col), make_accessor(v));
	}
	/// Bind a parameter to a column of a result set from a different statement.
	const sqlreturn &bind_parameter_to_result_column(SQLSMALLINT parameter_position, const statement& results, SQLSMALLINT column_position)
	{
		m_last_error=results.feed_column_to_target_parameter(column_position, this, parameter_position);
		return m_last_error;
	}
	//@}

	sqlreturn unbind()
	{
		unbind_columns().ok() && unbind_parameters().ok() && m_binder.unbind().ok();
		return m_last_error;
	}

	/// get the column length/indicator value
	SQLLEN   LWODBC_API get_column_length(const tstring &column_name) const { return get_column_length(find_column(column_name)); }
	SQLLEN   LWODBC_API get_column_length(SQLSMALLINT pos) const;
	bool   LWODBC_API is_column_null(const tstring &column_name) const { return is_column_null(find_column(column_name)); }
	bool   LWODBC_API is_column_null(SQLSMALLINT pos) const { return get_column_length(pos)==SQL_NULL_DATA; }

	sqlreturn   last_error() const { return m_last_error; }

	/// 'throw' errors instead of returning them. If set, only SQL_SUCCESS and SQL_SUCCESS_WITH_INFO will be returned.
	void LWODBC_API set_throw_on_error(bool do_throw=true) { m_last_error.set_throw_on_error(do_throw); }
	bool LWODBC_API is_throw_on_error() const { return m_last_error.is_throw_on_error(); }
	void LWODBC_API set_log_errors(bool do_log) { m_last_error.set_log_errors(do_log); }
	bool LWODBC_API is_log_errors() const { return m_last_error.is_log_errors(); }
	bool LWODBC_API ignore_once(const TCHAR *states_to_ignore) throw() { return m_last_error.ignore_once(states_to_ignore); }
	bool LWODBC_API ignore_once() throw() { return ignore_once(_T("*")); }
	bool LWODBC_API get_throw_on_error() const { return m_last_error.get_throw_on_error(); }
	LWODBC_API const TCHAR * get_ignore_once() const {return m_last_error.get_ignore_once(); }
	struct throwing:boost::noncopyable
	{
		throwing(statement &s, bool should_throw=true, bool should_log=true):m_s(s), m_is_throwing(s.is_throw_on_error()), m_is_logging(s.is_log_errors())
		{
			m_s.set_throw_on_error(should_throw);
			m_s.set_log_errors(should_log);
		}
		~throwing()
		{
			m_s.set_throw_on_error(m_is_throwing);
			m_s.set_log_errors(m_is_logging);
		}
		bool m_is_throwing;
		bool m_is_logging;
		statement &m_s;
	};
	/// query a string type column attribute
	const sqlreturn	LWODBC_API &get_column_attr(SQLSMALLINT pos, SQLSMALLINT field, tstring &value);
	/// query an integer type column attribute
	const sqlreturn	LWODBC_API &get_column_attr(SQLSMALLINT pos, SQLSMALLINT field, SQLINTEGER &value);
	/// query a boolean type column attribute
	const sqlreturn LWODBC_API &get_column_attr(SQLSMALLINT pos, SQLSMALLINT field, bool &value);

	sqlreturn	get_column_catalog(SQLSMALLINT pos, tstring &value)						{ return get_column_attr(pos, SQL_DESC_CATALOG_NAME, value); }
	sqlreturn	get_column_schema(SQLSMALLINT pos, tstring &value) 						{ return get_column_attr(pos, SQL_DESC_SCHEMA_NAME, value); }
	sqlreturn	get_column_table(SQLSMALLINT pos, tstring &value) 							{ return get_column_attr(pos, SQL_DESC_TABLE_NAME, value); }
	sqlreturn	get_column_name(SQLSMALLINT pos, tstring &value) 							{ return get_column_attr(pos, SQL_DESC_NAME, value); }
	sqlreturn	get_column_label(SQLSMALLINT pos, tstring &value)							{ return get_column_attr(pos, SQL_DESC_LABEL, value); }
	sqlreturn	get_column_type(SQLSMALLINT pos, SQLINTEGER &value)					{ return get_column_attr(pos, SQL_DESC_TYPE, value); }

	sqlreturn	LWODBC_API get_column_descriptor(SQLSMALLINT pos, column_descriptor &d) const;
	sqlreturn	get_column_size(SQLSMALLINT pos, SQLUINTEGER &column_size) const;

	/// Find a column named @p name and return its position.
	SQLSMALLINT LWODBC_API find_column(const tstring &name) const throw();
	SQLSMALLINT LWODBC_API find_column_by_target(const const_accessor &a) const throw() { return m_binder.find_column_by_target(a); }
	/// Find a parameter @p name and return its position
	SQLSMALLINT find_parameter(const tstring &name) const throw();
	/// Get the parameter information for parameter at position @p pos
	const parameter *get_parameter(SQLSMALLINT pos) const throw();

	const sqlreturn &set_last_error(sqlreturn &e) { return m_last_error=e; }
	const sqlreturn &set_last_error(SQLRETURN r) { return m_last_error=r; }

	/// get the connection this statement operates on
	connection &get_connection() const { return *m_the_connection; }

	/// set a macro value
	void set_macro_value(const tstring &name, const tstring &value) { m_macros[name]=value; }
	/// get a macro value
	/// Makros can be specified in a SQL statement with  $$\p name, where \p name  is the name of the macro and $$ denotes the start of a macro.
	tstring get_macro_value(const tstring &name) const;
	bool has_been_executed() const;
protected:
	friend class binder;
	friend class binder::binder_lists;
	friend class table;

	/// emulate optimistic concurrency locking
	bool	m_emulate_optimistic_concurrency;

	/// clear all information known about the result set
	void clear_result_set();

	sqlreturn   do_bind_parameter( SQLUSMALLINT pposition, SQLSMALLINT in_out, SQLSMALLINT c_type, SQLSMALLINT sql_type, 
		SQLULEN column_size, SQLSMALLINT decimal_digits, SQLPOINTER buffer, SQLLEN length, SQLLEN *len_ind);
	sqlreturn   do_bind_column(SQLSMALLINT col, SQLSMALLINT c_type, SQLPOINTER target_ptr, SQLINTEGER size, SQLLEN* len_ind);

	/// this binds the given result set column to the parameter of the target statement
	sqlreturn LWODBC_API feed_column_to_target_parameter(SQLSMALLINT col_position, statement *target, SQLSMALLINT par_pos) const;

	sqlreturn   LWODBC_API  describe_column(SQLSMALLINT col, column_descriptor &d);
	typedef map<tstring, SQLSMALLINT> column_index_t;
	/// contains the name index for the column. enables column lookup by name.
	column_index_t	m_column_index;
	/// contains the list of columns after execute
	vector<column_descriptor> m_column_list;
	/// contains the list of parameters after prepare
//	vector<column_descriptor> m_parameter_list;
	void		clear_column_descriptors()					{ m_column_list.clear(); m_column_index.clear(); }
	const sqlreturn	&get_column_descriptors()		
	{ 
		if (m_column_list.size()>0)
			return m_last_error=SQL_SUCCESS;
		return do_get_column_descriptors();
	}
//	const sqlreturn &get_parameter_descriptors() throw();
//	const column_descriptor &get_parameter_descriptor(SQLSMALLINT pos) throw() { return m_parameter_list[pos]; }

	/// retrieve the columns from the current result set and store them
	const sqlreturn	LWODBC_API	&do_get_column_descriptors();

	enum state {
		unknown,
		uninitialised,
		reset,
		setting_statement,
		statement_set,
		executed,
		fetched,
		closed,
		error
	}	m_state;

	bool prefetch() 
	{
		return (has_been_executed() || execute()) && (needs_bind_columns()==false || bind_columns());
	}
	bool postfetch() 
	{
		m_state=fetched;
		if (m_last_error.no_data()==false && m_binder.needs_get_columns()) {
			sqlreturn rc=m_binder.do_get_columns(*this);
			if (rc.fail())
				m_last_error=rc;
			return false;
		}
		return true;
	}

	bool has_been_prepared() const { return m_is_prepared || m_state==executed || m_state==fetched; }
	bool is_reusable() const { return m_state!=statement_set && !has_been_prepared(); }

	bool m_is_prepared;

	vector<parameter>	m_parameter_marker;
	vector<parameter> m_column_marker;

	/// parse the SQL statements for ([bind] marker) statements
	bool do_parse_bindings();
	void add_bind_marker(my_symbols &sym, parameter &p, size_t &next_col, size_t &next_param, const TCHAR*, const TCHAR*);       ///< add a bind marker

	bool needs_bind_columns() const throw() { return m_binder.needs_bind_columns(); }
	const sqlreturn &bind_columns();
	bool needs_put_columns() const throw() { return m_binder.needs_put_columns(); }
	const sqlreturn &put_columns();
	bool needs_bind_parameters() const throw() { return m_binder.needs_bind_parameters(); }
	const sqlreturn &bind_parameters();
	bool needs_put_parameters() const throw() { return m_binder.needs_put_parameters(); }
	const sqlreturn &put_parameters();

	binder m_binder;

	sqlreturn   allocate_handle(const connection &c);
	sqlreturn   free_handle();
	/// parse the SQL statement and extract any bindings specified in there
	bool		parse_bindings();

	sqlreturn_auto_set_diagnostics   m_last_error;

	sqlreturn is_column_valid(SQLSMALLINT pos) const 
	{
		if (m_column_list.size()==0)
			return sqlreturn(_("no columns available. call get_column_descriptors() first!"), odbc::err_logic_error);
		if (pos>=(SQLSMALLINT)m_column_list.size() || pos<0)
			return sqlreturn(_("no such column "), odbc::err_no_such_column);
		return sqlreturn(SQL_SUCCESS);
	}

private:

	/// initialize with a connection
	void	init(connection &c, const tstring &sql_statement=tstring());
	/// basic initialisation without a connection
	void	init();

	tstring     m_sql_statement;
	SQLHANDLE   m_handle;
	mutable map<tstring, tstring> m_macros;

	tstring     m_identifier_quote_char;

	/// pointer to the connection that owns this statement
	connection *m_the_connection;
	/// holds the reference to a connection from the connection pool IF the statement has been allocated from the pool
	/// is NULL otherwise
	shared_connection m_reference_counting_for_connection_pool;

	struct continuous_sql_binder
	{
		continuous_sql_binder()
		{ 
			clear(); 
		}
		void clear()
		{
			m_last_was_parameter=false;
			m_next_parameter=m_next_column=1;
			m_last_bind_type=in;
		}
		bool m_last_was_parameter;
		SQLSMALLINT m_next_parameter, m_next_column;
		bind_type m_last_bind_type;
	} m_continuous_sql_binder;
	void LWODBC_API add_accessor(const accessor &a);
    void LWODBC_API add_const_accessor(const const_accessor &a);
};

inline bool statement::has_been_executed() const { return m_state==executed || m_state==fetched; }

template <class Value>
SQLLEN column_length(const statement &s, const Value &v)
{
	return s.get_column_length(s.find_column_by_target(make_const_accessor(v)));
}
template <class Value>
bool is_null(const statement &s, const Value &v)
{
	return column_length(s, v)==SQL_NULL_DATA;
}
//template <class Value>
//void set_column_state(const statement &s, const Value &v, SQLLEN ind)
//{
//	s.set_column_state(s.find_column_by_target(make_const_accessor(v)), ind);
//}

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

inline sqlreturn statement::bind_column(SQLSMALLINT col, SQLSMALLINT c_type, SQLPOINTER target_ptr, SQLINTEGER size, SQLLEN* len_ind)
{
	return m_last_error=m_binder.bind_column(col, c_type, target_ptr, size, len_ind);
}
inline sqlreturn statement::do_bind_column(SQLSMALLINT col, SQLSMALLINT c_type, SQLPOINTER target_ptr, SQLINTEGER size, SQLLEN* len_ind)
{
	m_last_error=SQLBindCol(handle(), col, c_type, target_ptr, size, len_ind);
	return m_last_error;
}

//-----------------------------------------------------------------------------------------------------------//
inline sqlreturn statement::bind_parameter( SQLUSMALLINT pposition, SQLSMALLINT in_out, SQLSMALLINT c_type, SQLSMALLINT sql_type, 
					   SQLULEN column_size, SQLSMALLINT decimal_digits, SQLPOINTER buffer, SQLLEN length, SQLLEN *len_ind)
{
	m_last_error=m_binder.bind_parameter(pposition, in_out, c_type, sql_type, column_size, decimal_digits, buffer, length, len_ind);
	return m_last_error;
}
inline sqlreturn statement::bind_parameter(const bind_task &task)
{
	m_last_error=m_binder.bind_parameter(task);
	return m_last_error;
}
inline sqlreturn statement::bind_parameter_accessor(const tstring &name, const accessor &a, SQLLEN *len_ind)
{
	return m_binder.bind_parameter(name, a, len_ind);
}
inline sqlreturn statement::bind_parameter_accessor(SQLUSMALLINT pos, SQLSMALLINT in_out, const accessor &a, SQLLEN *len_ind)
{
	return m_binder.bind_parameter(pos, a, in_out, len_ind);
}

inline statement &statement::operator<<(bind_type next_bind_type)
{
	m_continuous_sql_binder.m_last_bind_type=next_bind_type;
	return *this;
}

inline statement::statement()
{
	init();
}

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
//inline bool is_empty(const column_descriptor &v)
//{
//	return v.m_sql_type==SQL_UNKNOWN_TYPE;
//}

};

};
#pragma warning(pop)

#endif
