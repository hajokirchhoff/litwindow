/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: internals.h,v 1.3 2006/06/27 11:23:54 Hajo Kirchhoff Exp $
*/
#ifndef _LWODBC_INTERNALS_
#define _LWODBC_INTERNALS_

#include <litwindow/lwbase.hpp>
#include <litwindow/tstring.hpp>
#include <litwindow/dataadapter.h>
#include "lwodbc.h"

#pragma once

#pragma warning(push, 4)
// now make intellisense work again by undefining LWODBC_API for intellisense
#ifdef INTELLISENSE_DOES_NOT_WORK_PSEUDO_DEFINE
#undef LWODBC_API
#define LWODBC_API
#endif

namespace litwindow {

namespace odbc {

struct scope {
	tstring catalog, schema, table;
	scope() {}
	scope(const tstring &t):table(t) {}
	scope(const tstring &t, const tstring &s):table(t),schema(s) {}
	scope(const tstring &t, const tstring &s, const tstring &c):table(t),schema(s),catalog(c) {}

	tstring make_name(TCHAR separator_char=_T('.'), TCHAR quote_char=_T('"')) const;

	bool operator < (const scope &s) const throw();
	bool operator==(const scope &s) const throw()
	{
		return table==s.table && schema==s.schema && catalog==s.catalog;
	}
	bool operator !=(const scope &s) const throw() { return !operator==(s); }

	bool partial_match(const tstring &to_match, const tstring &value) const
	{
		return to_match.length()==0 || is_any(to_match) || to_match==value;
	}

	bool partial_match(const scope &s) const
	{
		return partial_match(table, s.table) && partial_match(schema, s.schema) && partial_match(catalog, s.catalog);
	}

	bool is_fully_qualified() const
	{
		return is_any(table)==false && table.length()>0 || is_any(schema)==false && schema.length()>0 || is_any(catalog)==false && catalog.length()>0;
	}
};

/// parse a fully qualified identifier and split it into scope and name
extern pair<scope, tstring> LWODBC_API parse_scope(const tstring &name);

extern scope LWODBC_API null_scope;


/** This helper template class is a specialized map that allows looking up columns by partially qualified name.
This class is a lookup class for names that follow the catalog.schema.table.name naming scheme. It does an
exact match unless one of catalog/schema/table is 'odbc::any_string'. If one of the parts is odbc::any_string,
it matches any value */
template <class Value>
class scope_index
{
public:
	bool add(const tstring &name, scope sc, const Value &v, bool set_if_exists=false) throw();
	const Value &find(const tstring &name, const scope &sc, const Value &default_value) const throw();
	const Value &find(const tstring &name, const tstring &table, const tstring &schema, const tstring &catalog, const Value &default_value) const throw()
	{
		return find(name, scope(table, schema, catalog), default_value);
	}
	void clear() { m_scope_map.clear(); }
	bool has_fully_qualified_names() const
	{
		return m_scope_map.size()>1 || (m_scope_map.size()==1 && m_scope_map.begin()->first.is_fully_qualified());
	}
protected:
	bool is_match(const scope &sc1, const scope &sc2) const
	{
		return (is_any(sc2.catalog) || sc1.catalog==sc2.catalog) &&
			(is_any(sc2.schema) || sc1.schema==sc2.schema) &&
			(is_any(sc2.table) || sc1.table==sc2.table);
	}
	typedef map<tstring, Value> names_t;
	typedef map<scope, names_t> scopes_t;
	scopes_t m_scope_map;
};

template <class Value>
bool scope_index<Value>::add(const tstring &name, scope sc, const Value &v, bool set_if_exists)
{
	names_t &named_map(m_scope_map[sc]);
	names_t::iterator name_i=named_map.find(name);
	if (name_i==named_map.end()) {
		name_i=named_map.insert(make_pair(name, v)).first;
		return true;
	}
	if (set_if_exists) {
		name_i->second=v;
	}
	return false;
}

template <class Value>
const Value &scope_index<Value>::find(const tstring &name, const scope &sc, const Value &default_value) const
{
	scopes_t::const_iterator i=m_scope_map.find(sc);
	bool find_any=false;
	if (i==m_scope_map.end()) {
		if (sc.table.length()==0 || sc.schema.length()==0 || sc.catalog.length()==0 ||
			is_any(sc.table) || is_any(sc.schema) || is_any(sc.catalog) ) {
				i=m_scope_map.begin();
				find_any=true;
			}
	}
	while (i!=m_scope_map.end()) {
		if (find_any==false || sc.partial_match(i->first)) {
			names_t::const_iterator name_i=i->second.find(name);
			if (name_i!=i->second.end())
				return name_i->second;
		}
		if (find_any)
			++i;
		else
			break;
	}
	return default_value;
}

template <typename Column>
struct column_traits
{
	const tstring &get_name(const Column &c) const { return c.m_name; }
	const tstring &get_table(const Column &c) const { return c.m_table; }
	const tstring &get_schema(const Column &c) const { return c.m_schema; }
	const tstring &get_catalog(const Column &c) const { return c.m_catalog; }
	scope get_scope(const Column &c) const
	{
		return scope(get_table(c), get_schema(c), get_catalog(c));
	}
	int get_position(const Column &c) const { return c.m_position; }
	bool is_valid(const Column &c) const { return c.is_valid; }
};

template <typename Value, typename Traits=column_traits<Value>, typename position_t=int>
class column_container:public scope_index<position_t>
{
public:
	typedef scope_index<position_t> index_t;
	/// add a value to the container at column position
	void add(position_t pos, const Value &v)
	{
		if (pos+1>m_columns.size())
			m_columns.resize(pos+1);
		m_columns[pos]=v;
		index_t::add(m_traits.get_name(v), m_traits.get_scope(v), pos, true);
	}
	void clear()
	{
		m_columns.clear();
		index_t::clear();
	}
	bool is_valid(position_t pos) const { return pos<m_columns.size() && m_traits.is_valid(m_columns[pos]); }
protected:
	vector<Value> m_columns;
	Traits m_traits;
};

};

};

#pragma warning(pop)

#endif
