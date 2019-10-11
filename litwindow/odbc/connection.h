/** \file
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: connection.h,v 1.16 2008/02/14 11:08:08 Merry\Hajo Kirchhoff Exp $
*/

#ifndef __LWODBC_CONNECTION_H
#define __LWODBC_CONNECTION_H

#include <litwindow/lwbase.hpp>
#include <litwindow/tstring.hpp>
#include <boost/smart_ptr.hpp>
#include "./lwodbc.h"
#include "./dbms.h"
#include <sqlext.h>
#include <ctime>

#pragma warning(push, 4)

// now make intellisense work again by undefining LWODBC_API for intellisense
#ifdef INTELLISENSE_DOES_NOT_WORK_PSEUDO_DEFINE
#undef LWODBC_API
#define LWODBC_API
#endif
//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

namespace litwindow {

namespace odbc {;

//-----------------------------------------------------------------------------------------------------------//
class LWODBC_API environment_imp {
public:
	environment_imp();
	~environment_imp();

	SQLHANDLE handle() const throw() { return m_handle; }
private:
	SQLHANDLE m_handle;
};

typedef boost::shared_ptr<environment_imp> environment;

extern bool LWODBC_API CheckSQLReturn(SQLRETURN rc);
extern environment LWODBC_API get_default_environment();

/// parse an odbc_connection string and extract its DSN= UID= and PWD= parts. Returns false if there are other parts in the connection strings as well.
extern bool extract_dsn_uid_pwd(const tstring &odbc_connection_string, tstring &dsn, tstring &uid, tstring &pwd);

//-----------------------------------------------------------------------------------------------------------//

///\brief ODBC connection class
/** This class represents a single ODBC connection. */
class connection {
	friend class statement;
public:

	typedef boost::shared_ptr<connection> shared_ptr;

	class pool_imp_base
	{
	public:
		virtual shared_ptr LWODBC_API get(const tstring &name=tstring()) = 0;
		virtual void LWODBC_API set(const tstring &dsn, const tstring &uid, const tstring &pwd, const tstring &name=tstring()) = 0;
		virtual void LWODBC_API set(const tstring &connection_string, const tstring &name=tstring()) = 0;
		virtual void LWODBC_API put(const tstring &name, shared_ptr connection) = 0;
		virtual void LWODBC_API close_all() = 0;
		virtual shared_ptr open(const tstring &name = tstring()) = 0;
	};

	/// return the ODBC named connection pool
	static pool_imp_base LWODBC_API& pool() throw();

	enum cursor_implementation_enum {
		//use_odbc_cursors = SQL_CUR_USE_ODBC,
		use_driver_cursors = SQL_CUR_USE_DRIVER,
		//use_odbc_cursors_if_needed = SQL_CUR_USE_IF_NEEDED
	};
	/** specify which cursor implementation to use */
	const sqlreturn LWODBC_API &set_cursor_implementation(cursor_implementation_enum crs);
	static void LWODBC_API set_default_cursor_implementation(cursor_implementation_enum default_crs);

	/// Connect to a datasource via a DSN
	const sqlreturn LWODBC_API &open(const litwindow::tstring &dsn, const litwindow::tstring &uid, const litwindow::tstring &pwd) throw();

	/// Connect to a datasource via a connection string
	const sqlreturn LWODBC_API &open(const litwindow::tstring &connection_string, SQLHWND hwnd=0, SQLUSMALLINT completion=SQL_DRIVER_NOPROMPT) throw();

	/// Connect to a datasource via the current connection string/DSN
	const sqlreturn LWODBC_API &open(SQLHWND hwnd=0, SQLUSMALLINT completion=SQL_DRIVER_NOPROMPT) throw();

