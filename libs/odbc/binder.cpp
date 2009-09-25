/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: binder.cpp,v 1.10 2007/10/18 11:13:17 Hajo Kirchhoff Exp $
*/
#include "stdafx.h"
#include <sqlext.h>
#include <litwindow/dataadapter.h>
#include <litwindow/logging.h>
#include <boost/bind.hpp>
#include <boost/spirit.hpp>
#include <boost/spirit/actor.hpp>
#include <set>
#include <iomanip>
#include "./statement.h"

#define new DEBUG_NEW

#define HAS_BOOST_UUID
#ifdef HAS_BOOST_UUID
#include <boost/uuid.hpp>
template <>
litwindow::tstring litwindow::converter<boost::uuid>::to_string(const boost::uuid &v)
{
    basic_stringstream<TCHAR> out;
    out << v;
    return out.str();
}
template <>
size_t litwindow::converter<boost::uuid>::from_string(const litwindow::tstring &newValue, boost::uuid &v)
{
    basic_stringstream<TCHAR> in(newValue);
    in >> v;
    return sizeof(v);
}
LWL_IMPLEMENT_ACCESSOR(boost::uuid)
#endif


template <>
litwindow::tstring litwindow::converter<TIME_STRUCT>::to_string(const TIME_STRUCT &v)
{
	basic_stringstream<TCHAR> out;
	if (v.hour<=23 && v.minute<=59 && v.second<=59) {
		const TCHAR *fmt=_T("%X");
		struct tm t;
		memset(&t, 0, sizeof(t));
		t.tm_hour=v.hour;
		t.tm_min=v.minute;
		t.tm_sec=v.second;
		use_facet<time_put<TCHAR> >(locale()).put(out.rdbuf(), out, _T(' '), &t, fmt, fmt+sizeof(fmt)/sizeof(*fmt));
	} else {
		out << _T("time_invalid");
	}
	return out.str();
}
namespace {
	using namespace boost::spirit;
	template <typename NumberValue>
	bool parse_time(const litwindow::tstring &newValue, NumberValue &hours, NumberValue &minutes, NumberValue &seconds)
	{
		hours=0; minutes=0; seconds=0;
		return parse(
			newValue.begin(), 
			newValue.end(), 
			limit_d(0u, 23u)[uint_parser<NumberValue, 10, 1, 2>()[assign_a(hours)]] >> 
			!(
			limit_d(0u, 59u)[uint_parser<NumberValue, 10, 2, 2>()[assign_a(minutes)]] >>
			!limit_d(0u,59u)[uint_parser<NumberValue, 10, 2, 2>()[assign_a(seconds)]] ) >>
			!(as_lower_d[str_p(_T("am")) | str_p(_T("pm"))])
			,
			space_p | chset_p(_T(":.-"))
			).full;
	}
};
template <>
size_t litwindow::converter<TIME_STRUCT>::from_string(const litwindow::tstring &newValue, TIME_STRUCT &v)
{
	ios_base::iostate st = 0;
	struct tm t;
	memset(&t, 0, sizeof(t));
	{
		basic_istringstream<TCHAR> in(newValue);
		use_facet<time_get<TCHAR	> >(locale()).get_time(in.rdbuf(), basic_istream<TCHAR>::_Iter(0), in, st, &t);
	}
	if (st & ios_base::failbit) {
		size_t hours=0, minutes=0, seconds=0;
		if (parse_time(newValue, hours, minutes, seconds)==false) {
			v.hour=v.minute=v.second=0;
			throw lwbase_error("invalid time format");
		} else {
			v.hour=SQLUSMALLINT(hours);
			v.minute=SQLUSMALLINT(minutes);
			v.second=SQLUSMALLINT(seconds);
		}
	} else {
		v.hour=t.tm_hour;
		v.minute=t.tm_min;
		v.second=t.tm_sec;
	}
	return sizeof(TIME_STRUCT);
}
LWL_IMPLEMENT_ACCESSOR(TIME_STRUCT)

