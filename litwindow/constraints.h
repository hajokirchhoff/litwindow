/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: constraints.h,v 1.6 2006/09/19 16:39:40 Hajo Kirchhoff Exp $
*/
#ifndef _LW_CONSTRAINTS_
#define _LW_CONSTRAINTS_
#pragma once

#include "./lwbase.hpp"
#include "./tstring.hpp"
#include "./dataadapter.h"
#include "./expr.h"
#include <vector>
#include <map>
#include <set>

namespace litwindow {

#pragma warning(push)

using litwindow::tstring;
using std::map;
using std::set;
using std::vector;

class constraint_solver;
//#region objects storing the result of a rule evaluation
/** Base class for rule results.
Rules return pointer to a value_assign object. The value_assign object contains 
the result of the rule and has code to assign this result to the rule-target.
*/
class LWBASE_API value_assign_base
{
public:
	value_assign_base(const accessor &target)
		:m_target(target)
	{}
	virtual ~value_assign_base()
	{
	}
	//const string &target_name() const
	//{
	//    return target.name();
	//}
	accessor &target()
	{
		return m_target;
	}
	const accessor &target() const
	{
		return m_target;
	}
	virtual void do_assign(symbol_table_interface *) = 0;
	virtual bool will_modify(symbol_table_interface *) const = 0;
	virtual value_assign_base *get_undo() = 0;
protected:
	accessor m_target;
};

/** Result object for rules that return an accessor. */
class LWBASE_API value_assign_accessor:public value_assign_base
{
public:
	value_assign_accessor(const accessor &target, const const_accessor &source)
		:value_assign_base(target)
		,m_source(source)
	{
	}
	virtual void do_assign(symbol_table_interface *);
	virtual value_assign_base *get_undo();
	const_accessor &source()
	{
		return m_source;
	}
	const const_accessor &source() const
	{
		return m_source;
	}
	bool will_modify(symbol_table_interface *) const
	{
		return m_source.to_string()!=m_target.to_string();
	}
protected:
	const_accessor m_source;
};

/** Result object for rules that return the result in string form */
class LWBASE_API value_assign_string:public value_assign_base
{
public:
	value_assign_string(const accessor &target, const tstring &value, const string &name=string())
		:value_assign_base(target), m_value(value), m_name(name.length()==0 ? target.name() : name)
	{
	}
	const tstring &value() const
	{
		return m_value;
	}
	void do_assign(symbol_table_interface *)
	{
		m_target.from_string(m_value);
	}
	bool will_modify(symbol_table_interface *) const
	{
		return m_target.to_string()!=m_value;
	}
	/// return a value_assign_base object that can undo this object
	value_assign_base *get_undo()
	{
		return new value_assign_string(m_target, m_target.to_string());
	}
	bool equal_to(const value_assign_base *v) const;
protected:
	tstring m_value;
	string m_name;
};

/** Result object for rules that contain an expression (see expr.h) */
template <class E>
class value_assign_expr:public value_assign_base
{
	E                                       the_expr;
	typed_accessor<typename E::value_type>  typed_target;
	accessor    m_undo_value;