	/// Connect to a file based datasource. Iterate over all strategy objects to build a connection string, then open it.
	/// \p dsn_addition is a string that will be appended to the dsn before opening it. It allows extras such as MS-Excels IMEX=1.
	const sqlreturn LWODBC_API &open_file(const litwindow::tstring &file, const litwindow::tstring &uid, const litwindow::tstring &pwd, bool read_only=false, const litwindow::tstring &dsn_addition=litwindow::tstring(), const litwindow::tstring &file_type=litwindow::tstring()) throw();

	bool LWODBC_API				set_uid(const tstring &uid) throw();
	const tstring LWODBC_API		&get_uid() const throw();
	bool LWODBC_API				set_pwd(const tstring &pwd) throw();
	const tstring LWODBC_API		&get_pwd() const throw();
	bool LWODBC_API				set_dsn(const tstring &dsn) throw();
	const tstring LWODBC_API		&get_dsn() const throw();
	const sqlreturn LWODBC_API	&set_connection_string(const tstring &connection_string) throw();
	tstring LWODBC_API			get_connection_string() const throw();
	bool uses_connection_string() const { return m_remaining_connection_string.empty()==false; }
	bool uses_dsn() const { return !uses_connection_string() && get_dsn().empty()==false; }

	bool	LWODBC_API is_open_via_SQLConnect() const throw();

	bool      LWODBC_API is_open() const throw() { return m_is_connected; }

	/// get connection string returned by 'open'
	tstring   LWODBC_API get_out_connection_string() const throw();

	/// Disconnect a connection
	sqlreturn LWODBC_API close();

	/// Set an 'SQLUINTEGER' attribute
	const sqlreturn LWODBC_API &set_attribute(SQLINTEGER attribute, SQLUINTEGER value) throw();

	/// Set a 'SQLTCHAR*' attribute
	const sqlreturn LWODBC_API &set_attribute(SQLINTEGER attribute, const tstring &value) throw();

	/// Get an 'SQLUINTEGER' attribute
	sqlreturn LWODBC_API get_attribute(SQLINTEGER attribute, SQLUINTEGER &value) throw();

	/// Get a 'SQLTCHAR*' attribute
	sqlreturn LWODBC_API get_attribute(SQLINTEGER attribute, tstring &value) throw();

	sqlreturn LWODBC_API set_read_only(bool yes=true) throw();
	sqlreturn LWODBC_API set_read_write() throw()								{ return set_read_only(false); }

	bool      LWODBC_API is_autocommit_on() throw();
	const sqlreturn LWODBC_API &set_autocommit_on(bool yes=true) throw();
	const sqlreturn LWODBC_API &set_autocommit_off() throw()							{ return set_autocommit_on(false); }

	const sqlreturn LWODBC_API &begin_transaction();
	const sqlreturn LWODBC_API &end_transaction(SQLSMALLINT completion_type);
	const sqlreturn LWODBC_API &commit_transaction() { return end_transaction(SQL_COMMIT); }
	const sqlreturn LWODBC_API &rollback_transaction() { return end_transaction(SQL_ROLLBACK); }
	bool	LWODBC_API has_open_transaction() const { return m_nested_transactions>0; }

	sqlreturn LWODBC_API set_current_catalog(const tstring &catalog) throw();
	sqlreturn LWODBC_API get_current_catalog(tstring &catalog) throw();
	sqlreturn LWODBC_API get_user_name(tstring &uid) throw();

	/// execute an SQL script
	sqlreturn LWODBC_API execute(const litwindow::tstring &script);

	/// Ignore a list of states: do not log them or throw an exception
	bool LWODBC_API ignore_once(const TCHAR *states_to_ignore) throw() { return m_last_error.ignore_once(states_to_ignore); }

	//-----------------------------------------------------------------------------------------------------------//
	/// Create a connection using the default environment
	LWODBC_API connection();

	/// Create a connection on a specific environment and dbms strategy
	LWODBC_API connection(dbms_base *dbms_to_use, environment p=get_default_environment());

