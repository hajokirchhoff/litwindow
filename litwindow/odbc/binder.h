/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: binder.h,v 1.5 2006/09/28 12:39:26 Hajo Kirchhoff Exp $
*/
#ifndef _LWODBC_BINDER_
#define _LWODBC_BINDER_

#include <litwindow/lwbase.hpp>
#include <litwindow/tstring.hpp>
#include <litwindow/dataadapter.h>
#include <boost/optional/optional_fwd.hpp>
#include "lwodbc_def.h"
#include "internals.h"

#pragma once

#pragma warning(push, 4)
// now make intellisense work again by undefining LWODBC_API for intellisense
#ifdef INTELLISENSE_DOES_NOT_WORK_PSEUDO_DEFINE
#undef LWODBC_API
#define LWODBC_API
#endif


namespace litwindow {

namespace odbc {;

/** This class holds data that describes one column in a result set.
The class basically holds the data returned by SQLDescribeCol. */
struct column_descriptor {
	LWODBC_API column_descriptor()
		:m_sql_type(SQL_UNKNOWN_TYPE)
		,m_column_size(0)
		,m_decimal(0)
		,m_nullable(SQL_NULLABLE_UNKNOWN)
		,m_position(-1)
		,m_case_sensitive(false)
		,m_auto_unique_value(false)
		,m_updatable(0)
		,m_searchable(0)
		,m_octet_length(0)
		,m_unsigned(SQL_FALSE)
		,m_char_octet_length(-1)
	{}
	LWODBC_API column_descriptor(SQLSMALLINT type, SQLUINTEGER length, SQLSMALLINT decimal=0, SQLSMALLINT nullable=SQL_NULLABLE_UNKNOWN)
		:m_sql_type(type)
		,m_column_size(length)
		,m_decimal(decimal)
		,m_nullable(nullable)
		,m_position(-1)
		,m_case_sensitive(false)
		,m_auto_unique_value(false)
		,m_updatable(0)
		,m_searchable(0)
		,m_octet_length(length)
		,m_unsigned(SQL_FALSE)
		,m_char_octet_length(-1)
	{}

	tstring         m_name;         ///< the name of the column as returned by ODBC
	SQLSMALLINT		m_sql_type;     ///< the SQL_TYPE of the column
	SQLULEN			m_column_size;///< the size of the column
	SQLSMALLINT		m_decimal;      ///< the count of decimal digits
	SQLSMALLINT		m_nullable;     ///< is the column nullable

	SQLSMALLINT		m_position;		///< the column position in a result set
	bool				m_case_sensitive; ///< true if name comparison is case sensitive, false otherwise
	bool				m_auto_unique_value; ///< true if column is assigned a unique value on insert
	SQLLEN			m_updatable;
	SQLLEN			m_searchable;
	SQLLEN			m_octet_length;
	SQLLEN			m_unsigned;
	SQLLEN			m_char_octet_length;	///< maximum length of a VARCHAR or binary column if supported by the driver or -1 if not

	bool			operator ==(const tstring &name) const
	{
		const TCHAR *one=m_name.c_str();
		const TCHAR *two=name.c_str();
		return (m_case_sensitive ? _tcscmp(one, two) : _tcsicmp(one, two)) == 0;
	}

	bool	is_valid() const throw () { return m_sql_type!=SQL_UNKNOWN_TYPE; }

};

//inline bool column_order_pred(const column_descriptor &d1, const column_descriptor &d2)
//{
//	return d1.m_position<d2.m_position;
//}

enum bind_type {
	in = SQL_PARAM_INPUT,
	out = SQL_PARAM_OUTPUT,
	inout = SQL_PARAM_INPUT_OUTPUT,
	bindto,
	unknown_bind_type
};

/** The bind_descriptor holds data that describes the binding of a column
to a C variable in memory. */
struct bind_descriptor {
	LWODBC_API  bind_descriptor()
		:m_c_type(SQL_UNKNOWN_TYPE)
		,m_target_ptr(0)
		,m_target_size(0)
		, m_len_ind_p(0)
		,m_reset_len_ind_to_SQL_NTS_after_execute(false)
	{}
	LWODBC_API  bind_descriptor(SQLSMALLINT c_type, SQLPOINTER target, SQLINTEGER size, SQLLEN *len_ind=0)
		:m_c_type(c_type)
		,m_target_ptr(target)
		,m_target_size(size)
		,m_len_ind_p(len_ind)
		,m_reset_len_ind_to_SQL_NTS_after_execute(false)
	{}

