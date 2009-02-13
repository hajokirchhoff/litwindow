/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: table.cpp,v 1.8 2006/11/28 13:44:03 Hajo Kirchhoff Exp $
*/

#include "stdafx.h"
#include <litwindow/logging.h>
#include <litwindow/check.hpp>
#include "table.h"

#define new DEBUG_NEW

//#pragma warning(disable: 4312 4267)

namespace litwindow {

namespace odbc {;

table::table() { init(default_connection()); }
table::table(connection &c) { init(c); }
table::table(shared_connection &s) { init(s); }
table::table(const tstring &table_name) { init(default_connection()); set_table(table_name); }
table::table(const tstring &table_name, connection &c) { init(c); set_table(table_name); }
table::table(const tstring &table_name, shared_connection &s) { init(s); set_table(table_name); }

void table::init(shared_connection &s)
{
	set_connection(s);
	m_update_statement.set_connection(s);
	m_delete_statement.set_connection(s);
	m_insert_statement.set_connection(s);
}

void table::init(connection &c)
{
	set_connection(c);
	m_update_statement.set_connection(c);
	m_delete_statement.set_connection(c);
	m_insert_statement.set_connection(c);
}

/** Update the current row.
	\note For state 01001 (cursor operation conflict), update_row() will change the return code from SQL_SUCCESS_WITH_INFO to SQL_ERROR. update_row() should
		delete exactly one row. If it does not, it should return SQL_ERROR, not SQL_SUCCESS_WITH_INFO which SQLSetPos returns.*/
const sqlreturn &table::update_row()
{
	bool simulate_positioned_update=false;
	m_update_row_count=0;
	if (statement::has_update_row()) {	// use statement::update() if it is supported
		if (statement::update_row()) {
			SQLRETURN rc=SQLRowCount(handle(), &m_update_row_count);
			if (rc!=SQL_SUCCESS)
				return m_last_error=rc;
		}
	} else {
		m_update_statement.set_throw_on_error(get_throw_on_error());
		m_update_statement.ignore_once(get_ignore_once());
		simulate_positioned_update=has_POSITIONED_UPDATE()==false;
		if (!build_update_statement(simulate_positioned_update))
			return m_last_error;
		m_last_error=m_update_statement.execute();
		m_update_statement.get_row_count(m_update_row_count);
	}
	if (m_update_row_count!=1 && m_update_row_count!=-1) {
		if (simulate_positioned_update) {
			m_last_error.set(SQL_ERROR);
			m_last_error.append_diag(sqldiag(_T("01001"), err_cursor_operation_conflict, _("Cursor operation conflict. Positioned updated/delete: original row has changed.")));
		} else if (m_last_error.cursor_operation_conflict()) {
			m_last_error.set_code(SQL_ERROR);
		}
	} 
	return m_last_error;
}

const sqlreturn &table::delete_row()
{
	bool simulate_positioned_delete=false;
	m_delete_row_count=0;
	if (statement::has_delete_row()) {	// use statement::delete_row() if it is supported
		if (statement::delete_row()) {
			SQLRETURN rc=SQLRowCount(handle(), &m_delete_row_count);
			if (rc!=SQL_SUCCESS)
				return m_last_error=rc;
		}
	} else {
		m_delete_statement.set_throw_on_error(get_throw_on_error());
		m_delete_statement.ignore_once(get_ignore_once());
		simulate_positioned_delete=has_POSITIONED_DELETE()==false;
		if (!build_delete_statement(simulate_positioned_delete))
			return m_last_error;
		m_last_error=m_delete_statement.execute();
		m_delete_statement.get_row_count(m_delete_row_count);
	}
	if (m_delete_row_count!=1) {
		if (simulate_positioned_delete) {
			m_last_error.set(SQL_ERROR);
			m_last_error.append_diag(sqldiag(_T("01001"), err_cursor_operation_conflict, _("Cursor operation conflict. Positioned updated/delete: original row has changed.")));
		} else if (m_last_error.cursor_operation_conflict()) {
			m_last_error.set_code(SQL_ERROR);
		}
	} 
	return m_last_error;
}

/* Folgende Probleme:
	postgres verträgt SQL_NTS nicht als len_ind für C-String Input Parameters
	MySQL liefert bei SQLDescribeCol nicht die maximale Länge einer varchar Spalte zurück, sondern die Länge des ersten Datensatzes
	Insert per 'statement::insert_row' setzt ein Execute des SQL statements voraus. Das ist u.U. sehr kostspielig, wenn der Benutzer
		einfach nur eine neue Zeile in eine Tabelle einfügen möchte. Dann wäre ein simples, selbstgebautes Insert Statement einfacher.

	Folgen für insert_row:
	für postgres muß ich alle SQL_NTS selbst berechnen, bevor ich ein Statement ausführe.
	Generell muß ich eine andere Methode finden, um die Länge von Spalten herauszufinden.
	*/
const sqlreturn &table::insert_row()
{
	bool use_statement_insert_row=has_insert_row();
	if (use_statement_insert_row) {
		statement::insert_row();
	} else {
		// build insert statement and execute it
        if (m_insert_statement.has_been_prepared()==false) {
		tstring sql;
		m_insert_statement.set_throw_on_error(get_throw_on_error());
		m_insert_statement.ignore_once(get_ignore_once());
		m_last_error=m_binder.build_insert_statement_and_bind(sql, m_table_name, &m_insert_statement);
        }
		if (m_last_error)
			m_last_error=m_insert_statement.execute();
		SQLINTEGER rowcount;
		//SQLRETURN rc=SQLRowCount(m_insert_statement.handle(), &rowcount);
	}
	return m_last_error;
}

const sqlreturn &table::build_update_statement(bool simulate_positioned_update)
{
	return build_positioned_statement(simulate_positioned_update, true);
}

const sqlreturn &table::build_delete_statement(bool simulate_positioned_delete)
{
	return build_positioned_statement(simulate_positioned_delete, false);
}

const sqlreturn &table::build_positioned_statement(bool simulate_positioned, bool build_update)
{
	statement &the_statement(build_update ? m_update_statement : m_delete_statement);
	if (simulate_positioned && m_binder.m_columns.has_cache()==false)
		return m_last_error=sqlreturn(_T("internal error: cannot simulate positioned update - binder cache was not used."), err_logic_error);
	size_t cCount=m_binder.m_columns.size();
	size_t i;
	tstring sql;
	tstring where_of;
	if (simulate_positioned==false) {
		if (m_cursor_name.length()==0 && get_cursor_name(m_cursor_name).fail())
			return m_last_error;
		where_of=_T(" WHERE CURRENT OF ")+m_cursor_name;
	}
	SQLSMALLINT parameter_position=1;
	// make two passes over all bound columns
	// pass one to build the sql statement and bind all columns that can be updated
	// pass two only if simulate_positioned to bind all cached values in the WHERE clause
	size_t bind_parameter_pass=build_update ? 0 : 1;
	while (bind_parameter_pass==0 || simulate_positioned && bind_parameter_pass==1) {
		for (i=0; i<cCount; ++i) {
			const bind_task &current(m_binder.m_columns.get_bind_task(i));
			if (current.m_bind_info.m_len_ind_p==0 || (*current.m_bind_info.m_len_ind_p!=SQL_IGNORE && *current.m_bind_info.m_len_ind_p!=SQL_DEFAULT)) {
				litwindow::AbortOn(current.m_by_position>=(int)m_column_list.size() || current.m_by_position<0, "internal error: a binder element has no column in the result set");
				column_descriptor &cur_col(m_column_list.at(current.m_by_position));
				if (bind_parameter_pass==1 || cur_col.m_updatable) {
					if (bind_parameter_pass==1) {
						where_of+= (where_of.length()==0 ? _T(" WHERE ") : _T(" AND ")) + cur_col.m_name + _T("=?");
					} else {
						if (sql.length())
							sql+=_T(", ");
						sql+=cur_col.m_name+_T("=?");
					}
					// bind either the current (new) column value (pass==0) or the cached value (pass==1)

					bind_task p;
					p.m_by_position=parameter_position;
					p.m_in_out=odbc::in;
					p.m_bind_info=current.m_bind_info;
					if (bind_parameter_pass==1) {
						p.m_bind_info.m_target_ptr=current.m_cache;
						p.m_bind_info.m_len_ind_p=current.m_cache_len_ind_p;
					}
#ifdef _NOT
					//BUG: This will fail for parameters that need a bind_helper and where the target_ptr has changed after the last fetch/bind operation.
					// This happens for example in the following case:
					//     tstring a_string;    stmt.bind(a_string); stmt.fetch(); a_string="Other value"; stmt.update();
					//  In this case the target_ptr will still point to the value before the   ="Other value"  assignment. stmt.update() will fail.
					// Reason is that parameters with a bind_helper must not be bound directly to their m_target_ptr!
					the_statement.bind_parameter(parameter_position, odbc::in, current.m_bind_info.m_c_type, cur_col.m_sql_type,
						cur_col.m_column_size, cur_col.m_decimal, 
						bind_parameter_pass==0 ? current.m_bind_info.m_target_ptr : current.m_cache,
						current.m_bind_info.m_target_size, 
						bind_parameter_pass==0 ? current.m_bind_info.m_len_ind_p : current.m_cache_len_ind_p);
#endif // _NOT
					// This is the correct way to bind parameters here. Fix for bug above.
					the_statement.bind_parameter(p);
					++parameter_position;
				}
			}
		}
		++bind_parameter_pass;
	};
	if (build_update) {
		sql=_T("UPDATE ")+m_table_name+_T(" SET ")+sql+where_of;
	} else {
		sql=_T("DELETE FROM ")+m_table_name+where_of;
	}
	m_last_error=the_statement.set_statement(sql);
	return m_last_error;
}

sqlreturn table::bind_aggregate(const aggregate &a)
{
	if (!is_reusable()) {
		return sqlreturn(_T("table must be closed before binding it to a different structure"), odbc::err_state_sequence_error);
	}
	m_last_error=statement::bind_column(a);
	if (m_table_name.length()==0)
		set_table(s2tstring(a.get_class_name()));
	return m_last_error;
}

const sqlreturn &table::open(bool updatable, bool scrollable)
{
	return open(updatable ? optimistic : none, scrollable);
}

const sqlreturn &table::open(concurrency_enum cy, bool scrollable)
{
	if (cy==lock && has_update_row()==false && has_POSITIONED_UPDATE()==false)
		return m_last_error=sqlreturn(_("concurrency==lock (row locking) requires SQLSetPos or Positioned Update (WHERE CURRENT OF), which this connection does not support."), err_not_supported_by_dbms);
	concurrency_enum actual_concurrency;
	if (!set_updatable(cy))
		return m_last_error;
	if (cy==lock && (!get_concurrency(actual_concurrency) || actual_concurrency!=lock)) {
		return m_last_error=sqlreturn(_("concurrency==lock (row locking) is not supported by this DBMS"), err_not_supported_by_dbms);
	}
	set_statement(_T("SELECT ")+m_binder.dump_columns()+_T(" FROM ")+m_table_name+m_where_clause+m_order_by_clause) &&
		set_scrollable(scrollable) &&
		set_use_cache(cy==optimistic && has_update_row()==false && has_POSITIONED_UPDATE()==false) &&
		execute();
	return m_last_error;
}

bool table::is_open() const
{
	return has_been_executed();
}

const sqlreturn &table::append_where(const tstring &where, const tstring &op)
{
	litwindow::context_t c("odbc::table::append_where");
	litwindow::Precondition(is_open()==false, "cannot set where condition for an open table");
	if (m_where_clause.length()>0)
		m_where_clause=_T("(") + m_where_clause + _T(") ") + op;
	else
		m_where_clause=_T(" WHERE");
	m_where_clause+=_T(" (") + where + _T(")");
	return m_last_error= SQL_SUCCESS;
}

const sqlreturn &table::close_cursor()
{
	m_update_statement.clear();
	m_delete_statement.clear();
    m_insert_statement.clear();
	statement::close_cursor();
	return m_last_error;
}
const sqlreturn &table::clear()
{
	close_cursor();
	m_table_name.clear();
	m_order_by_clause.clear();
	m_where_clause.clear();
	m_select_clause.clear();
	m_cursor_name.clear();
	statement::clear();
	return m_last_error;
}

};

};