	private:
		/// Copy a connection - open a new connection based on the data of the current connection
	LWODBC_API connection(const connection &c);

	/// Assign a connection - close the current connection and open a new connection based on the parameters
	LWODBC_API const connection &operator =(const connection &c);
	public:

	/// Free all handles and destroy the connection
	LWODBC_API ~connection();
	//-----------------------------------------------------------------------------------------------------------//

	SQLHANDLE LWODBC_API handle() const { touch(); return m_handle; }

	/// Free the handle, reset the object, reallocate a new handle
	const sqlreturn LWODBC_API &reset() throw();

	const sqlreturn LWODBC_API &alloc_handle(environment env);
	const sqlreturn LWODBC_API &free_handle();

	inline void LWODBC_API touch() const { std::time(&m_last_time_handle_accessed); }

	/// Get an 'SQLUINTEGER' info value
	sqlreturn LWODBC_API get_info(SQLUSMALLINT info, SQLUINTEGER &value) throw();
	/// Get a 'tstring' info value
	sqlreturn get_info(SQLUSMALLINT info, tstring &value);

	/// Use SQLGetInfo and related functions to load the drivers capabilities and check if the driver meets the minimum requirements.
	/// @return an error if the driver does not meet the minimal functionality or is broken
	const sqlreturn &get_driver_capabilities_and_parameters();

	/// Get the DBMS strategy object used for this connection
	dbms_base *get_dbms() const throw()                                                     { return m_dbms.get(); }

	/// test capabilities of the DBMS
	bool has_capability(dbms_base::capabilities c) const throw()                              { return m_dbms->has_capability(c); }
	bool supports(dbms_base::capabilities c) const throw()                                    { return has_capability(c); }
	bool has_schema() const throw() { return has_capability(dbms_base::has_schema); }
	bool has_create_schema() const throw() { return has_capability(dbms_base::has_create_schema); }
	bool has_user_accounts() const throw() { return has_capability(dbms_base::has_user_accounts); }

	///@name DBMS specific methods, creating users, groups, databases, schemas
	//@{
	sqlreturn LWODBC_API create_group(const tstring &group) throw()                         { return get_dbms()->create_group(this, group); }
	sqlreturn LWODBC_API drop_group(const tstring &group) throw()                           { return get_dbms()->drop_group(this, group); }
	sqlreturn LWODBC_API create_user(const tstring &user, const tstring &password) throw()  { return get_dbms()->create_user(this, user, password); }
	sqlreturn LWODBC_API drop_user(const tstring &user) throw()                             { return get_dbms()->drop_user(this, user, tstring()); }
	sqlreturn LWODBC_API add_user(const tstring &user, const tstring &to_group) throw()     { return get_dbms()->add_user(this, user, to_group); }
	sqlreturn LWODBC_API drop_user(const tstring &user, const tstring &from_group) throw()  { return get_dbms()->drop_user(this, user, from_group); }
	sqlreturn LWODBC_API create_database(const tstring &database) throw()                   { return get_dbms()->create_database(this, database); }
	sqlreturn LWODBC_API use_database(const tstring &database) throw()                      { return get_dbms()->use_database(this, database); }
	sqlreturn LWODBC_API drop_schema(const tstring &schema) throw()			{ return get_dbms()->drop_schema(this, schema); }
	sqlreturn LWODBC_API create_schema(const tstring &schema) throw()			{ return get_dbms()->create_schema(this, schema); }
	sqlreturn LWODBC_API change_password(const tstring &oldpw, const tstring &newpw, const tstring &uid=tstring()) throw() 
	{ 
		return get_dbms()->change_password(this, oldpw, newpw, uid.empty() ? get_uid() : uid);
	}

	sqlreturn LWODBC_API get_current_sequence_value(const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column) throw()
	{ 
		return get_dbms()->get_current_sequence_value(this, target, sequence_name, expand_sequence_name_from_column);
	}