	SQLSMALLINT     m_c_type;       ///< the C_TYPE of the target
	SQLPOINTER      m_target_ptr;   ///< a pointer to the target buffer
	SQLULEN			m_target_size;  ///< the size of the target buffer
	SQLLEN*         m_len_ind_p;    ///< pointer to the length/indicator buffer
	bool	m_reset_len_ind_to_SQL_NTS_after_execute;	///< if true, len_ind will be set to SQL_NTS for a non-output parameter

	bool            is_bound() const { return m_c_type!=SQL_UNKNOWN_TYPE; }

};

struct extended_bind_helper;

struct data_type_info:public column_descriptor, public bind_descriptor
{
	accessor			m_accessor;
	prop_t				m_type;
	extended_bind_helper *m_helper;

	LWODBC_API data_type_info(prop_t type, SQLSMALLINT c_type, SQLSMALLINT sql_type, size_t col_size, extended_bind_helper *bhelper);
	LWODBC_API data_type_info(prop_t type=0)
		:m_type(type), m_helper(0)
	{ }
	inline bool operator<(const data_type_info &t) const { return m_type<t.m_type && !operator==(t); }
	inline bool operator==(const data_type_info &t) const { return is_type_alias(m_type, t.m_type); }

	virtual bool can_handle(prop_t type) const
	{
		bool rc= is_type_alias(m_type, type);
		return rc;
	}
};

extern LWODBC_API data_type_info  no_column;

/// helper class to bind SQL columns to C datatypes that are not directly supported by ODBC
struct extended_bind_helper
{
	/// prepare data_type_info @p info for binding and return the size of the intermediate buffer required by this column
	virtual SQLULEN prepare_bind_buffer(data_type_info &info, statement &s, bind_type bind_howto) const = 0;
	/// get the data from the statement
	virtual sqlreturn get_data(data_type_info &info, statement &s) const = 0;
	/// copy data to the column buffer
	virtual sqlreturn put_data(data_type_info &info, statement &s) const = 0;
	/// get the current length of the data
	virtual SQLINTEGER get_length(data_type_info &info) const = 0;
};

struct bind_task
{
	bind_task():m_by_position(-1),m_in_out(unknown_bind_type),m_cache(0),m_cache_len_ind_p(0) {}
	SQLSMALLINT m_by_position;
	tstring	m_by_name;
	SQLSMALLINT m_in_out;
	data_type_info m_bind_info;
	SQLLEN *m_cache_len_ind_p;
	SQLPOINTER m_cache;
};

class binder
{
public:
	/// Specify how nested aggregate member names are converted to the catalog.schema.table.column scheme
	/** Example:
	\code
		struct superclass {
			int super_one;
		};
		struct inner {
			int inner_one;
		};
		struct outer:public superclass {
			int outer_one;
			inner inner;
		};
	\endcode
	*/
	enum solve_nested_names_enum {
			/** use the first level member name as table name.
				  C++					SQL
				example.inner.inner_one "inner"."inner_one"
				*/
		first_level_is_table = 1,

			/** construct column name by concatenating nested member names using the separator character.
				  C++					SQL
				example.inner.inner_one "inner_inner_one"
				*/
		use_nested_levels = 2,

			/** consider 'inheritance' as nested, like aggregates. If not set, inheritance is considered flat.
				  C++					SQL
				example.super_one	  "superclass"."super_one"   if 'inheritance_as_nested' is set
				example.super_one	  "super_one"			     if 'inheritance_as_nested' is not set
			    */
		inheritance_as_nested = 4,

			/** use the member name only, ignore any nesting
				  C++					SQL
				example.inner.inner_one "one"
				*/
		flat_names = 0,

		default_column_strategy = 3,
		default_parameter_strategy = use_nested_levels
	};

