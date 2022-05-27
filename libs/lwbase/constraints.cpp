/* 
* Copyright 2004-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: constraints.cpp,v 1.8 2006/11/07 14:10:22 Hajo Kirchhoff Exp $
*/
#include "stdafx.h"
#include "litwindow/algorithm.h"
#include "litwindow/logging.h"
#include "litwindow/constraints.h"
#include <algorithm>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace litwindow {

void constraint_solver::solve()
{
	// Warning: do _not_ use an iterator here!
	// assign_value may change m_unsolved_dependencies in which case it
	// depends on the implementation of the container if an iterator
	// here would become invalid.
	// The current implementation uses a vector<> for m_unsolved_dependencies
	// so using a size_t for an index comes with little performance penalties.
	// An alternative implementation could use a list/queue.
	size_t i;
	try {
		for (i=0; i<m_unsolved_dependencies.size(); ++i) {
			// get next rule
			m_current_rule_solved=m_unsolved_dependencies[i];

			// evaluate this rule, store the new value.
			// assign_value will also add dependencies to m_unsolved_dependencies
			m_suppress_mirror_rule=m_current_rule_solved->get_mirror();
			assign_value(evaluate(m_current_rule_solved));
		}
		reset();
	}
	catch (...) {
		try {
			// avoid double exception
			undo_assign_values();
		}
		catch (...) {
			lw_err() << _T("double exception while trying to undo solving") << endl;
			// do not throw here... fail silently
		}
		--m_calc_cycle;
		reset();
		throw;
	}
}

value_assign_base *constraint_solver::evaluate(rule_base *b)
{
	//lw_log() << _T("  evaluating ") << b->name() << endl;
	return b->evaluate(this);
}

void constraint_solver::execute_immediate(rule_base *i)
{
	m_current_rule_solved=i;
	//lw_log() << _T("  rule execute_immediate ") << i->name() << endl;
	m_suppress_mirror_rule=m_current_rule_solved->get_mirror();
	i->execute_immediate(this);
}

void constraint_solver::execute_all_immediate()
{
	++m_calc_cycle;
	rules_t::const_iterator i;
	m_suppress_mirror_rule=0;
	for (i=m_rules.begin(); i!=m_rules.end(); ++i) {
		if (*i!=m_suppress_mirror_rule)
			execute_immediate(*i);
		m_suppress_mirror_rule=(*i)->get_mirror();
	}
    reset();
}

void constraint_solver::undo_assign_values()
{
	values_t::reverse_iterator i;
	for (i=m_undo_values.rbegin(); i!=m_undo_values.rend(); ++i)
		(*i)->do_assign(get_symbol_table());
}

void constraint_solver::add_to_group(rule_base *new_rule, const tstring &group1, const tstring &group2, const tstring &group3)
{
	if (all_rules_group().find(new_rule)==all_rules_group().end()) {
		m_rules.push_back(new_rule);
		all_rules_group().insert(new_rule);
	}
	if (group1.size()>0)
		(*m_group_map)[group1].insert(new_rule);
	if (group2.size()>0)
		(*m_group_map)[group2].insert(new_rule);
	if (group3.size()>0)
		(*m_group_map)[group3].insert(new_rule);
}

void constraint_solver::add_to_group(rule_base *new_rule, const tstring &group1)
{
	add_to_group(new_rule, group1, tstring(), tstring());
}

void constraint_solver::mark_value_changed(const const_accessor &target, bool recursive)
{
	if (is_enabled())
		unsolve_depencencies_of(target, recursive);
	// else solver has been disabled
}

void constraint_solver::mark_group_changed(const tstring &group_name)
{
	group_map_t::iterator group=m_group_map->find(group_name);
	if (group!=m_group_map->end()) {
		set_of_rules_t the_group=group->second;
		set_of_rules_t::iterator i;
		for (i=the_group.begin(); i!=the_group.end(); ++i) {
			m_unsolved_dependencies.push_back(*i);
		}
	}
}

void constraint_solver::unsolve_depencencies_of(const const_accessor &target, bool recursive)
{
	if (recursive && target.is_aggregate()) {
		// first solve child accessors (if any)
		const_aggregate a=target.get_aggregate();

		const_aggregate::const_iterator i;
		for (i=a.begin(); i!=a.end(); ++i) {
			unsolve_depencencies_of(*i, recursive);
		}
	}
	// find all rules that are affected by 'target' ... have 'target' as an argument
	// and add them to the unsolved list
	rules_t::const_iterator rules_iterator;
	for (rules_iterator=m_rules.begin(); rules_iterator!=m_rules.end(); ++rules_iterator) {
		if ((*rules_iterator)->is_dependent_on(target, get_symbol_table())!=no_dependency) {
			//lw_log() << _T("rule for ") << s2tstring(target.class_name()) << _T("::") << s2tstring(target.name()) << endl;
			if (*rules_iterator == m_suppress_mirror_rule) {
				//lw_log() << " - mirror rule suppressed - " << endl;
			} else
				m_unsolved_dependencies.push_back(*rules_iterator);
		}
	}
}