	SQLSMALLINT LWODBC_API sql_to_c_type(SQLSMALLINT sql_type)
	{
		return get_dbms()->sql_to_c_type(sql_type);
	}
	tstring LWODBC_API sql_to_create_table_name(SQLSMALLINT sql_type, SQLLEN length)
	{
		return get_dbms()->sql_to_create_table_name(this, sql_type, length);
	}

    const sqlreturn LWODBC_API  &create_table_for(const_aggregate ag, const tstring &table_name=tstring(), const tstring &primarykey_name=tstring());
    const sqlreturn LWODBC_API  &add_or_alter_column_for(prop_t a, const tstring &column_name, const tstring &table_name);
    const sqlreturn LWODBC_API  &drop_column(const tstring &table_name);
    const sqlreturn LWODBC_API  &change_primary_key(const tstring &table_name, const tstring &primarykey_name);
	//@}

	void set_log(bool on=true)                                                  { m_log=on; }
	bool is_log() const { return m_log; }

	sqlreturn   LWODBC_API last_error() const throw () { return m_last_error; }
	void        set_last_error(const sqlreturn &r) { m_last_error=r; }
	/// set the default 'throw_on_error' flag for statements created for this connection
	void	set_throw_on_error_default(bool throw_on_error_default) { m_throw_on_error_default=throw_on_error_default; }
	bool get_throw_on_error_default() const { return m_throw_on_error_default; }

	static  int         LWODBC_API  &unit_test_mode_();
	static  tstring     LWODBC_API  &unit_test_result_();

	tstring capabilities_as_string();

	/// Clear specific cursor_attributes1 flags. Prevent the lwodbc library from using specific cursor features (which may be broken for specific drivers such as MySQL).
	void LWODBC_API clear_cursor_attributes1(SQLUINTEGER clear_flags) throw();

	///\name: Macros
	//@{
	void LWODBC_API set_macro_value(const tstring &name, const tstring &value);
	tstring get_macro_value(const tstring &name) const;
	//@}

	struct dbversion {
		int major;
		int minor;

		dbversion() : major(0), minor(0) {}
		dbversion(int new_major, int new_minor) : major(new_major), minor(new_minor) {}
		dbversion &operator =(std::wstring &new_version);
	};
	tstring get_dbms_name() { return m_dbms_name; }
	dbversion get_odbc_version() { return m_dbms_odbc_ver; }
	dbversion get_driver_version() { return m_dbms_driver_ver; }

private:
	size_t m_nested_transactions;
	bool m_autocommit_state;	///< autocommit state when no transaction is active - begin_transaction has not been called - m_nested_transactions==0
	static cursor_implementation_enum g_default_cursor_implementation;
	static  int sm_unit_test_mode;
	static  tstring sm_unit_test_result;

	mutable time_t  m_last_time_handle_accessed;

	boost::shared_ptr<dbms_base> m_dbms;

	void init(environment env);

	bool require(SQLUSMALLINT what, SQLUINTEGER expected, SQLUINTEGER bitmask=~0);

	environment m_env;
	SQLHANDLE   m_handle;
	bool		m_is_connected;
	tstring		m_uid, 
				m_pwd, 
				m_dsn, 
				m_remaining_connection_string;
	tstring		m_out_connection_string;

	sqlreturn_auto_set_diagnostics	m_last_error;

	/// true if log to lw_log() is active
	bool        m_log;

	///@name parameter and capability variables
	//@{
	tstring		m_identifier_quote_char;
	tstring		m_dbms_name;
	tstring		m_dbms_ver;
	dbversion	m_dbms_odbc_ver;
	dbversion	m_dbms_driver_ver;
	SQLUINTEGER m_bookmark_persistence;
	SQLUINTEGER m_scroll_concurrency;

	SQLUINTEGER m_pos_operations;

