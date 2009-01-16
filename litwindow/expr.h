/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: expr.h,v 1.3 2006/11/17 13:09:07 Hajo Kirchhoff Exp $
*/
#ifndef _LW_EXPR_
#define _LW_EXPR_

#ifndef x_DOC_DEVELOPERS        // doxygen for developers only

#pragma once

#include <memory>
#include <functional>

#include "./dataadapter.h"

namespace litwindow {
using namespace std;

/**@defgroup expr Expressions
    */
///@{

//#region expression objects

typedef vector<const_accessor> expr_accessors;

enum dependency_t {
    no_dependency,
    static_dependency,
    dynamic_dependency
};

/** Interface to a symbol table.
Looks up a variable in a symbol table and returns a pointer to an accessor 
to the variable. Expressions with variables store a pointer to the
symbol table and lookup the variables before evaluating the expression.
*/
class LWBASE_API symbol_table_interface
{
public:
    virtual accessor lookup_variable(const string &name) = 0;
    accessor get(const string &name)
    {
        return lookup_variable(name);
    }
    virtual ~symbol_table_interface()
    {
    }
};

/** This class serves as the root expression object.
Objects of type expr_root<int> are a lot easier to handle
than expr_primary<expr_binary<expr_const<int>, expr_unary<... >
You get the picture.
*/
template <class Type>
class expr_root
{
public:
    /// Evaluate the expression and return the result.
    virtual Type evaluate(symbol_table_interface *) const = 0;

    virtual dependency_t is_dependent_on(const const_accessor &, symbol_table_interface *) const = 0;
    /** Create a copy of the expression on the heap and return a pointer.
    */
    virtual expr_root<Type> *clone() const = 0;
    virtual ~expr_root()
    {
    }
};

/** This template class is a concrete version of the abstract expr_root.
*/
template <class E>
class expr_concrete_root:public expr_root<typename E::value_type>
{
    E e;
public:
    typedef typename E::value_type value_type;
    expr_concrete_root(E _e):e(_e) {}
    value_type evaluate(symbol_table_interface *s) const
    {
        return e.evaluate(s);
    }
    expr_root<typename E::value_type> *clone() const
    {
        return new expr_concrete_root<E>(*this);
    }
    dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *s) const
    {
        return e.is_dependent_on(a, s);
    }
};

template <class E>
inline expr_concrete_root<E> *make_wrapper(E e)
{
    return new expr_concrete_root<E>(e);
}

/// The basic building block for expressions. All operators return expr_primary<Type> objects.
template <class E>
class expr_primary
{
    E e;
public:
    /// The type of this expression.
    typedef typename E::value_type value_type;
    explicit expr_primary(const E &_e):e(_e) {}
    /// Evaluate the expression and return the result.
    value_type evaluate(symbol_table_interface *s) const
    {
        return e.evaluate(s);
    }
    /** Automatically convert the expression into a concrete root. 
    Used so it can be passed automatically to an expression<Type>
    object.
    */
    operator expr_concrete_root<expr_primary<E> >*()
    {
        return make_wrapper(*this);
    }
    /** Return the nested expression. */
    E get() const
    {
        return e;
    }
    dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *s) const
    {
        return e.is_dependent_on(a, s);
    }
};

template <class E>
expr_primary<E> make_primary(const E &e)
{
    return expr_primary<E>(e);
}

/// Unary function expression object.
template <class E, class Op, class V>
class expr_unary
{
    E e;
    Op op;
public:
    typedef V value_type;
    expr_unary(E _e, Op _op):e(_e),op(_op) {}
    value_type evaluate(symbol_table_interface *s) const
    {
        return op(e.evaluate(s));
    }
    dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *s) const
    {
        return e.is_dependent_on(a, s);
    }
};

template <class E, class Op>
inline expr_unary<E, Op, typename E::value_type> make_unary(const E &e, const Op &op)
{
    return expr_unary<E, Op, E::value_type>(e, op);
}

/// Binary function expression object.
template <class E1, class E2, class Op>
class expr_binary
{
    E1 e1;
    E2 e2;
    Op op;
public:
    typedef typename Op::result_type value_type;
    expr_binary(const E1 &_e1, const E2 &_e2, const Op &_op):e1(_e1),e2(_e2),op(_op) {}
    value_type evaluate(symbol_table_interface *s) const
    {
        return op(e1.evaluate(s), e2.evaluate(s));
    }
    dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *s) const
    {
        dependency_t rc=e1.is_dependent_on(a, s);
        if (rc==dynamic_dependency) // dynamic dependency takes preference
            return rc;
        dependency_t rc2=e2.is_dependent_on(a, s);
        if (rc2==dynamic_dependency)
            return rc2;
        return (rc==static_dependency || rc2==static_dependency) ? static_dependency : no_dependency;
    }
};

template <class E1, class E2, class Op>
inline expr_binary<E1, E2, Op> make_binary(const E1 &e1, const E2 &e2, const Op &op)
{
    return expr_binary<E1, E2, Op>(e1, e2, op);
}