void constraint_solver::assign_value(value_assign_base *new_value)
{
	//lw_log() << _T("  assigning value for rule ") << m_current_rule_solved->name() << endl;
	std::unique_ptr<value_assign_base> guard(new_value);

	// Check for circular rules, i.e. if this target has already been changed. 
	values_changed_t::const_iterator a_value=m_values_changed->find(new_value->target());

	if (m_undo_enabled) {
		m_undo_values.push_back(new_value->get_undo());
	}
	if (a_value!=m_values_changed->end()) {
		//lw_log() << _T("     value has already been assigned. Rechecking...") << endl;
		// This target has already been assigned a value.
		if (new_value->will_modify(get_symbol_table())) {
			// The new value is different than the existing value.
			// This is a conflict.
			lw_err() << _T("  rule ") << m_current_rule_solved->name() << _T(" causes a conflict with existing value ") << accessor_as_debug(new_value->target()) << endl;
			throw rules_conflict("rules would assign two different values to target", new_value->target(), const_accessor());
		} // else everything is okay.

	} else {

		m_values_changed->insert(new_value->target());
		try {
			new_value->do_assign(get_symbol_table());
			++m_calc_cycle;
		}
		catch (lwbase_error &e) {
			lw_err() << _T("  exception ") << e.what() << _T(" for rule ") << m_current_rule_solved->name() << _T(" value assign failed for ") << accessor_as_debug(new_value->target()) << endl;
			throw rules_conflict("value assign failed: "+string(e.what()), new_value->target(), const_accessor());
		}

		// next add all rules to the list of unsolved rules where this target is an argument of the rule.
		unsolve_depencencies_of(new_value->target(), true);

	}
}

constraint_solver::constraint_solver(const constraint_solver &c)
:m_values_changed(0)
,m_symbol_table(0)
,m_group_map(0)
,m_calc_cycle(c.m_calc_cycle)
{
	clear();
	copy(c);
}

void constraint_solver::copy(const constraint_solver&)
{
	not_implemented("contraint_solver::copy");
}

constraint_solver::~constraint_solver()
{
	clear();
	delete m_values_changed;
	delete m_group_map;
}

constraint_solver::constraint_solver()
:m_values_changed(new values_changed_t)
,m_symbol_table(0)
,m_undo_enabled(true)
,m_group_map(new group_map_t)
,m_calc_cycle(0)
{ 
	clear();
}

void constraint_solver::reset()
{
	m_suppress_mirror_rule=0;
	for_each(m_undo_values.begin(), m_undo_values.end(), delete_ptr<value_assign_base>());
	//values_changed_t::iterator i;
	//for (i=m_values_changed->begin(); i!=m_values_changed->end(); ++i)
	//    delete i->second;
	m_values_changed->clear();
	m_undo_values.clear();
	m_unsolved_dependencies.clear();
}

void constraint_solver::clear()
{
	reset();
	m_group_map->clear();
	for_each(m_rules.begin(), m_rules.end(), delete_ptr<rule_base>());
	m_rules.clear();
	m_suppress_mirror_rule=0;
	m_disabled_count=0;
}

value_assign_base *rule_assign::evaluate(const constraint_solver *s) const
{
	return new value_assign_accessor(target(), source());
}

void rule_base::execute_immediate(const constraint_solver *a_solver) const
{
	unique_ptr<value_assign_base> v(evaluate(a_solver));
	v->do_assign(a_solver->get_symbol_table());
}

void value_assign_accessor::do_assign(symbol_table_interface *)
{
	const_accessor src=source();
	accessor trg=target();
	try {
		if (!trg.is_type(src)) {
			// if source is an accessor or const_accessor itself, dereference source
			if (is_type<accessor>(src)) {
				accessor tmp;
				typed_const_accessor<accessor>(src).get(tmp);
				src=tmp;
			} else if (is_type<const_accessor>(src)) {
				typed_const_accessor<const_accessor>(src).get(src);
			}
		}
	//lw_log() << "do_assign { "<<accessor_as_debug(trg, true)<< " } <- { " << accessor_as_debug(src, true) << " }" << endl;

		if (trg.is_type(src) || is_type<accessor>(trg))
			trg.assign(src);
		else {
			tstring value=src.to_string();
			trg.from_string(value);
		}
	}
	catch (...) {
		lw_log() << _T("exception while do_assign {") << s2tstring(trg.get_name()) << _T(":") << accessor_as_debug(trg, true) << _T("} = {") << s2tstring(src.get_name()) << _T(":") << accessor_as_debug(src, true) << _T("}") << endl;
		throw;
	}
}

value_assign_base *value_assign_accessor::get_undo()
{
	//if (m_target.has_copy())
	//    return 0;
	return new value_assign_string(m_target, m_target.to_string());
}

accessor constraint_solver::symbol_table::lookup_variable(const string &name)
{
	accessor rc=m_map.insert(make_pair(name, accessor())).first->second;
	return rc;
}

tstring rule_assign::name() const
{
	return s2tstring(m_target.class_name()+"::"+m_target.name()) + _T(":=") + s2tstring(m_source.class_name()+"::"+m_source.name());
}

};