	SQLUINTEGER m_forward_only_cursor_attributes1;
	SQLUINTEGER m_static_cursor_attributes1;
	SQLUINTEGER m_dynamic_cursor_attributes1;
	SQLUINTEGER m_keyset_cursor_attributes1;

	SQLUINTEGER m_max_concurrent_activities;
	SQLUINTEGER m_max_driver_connections;
	SQLUINTEGER get_cursor_attributes1(SQLSMALLINT ctype) const;
	//@}
	bool m_throw_on_error_default;

	map<tstring, tstring> m_macros;
};

class stop_throw:boost::noncopyable
{
	bool old_value;
	connection &c;
public:
	stop_throw(connection &conn, bool do_stop=true):c(conn)
	{
		old_value=c.get_throw_on_error_default();
		c.set_throw_on_error_default(!do_stop);
	}
	~stop_throw()
	{
		c.set_throw_on_error_default(old_value);
	}
};
	//-----------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------//
class transaction:boost::noncopyable
{
public:
	LWODBC_API transaction(connection &c)
		:m_connection(c)
		,m_transaction_is_open(true)
	{
		sqlreturn rc=c.begin_transaction();
		if (rc.fail())
			throw rc;
	}
	LWODBC_API ~transaction();
	sqlreturn LWODBC_API commit();
	sqlreturn LWODBC_API rollback();
	sqlreturn LWODBC_API begin()
	{
		m_transaction_is_open=true;
		return m_connection.begin_transaction();
	}
protected:
	connection &m_connection;
	bool m_transaction_is_open;
};

typedef connection::shared_ptr shared_connection;

inline shared_connection LWODBC_API named_connection(const tstring &name)
{
	return connection::pool().open(name);
}
inline shared_connection LWODBC_API default_connection()
{
	return named_connection(tstring());
}

inline sqlreturn connection::set_read_only(bool yes) throw()
{
	return set_attribute(SQL_ATTR_ACCESS_MODE, yes ? SQL_MODE_READ_ONLY : SQL_MODE_READ_WRITE);
}
inline const sqlreturn &connection::set_autocommit_on(bool yes) throw()
{
	if (m_nested_transactions==0)
		m_autocommit_state=yes;
	return set_attribute(SQL_ATTR_AUTOCOMMIT, yes ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF);
}
inline bool connection::is_autocommit_on() throw()
{
	SQLUINTEGER value;
	get_attribute(SQL_ATTR_AUTOCOMMIT, value);
	return value==SQL_AUTOCOMMIT_ON;
}
inline sqlreturn connection::set_current_catalog(const tstring &catalog) throw()
{
	return set_attribute(SQL_ATTR_CURRENT_CATALOG, catalog);
}
inline sqlreturn connection::get_current_catalog(tstring &catalog) throw()
{
	return get_attribute(SQL_ATTR_CURRENT_CATALOG, catalog);
}
inline tstring connection::get_out_connection_string() const throw()
{
	return m_out_connection_string;
}
inline bool connection::set_uid(const tstring &uid) throw()
{
	m_uid=uid;
	return true;
}
inline bool connection::set_pwd(const tstring &pwd) throw()
{
	m_pwd=pwd;
	return true;
}
inline const tstring &connection::get_pwd() const throw()
{
	return m_pwd;
}
inline const tstring &connection::get_uid() const throw()
{
	return m_uid;
}
inline const tstring& connection::get_dsn() const  throw()

{
	return m_dsn;
}
inline bool connection::is_open_via_SQLConnect() const throw()
{
	return m_remaining_connection_string.length()==0;
}
inline tstring connection::get_macro_value(const tstring &name) const
{
	map<tstring, tstring>::const_iterator i=m_macros.find(name);
	return i!=m_macros.end() ? i->second : tstring(); 
}

#ifdef TEST
void LWODBC_API test(const litwindow::tstring &a, const litwindow::tstring &b, const litwindow::tstring &cc);
#endif

};

};

#pragma warning(pop)

#endif