/// Ternary function expression object.
template <class Condition, class Value1, class Value2, class Op>
class expr_ternary
{
    Condition e1;
    Value1   e2;
    Value2   e3;
    Op op;
public:
    typedef typename Op::result_type value_type;
    expr_ternary(const Condition &_e1, const Value1 &_e2, const Value2 &_e3, const Op &_op):e1(_e1),e2(_e2),e3(_e3),op(_op) {}
    value_type evaluate(symbol_table_interface *s) const
    {
        return op(e1.evaluate(s), e2.evaluate(s), e3.evaluate(s));
    }
    dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *s) const
    {
        dependency_t rc=e1.is_dependent_on(a, s);
        if (rc==dynamic_dependency) // dynamic dependency takes preference
            return rc;
        dependency_t rc2=e2.is_dependent_on(a, s);
        if (rc2==dynamic_dependency)
            return rc2;
        dependency_t rc3=e3.is_dependent_on(a, s);
        if (rc3==dynamic_dependency)
            return rc3;
        return (rc==static_dependency || rc2==static_dependency || rc3==static_dependency) ? static_dependency : no_dependency;
    }
};

template <class E1, class V1, class V2, class Op>
inline expr_ternary<E1, V1, V2, Op> make_ternary(const E1 &e1, const V1 &e2, const V2 &e3, const Op &op)
{
    return expr_ternary<E1, V1, V2, Op>(e1, e2, e3, op);
}

template <class Value>
struct expr_op_if_else
{
    typedef Value result_type;
    Value operator()(bool condition, const Value &v1, const Value &v2) const
    {
        return condition ? v1 : v2;
    }
};

/// A constant expression object
template <class Type>
class expr_const
{
    Type v;
public:
    typedef Type value_type;
    expr_const(Type _v):v(_v) {}
    Type evaluate(symbol_table_interface *) const
    {
        return v;
    }
    operator Type() const
    {
        return evaluate(0);
    }
    dependency_t is_dependent_on(const const_accessor &, symbol_table_interface *) const
    {
        return no_dependency;
    }
};

template <class Type>
inline expr_primary<expr_const<Type> > make_const(Type v)
{
    return expr_primary<expr_const<Type> >(expr_const<Type>(v));
}

template <class Type>
class expr_typed_accessor
{
    typed_const_accessor<Type> e;
public:
    typedef Type value_type;
    value_type evaluate(symbol_table_interface *) const
    {
        return e.get();
    }
    expr_typed_accessor(const const_accessor &_e)
    {
        e=dynamic_cast_accessor<Type>(_e);
        if (!e.is_valid())
            throw lwbase_error("accessor type mismatch");
    }
    dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *) const
    {
        return e.is_alias_of(a) ? static_dependency : no_dependency;
    }
};

struct variable_name_t:public string
{
    variable_name_t(const TCHAR* _name)
        :string(t2string(_name))
    {
    }
#ifdef _UNICODE
	variable_name_t(const char* _name)
		:string(_name)
	{
	}
#endif
};

template <class Type>
class expr_variable
{
    string name;
public:
    typedef Type value_type;
    value_type evaluate(symbol_table_interface *s) const
    {
        typed_const_accessor<Type> e;
        accessor a;
        a=s->get(name);
        if (!a.is_valid())
            throw lwbase_error("Variable "+name+" not found");
        e=dynamic_cast_accessor<Type>(a);
        if (!e.is_valid())
            throw lwbase_error("Variable "+name+" has type "+a.type_name()+" but type "+get_prop_type<Type>()->get_type_name()+" expected.");
        return e.get();
    }
    dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *s) const
    {
        return a.is_alias_of(s->get(name)) ? dynamic_dependency : no_dependency;
    }
    explicit expr_variable(const string &_name)
        :name(_name)
    {
    }
    explicit expr_variable(const char *_name)
        :name(_name)
    {
    }
    expr_variable(variable_name_t n)
        :name(n)
    {
    }
};

template <>
accessor expr_variable<accessor>::evaluate(symbol_table_interface *s) const
{
    return s->get(name);
}

template <class Type>
inline expr_primary<expr_variable<Type> > make_expr(variable_name_t name)
{
    return make_primary(expr_variable<Type>(name));
}

template <class Type>
inline expr_primary<expr_typed_accessor<Type> > make_expr(const accessor &e)
{
    return expr_primary<expr_typed_accessor<Type> > (expr_typed_accessor<Type>(e));
}

template <class Type>
inline expr_primary<expr_typed_accessor<Type> > make_expr(const const_accessor &e)
{
    return expr_primary<expr_typed_accessor<Type> >(expr_typed_accessor<Type>(e));
}

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
#define _E_SHORTCUTS
#ifdef _E_SHORTCUTS
template <class Type>
inline expr_primary<expr_typed_accessor<Type> > _e(const accessor &e)
{
    return make_expr(e);
}