	/// bind a parameter by position to a C variable.
	sqlreturn LWODBC_API  bind_parameter( SQLUSMALLINT pposition, SQLSMALLINT in_out, SQLSMALLINT c_type, SQLSMALLINT sql_type, 
		SQLULEN column_size, SQLSMALLINT decimal_digits, SQLPOINTER buffer, SQLLEN length, SQLLEN *len_ind) throw();
	/// bind a parameter accessor to a position
	sqlreturn LWODBC_API bind_parameter(SQLUSMALLINT pposition, const accessor &a, SQLSMALLINT in_out=unknown_bind_type, SQLLEN *len_ind=0) throw();
	/** bind a parameter by name - this requires a parameter marker in the sql statement. 

	Example: SELECT * FROM my_table WHERE id=?<b><em>([in]variable)</em></b>
	*/
	sqlreturn	LWODBC_API	bind_parameter( const tstring &name, const accessor &a, SQLLEN *len_ind=0) throw();
	/** bind an aggregate as parameters */
	sqlreturn	LWODBC_API	bind_parameter(const aggregate &a, solve_nested_names_enum solver=default_parameter_strategy) throw();
	/** Bind a parameter using a bind_task object. */
	sqlreturn	LWODBC_API	bind_parameter(const bind_task &task);
	/// clear parameter bindings
	sqlreturn LWODBC_API unbind_parameters() throw() 
	{ 
		m_parameters.unbind();
		return sqlreturn(SQL_SUCCESS);
	}

	sqlreturn   LWODBC_API  bind_column(SQLSMALLINT col, SQLSMALLINT c_type, SQLPOINTER target_ptr, SQLINTEGER size, SQLLEN* len_ind) throw();
	sqlreturn   LWODBC_API  bind_column(SQLSMALLINT col, const accessor &a, SQLLEN *len_ind=0);
	/// bind an accessor adapter to a column
	sqlreturn	LWODBC_API	bind_column(const tstring &column_name, const accessor &a, SQLLEN *len_ind=0);

	/// bind an aggregate adapter to the result columns
	sqlreturn	LWODBC_API	bind_column(const aggregate &a, const tstring &table=_T(""), solve_nested_names_enum solver=default_column_strategy);

	sqlreturn               unbind_columns() throw()        
	{ 
		m_columns.unbind();
		return sqlreturn(SQL_SUCCESS);
	}

	sqlreturn unbind() throw() { unbind_parameters(); unbind_columns(); return sqlreturn(SQL_SUCCESS); }

	binder()
		:m_aggregate_scope_separator_char(_T('_'))
	{}

	const data_type_info &get_column(SQLSMALLINT pos) const throw();
	sqlreturn get_column_length(SQLSMALLINT pos, SQLLEN &value) const;
	tstring dump_columns(TCHAR quote_char=_T('"')) const;

	SQLSMALLINT LWODBC_API find_column_by_target(const const_accessor &a) const;

	void set_use_cache(bool yes) throw() { m_columns.use_cache(yes); }
	bool has_cache() const throw() { return m_columns.has_cache(); }
	/// make a column name from a c identifier
	static tstring make_column_name(const tstring &c_identifier);
protected:
	friend class statement;
	friend class table;

	bool needs_bind_parameters() const { return m_parameters.m_needs_bind; }
	/// called just before execute - binds all parameters
	sqlreturn do_bind_parameters(statement &s);

	bool needs_put_parameters() const { return m_parameters.m_needs_put; }
	/// called just before execute - copies data to the parameter buffers
	sqlreturn do_put_parameters(statement &s);

	bool needs_get_parameters() const { return m_parameters.m_needs_get; }
	/// called after fetch/execute - copies data from the parameter buffers to variables
	sqlreturn do_get_parameters(statement &s);

	bool needs_bind_columns() const throw() { return m_columns.m_needs_bind; }
	/// called before the first fetch - bind all columns
	sqlreturn do_bind_columns(statement &s);

	bool needs_put_columns() const throw() { return m_columns.m_needs_put; }
	/// called before insert via SQLBulkOperations - copy the values of the bound column variables to the column buffer
	sqlreturn do_put_columns(statement &s) throw() { return m_columns.put(s); }

	void reset_column_bindings() { m_columns.reset(); }

	bool needs_get_columns() const throw() { return m_columns.m_needs_get; }
	/// called after each fetch - copies data from column buffers to variables
	sqlreturn do_get_columns(statement &s);

	/// call bind_column or bind_parameter for all members of an aggregate
	sqlreturn bind_aggregate(const aggregate &a, size_t level, const tstring &prefix, solve_nested_names_enum solver, bool bind_to_columns);
	sqlreturn bind_aggregate(const aggregate &a, const tstring &name, solve_nested_names_enum solver, bool bind_to_columns);

	typedef vector<bind_task> bind_tasks_t;

	enum fix_workaround_enum {
		set_length,
		reset_length
	};
	void fix_SQL_NTS_len_ind_parameter_workaround(fix_workaround_enum action) throw();

