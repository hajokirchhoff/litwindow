/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: lwodbc.h,v 1.9 2007/07/24 17:35:24 Hajo Kirchhoff Exp $
*/

#ifndef __LWODBC__
#define __LWODBC__

#include <litwindow/lwbase.hpp>
#include <litwindow/tstring.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>
#include <sqlext.h>
#include <vector>
#include "./lwodbc_def.h"
#include "./odbc_fwd.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

namespace litwindow {

namespace odbc {

const size_t maximum_text_column_length_retrieved=65535;

//-----------------------------------------------------------------------------------------------------------//
using namespace std;

extern LWODBC_API const tstring null_string;
extern LWODBC_API const tstring default_string;
extern LWODBC_API const tstring any_string; ///< wild card
inline bool LWODBC_API is_null(const tstring &test) { return null_string==test; }
inline bool LWODBC_API is_default(const tstring &test) { return test==default_string; }
inline bool LWODBC_API is_any(const tstring &test) { return test==any_string; }

enum error_code {
	/// bind has been called but the statement contains no ([bind_type] identifier) elements
	err_statement_has_no_parameters = 1,
	/// bind has been called with parameter 'id' but statement does not contain a parameter of this name
	err_no_such_parameter,
	/// the c data type has not been registered with the lwodbc C - SQL type conversion system
	err_no_such_type,
	/// the result column description is not yet available, because neither prepare nor execute has been called
	err_result_column_description_not_available,
	/// attempted to bind a column by name that has already been bound. Call 'unbind' first if you want to rebind a column.
	err_cannot_bind_column_twice,
	/// cannot parse SQL statement
	err_parsing_SQL_statement,
	/// error while executing a script
	err_script_execution_error,
	/// DBMS specific error, see error message for details
	err_dbms_specific_error,
	/// action is not supported by this DBMS
	err_not_supported_by_dbms,
	/// sequence error: a function was called on a connection, statement or table, but the object was not in the right state. This is a programming error.
	err_state_sequence_error,
	/// the column (ordinal) does not exist
	err_no_such_column,
	/// the column has not been bound
	err_column_not_bound,
	/// the simulated positioned update/delete operation failed due to a concurrency conflict. Another process has updated the row.
	err_cursor_operation_conflict,
	/// the string data was right truncated
	err_data_right_truncated,

	/// programming logic
	err_logic_error,
	/// completely unexpected error
	err_unknown_error
};

struct sqldiag {
	TCHAR state[5];
	SQLINTEGER native_error;
	tstring	msg;
	sqldiag() {}
	sqldiag(TCHAR p_state[5], SQLINTEGER p_native_error, const tstring &p_msg):native_error(p_native_error),msg(p_msg)
	{
		memcpy(state, p_state, sizeof(state));
	}
	const tstring	LWODBC_API &message() const { return msg; }
};

/// encapsules the SQLRETURN code and handles errors
class sql_diagnostic_records:public boost::noncopyable {
public:
	sql_diagnostic_records():m_ref_count(0) {}
	sql_diagnostic_records(const sql_diagnostic_records &r)
		:m_ref_count(0),m_records(r.m_records)
	{}

	void LWODBC_API copy_diag(const sql_diagnostic_records *r);

	/// get diagnostic records
	int LWODBC_API get_diagnostic_records(SQLSMALLINT htype, SQLHANDLE handle);

	LWODBC_API const sqldiag & diag(int idx=-1) const;

	bool LWODBC_API is_state(const TCHAR state[5]) const;

	void append_diag(const sqldiag &r);
	void clear() { m_records.clear(); }
	void inc_ref() { ++m_ref_count; }
	size_t dec_ref() { return --m_ref_count; }
	bool is_last_ref() const { return m_ref_count==1; }
	void log_to_stream(tostream &out) const throw();
	size_t size() const { return m_records.size(); }
	const sqldiag &back() const throw() { return m_records.back(); }
	const sqldiag &get(size_t i) const { return m_records.at(i); }
protected:
	size_t	m_ref_count;
	vector<sqldiag> m_records;
};

inline void intrusive_ptr_add_ref(sql_diagnostic_records *r)
{
	r->inc_ref();
}

inline void intrusive_ptr_release(sql_diagnostic_records *r)
{
	if (r->dec_ref()==0)
		delete r;
}
typedef boost::intrusive_ptr<sql_diagnostic_records> sql_diag_ptr_t;

/** store SQLRETURN codes and provide access to diagnostics records.
*/
class sqlreturn
{
public:
	explicit LWODBC_API sqlreturn(SQLRETURN rc=SQL_SUCCESS)			///< create new object from SQLRETURN code
		:m_rc(rc),m_diag(0)
	{}
	LWODBC_API sqlreturn(const tstring &msg, error_code error, TCHAR state[5]=0);	///< create new object from error message, set code to SQL_ERROR, state to 'LWODB'
    LWODBC_API sqlreturn(const TCHAR *msg, error_code error, TCHAR state[5]=0);	///< create new object from error message, set code to SQL_ERROR, state to 'LWODB'