	void operator=(const value_assign_expr&);
	value_assign_expr(const value_assign_expr&);
public:
	value_assign_expr(const typed_accessor<typename E::value_type> &target, const E &e)
		:value_assign_base(target.get_accessor())
		,the_expr(e)
		,typed_target(target)
	{
	}
	~value_assign_expr()
	{
		m_undo_value.destroy();
	}
	void do_assign(symbol_table_interface *s)
	{
		typed_target.set(the_expr.evaluate(s));
	}
	bool will_modify(symbol_table_interface *s) const
	{
		return typed_target.get()!=the_expr.evaluate(s);
	}
	value_assign_base *get_undo()
	{
		m_undo_value=target().clone();
		return new value_assign_accessor(target(), m_undo_value);
	}
};
//#endregion
//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

/** Base class for constraint rules.
All rules used in the constraint mechanism inherit from this class.
*/
class LWBASE_API rule_base
{
public:
	virtual ~rule_base()
	{
	}
	rule_base(const accessor &t)
		:m_target(t)
		,m_mirror(0)
	{
		if (!t.is_valid())
			throw lwbase_error("target accessor is not valid!");
	}
	/** Evaluate the rule and return a result object.
	This function evaluates the rule. Rules do not immediately assign the result.
	Instead they return a result object, which can later be used to assign the result
	to the target.
	@return a result object.
	*/
	virtual value_assign_base* evaluate(const constraint_solver*) const = 0;
	/// Evaluate the rule and immediately assign the result to the target.
	void execute_immediate(const constraint_solver *a_solver) const;
	/// The programmer specified name of the rule.
	virtual tstring name() const
	{
		return _T("generic");
	}
	/// Test if two rules update the same target.
	bool operator==(const accessor &a) const
	{
		return a.is_alias_of(m_target);
	}
	bool operator!=(const accessor &a) const
	{
		return !operator==(a);
	}
	/// Get the target.
	const accessor &target() const
	{
		return m_target;
	}
	virtual dependency_t is_dependent_on(const const_accessor &, symbol_table_interface *) const = 0;

	/**@name Mirror rules
	Mirror rules are used for twoway rules. If a<->b is a two way rule,
	b<-a is the mirror rule of a<-b and vice versa.
	When solving a rule set, evaluation of mirror rules will be suppressed 
	when the rule itself is evaluated. If a<-b is evaluated and a changes,
	b<-a would be evaluated because it depends on a. But if b<-a is the mirror
	rule of a<-b, its evaluation will be suppressed.
	*/
	///@{
	void set_mirror(rule_base *r)
	{
		m_mirror=r;
	}
	rule_base *get_mirror() const
	{
		return m_mirror;
	}
	///@}

protected:
	accessor m_target;
	rule_base *m_mirror;
};

/** Simple assign rules of the form  target = source.
*/
class LWBASE_API rule_assign:public rule_base
{
public:
	rule_assign(const accessor &target, const const_accessor &source)
		:rule_base(target), m_source(source)
	{
	}
	/** Assign target = source.
	This function handles four different cases:
	-   target and source have the same type
	-   target is an accessor
	-   source is an accessor
	-   neither target nor source is an accessor
	*/
	value_assign_base* evaluate(const constraint_solver *s) const;
	const const_accessor &source() const
	{
		return m_source;
	}
	dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *) const
	{
		return a.is_alias_of(source()) ? static_dependency : no_dependency;
	}
	tstring name() const;
protected:
	const_accessor m_source;
};

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
///@name rules for expr<E> from expr.h
/**There is a lot of wrapping going on here.
-   The basic building block is rule_expr_assign_concrete \<E\>. This object stores the assignment 
typed_accessor<E>=expr\<E\>.
-   rule_expr_assign_abstract is an abstract base class for the concrete assignment. This abstract base
class hides any implementation dependant types and code. It also hides the template instantiation and
solves the MS Visual Studio error 'class must have DLL interface to be used by clients'. Expressions
created by clients of the constraints mechanism will have their template instantiations in their own
code space. The constraints mechanism doesn't need to have any access besides 'evaluate'.
-   rule_expr is the concrete rule for the abstract rule_base class. It hides the type of the rule
from the constraints mechanism.

*/
///@{
class rule_expr_assign_abstract
{
public:
	virtual ~rule_expr_assign_abstract() {}
	virtual value_assign_base *evaluate(const constraint_solver *s) const = 0;
	virtual dependency_t is_dependent_on(const const_accessor &, symbol_table_interface *) const = 0;
};

/** Rules containing expressions (expr.h)
*/
class LWBASE_API rule_expr:public rule_base
{
	rule_expr_assign_abstract *the_rule;
	rule_expr(const rule_expr &);
	void operator=(const rule_expr&);
	tstring m_name;
public:
	rule_expr(const accessor &target, rule_expr_assign_abstract *a_rule, const TCHAR *name=_T("rule_expr"))
		:rule_base(target), the_rule(a_rule), m_name(name)
	{
	}
	~rule_expr()
	{
		delete the_rule;
	}
	virtual value_assign_base* evaluate(const constraint_solver *s) const
	{
		return the_rule->evaluate(s);
	}
	dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *v) const
	{
		return the_rule->is_dependent_on(a, v);
	}
	tstring name() const
	{
		return m_name;
	}
};