	class binder_lists
	{
	protected:
		size_t	m_version;
		bind_tasks_t m_elements;
		vector<bind_task*> m_index;
		SQLULEN m_intermediate_buffer_size;
		boost::shared_array<unsigned char> m_intermediate_buffer;
		/// reset the state of BIND_TASK (not COLUMN!!!) at 'pos'
		sqlreturn reset_bind_task_state(size_t pos, SQLINTEGER len_ind) throw();
	public:
		bool m_needs_bind, m_needs_get, m_needs_put, m_use_cache;
		void unbind()
		{
			m_elements.clear(); 
			m_index.clear();
			m_intermediate_buffer.reset();
			m_needs_bind=m_needs_get=m_needs_put=false;
		}
		void reset()
		{
			m_needs_bind=m_elements.size()>0;
			m_needs_get=m_needs_put=false;
		}
		sqlreturn reset_states(SQLINTEGER len_ind) throw();
		void add(const bind_task &b)
		{
			m_elements.push_back(b);
			//if (m_elements.back().m_by_name.length() && m_elements.back().m_by_name[0]!=_T('"')) {
			//	litwindow::toupper(m_elements.back().m_by_name);
			//}
			++m_version;
			m_needs_bind=true;
		}
		void use_cache(bool yes=true) { m_use_cache=yes; }
		bool has_cache() const { return m_use_cache; }
		binder_lists():m_needs_bind(false),m_needs_get(false),m_needs_put(false),m_version(0),m_use_cache(false) {}
		sqlreturn prepare_binding(statement &s, bool bind_as_columns, size_t columns_to_expect=0) throw();
		sqlreturn do_get_columns_or_parameters(statement &s, bool type_is_columns);
		sqlreturn put(statement &stmt) throw();
		size_t size() const { return m_elements.size(); }
		sqlreturn set_column_state(SQLUSMALLINT col, SQLLEN len_ind) throw();

		/// return the bind_task for the given column. db column offsets usually start at 1. 0 is reserved for the bookmark column, if there is one.
		bool is_valid_column_index(SQLSMALLINT col) const { return col>=0 && col<(SQLSMALLINT)m_index.size() && m_index[col]!=0; }
		const bind_task *get_bind_task_for_column(SQLSMALLINT col) const { return m_index[col]; }
		/// return the bind_task for the bind_task index. This is different from column indizes! Don't confuse them.
		const bind_task &get_bind_task(size_t index) const { return m_elements[index]; }

		void fix_SQL_NTS_len_ind_parameter_workaround(fix_workaround_enum action);
	};

	SQLSMALLINT find_column_or_parameter_by_target(const binder_lists &list, const const_accessor &a) const;

	enum {
		parameters=0,
		columns=1
	};
	binder_lists m_columns;
	binder_lists m_parameters;

	sqlreturn get_bind_info(const accessor &a, data_type_info &p_desc);

	sqlreturn reset_column_states(SQLINTEGER len_ind) throw() { return m_columns.reset_states(len_ind); }
	sqlreturn set_column_state(SQLUSMALLINT col, SQLLEN len_ind) throw() { return m_columns.set_column_state(col, len_ind); }
	sqlreturn reset_parameter_states(SQLINTEGER len_ind) throw() { return m_parameters.reset_states(len_ind); }
	sqlreturn set_parameter_state(SQLUSMALLINT col, SQLLEN len_ind) throw() { return m_parameters.set_column_state(col, len_ind); }

	sqlreturn do_bind_parameter(bind_task &t, statement  &s) const;
	sqlreturn do_bind_column(bind_task &t, statement &s) const;

	TCHAR m_aggregate_scope_separator_char;