	bool success() const throw()	 { return m_rc==SQL_SUCCESS || m_rc==SQL_SUCCESS_WITH_INFO; }	///< test for SQL_SUCCESS or SQL_SUCCESS_WITH_INFO
	bool ok() const throw()						{ return success(); }
	bool fail() const throw()						{ return !ok(); }
	void assert_success() const												///< throw runtime_error if not success
	{
		if (log_errors())
			throw runtime_error("sqlreturn.ok()==false");
	}

	bool no_data() const throw()												///< test for SQL_NO_DATA
	{ return m_rc==SQL_NO_DATA; }
	/// test for odbc state HYC00 - optional feature not implemented by driver
	bool optional_feature_not_implemented() const throw() { return m_rc==SQL_ERROR && is_state(_T("HYC00")); }
	bool option_value_changed() const throw() { return m_rc==SQL_SUCCESS_WITH_INFO && is_state(_T("01S02")); }
	bool cursor_operation_conflict() const throw() { return (m_rc==SQL_SUCCESS_WITH_INFO || m_rc==SQL_ERROR) && is_state(_T("01001")); }
	bool string_data_right_truncation() const throw() { return m_rc==SQL_ERROR && is_state(_T("22001")); }
	bool driver_does_not_support_this_function() const throw() { return m_rc==SQL_ERROR && is_state(_T("IM001")); }
	/// constraints violation such as foreign key, NULL-nonNULLable etc...
	bool constraints_violation() const throw() { return m_rc==SQL_ERROR && is_state(_T("23***")); }
	/// test for 'permission denied'. \todo Prüfen, ob dies für alle Datenbanken funktioniert, oder ob man pro Datenbank testen muß... Vermutlich pro Datenbank.
	bool permission_denied() const throw() { return m_rc==SQL_ERROR && is_state(_T("42501")); }
    /// integrity constraint is usually returned by inserts and updates
    bool integrity_constraint_violation() const throw() { return m_rc==SQL_ERROR && is_state(_T("23000")); }
	/// log any errors and return true if errors logged or false if ok()
	bool LWODBC_API log_errors() const throw() { return success() ? false : do_log_errors(); }
	bool LWODBC_API ok_log() const { return log_errors()==false; }

	const sqldiag LWODBC_API &diagnostics(size_t index) const;

	bool operator==(SQLRETURN code) const throw()	{ return m_rc==code; }
	bool operator!=(SQLRETURN code) const throw()	{ return !operator==(code); }

	const sqlreturn LWODBC_API &operator=(const sqlreturn &c);	///< copy code and diagnostics

	void clear() throw()							{ set(SQL_SUCCESS); }

	bool LWODBC_API	is_state(const TCHAR state[5]) const				///< test if the first sqlreturn diagnostics record has a given state
	{ return m_diag && m_diag->is_state(state); }

	tstring LWODBC_API as_string() const throw();					///< format the error as a string so that it can be shown to the user
	void LWODBC_API append_diag(const sqldiag &r);

	const sqlreturn LWODBC_API &set(SQLRETURN code);			///< set the return code, clear diagnostics

	typedef sql_diag_ptr_t sqlreturn::*unspecified_bool_type;
	operator unspecified_bool_type() const
	{
		return success() ? &sqlreturn::m_diag : 0;
	}

	bool has_diagnostics() const { return m_diag && m_diag->size()>0; }

	SQLRETURN get_code() const throw() { return m_rc; }
	/// set the return code but leave diagnostic records unchanged
	void set_code(SQLRETURN rc) { m_rc=rc; }
	SQLINTEGER native_error() const throw() { return has_diagnostics() ? m_diag->back().native_error : err_logic_error; }

	static TCHAR g_lwodb_state[];
protected:
	//const sqlreturn LWODBC_API &operator=(SQLRETURN c);
	SQLRETURN m_rc;
	sql_diag_ptr_t	m_diag;
	bool do_log_errors() const throw();