template <class E>
rule_expr *make_rule(const accessor &target, const E &e)
{
	return new rule_expr(target, new rule_expr_assign_concrete<E>(dynamic_cast_accessor<E::value_type> (target), e));
}

///@}

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
template <class TargetValue>
class rule
{
public:
	rule(TargetValue &_target)
		:target(make_accessor(_target)), _rule(0)
	{}
	rule(const accessor &_target)
		:target(_target), rule(0)
	{}
	rule_base *get_rule() const { return _rule; }
	operator rule_base*() const { return get_rule(); }
	template <class Value>
		rule &operator =(Value &v)
	{
		_rule=new rule_assign(target, make_accessor(v));
		return *this;
	}
	rule_base *_rule;
	accessor target;
};

template <class Value>
inline rule<Value> target(Value &v)
{
	return rule<Value>(v);
}
//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

#pragma warning(disable:4231)
STL_EXPORT_VECTOR(rule_base*);
STL_EXPORT_VECTOR(value_assign_base*);

typedef vector<rule_base*> rules_t;
typedef vector<value_assign_base*> values_t;
typedef set<accessor> values_changed_t;

typedef set<rule_base*> set_of_rules_t;
typedef set<const_accessor> set_of_accessors_t;

typedef map<tstring, set_of_rules_t> group_map_t;


class LWBASE_API constraint_solver
{
public:

	void operator += (rule_base *r)
	{
		add_rule(r);
	}
	void add(rule_base *r)
	{
		add_rule(r);
	}
	//@{
	/// add the rule to the specified group(s)
	void add_to_group(rule_base *r, const tstring &group);
	void add_to_group(rule_base *r, const tstring &group, const tstring &group2, const tstring &group3=tstring());
	//@}
	constraint_solver &operator << (rule_base *r)
	{
		add_rule(r);
		return *this;
	}

	class rules_conflict:public lwbase_error
	{
	public:
		rules_conflict(const string &msg, const const_accessor &target, const const_accessor &source)
			:lwbase_error(msg),m_target(target), m_source(source)
		{}
		const_accessor m_target, m_source;
	};

	/// Add a rule to the solver.
	void add_rule(rule_base *new_rule)
	{
		add_to_group(new_rule, _T(""));
	}

	/// Set the undo mode for the solver.
	void enable_undo(bool do_enable=true)
	{
		m_undo_enabled=do_enable;
	}
	bool is_undo() const
	{
		return m_undo_enabled;
	}

	/// Set a value for a target of the solver and mark all dependent rules as unsolved.
	void assign_value(value_assign_base *new_value);
	void assign_value(const accessor &target, const tstring &value)
	{
		assign_value(new value_assign_string(target, value));
	}

	/// evaluate the rule
	value_assign_base *evaluate(rule_base *b);
	/// execute the rule unconditionally
	void execute_immediate(rule_base *b);

	void mark_value_changed(const const_accessor &target, bool recursive);
	void mark_group_changed(const tstring &group);

	/// Solve the rule set.
	void solve();

	/// Execute all rules once, beginning with the first rule until the last rule.
	void execute_all_immediate();

	/// Clear the solver. Erase all rules, values etc...
	void clear();

	/// Reset all calculated values, but leave rules intact.
	void reset();

	/// disable the solver. Changes made to variables are not propagated.
	void disable()
	{
		++m_disabled_count;
	}
	/// enable the solver. Changes made to variables will be propagated
	void enable()
	{
		if (m_disabled_count>0)
			--m_disabled_count;
	}
	/// return the status of the solver.
	bool is_enabled() const
	{
		return m_disabled_count==0;
	}
	/// Return the number of the last calculation cycle. This number is being incremented everytime 'solve' is called.
	/// It can be used to determine a 'dirty' flag.
	unsigned long	get_calc_cycle() const
	{
		return m_calc_cycle;
	}
	const rules_t &get_rules() const { return m_rules; }

protected:
	long m_calc_cycle;
	rules_t             m_rules;
	/// Contains a pointer to the rule that is currently being solved while solving. Also points to the rule that caused an exception if solving failed.
	rule_base		*m_current_rule_solved;