	/// build an 'INSERT INTO ...' statement and bind the current columns as parameters to the new statement
	sqlreturn build_insert_statement_and_bind(tstring &sql, const tstring &table_name, statement *bind_to) const throw();
};


//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
class data_type_registrar;

class data_type_lookup
{
public:
	void		LWODBC_API add(const data_type_info &i, const data_type_registrar *r);
	sqlreturn	LWODBC_API get(litwindow::prop_t type, data_type_info &i);
};

class data_type_registrar
{
public:
};

template <class Value>
class register_data_type:public data_type_registrar
{
public:
	register_data_type(SQLSMALLINT c_type, SQLSMALLINT sql_type, size_t col_size=sizeof(Value), extended_bind_helper *bhelper=0)
		:m_bind_helper_wrapper(bhelper)
	{
		data_type_lookup().add(data_type_info(get_prop_type<Value>(), c_type, sql_type, col_size, bhelper), this);
		data_type_lookup().add(data_type_info(get_prop_type<boost::optional<Value>>(), c_type, sql_type, col_size, &m_bind_helper_wrapper), this);
	}
protected:
	class extended_optional_bind_helper :public extended_bind_helper
	{
	public:
		extended_optional_bind_helper(extended_bind_helper* original_bind_helper)
			:m_original_bind_helper(original_bind_helper) {}
		SQLULEN prepare_bind_buffer(data_type_info& info, statement& s, bind_type bind_howto) const override
		{
			SQLULEN rc;
			auto &a = dynamic_cast_accessor<boost::optional<Value>>(info.m_accessor).get_ref();
			if (m_original_bind_helper) {
				data_type_info original_info(info);
				if (!a) {
					Value v;
					original_info.m_accessor = litwindow::make_accessor(v);
					rc = info.m_target_size = m_original_bind_helper->prepare_bind_buffer(original_info, s, bind_howto);
				}
				else {
					original_info.m_accessor = litwindow::make_accessor(a.get());
					rc = info.m_target_size = m_original_bind_helper->prepare_bind_buffer(original_info, s, bind_howto);
				}
				info.m_target_ptr = original_info.m_target_ptr;
			}
			else {
				info.m_target_ptr = 0;
				rc = info.m_target_size = m_original_bind_helper ? 0 : sizeof(Value);
			}
			return rc;
		}
		sqlreturn get_data(data_type_info& info, statement& s) const override
		{
			sqlreturn rc(SQL_SUCCESS);
			auto a = dynamic_cast_accessor<boost::optional<Value>>(info.m_accessor);
			if (info.m_len_ind_p && *info.m_len_ind_p == SQL_NULL_DATA)
				a.set(boost::optional<Value>());
			else {
				if (m_original_bind_helper) {
					boost::optional<Value>& v_optional(a.get_ref());
					if (!v_optional)
						v_optional.emplace(Value());
					data_type_info original_info(info);
					original_info.m_accessor = litwindow::make_accessor(v_optional.get());
					rc = m_original_bind_helper->get_data(original_info, s);
				}
				else {
					const Value* g = reinterpret_cast<const Value*>(info.m_target_ptr);
					a.set(boost::optional<Value>(*g));
				}
			}
			return rc;
		}
		sqlreturn put_data(data_type_info& info, statement& s) const override
		{
			if (info.m_len_ind_p == nullptr)
				return sqlreturn(_("boost::optional requires m_len_ind_p"), err_logic_error);
			auto a = dynamic_cast_accessor<boost::optional<Value>>(info.m_accessor);
			if (a.get_ptr() == nullptr)
				return sqlreturn(_("boost::optional requires non-temporary"), err_logic_error);
			boost::optional<Value>& optional_a(a.get_ref());
			if (optional_a == boost::none) {
				*info.m_len_ind_p = SQLLEN(SQL_NULL_DATA);
				return sqlreturn(SQL_SUCCESS);
			}
			data_type_info original_info(info);
			auto& v(optional_a.get());
			if (m_original_bind_helper) {
				original_info.m_accessor = litwindow::make_accessor(v);
				return m_original_bind_helper->put_data(original_info, s);
			}
			*reinterpret_cast<Value*>(info.m_target_ptr) = v;
			return sqlreturn(SQL_SUCCESS);
		}
		SQLINTEGER get_length(data_type_info& info) const override
		{
			if (m_original_bind_helper) {
				data_type_info original_info(info);
				boost::optional<Value>& v_optional = dynamic_cast_accessor<boost::optional<Value>>(info.m_accessor).get_ref();
				if (!v_optional) {
					Value v;
					original_info.m_accessor = litwindow::make_accessor(v);
					return m_original_bind_helper->get_length(original_info);
				}
				original_info.m_accessor = litwindow::make_accessor(v_optional.get());
				m_original_bind_helper->get_length(original_info);
			}
			return sizeof(Value);
		}
	protected:
		litwindow::accessor unwrap(const litwindow::accessor& in, bool construct_value) const
		{
			auto& ac(dynamic_cast_accessor<boost::optional<Value>>(in).get_ref());
			if (!ac && construct_value)
				ac.emplace(Value());
			if (!ac)
				return litwindow::accessor();
			auto& v(ac.get());
			return litwindow::make_accessor(v);
		}
		extended_bind_helper* m_original_bind_helper;
	};
	extended_optional_bind_helper m_bind_helper_wrapper;
};

};

};

LWL_DECLARE_ACCESSOR(TIMESTAMP_STRUCT, LWODBC_API);

#pragma warning(pop)

#endif