	/// create a new copy of m_diag data and return it - if neccessary
	sql_diagnostic_records *copy_on_write() throw();
};

class sqlreturn_auto_set_diagnostics:public sqlreturn
{
public:
	sqlreturn_auto_set_diagnostics():m_htype(0),m_handle(0),m_throw_on_error(false),m_log_errors(true) {}
	const sqlreturn_auto_set_diagnostics LWODBC_API &operator=(const sqlreturn &code);
	const sqlreturn_auto_set_diagnostics LWODBC_API &operator=(SQLRETURN code);

	void set_handles(SQLSMALLINT htype, SQLHANDLE handle)	///< associate a handle with the sqlreturn object
	{
		m_htype=htype; m_handle=handle;
	}
	void zero_handles()															///< remove associated handles
	{
		m_htype=0; m_handle=0;
	}
	void LWODBC_API set_throw_on_error(bool do_throw) throw() { m_throw_on_error=do_throw; }
	bool LWODBC_API is_throw_on_error() const throw() { return m_throw_on_error; }
	bool LWODBC_API get_throw_on_error() const { return is_throw_on_error(); }
	void LWODBC_API set_log_errors(bool do_log) throw() { m_log_errors=do_log; }
	bool LWODBC_API is_log_errors() const throw() { return m_log_errors; }
	static void LWODBC_API set_log_diagnostics(bool do_log=true);
	/// ignore a state once - do not log and do not throw if this state is detected
	/// can be called multiple times to add ignore states and can contain a comma separated list of states
	bool LWODBC_API ignore_once(const TCHAR *state) throw();
	bool LWODBC_API is_ignored_state() const throw();
	LWODBC_API const TCHAR *get_ignore_once() const throw();
protected:
	static bool g_log_diagnostics;
	tstring m_ignore_once;
	void get_diagnostics();
	SQLSMALLINT m_htype;
	SQLHANDLE m_handle;
	bool m_throw_on_error;
	bool m_log_errors;
};

class multi_part_name
{
public:
	LWODBC_API multi_part_name(size_t parts);
	tstring LWODBC_API full_name(bool quoted_identifiers=true) const;
protected:
	void parse(const tstring &name);
	vector<tstring> m_names;
};

class table_name:public multi_part_name
{
protected:
	table_name(size_t parts):multi_part_name(parts) {}	// used by column_name
public:
	LWODBC_API table_name(const tstring &identifier=tstring()):multi_part_name(3) { parse(identifier); }
	LWODBC_API table_name(const tstring &catalog, const tstring &schema, const tstring &table);
	LWODBC_API const tstring &catalog() const { return m_names[0]; }
	LWODBC_API const tstring &schema() const { return m_names[1]; }
	LWODBC_API const tstring &table() const { return m_names[2]; }

	LWODBC_API void set_catalog(const tstring &c) { m_names[0]=c; }
	LWODBC_API void set_schema(const tstring &s) { m_names[1]=s; }
	LWODBC_API void set_table(const tstring &t) { m_names[2]=t; }
};

class column_name:public table_name
{
public:
	LWODBC_API column_name(const tstring &identifier=tstring()):table_name(4) { parse(identifier); }
	LWODBC_API column_name(const tstring &catalog, const tstring &schema, const tstring &table, const tstring &column);
	LWODBC_API const tstring &column() const { return m_names[3]; }
	LWODBC_API void set_column(const tstring &c) { m_names[3]=c; }
};

struct sql_type_info_result_set
{
	tstring		TYPE_NAME;
	SQLSMALLINT	DATA_TYPE;
	SQLINTEGER	COLUMN_SIZE;
	tstring		LITERAL_PREFIX, LITERAL_SUFFIX, CREATE_PARAMS;
	SQLSMALLINT	NULLABLE, CASE_SENSITIVE, SEARCHABLE, UNSIGNED_ATTRIBUTE, FIXED_PREC_SCALE,
		AUTO_UNIQUE_VALUE;
	tstring		LOCAL_TYPE_NAME;
	SQLSMALLINT	MINIMUM_SCALE, MAXIMUM_SCALE, SQL_DATA_TYPE, SQL_DATETIME_SUB;
	SQLINTEGER	NUM_PREC_RADIX;
	SQLSMALLINT	INTERVAL_PRECISION;
};

};

};

#pragma warning(pop)

#endif