	///@name dependencies
	///The following members handle dependencies. They return a list of rules 
	///that must be reevaluated when the target 'object_id' changes.
	///@{
	/// Add those rules to the unsolved list that have target as an argument.
	void unsolve_depencencies_of(const const_accessor &target, bool recursive);

	// mutable: allow lazy evaluation. 

	/// m_accessor_lookup contains a list of rules that depend on a specific argument. 

	/// @note VC6.0 restriction: m_accessor_lookup @b must be a pointer to the accessor_lookup_t object,
	/// otherwise using constraint_solver in a DLL will crash.
	/// Reason: Template implementation restrictions in VC6.0 require all non-vector templates
	/// to be accessed in the same executable environment where they where created.
	/// When the constraints_solver is being compiled into a DLL, all access to the accessor_lookup_t
	/// template instantiation must be made from the same executable environment where the accessor_lookup_t
	/// object was created from. If it were a member variable, the object would be created in the callers
	/// executable environment, but calc_dependencies() is part of the DLL environment, so access to the
	/// member variable would be made from a different execution environment.
	/// For more information, see Microsoft Knowledge Base Q172396 "PRB: Access Violation When Accessing STL Object in DLL"
	///@}

	/** Record all values that have changed to detect circular rules.
	@note m_values_index must be a pointer. See notes for m_accessor_lookup.
	*/
	values_changed_t    *m_values_changed;

	/// map for rule-groups.
	group_map_t     *m_group_map;

	set_of_rules_t     &all_rules_group()
	{
		return (*m_group_map)[_T("")];
	}
	/// undo all assignments, restore previous values
	void undo_assign_values();

	/// a list of rules that will be evaluated when calling 'solve'.
	rules_t             m_unsolved_dependencies;

	/// if true, solve() will undo all changes when an error occurs.
	bool                m_undo_enabled;
	/// If undo is enabled, store the undo values here.
	values_t            m_undo_values;

	/// number of calls to 'disable()'. solver is disabled if count>0.
	int					m_disabled_count;

	/// This member holds the mirror rule for the currently evaluated rule.
	rule_base           *m_suppress_mirror_rule;

private:
	/// not yet implemented.
	constraint_solver(const constraint_solver &);
	void operator=(const constraint_solver&);
	void copy(const constraint_solver&);
public:
	constraint_solver();
	virtual ~constraint_solver();
protected:
	class symbol_table:public symbol_table_interface
	{
	protected:
		typedef map<string, accessor> symbol_map_t;
		symbol_map_t m_map;
	public:
		/// Symbol table lookup implementation.
		accessor            lookup_variable(const string &name);
	};
	symbol_table_interface  *m_symbol_table;
public:
	symbol_table_interface  *get_symbol_table() const
	{
		return m_symbol_table;
	}
	void            set_symbol_table(symbol_table_interface *new_table)
	{
		m_symbol_table=new_table;
	}
};

template <class E>
class rule_expr_assign_concrete:public rule_expr_assign_abstract
{
	typed_accessor<typename E::value_type>  typed_target;
	E                                       the_expr;
public:
	rule_expr_assign_concrete(const typed_accessor<typename E::value_type> &target, const E &e)
		:typed_target(target), the_expr(e)
	{
	}
	value_assign_base *evaluate(const constraint_solver *s) const
	{
		if (typed_target.is_valid())
			return new value_assign_expr<E>(typed_target, the_expr);
		if (is_type<accessor>(typed_target.get_accessor())) {
			// the target is itself an accessor
			return 0;
		}
		E::value_type v=the_expr.evaluate(s->get_symbol_table());
		tstring value=make_const_accessor(v).to_string();
		return new value_assign_string(typed_target.get_accessor(), value) ;
	}
	dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *v) const
	{
		return the_expr.is_dependent_on(a, v);
	}
};

bool LWBASE_API parse_rules(const tstring &rules);

#pragma warning(pop)
};

#endif