namespace litwindow {

namespace odbc {;

#ifndef SQL_TVARCHAR
#ifdef UNICODE
#define SQL_TVARCHAR SQL_WVARCHAR
#else
#define SQL_TVARCHAR SQL_VARCHAR
#endif
#endif

//#region binder public interface
sqlreturn binder::bind_parameter(SQLUSMALLINT position, SQLSMALLINT in_out, SQLSMALLINT c_type, SQLSMALLINT sql_type,
				 SQLULEN column_size, SQLSMALLINT decimal_digits, SQLPOINTER buffer, SQLLEN length, SQLLEN *len_ind)
{
	bind_task bi;
	data_type_info &info=bi.m_bind_info;
	info.m_c_type=c_type;
	info.m_column_size=column_size;
	info.m_decimal=decimal_digits;
	info.m_len_ind_p=len_ind;
	info.m_position=position;
	info.m_sql_type=sql_type;
	info.m_target_ptr=buffer;
	info.m_target_size=length;
	info.m_type=0;
	bi.m_by_position=position;
	bi.m_in_out=in_out;
	m_parameters.add(bi);
	return sqlreturn(SQL_SUCCESS);
}

sqlreturn binder::bind_parameter(const bind_task &task)
{
	m_parameters.add(task);
	return sqlreturn(SQL_SUCCESS);
}

sqlreturn binder::bind_parameter(SQLUSMALLINT pposition, const accessor &a, SQLSMALLINT in_out, SQLLEN *len_ind)
{
	bind_task bi;
	sqlreturn rc=get_bind_info(a, bi.m_bind_info);
	if (rc.ok()) {
		bi.m_bind_info.m_len_ind_p=len_ind;
		bi.m_by_position=pposition;
		bi.m_in_out=in_out;
		m_parameters.add(bi);
	}
	return rc;
}

sqlreturn binder::bind_parameter(const tstring &name, const accessor &a, SQLLEN *len_ind)
{
	bind_task bi;
	sqlreturn rc=get_bind_info(a, bi.m_bind_info);
	if (rc.ok()) {
		bi.m_bind_info.m_len_ind_p=len_ind;
		bi.m_by_name=name;
		bi.m_in_out=unknown_bind_type;
		m_parameters.add(bi);
	}
	return rc;
}

sqlreturn	binder::bind_parameter(const aggregate &a, solve_nested_names_enum solver)
{
	return bind_aggregate(a, tstring(), solver, false);
}
sqlreturn   binder::bind_column(SQLSMALLINT col, SQLSMALLINT c_type, SQLPOINTER target_ptr, SQLINTEGER size, SQLLEN* len_ind)
{
	bind_task bi;
	bi.m_bind_info.m_c_type=c_type;
	bi.m_bind_info.m_target_ptr=target_ptr;
	bi.m_bind_info.m_target_size=size;
	bi.m_bind_info.m_position=col;
	bi.m_bind_info.m_len_ind_p=len_ind;
	bi.m_by_position=col;
	m_columns.add(bi);
	return sqlreturn(SQL_SUCCESS);
}

sqlreturn   binder::bind_column(SQLSMALLINT col, const accessor &a, SQLLEN *len_ind)
{
	bind_task bi;
	sqlreturn rc=get_bind_info(a, bi.m_bind_info);
	if (rc.ok()) {
		bi.m_by_position=col;
		if (len_ind)
			bi.m_bind_info.m_len_ind_p=len_ind;
		m_columns.add(bi);
	}
	return rc;
}

sqlreturn	binder::bind_column(const tstring &column_name, const accessor &a, SQLLEN *len_ind)
{
	bind_task bi;
	sqlreturn rc=get_bind_info(a, bi.m_bind_info);
	if (rc.ok()) {
		bi.m_by_name=column_name;
		if (len_ind)
			bi.m_bind_info.m_len_ind_p=len_ind;
		m_columns.add(bi);
	}
	return rc;
}

sqlreturn	binder::bind_column(const aggregate &a, const tstring &table, solve_nested_names_enum solver)
{
	return bind_aggregate(a, table, solver, true);
}
//#endregion

sqlreturn binder::build_insert_statement_and_bind(tstring &sql, const tstring &table_name, statement *bind_to) const
{
	tstring bind_parameters;
	size_t i;
	sqlreturn rc;
	if (m_columns.size()>0) {
		size_t last_element=m_columns.size()-1;
		SQLUSMALLINT parameter_index=0;
		for (i=0; i<=last_element && rc; ++i) {
			const bind_task &b(m_columns.get_bind_task(i));
			if (b.m_bind_info.m_len_ind_p==0 || (*b.m_bind_info.m_len_ind_p!=SQL_IGNORE && *b.m_bind_info.m_len_ind_p!=SQL_DEFAULT)) {
					// bind values only if they are neither SQL_IGNORE or SQL_DEFAULT
				if (sql.length()==0)
					sql=_T("INSERT INTO ")+table_name+_T(" (");
				else
					sql+=_T(", ");
				sql+= b.m_by_name;
				bind_parameters+= bind_parameters.length()>0 ? _T(", ?") : _T(") VALUES (?");
				if (bind_to) {
					if (b.m_bind_info.m_accessor.is_valid())
						rc=bind_to->bind_parameter_accessor(++parameter_index, SQL_PARAM_INPUT, b.m_bind_info.m_accessor, b.m_bind_info.m_len_ind_p);
					else
						rc=bind_to->bind_parameter(++parameter_index, SQL_PARAM_INPUT, b.m_bind_info.m_c_type, b.m_bind_info.m_sql_type,
										b.m_bind_info.m_column_size, b.m_bind_info.m_decimal, b.m_bind_info.m_target_ptr, b.m_bind_info.m_target_size, b.m_bind_info.m_len_ind_p);
				}
			}
		}
	}
	if  (sql.length()==0) {
		return sqlreturn (_T("Table has no columns or all columns are being SQL_IGNORE-d. Cannot build insert statement."), err_logic_error);
	}
	sql+=bind_parameters + _T(")");
	if (rc && bind_to)
		bind_to->set_statement(sql);
	return rc;
}

tstring binder::make_column_name(const tstring &c_identifier)
{
	if (c_identifier.substr(0, 2)==_T("m_"))
		return c_identifier.substr(2);
	return c_identifier;
}

sqlreturn binder::bind_aggregate(const aggregate &a, size_t level, const tstring &prefix, solve_nested_names_enum solver, bool bind_to_columns)
{
	sqlreturn rc;
	aggregate::iterator i;
	for (i=a.begin(); i!=a.end() && rc.ok(); ++i) {
		tstring column_name(make_column_name(s2tstring(i->get_name())));
		tstring full_column_name;
		if ((solver & use_nested_levels)!=0  && prefix.length()>0)
			full_column_name=prefix+m_aggregate_scope_separator_char+column_name;
		else
			full_column_name=column_name;
		// attempt to bind the member as it is (completely) to a column.
		if (bind_to_columns)
			rc=bind_column(full_column_name, *i);
		else
			rc=bind_parameter(full_column_name, *i);
		if (rc.ok()==false) {
			// member could not be bound directly. See if it is an aggregate itself and bind its individual members if it is.
			if (i->is_aggregate()) {
				tstring new_prefix;
				if (i->is_inherited() && (solver&inheritance_as_nested)==0)
					new_prefix=prefix;
				else
					new_prefix=full_column_name;
				rc=bind_aggregate(i->get_aggregate(), level+1, new_prefix, solver, bind_to_columns);
			}
		}
	}
	return rc;
}

/// bind an aggregate adapter to the result columns
sqlreturn	binder::bind_aggregate(const aggregate &a, const tstring &prefix, solve_nested_names_enum solver, bool bind_to_columns)
{
	tstring use_name;
	if (is_null(prefix))
		use_name=s2tstring(a.get_class_name());
	else
		use_name=prefix;
	return bind_aggregate(a, 0, use_name, solver, bind_to_columns);
}

sqlreturn binder::get_bind_info(const accessor &a, data_type_info &p_desc)
{
	sqlreturn rc=data_type_lookup().get(a.get_type(), p_desc);
	if (rc.ok()) {
		p_desc.m_accessor=a;
		p_desc.m_target_ptr=a.get_member_ptr();
		if (p_desc.m_target_size==0)
			p_desc.m_target_size=(SQLINTEGER)a.get_sizeof();
	}
	return rc;
}

sqlreturn binder::do_bind_parameter(bind_task &b, statement &s) const
{
	const parameter *p=s.get_parameter(b.m_by_position);
	if (p) {
		if (b.m_in_out==unknown_bind_type) {
			b.m_in_out=p->m_bind_type;
		} else if (b.m_in_out!=p->m_bind_type) {
			return sqlreturn(_("bind type used in the statement is different from bind parameter used in the C++ source")+b.m_by_name, odbc::err_logic_error);
		}
	} // else no parameter marker in statement
	sqlreturn rc=s.do_bind_parameter(b.m_by_position, b.m_in_out, b.m_bind_info.m_c_type, b.m_bind_info.m_sql_type, b.m_bind_info.m_column_size,
		b.m_bind_info.m_decimal, b.m_bind_info.m_target_ptr, b.m_bind_info.m_target_size, b.m_bind_info.m_len_ind_p);
	return rc;
}
sqlreturn binder::do_bind_column(bind_task &b, statement &s) const
{
	sqlreturn rc=s.do_bind_column(b.m_by_position, b.m_bind_info.m_c_type, b.m_bind_info.m_target_ptr, b.m_bind_info.m_target_size, b.m_bind_info.m_len_ind_p);
	return rc;
}
/** bind all parameters in the 'to bind' list to the statement. */
sqlreturn binder::do_bind_parameters(statement &s)
{
	return m_parameters.prepare_binding(s, false);
}

sqlreturn binder::do_bind_columns(statement &s)
{
	return m_columns.prepare_binding(s, true, s.get_column_count());
}

sqlreturn binder::binder_lists::reset_bind_task_state(size_t pos, SQLINTEGER len_ind)
{
	bind_task &b(m_elements[pos]);
	if (len_ind==statement::use_defaults) {
		if (b.m_bind_info.m_helper)	// use the bind helper
		{
			*b.m_bind_info.m_len_ind_p= b.m_bind_info.m_helper->get_length(b.m_bind_info);
		} else if (b.m_bind_info.m_accessor.is_c_vector() && // use SQL_NTS if it is a char or wchar_t vector (old style C string)
			((is_type<wchar_t>(b.m_bind_info.m_type) && b.m_bind_info.m_accessor.get_sizeof()>sizeof(wchar_t)) ||
			(is_type<char>(b.m_bind_info.m_type) && b.m_bind_info.m_accessor.get_sizeof()>sizeof(char))) ) 
		{
			*b.m_bind_info.m_len_ind_p=SQL_NTS;
		} else	// use target_size as default size
		{
			*b.m_bind_info.m_len_ind_p=b.m_bind_info.m_target_size;
		}
	} else
		*b.m_bind_info.m_len_ind_p=len_ind;
	return sqlreturn(SQL_SUCCESS);
}
sqlreturn binder::binder_lists::reset_states(SQLINTEGER len_ind)
{
	size_t i;
	for (i=0; i<m_elements.size(); ++i) {
		reset_bind_task_state(i, len_ind);
	}
	return sqlreturn(SQL_SUCCESS);
}
sqlreturn binder::binder_lists::set_column_state(SQLUSMALLINT col, SQLLEN len_ind)
{
	if (col>=m_index.size())
		return sqlreturn(_("no such column."), odbc::err_no_such_column);
	*m_index[col]->m_bind_info.m_len_ind_p=len_ind;
	return sqlreturn(SQL_SUCCESS);
}
sqlreturn binder::binder_lists::put()
{
	sqlreturn rc;
	size_t i;
	for (i=0; i<m_elements.size() && rc; ++i) {
		bind_task &b(m_elements[i]);
		if (b.m_bind_info.m_helper) {
			rc=b.m_bind_info.m_helper->put_data(b.m_bind_info);
		}
	}
	return rc;
}

struct reset_intermediate_buffer_pointers 
{
	reset_intermediate_buffer_pointers(const unsigned char *buffer, SQLUINTEGER size)
	{
		begin_ptr=buffer;
		if (buffer)
			end_ptr=buffer+size;
		else
			end_ptr=0;
	}
	const unsigned char *begin_ptr, *end_ptr;
	void operator()(SQLPOINTER &p) const
	{
		if  ((const unsigned char*)p>=begin_ptr && (const unsigned char*)p<end_ptr)
			p=0;
	}
	void operator()(SQLINTEGER * &p) const
	{
		if  ((const unsigned char*)p>=begin_ptr && (const unsigned char*)p<end_ptr)
			p=0;
	}
};
sqlreturn binder::binder_lists::prepare_binding(statement &s, bool bind_as_columns, size_t columns_to_expect)
{
	if (columns_to_expect)
		m_index.resize(columns_to_expect+1, 0);
	sqlreturn rc;
	m_needs_bind=false;
	fill(m_index.begin(), m_index.end(), static_cast<bind_task*>(0));
	// the 'intermediate' buffer will be reset in this method
	// some pointers (cache, len_ind_p) might point into the intermediate buffer
	// to find and reset them, store beginning and end of the intermediate buffer
	reset_intermediate_buffer_pointers reset_ptrs((const unsigned char*)m_intermediate_buffer.get(), m_intermediate_buffer_size);
	m_intermediate_buffer_size=0;
	size_t i;
	for (i=0; i<m_elements.size(); ++i) {
		bind_task &b(m_elements[i]);
		if (m_intermediate_buffer.get()) {
			// reset pointers if they point into an existing intermediate buffer
			reset_ptrs(b.m_bind_info.m_len_ind_p);
			reset_ptrs(b.m_bind_info.m_target_ptr);
			reset_ptrs(b.m_cache);
			reset_ptrs(b.m_cache_len_ind_p);
		}
		if (b.m_by_position==-1) {
			SQLSMALLINT p;
			if (bind_as_columns) {
				p=s.find_column(b.m_by_name);
			} else {
				p=s.find_parameter(b.m_by_name);
				if (p!=-1) {
					const parameter *para=s.get_parameter(p);
					if (b.m_in_out==unknown_bind_type) {
						b.m_in_out=para->m_bind_type;
					} else if (b.m_in_out!=para->m_bind_type) {
						return sqlreturn(_("bind type used in the statement is different from bind parameter used in the C++ source")+b.m_by_name, odbc::err_logic_error);
					}
				}
			}
			if (p==-1) {
				return sqlreturn(tstring(_("no such "))+(bind_as_columns ? _("column ") : _("parameter "))+b.m_by_name, bind_as_columns ? err_no_such_column : err_no_such_parameter);
			}
			b.m_by_position=p;
		} else {
			tstringstream str;
			str << _("#:") << b.m_by_position;
			b.m_by_name=str.str();
		}
		if (b.m_by_position>=(SQLSMALLINT)m_index.size())
			m_index.resize(b.m_by_position+1, 0);
		m_index[b.m_by_position]=&m_elements[i];
		b.m_bind_info.m_position=b.m_by_position;
		if (b.m_bind_info.m_helper) {
			// this is an extended binder, prepare the intermediate buffer if neccessary
			m_intermediate_buffer_size+=b.m_bind_info.m_helper->prepare_bind_buffer(b.m_bind_info, s, bind_as_columns ? bindto : bind_type(b.m_in_out));
		}
		if (bind_as_columns==false && (b.m_bind_info.m_column_size==0 || b.m_bind_info.m_column_size==SQL_NTS) && b.m_bind_info.m_accessor.is_valid()) {
			// no column size specified. Try to calculate it.
			if (is_type<char>(b.m_bind_info.m_accessor))
				b.m_bind_info.m_column_size=SQLUINTEGER(b.m_bind_info.m_accessor.get_sizeof()/sizeof(char)-1);
			else if (is_type<wchar_t>(b.m_bind_info.m_accessor))
				b.m_bind_info.m_column_size=SQLUINTEGER(b.m_bind_info.m_accessor.get_sizeof()/sizeof(wchar_t)-1);
		}
		if (b.m_bind_info.m_len_ind_p==0) {
			m_intermediate_buffer_size+=sizeof(SQLLEN);
		}
		if (has_cache() && b.m_bind_info.m_target_size) {
			m_intermediate_buffer_size+=b.m_bind_info.m_target_size+sizeof(SQLLEN);
		}
	}
	m_intermediate_buffer.reset(m_intermediate_buffer_size>0 ? new unsigned char[m_intermediate_buffer_size] : 0);
	unsigned char *buffer=m_intermediate_buffer.get();
	m_needs_get=m_needs_put=false;
	SQLUINTEGER size_left=m_intermediate_buffer_size;
	for (i=0; i<m_elements.size() && rc.ok(); ++i) {
		bind_task &b(m_elements[i]);
		if (b.m_bind_info.m_helper && b.m_bind_info.m_target_ptr==0 && b.m_bind_info.m_target_size>0) {
			if (b.m_bind_info.m_target_size>size_left) {
				return sqlreturn(_("extended bind helper requesting more buffer space than is available"), odbc::err_logic_error);
			}
			b.m_bind_info.m_target_ptr=buffer;
			buffer+=b.m_bind_info.m_target_size;
			size_left-=b.m_bind_info.m_target_size;
			if (bind_as_columns)
				m_needs_put=m_needs_get=true;
			else {
				m_needs_put= b.m_in_out==SQL_PARAM_INPUT || b.m_in_out==SQL_PARAM_INPUT_OUTPUT;
				m_needs_get= b.m_in_out==SQL_PARAM_INPUT_OUTPUT || b.m_in_out==SQL_PARAM_OUTPUT;
			}
		}
		if (b.m_bind_info.m_len_ind_p==0) {
			b.m_bind_info.m_len_ind_p=(SQLLEN*)buffer;
			reset_bind_task_state(i, statement::use_defaults);
			//*b.m_bind_info.m_len_ind_p=b.m_bind_info.m_target_size;
			buffer+=sizeof(SQLLEN);
			size_left-=sizeof(SQLLEN);
		}
		if (has_cache() && b.m_bind_info.m_target_size) {
			b.m_cache=(SQLPOINTER)buffer;
			buffer+=b.m_bind_info.m_target_size;
			b.m_cache_len_ind_p=(SQLLEN*)buffer;
			buffer+=sizeof(SQLLEN);
			m_needs_get=true;
			size_left-=sizeof(SQLLEN)+b.m_bind_info.m_target_size;
		} else {
			b.m_cache=0;
			b.m_cache_len_ind_p=0;
		}
		if (b.m_bind_info.m_target_size>0 && b.m_bind_info.m_target_ptr!=0) {
			if (bind_as_columns) {
				rc=s.do_bind_column(b.m_by_position, b.m_bind_info.m_c_type, b.m_bind_info.m_target_ptr, b.m_bind_info.m_target_size, b.m_bind_info.m_len_ind_p);
			} else {
				if (b.m_bind_info.m_column_size==0)
					m_needs_bind=true;	// dynamic column size needs rebinding every time
				rc=s.do_bind_parameter(b.m_by_position, b.m_in_out, b.m_bind_info.m_c_type, b.m_bind_info.m_sql_type, b.m_bind_info.m_column_size,
					b.m_bind_info.m_decimal, b.m_bind_info.m_target_ptr, b.m_bind_info.m_target_size, b.m_bind_info.m_len_ind_p);
			}
		}
	}
	return rc;
}

void binder::binder_lists::fix_SQL_NTS_len_ind_parameter_workaround(fix_workaround_enum action)
{
	size_t pos;
	for (pos=0; pos<m_elements.size(); ++pos) {
		bind_task &b(m_elements[pos]);
		if (action==set_length) {
			b.m_bind_info.m_reset_len_ind_to_SQL_NTS_after_execute=false;
			if (b.m_in_out!=out && b.m_bind_info.m_len_ind_p && *b.m_bind_info.m_len_ind_p==SQL_NTS) {
				if (b.m_bind_info.m_c_type==SQL_C_CHAR)
					*b.m_bind_info.m_len_ind_p=(SQLINTEGER)strlen((const char*)b.m_bind_info.m_target_ptr) * sizeof(char);
				else if (b.m_bind_info.m_c_type==SQL_C_WCHAR)
					*b.m_bind_info.m_len_ind_p=(SQLINTEGER)wcslen((const wchar_t*)b.m_bind_info.m_target_ptr) * sizeof(wchar_t);
				b.m_bind_info.m_reset_len_ind_to_SQL_NTS_after_execute=true;
			}
		} else if (action==reset_length && b.m_bind_info.m_reset_len_ind_to_SQL_NTS_after_execute && b.m_in_out==in) {
			*b.m_bind_info.m_len_ind_p=SQL_NTS;
		}
	}
}

void binder::fix_SQL_NTS_len_ind_parameter_workaround(fix_workaround_enum action)
{
	m_parameters.fix_SQL_NTS_len_ind_parameter_workaround(action);
}

SQLSMALLINT binder::find_column_or_parameter_by_target(const binder_lists &list, const const_accessor &a) const
{
	size_t i;
	for (i=0; i<list.size() && a.is_alias_of(list.get_bind_task(i).m_bind_info.m_accessor)==false; ++i)
		;
	return i<list.size() ? list.get_bind_task(i).m_by_position : -1;
}

SQLSMALLINT binder::find_column_by_target(const const_accessor &a) const
{
	return find_column_or_parameter_by_target(m_columns, a);
}

sqlreturn binder::do_put_parameters(statement &s)
{
	return m_parameters.put();
}
sqlreturn binder::do_get_parameters(statement &s)
{
	return sqlreturn(SQL_ERROR);
}

sqlreturn binder::binder_lists::do_get_columns_or_parameters(statement &s, bool type_is_columns)
{
	if (type_is_columns==false)
		DebugBreak();	//TODO: get_parameters not implemented yet!!!
	size_t i;
	sqlreturn rc;
	for (i=0; i<m_elements.size() && rc.ok(); ++i) {
		bind_task &b(m_elements[i]);
		if (b.m_bind_info.m_helper) {
			rc=b.m_bind_info.m_helper->get_data(b.m_bind_info, s);
		}
		if (b.m_cache) {
			memcpy(b.m_cache, b.m_bind_info.m_target_ptr, b.m_bind_info.m_target_size);
			*b.m_cache_len_ind_p=*b.m_bind_info.m_len_ind_p;
		}
	}
	return rc;
}

sqlreturn binder::do_get_columns(statement &s)
{
	return m_columns.do_get_columns_or_parameters(s, true);
}

data_type_info no_column;

tstring binder::dump_columns(TCHAR quote_char) const
{
	size_t i;
	tstring rc;
	for (i=0; i<m_columns.size(); ++i) {
		if (rc.length()>0)
			rc.append(_T(", "));
		if (quote_char)
			rc.append(1, quote_char);
		rc.append(m_columns.get_bind_task(i).m_by_name);
		if (quote_char)
			rc.append(1, quote_char);
	}
	return rc;
}
const data_type_info &binder::get_column(SQLSMALLINT pos) const
{
	return m_columns.is_valid_column_index(pos) ? m_columns.get_bind_task_for_column(pos)->m_bind_info : no_column;
}

sqlreturn binder::get_column_length(SQLSMALLINT pos, SQLLEN &value) const
{
	const data_type_info i=get_column(pos);
	if (&i==&no_column)
		return sqlreturn(_("no such column"), odbc::err_no_such_column);
	else if (i.m_len_ind_p==0)
		return sqlreturn(_("column not bound"), odbc::err_column_not_bound);
	value=*i.m_len_ind_p;
	return sqlreturn(SQL_SUCCESS);
}

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
// look up types

typedef vector<data_type_info> data_type_info_register;

data_type_info_register &get_data_type_info_register()
{
	static data_type_info_register s_map;
	return s_map;
}

data_type_info_register::const_iterator find_data_type_info(prop_t type)
{
	data_type_info_register::const_iterator rc=find(get_data_type_info_register().begin(), get_data_type_info_register().end(), type);
	return rc;
}

void data_type_lookup::add(const data_type_info &i, const data_type_registrar *)
{
#ifdef REGISTER_IS_A_SET
	pair<data_type_info_register::iterator, bool> insertion=get_data_type_info_register().insert(i);
	if (insertion.second==false) {
		lw_err() << _("Type cannot be inserted twice! ") << litwindow::s2tstring(i.m_type->get_type_name()) << endl;
	}
#endif
	get_data_type_info_register().push_back(i);
}

sqlreturn data_type_lookup::get(prop_t type, data_type_info &i)
{
	data_type_info_register &the_map(get_data_type_info_register());
	data_type_info_register::const_iterator data=find_data_type_info(type);
	if (data==the_map.end())
		data=find_if(the_map.begin(), the_map.end(), boost::bind(&data_type_info::can_handle, _1, type));
	if (data==the_map.end())
		return sqlreturn(_("There is no registered SQL binder for type ")+s2tstring(type->get_type_name()), odbc::err_no_such_type);
	i=*data;
	return sqlreturn(SQL_SUCCESS);
}

data_type_info::data_type_info(prop_t type, SQLSMALLINT c_type, SQLSMALLINT sql_type, size_t col_size, extended_bind_helper *bhelper)
{ 
	m_sql_type=sql_type; 
	m_c_type=c_type; 
	m_type=type; 
	m_helper=bhelper; 
	m_column_size=(SQLUINTEGER)col_size;
}


namespace {
	static register_data_type<long> tlong(SQL_C_SLONG, SQL_INTEGER);
	static register_data_type<int> tint(SQL_C_SLONG, SQL_INTEGER);
	static register_data_type<short> tshort(SQL_C_SSHORT, SQL_INTEGER);
	static register_data_type<unsigned long> tulong(SQL_C_ULONG, SQL_INTEGER);
	static register_data_type<unsigned int>tuint(SQL_C_ULONG, SQL_INTEGER);
	static register_data_type<unsigned short> tushort(SQL_C_USHORT, SQL_INTEGER);

#define LWODBC_SQL_C_BOOL SQL_C_CHAR
	static register_data_type<bool> tbool(/*LWODBC_SQL_C_BOOL*/SQL_C_BIT, SQL_CHAR);
	static register_data_type<char> tchar(SQL_C_CHAR, SQL_VARCHAR, 0);
	static register_data_type<wchar_t> twchar(SQL_C_WCHAR, SQL_WVARCHAR, 0);
	static register_data_type<TIMESTAMP_STRUCT> ttimestampstruct(SQL_C_TIMESTAMP, SQL_TIMESTAMP);
	static register_data_type<float> tfloat(SQL_C_FLOAT, SQL_REAL); 
	static register_data_type<double> tdouble(SQL_C_DOUBLE, SQL_DOUBLE);
	static register_data_type<double> tdouble2(SQL_C_DOUBLE, SQL_FLOAT);
	static register_data_type<TIME_STRUCT> ttime_struct(SQL_C_TIME, SQL_TIME);
    static register_data_type<boost::uuid> tuuid(SQL_C_GUID, SQL_GUID);

