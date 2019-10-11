/*
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library
 * distribution, file LICENCE.TXT
 * $Id: dataadaptercontainerimp.h,v 1.3 2006/12/19 14:09:47 Hajo Kirchhoff Exp $
 */
#ifndef _DATAADAPTERCONTAINERIMP_
#define _DATAADAPTERCONTAINERIMP_

#ifdef _MSC_VER
#pragma once
#endif

#include <map>
#include "./dataadapterimp.h"

namespace litwindow {

    /*! base class for container iterator implementations */
    //-----------------------------------------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------------------------------------//
class container;

class container_iterator_imp_base
{
public:
    virtual ~container_iterator_imp_base() {}
    virtual container_iterator_imp_base *clone() const = 0;
    virtual accessor get() const = 0;
    virtual prop_t get_element_type() const = 0;
    virtual void inc() = 0;
    virtual void advance(size_t off) = 0;
    virtual bool operator==(const container_iterator_imp_base &c) const = 0;
    bool operator != (const container_iterator_imp_base &c) const
    {
        return !operator==(c);
    }
        /// Insert the object into the container after the position pointed to by this iterator
    virtual bool insert_into(container &where, const accessor &what) = 0;
        /// Erase the object from the container. Modify the iterator so that it points to the element after the erased element.
    virtual bool erase_from(container &where) = 0;
};

class const_container_iterator_imp_base
{
public:
    virtual ~const_container_iterator_imp_base() {}
    virtual const_container_iterator_imp_base *clone() const = 0;
    virtual const_accessor get() const = 0;
    virtual prop_t get_element_type() const = 0;
    virtual void inc() = 0;
    virtual void advance(size_t off) = 0;
    virtual bool operator==(const const_container_iterator_imp_base &c) const = 0;
    bool operator != (const const_container_iterator_imp_base &c) const
    {
        return !operator==(c);
    }
};

    //-----------------------------------------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------------------------------------//
template <class Container, class Iterator=Container::const_iterator>
class const_container_const_iterator:public const_container_iterator_imp_base
{
public:
	typedef Iterator iterator;
	typedef typename Container::value_type value_type;
    iterator i;
    const_container_const_iterator(const iterator &_i)
        :i(_i)
    {}
    virtual const_container_iterator_imp_base *clone() const
    {
        return new const_container_const_iterator<Container, Iterator>(*this);
    }
    virtual const_accessor get() const
    {
        return make_const_accessor(*i);
    }
    virtual prop_t get_element_type() const
    {
	    return prop_type_object<value_type>::get(0);
    }
      virtual bool operator == (const const_container_iterator_imp_base &c) const
    {
            const const_container_const_iterator<Container, Iterator> *my_c=dynamic_cast<const const_container_const_iterator<Container, Iterator> *>(&c);
            return my_c && i==my_c->i;
        //return get().is_alias_of(c.get());
    }

    virtual void inc()
    {
        ++i;
    }
    virtual void advance(size_t off)
    {
        while (off--)
            inc();
    }
};

template <class _Cont>
bool inserter(_Cont &c, /*const typename _Cont::value_type*/ const typed_const_accessor<typename _Cont::value_type> &v, typename _Cont::iterator &hint)
{
    const typename _Cont::value_type *ptr=v.get_ptr();
    hint=c.insert(hint, ptr ? *ptr : v.get());
      ++hint;
    return true;
}

//template <class _value>
//bool inserter(std::map<typename _value::first_type, typename _value::second_type> &c, const typed_const_accessor<typename std::map<typename _value::first_type, typename _value::second_type>::value_type> &v, typename std::map<typename _value::first_type, typename _value::second_type>::iterator &hint)
//{
//    hint=c.insert(hint, *v.get_ptr());
//    ++hint;
//    return true;
//}
//
template <class _Cont>
bool eraser(_Cont &c, typename _Cont::iterator &where)
{
    where=c.erase(where);
    return true;
}

template <class _Cont, class _Iter=typename _Cont::iterator, class _Value=typename _Cont::value_type >
class container_iterator:public container_iterator_imp_base
{
public:
    _Iter i;

    container_iterator(const _Iter &_i)
        :i(_i)
    {}
    ~container_iterator()
    {
    }
    const container_iterator &operator=(const container_iterator &c)
    {
        i=c.i;
    }
      virtual bool operator == (const container_iterator_imp_base &c) const
    {
            const container_iterator<_Cont, _Iter, _Value> *my_c=dynamic_cast<const container_iterator<_Cont, _Iter, _Value> *>(&c);
            return my_c && i==my_c->i;
        //return get().is_alias_of(c.get());
    }