template <class Type>
inline expr_primary<expr_typed_accessor<Type> > _e(const const_accessor &e)
{
    return make_expr(e);
}

template <class Type>
inline expr_primary<expr_variable<Type> > _e(variable_name_t name)
{
    return make_expr<Type>(name);
}

template <class Type>
inline expr_primary<expr_const<Type> > _c(Type v)
{
    return make_const(v);
}

inline variable_name_t _v(const TCHAR *name)
{
    return variable_name_t(name);
}

#endif


//#endregion
//#region operators
//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
///@name expression operators
///@{
#define DEFINE_OPERATOR1(CH, OP)    \
    template <class E>    \
    inline expr_primary<expr_unary<E, OP<typename E::value_type>, typename E::value_type > > operator CH (const expr_primary<E> &e)    \
{    \
    return make_primary(make_unary(e.get(), OP<E::value_type>()));    \
}

DEFINE_OPERATOR1(!, std::logical_not)

#define DEFINE_OPERATOR2(CH, OP) \
    template <class E1, class E2 >    \
    inline expr_primary<expr_binary<E1, E2, OP<typename E1::value_type> > > operator CH (const expr_primary<E1> &e1, const expr_primary<E2> &e2)    \
{    \
    return make_primary(make_binary(e1.get(), e2.get(), OP<E1::value_type>()));    \
} \
    template <class E1> \
    inline expr_primary<expr_binary<E1, expr_const<typename E1::value_type>, OP<typename E1::value_type> > > operator CH (const expr_primary<E1> &e1, typename E1::value_type e2) \
{ \
    return make_primary(make_binary(e1.get(), make_const(e2).get(), OP<E1::value_type>())); \
} \
    template <class E1> \
    inline expr_primary<expr_binary<E1, expr_typed_accessor<typename E1::value_type>, OP<typename E1::value_type> > > operator CH (const expr_primary<E1> &e1, const const_accessor &e2) \
{ \
    return make_primary(make_binary(e1.get(), make_expr<E1::value_type>(e2).get(), OP<E1::value_type>())); \
} \
    template <class E1> \
    inline expr_primary<expr_binary<E1, expr_variable<typename E1::value_type>, OP<typename E1::value_type> > > operator CH (const expr_primary<E1> &e1, const variable_name_t e2) \
{ \
    return make_primary(make_binary(e1.get(), expr_variable<E1::value_type>(e2), OP<E1::value_type>())); \
}

DEFINE_OPERATOR2(-, std::minus)
DEFINE_OPERATOR2(+, std::plus)
DEFINE_OPERATOR2(*, std::multiplies)
DEFINE_OPERATOR2(/, std::divides)
DEFINE_OPERATOR2(&&, std::logical_and)
DEFINE_OPERATOR2(||, std::logical_or)
DEFINE_OPERATOR2(==, std::equal_to)
DEFINE_OPERATOR2(<, std::less)
DEFINE_OPERATOR2(>, std::greater)
DEFINE_OPERATOR2(<=, std::less_equal)
DEFINE_OPERATOR2(>=, std::greater_equal)
DEFINE_OPERATOR2(!=, std::not_equal_to)

template <class Condition, class Value1, class Value2>
inline expr_primary<expr_ternary<Condition, Value1, Value2, expr_op_if_else<typename Value1::value_type> > > if_else(const expr_primary<Condition> &e_if_expression, const expr_primary<Value1> &e_if_value, const expr_primary<Value2> &e_else_value)
{
    return make_primary(make_ternary(e_if_expression.get(), e_if_value.get(), e_else_value.get(), expr_op_if_else<Value1::value_type>()));
}
template <class Condition, class Value>
inline expr_primary<expr_ternary<Condition, Value, Value, expr_op_if_else<typename Value::value_type> > > if_else(const expr_primary<Condition> &e_if_expression, const expr_primary<Value> &e_if_value, const const_accessor &e_else_value)
{
    return make_primary(make_ternary(e_if_expression.get(), e_if_value.get(), make_expr<Value::value_type>(e_else_value).get(), expr_op_if_else<Value::value_type>()));
}
///@}

//#endregion

template <class Type>
class expression
{
    expr_root<Type> *root;
public:
    expression(expr_root<Type> *_root=0):root(_root) {}
    void operator=(expr_root<Type> *_root)
    {
        delete root;
        root=_root;
    }
    expression(const expression<Type> &c)
    {
        root=c.root->clone();
    }
    void operator=(const expression<Type> &c)
    {
        delete root;
        root=c.root->clone();
    }
    ~expression()
    {
        delete root;
    }
    Type evaluate(symbol_table_interface *s) const
    {
        return root->evaluate(s);
    }
    dependency_t is_dependent_on(const const_accessor &a, symbol_table_interface *s) const
    {
        return root->is_dependent_on(a, s);
    }
};

template <class E>
inline expression<typename E::value_type> make_expression(E e)
{
    return expression<E::value_type>(make_wrapper(e.get()));
}

///@}

};

#endif

#endif