    struct tuuid_bind_helper:public extended_bind_helper
    {
        virtual sqlreturn get_data(data_type_info &info, statement &s) const
        {
            sqlreturn rc;
            typed_accessor<boost::uuid> a=dynamic_cast_accessor<boost::uuid>(info.m_accessor);
            boost::uuid target;
            unsigned char u[16];

        }
    };

struct tstring_bind_helper:public extended_bind_helper
{
	virtual SQLUINTEGER prepare_bind_buffer(data_type_info &info, statement &s, bind_type bind_howto) const
	{
		SQLSMALLINT pos=info.m_position;
		SQLUINTEGER sz;
		info.m_target_ptr=0;	// tell the binder we need an intermediate buffer
		sqlreturn rc;
		if (bind_howto==bindto) {
			if (info.m_sql_type==SQL_TVARCHAR)
				sz=maximum_text_column_length_retrieved;
			else {
				rc=s.get_column_size(pos, sz);
				if (sz==0)
					sz=maximum_text_column_length_retrieved;	// use default maximum size
			}
		} else {
			if (info.m_column_size==0) {
				typed_const_accessor<tstring> ta=dynamic_cast_accessor<tstring>(info.m_accessor);
				sz=SQLINTEGER(ta.get_ptr() ? ta.get_ptr()->length() : ta.get().length());
			} else
				sz=info.m_column_size;
		}
		sz+=1;	// trailing \0 character
		sz*=sizeof(TCHAR);	// might be unicode representation
		info.m_target_size=rc.ok() ? sz : 0;	// tell the binder the size of the intermediate buffer
		return sz;
	}
	virtual sqlreturn get_data(data_type_info &info, statement &s) const
	{
		sqlreturn rc;
		typed_accessor<tstring> a=dynamic_cast_accessor<tstring>(info.m_accessor);
		if (info.m_len_ind_p && (*info.m_len_ind_p==SQL_NULL_DATA || *info.m_len_ind_p==0))
			a.set(_T(""));
		else 
			a.set(tstring((const TCHAR*)info.m_target_ptr));
		return sqlreturn(SQL_SUCCESS);
	}
	sqlreturn put_data(data_type_info &info) const
	{
		bool data_truncated=false;
// 		if (info.m_len_ind_p)
// 			*info.m_len_ind_p=SQL_NTS;
		if (info.m_len_ind_p==0 || (*info.m_len_ind_p!=SQL_NULL_DATA && *info.m_len_ind_p!=SQL_DEFAULT_PARAM)) {
			typed_accessor<tstring> a=dynamic_cast_accessor<tstring>(info.m_accessor);
			tstring *s=a.get_ptr();
			size_t required_length;
			if (s) {
				required_length=s->length()*sizeof(TCHAR)+sizeof(TCHAR);
				if (required_length>info.m_target_size) {
					required_length=info.m_target_size;
					data_truncated=true;
				}
				memcpy(info.m_target_ptr, s->c_str(), required_length);
			} else {
				// needs temporary tstring
				tstring temp;
				a.get(temp);
				required_length=temp.length()*sizeof(TCHAR)+sizeof(TCHAR);
				if (required_length>info.m_target_size) {
					required_length=info.m_target_size;
					data_truncated=true;
				}
				memcpy(info.m_target_ptr, temp.c_str(), required_length);
			}
		}
		return data_truncated ? sqlreturn(_("String data right truncation (tstring)"), err_data_right_truncated, _T("22001")) : sqlreturn(SQL_SUCCESS);
	}
	SQLINTEGER get_length(data_type_info &info) const
	{
		return SQL_NTS;
	}
} g_tstring_bind_helper;

static register_data_type<tstring> ttstring(SQL_C_TCHAR, SQL_TVARCHAR, 0, &g_tstring_bind_helper);
};

};

};

using namespace std;
template <>
litwindow::tstring litwindow::converter<TIMESTAMP_STRUCT>::to_string(const TIMESTAMP_STRUCT &v)
{
	tstringstream str;
	str << setfill(_T('0')) << setw(4) << v.year << _T('-') << setw(2) << (unsigned)v.month << _T('-') << setw(2) << (unsigned)v.day << _T(' ') << setw(2) << (unsigned)v.hour << _T(':') << setw(2) << (unsigned)v.minute << _T(':') << setw(2) << (unsigned)v.second << _T('.') << v.fraction;
	return str.str();
}
LWL_IMPLEMENT_ACCESSOR(TIMESTAMP_STRUCT);