    virtual container_iterator_imp_base *clone() const
    {
        return new container_iterator<_Cont, _Iter, _Value>(*this);
    }
    virtual accessor get() const
    {
        return make_accessor(*i);
    }
    virtual prop_t get_element_type() const
    {
	    return prop_type_object<_Value>::get(0);
    }
    virtual void inc()
    {
        ++i;
    }
    virtual void advance(size_t off)
    {
        while (off--)
            inc();
    }
    void verify_valid_container(const container &where) const
    {
        if (!where.is_type(get_prop_type<_Cont>()))
            throw lwbase_error("container and iterator type mismatch.");
    }
    virtual bool insert_into(container &where, const accessor &what)
    {
        typed_accessor<_Cont> c=dynamic_cast_accessor<_Cont>(where);
        typed_const_accessor<_Value> v=dynamic_cast_accessor<_Value>(what);
        if (c.is_valid()==false)
            throw lwbase_error("container type and iterator do not match");
        if (v.is_valid()==false)
            throw lwbase_error("container type and value type do not match");
        return inserter(c.get_ref(), v, i);
    }
    virtual bool erase_from(container &where)
    {
        typed_accessor<_Cont> c=dynamic_cast_accessor<_Cont>(where);
        if (c.is_valid()==false)
            throw lwbase_error("container type and iterator do not match");
        return eraser(c.get_ref(), i);
    }
};

template <class Container, class Iterator>
const_container_const_iterator<Container, Iterator> *make_const_container_const_iterator(const Iterator &i)
{
    return new const_container_const_iterator<Container, Iterator>(i);
}

template <class _Cont, class _Iter, class _Value>
container_iterator<_Cont, _Iter, _Value> *make_container_iterator(const _Iter &i)
{
    return new container_iterator<_Cont, _Iter, _Value>(i);
}

template <class Container>
class const_container_converter:public converter<Container>
{
public:
	const_container_converter(const string &name, const prop_type_registrar *r)
		:converter<Container>(name, r)
	{}
	bool is_const_container() const
	{
		return true;
	}
	const_container_iterator_imp_base *get_const_begin(const schema_entry *se, const_prop_ptr member_ptr) const
	{
		return make_const_container_const_iterator<Container>(this->member(member_ptr).begin());
	}
	const_container_iterator_imp_base *get_const_end(const schema_entry *se, const_prop_ptr member_ptr) const
	{
		return make_const_container_const_iterator<Container>(this->member(member_ptr).end());
	}
};

template <class Container>
class container_converter:public const_container_converter<Container>
{
public:
	container_converter(const string& name, const prop_type_registrar* r)
        :const_container_converter<Container>(name, r)
    {}
	bool is_container() const
	{
		return true;
	}
    container_iterator_imp_base *get_begin(const schema_entry *se, prop_ptr member_ptr) const
    {
        return make_container_iterator<Container, Container::iterator, Container::value_type>(this->member(member_ptr).begin());
    }
    container_iterator_imp_base *get_end(const schema_entry *se, prop_ptr member_ptr) const
    {
        return make_container_iterator<Container, Container::iterator, Container::value_type>(this->member(member_ptr).end());
    }
};

    //-----------------------------------------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------------------------------------//
#define LWL_DECLARE_CONTAINER(tp, _the_decl_spec) \
    template <> litwindow::prop_t _the_decl_spec litwindow::prop_type_object<tp >::get(const tp*);

#define LWL_IMPLEMENT_CONTAINER(tp) \
    template <> \
    litwindow::prop_type_registrar litwindow::prop_type_object<tp >::____register_prop_t(litwindow::prop_type_object<tp >::get(0)); \
	LWBASE_DLL_EXPORT ::litwindow::prop_t     get_prop_type_data_adapter_mechanism(const tp*)    \
	{    \
		static litwindow::container_converter<tp > theConverter(#tp, &litwindow::prop_type_object<tp>::____register_prop_t);    \
		return &theConverter;    \
	} \

#define LWL_IMPLEMENT_CONST_CONTAINER(tp) \
	template <> \
	litwindow::prop_type_registrar litwindow::prop_type_object<tp >::____register_prop_t(litwindow::prop_type_object<tp >::get(0)); \
	LWBASE_DLL_EXPORT ::litwindow::prop_t     get_prop_type_data_adapter_mechanism(const tp*)    \
{    \
	static litwindow::const_container_converter<tp > theConverter(#tp, &litwindow::prop_type_object<tp>::____register_prop_t);    \
	return &theConverter;    \
} \
/*template <> \
::litwindow::prop_t LWBASE_DLL_EXPORT ::litwindow::prop_type_object<tp >::get(const tp *) \
    { \
        static litwindow::container_converter<tp > theConverter(#tp, &____register_prop_t); \
        return &theConverter; \
    }*/

#define DECLARE_ADAPTER_CONTAINER         LWL_DECLARE_CONTAINER
#define IMPLEMENT_ADAPTER_CONTAINER LWL_IMPLEMENT_CONTAINER

};
#endif
