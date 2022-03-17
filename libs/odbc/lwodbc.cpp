/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: lwodbc.cpp,v 1.8 2007/01/12 11:46:11 Hajo Kirchhoff Exp $
*/

#include "stdafx.h"
#include <sqlext.h>
#include <malloc.h>
#include <litwindow/logging.h>
#include "litwindow/odbc/lwodbc.h"
#include "litwindow/odbc/connection.h"
#include <iomanip>

#pragma warning(disable: 4312 4267)

#define new DEBUG_NEW

namespace litwindow {

namespace odbc {;

multi_part_name::multi_part_name(size_t size)
{
	m_names.resize(size);
}

tstring multi_part_name::full_name(bool quoted_identifiers) const
{
	tstring rc;
	for (size_t i=0; i<m_names.size(); ++i) {
		if (rc.length() || m_names[i].length()) {
			if (rc.length())
				rc+=_T('.');
			if (quoted_identifiers)
				rc+=_T('"');
			rc+=m_names[i];
			if (quoted_identifiers)
				rc+=_T('"');
		}
	}
	return rc;
}

void multi_part_name::parse(const tstring &name)
{
	vector<tstring> rc;
	litwindow::split_string(rc, name, _T("."), _T("\""));
	if (rc.size()<=m_names.size()) {
		std::copy(rc.begin(), rc.end(), m_names.end()-rc.size());
		if (rc.size()<m_names.size())
			std::fill(m_names.begin(), m_names.end()-rc.size(), tstring());
	} else {
		throw runtime_error("invalid multi_part_name: too many parts");
	}
}

table_name::table_name(const tstring &catalog, const tstring &schema, const tstring &table)
:multi_part_name(3)
{
	m_names[0]=catalog; m_names[1]=schema; m_names[2]=table;
}

column_name::column_name(const tstring &catalog, const tstring &schema, const tstring &table, const tstring &column)
:table_name(4)
{
	m_names[0]=catalog; m_names[1]=schema; m_names[2]=table; m_names[3]=column;
}

// null_string should be different from ("") so that it is not accidentially compared to ""
static TCHAR NULL_STRING[]=_T("null//81CB2CF8-1D32-40e7-B26F-C4D9307E6302//\0\x1\0");
const tstring null_string(NULL_STRING, sizeof(NULL_STRING));
static TCHAR DEFAULT_STRING[]=_T("default//F39AA77F-6561-4d87-B353-8C29078016D4//\0\0\x1\0");
const tstring default_string(DEFAULT_STRING, sizeof(DEFAULT_STRING));
static TCHAR ANY_STRING[]=_T("any//1AF0C300-46E3-43b8-8619-0271B5987F62//\x1\0\0");
const tstring any_string(ANY_STRING, sizeof(ANY_STRING));

sqlreturn::sqlreturn(const tstring &msg, error_code error, TCHAR state[5])
{
	if (state==0)
		state=g_lwodb_state;
	copy_on_write()->append_diag(sqldiag(state, error, msg));
	m_rc=SQL_ERROR;
}

sqlreturn::sqlreturn(const TCHAR *msg, error_code error, TCHAR state[5])
{
    if (state==0)
        state=g_lwodb_state;
    copy_on_write()->append_diag(sqldiag(state, error, tstring(msg)));
    m_rc=SQL_ERROR;
}

TCHAR sqlreturn::g_lwodb_state[]={_T("LWODB")};

bool sqlreturn::do_log_errors() const
{
	lw_log() << as_string();
	return true;
}

tstring sqlreturn::as_string() const
{
	const TCHAR* p;
	tostringstream out;
	switch (m_rc) {
		    case SQL_SUCCESS : p=_T("SQL_SUCCESS"); break;
		    case SQL_SUCCESS_WITH_INFO: p=_T("SQL_SUCCESS_WITH_INFO"); break;
		    case SQL_ERROR: p=_T("SQL_ERROR"); break;
		    default: p=0; out << _T("SQL error code [0x") << hex << setw(8) << m_rc << _T("]"); break;
	}
	if (p)
		out << p;
	if (m_diag)
		m_diag->log_to_stream(out);
	return out.str();
}

const sqlreturn &sqlreturn::set(SQLRETURN code)
{
	m_rc=code;
	if (m_diag) {	// don't clear if there isn't a diagnostics anyway
		copy_on_write()->clear();
	}
	return *this;
}

//const sqlreturn &sqlreturn::operator=(SQLRETURN code)
//{
//	return set(code);
//}
//
const sqlreturn &sqlreturn::operator =(const sqlreturn &c)
{
	m_rc=c.m_rc;
	if (m_diag!=c.m_diag) {
		//if (m_diag) {
		//	if (m_diag->is_last_ref()==false)
		//		m_diag=new sql_diagnostic_records(m_diag->m_htype, m_diag->m_handle);	// copy on write and keep our own handle/type
		//	m_diag->copy_diag(c.m_diag.get());
		//} else
		m_diag=c.m_diag;
	}
	return *this;
}

void sqlreturn::append_diag(const sqldiag &r)
{
	copy_on_write()->append_diag(r);
}

//-----------------------------------------------------------------------------------------------------------//


bool LWODBC_API sql_diagnostic_records::is_state(const TCHAR state[5]) const
{
	if (m_records.size()==0)
		return false;				// no state at all
	// dangerous gotcha: do not use sizeof(state) since 'state' is actually a pointer here!!!
	for (size_t i=0; i<5; ++i) {
		if (state[i]!=m_records.back().state[i] && state[i]!='*')
			return false;
	}
	return true;
}
//const sql_diagnostic_records::sqldiag &sql_diagnostic_records::diag(int idx) const
//{
//	if (idx==-1)
//		idx=m_number_of_fields-1;
//	return m_diag[idx];
//}

#ifdef _DEBUG
static TCHAR *debug_break_on_state=/*_T("HY010")*/_T("");
#endif

int sql_diagnostic_records::get_diagnostic_records(SQLSMALLINT htype, SQLHANDLE handle)
{
	m_records.clear();
	SQLSMALLINT i=0;
	while (true) {
		SQLTCHAR state[6];
		SQLTCHAR msg[4192];
		SQLSMALLINT msg_length;
		SQLINTEGER native_error;
		SQLRETURN rc=SQLGetDiagRec(htype, handle, ++i, state, &native_error, msg, sizeof(msg)/sizeof(*msg), &msg_length);
		if (rc==SQL_NO_DATA)
			break;
		else if (rc==SQL_SUCCESS || rc==SQL_SUCCESS_WITH_INFO) {
			m_records.push_back(sqldiag());
			memcpy(m_records.back().state, state, sizeof(m_records.back().state));
			m_records.back().msg=(TCHAR*)msg;
			m_records.back().native_error=native_error;
#ifdef _DEBUG
			if (is_state(debug_break_on_state))
				DebugBreak();
#endif
		} else {
			m_records.push_back(sqldiag(_T("LWODB"), err_unknown_error, _T("No diagnostic record available: SQLGetDiagRec returned an error")));
			break;
		}
	}
	return m_records.size();
}
void sql_diagnostic_records::append_diag(const sqldiag &new_d)
{
	m_records.push_back(new_d);
}
void sql_diagnostic_records::log_to_stream(tostream &out) const
{
	if (m_records.size()>1)
		out << endl;
	else
		out << _T(": ");
	size_t i;
	for (i=0; i<m_records.size(); ++i) {
		const sqldiag &current(m_records[i]);
		if (m_records.size()>1)
			out << setw(2) << i << _T(": ");
		out << _T("[") << tstring(current.state, 5) << _T("] (") << current.native_error << _T(") ") << current.msg << endl;
	}
}

//-----------------------------------------------------------------------------------------------------------//

sql_diagnostic_records *sqlreturn::copy_on_write()
{
	if (m_diag==0)
		m_diag=new sql_diagnostic_records;
	else if (m_diag->is_last_ref()==false) {
		m_diag=new sql_diagnostic_records(*m_diag);
	}
	return m_diag.get();
}

const sqldiag &sqlreturn::diagnostics(size_t index) const
{
	if (m_diag && m_diag->size()>=1 && index<m_diag->size()) {
		return m_diag->get(index);
	}
	static sqldiag err_no_diag_record(_T("LWODB"), err_logic_error, _("<<Logic error: Requested diagnostics record does not exist!>>"));
	return err_no_diag_record;
}

const sqlreturn_auto_set_diagnostics &sqlreturn_auto_set_diagnostics::operator=(const sqlreturn &code)
{ 
	sqlreturn::operator=(code); 
	if (m_throw_on_error && m_rc==SQL_ERROR && (m_ignore_once.length()==0 || is_ignored_state()==false) )
		throw *this;
	m_ignore_once.clear();
	return *this; 
}
const sqlreturn_auto_set_diagnostics &sqlreturn_auto_set_diagnostics::operator=(SQLRETURN code)
{
	set(code);
	get_diagnostics();
	if (m_throw_on_error && m_rc==SQL_ERROR && (m_ignore_once.length()==0 || is_ignored_state()==false) )
		throw *this;
	m_ignore_once.clear();
	return *this;
}

void sqlreturn_auto_set_diagnostics::get_diagnostics()
{
	if (m_htype!=0 && m_handle!=0) {
		copy_on_write()->get_diagnostic_records(m_htype, m_handle);
		if (m_log_errors && g_log_diagnostics && has_diagnostics() && (m_ignore_once.length()==0 || is_ignored_state()==false) && !success()) {
			lw_log() << as_string();
		}
	} else
		m_diag=0;
}

bool sqlreturn_auto_set_diagnostics::g_log_diagnostics=SQLRETURN_GLOBAL_LOG_DEFAULT ;

void sqlreturn_auto_set_diagnostics::set_log_diagnostics(bool do_log) { g_log_diagnostics=do_log; }

bool sqlreturn_auto_set_diagnostics::ignore_once(const TCHAR *state)
{
	if (!m_ignore_once.empty())
		m_ignore_once += _T(',');
	m_ignore_once+=state;
	return true;
}

const TCHAR * sqlreturn_auto_set_diagnostics::get_ignore_once() const
{
	return m_ignore_once.c_str();
}

bool sqlreturn_auto_set_diagnostics::is_ignored_state() const
{
	if (m_diag==0)
		return false;
	TCHAR test_state[5];
	const TCHAR *p=m_ignore_once.c_str();
	/*
	 Hier müßte man alle m_ignore_once states mit allen m_diag states vergleichen. Zur Zeit werden
	 nur alle m_ignore_once states gegen den letzten m_diag state verglichen (.back() in is_state).
	*/
	//for (size_t i = 0; i < m_diag->size() && *p; ++i) {
	while (*p) {
		while (*p==_T(' ') || *p==_T(','))
			++p;
		for (size_t j=0; j<5; ++j) {
			if (*p && *p!=_T(','))
				test_state[j]=*p++;
			else
				test_state[j]=0;
		}
		if (test_state[0]==_T('*') && test_state[1]==0)
			return true;	// ignore everything
		if (m_diag->is_state(test_state))
			return true;
	}
	return false;
}
};

};

LWL_BEGIN_AGGREGATE(litwindow::odbc::sql_type_info_result_set)
PROP(TYPE_NAME)
PROP(DATA_TYPE)
PROP(COLUMN_SIZE)
PROP(LITERAL_PREFIX)
PROP(LITERAL_SUFFIX)
PROP(CREATE_PARAMS)
PROP(NULLABLE)
PROP(CASE_SENSITIVE)
PROP(SEARCHABLE)
PROP(UNSIGNED_ATTRIBUTE)
PROP(FIXED_PREC_SCALE)
PROP(AUTO_UNIQUE_VALUE)
PROP(LOCAL_TYPE_NAME)
PROP(MINIMUM_SCALE)
PROP(MAXIMUM_SCALE)
PROP(SQL_DATA_TYPE)
PROP(SQL_DATETIME_SUB)
PROP(NUM_PREC_RADIX)
PROP(INTERVAL_PRECISION)
LWL_END_AGGREGATE()
