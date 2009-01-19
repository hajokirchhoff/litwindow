/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: table.h,v 1.3 2006/06/27 11:23:54 Hajo Kirchhoff Exp $
*/

#ifndef __LWODBC_TABLE_H
#define __LWODBC_TABLE_H

#include <litwindow/dataadapter.h>
#include <boost/utility.hpp>
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

namespace odbc {;

#pragma warning(disable: 4511 4512) // disable 'copy/assignment constructor' could not be generated

class table:public statement
{
public:
	LWODBC_API table();
	LWODBC_API table(connection &c);
	LWODBC_API table(shared_connection &s);
	LWODBC_API table(const tstring &table_name);
	LWODBC_API table(const tstring &table_name, connection &c);
	LWODBC_API table(const tstring &table_name, shared_connection &s);

	/// Bind an aggregate and the table
	sqlreturn LWODBC_API bind_aggregate(const aggregate &a) throw();
	/// Bind an accessor to the table
	sqlreturn LWODBC_API bind_accessor(const accessor &a) throw();
	/// Bind any class that has a data adapter defined to the table
	template <class V>
		sqlreturn bind(V &v)
	{ return bind_aggregate(make_aggregate(v)); }
	template <class V>
		sqlreturn bind(const tstring &column_name, V &v)
	{ return bind_column(column_name, make_accessor(v)); }

	/// Open the table - execute the sql statement created by 'bind'
	const sqlreturn LWODBC_API &open(bool updatable=true, bool scrollable=true) throw();
	const sqlreturn LWODBC_API &open(concurrency_enum cy, bool scrollable=true) throw();
	template <typename Value>
		const sqlreturn &open(Value &v, bool updatable=true, bool scrollable=true) throw()
	{
		bind(v) && open(updatable, scrollable);
		return m_last_error();
	}
	inline const sqlreturn open_read_only() throw() { return open(false, true); }
	inline const sqlreturn open_read_only_forward() throw() { return open(false, false); }
	bool LWODBC_API is_open() const throw();

	const sqlreturn &close() throw() { return clear(); }
	const sqlreturn LWODBC_API &clear() throw();

	const sqlreturn LWODBC_API &close_cursor() throw(); 

	/// add a 'WHERE' clause to the statement
	const sqlreturn LWODBC_API &append_where(const tstring &where, const tstring &op) throw();
	const sqlreturn LWODBC_API &and_where(const tstring &where) throw() { return append_where(where, _T("AND")); }
	const sqlreturn LWODBC_API &or_where(const tstring &where) throw() { return append_where(where, _T("OR")); }
	const sqlreturn LWODBC_API &clear_where() throw() { m_where_clause.erase(); return m_last_error=SQL_SUCCESS; }
	const sqlreturn LWODBC_API &set_where(const tstring &where) throw() { clear_where(); return and_where(where); }

	bool LWODBC_API set_order_by(const tstring &where) throw() { m_order_by_clause=_T(" ORDER BY ")+where; return true; }
	bool LWODBC_API add_order_by(const tstring &where) throw() { m_order_by_clause+= (m_order_by_clause.length()>0 ? _T(", ") : _T(" ORDER BY ")) + where; return true; }

	/** Update the current row - write the values of the bound columns back to the data storage */
	const sqlreturn LWODBC_API &update_row() throw();
	/** Return the number of rows updated by the last call to 'update_row'. Will be ==1 if successful. */
	SQLINTEGER LWODBC_API update_row_count() const throw() { return m_update_row_count; }
	/** Delete the current row - remove the values from the data storage */
	const sqlreturn LWODBC_API &delete_row() throw();
	/** Return the number of rows deleted by the last call to 'update_row'. Will be ==1 if successful. */
	SQLINTEGER LWODBC_API delete_row_count() const throw() { return m_delete_row_count; }
	/** Insert a new row. Copy the values in the bound columns to the data storage */
	const sqlreturn LWODBC_API &insert_row() throw();

	bool set_table(const tstring &table_name) throw() { m_table_name=table_name; return true; }
protected:
	void init(shared_connection &s);
	void init(connection &c);
	const sqlreturn &build_update_statement(bool simulate_positioned_update) throw();
	const sqlreturn &build_delete_statement(bool simulate_positioned_delete) throw();
	const sqlreturn &build_positioned_statement(bool simulate_positioned, bool build_update) throw();
	tstring m_table_name;
	tstring m_order_by_clause;
	tstring m_select_clause;
	tstring m_where_clause;
	tstring m_cursor_name;
	statement m_update_statement;
	SQLINTEGER m_update_row_count;
	statement m_delete_statement;
	SQLINTEGER m_delete_row_count;
	statement m_insert_statement;
};

};

};
#pragma warning(pop)

#endif

