#include "stdwx.h"
/////////////////////////////////////////////////////////////////////////////
// Name:        generic_list_sample.cpp
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     08/09/2009 09:06:44
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <wx/dataview.h>
#include <litwindow/ui/list_mediator.hpp>
#include <litwindow/wx/list_mediator.hpp>
#include <boost/function.hpp>
using namespace litwindow;
using namespace litwindow::ui;
#pragma warning(push, 3)

#pragma warning(disable: 4101)

template <typename ColumnAccessor, typename Enabler = void>
struct column_accessor_handler
{
};

template <typename R, typename C>
struct column_accessor_handler<R (C::*)>
{
	typedef R column_value_type;
	typedef C row_value_type;
	typedef column_value_type (row_value_type::*accessor_type);
	typedef boost::function<column_value_type (accessor_type)> getter_type;
	template <typename I>
	static inline column_value_type get(accessor_type a, const I &r) { return r.*a; }
	template <typename I>
	struct caller
	{
		accessor_type a;
		column_value_type operator()(const I& v) const
		{
			return get(a, v);
		}
		caller(const accessor_type &a_):a(a_){}
	};
	template <typename I>
	struct comparator
	{
		accessor_type a;
		bool operator()(const I& left, const I& right) const
		{
			return left<right;
		}
	};
	template <typename I>
	static inline boost::function<column_value_type (const I&)> getter(accessor_type a) { return caller<I>(a); }
	accessor_type a;
	column_accessor_handler(accessor_type a_):a(a_){}
	template <typename I>
	inline column_value_type get(const I& r) const { return get(a, r);; }
};
template <typename R, typename C>
struct column_accessor_handler<R (C::*)() const>
{
	typedef R column_value_type;
	typedef C row_value_type;
	typedef column_value_type (row_value_type::*accessor_type)() const;
	template <typename I>
	static inline column_value_type get(accessor_type a, const I &r) { return (r.*a)(); }
	template <typename I>
	struct caller
	{
		accessor_type a;
		column_value_type operator()(const I& v) const
		{
			return get(a, v);
		}
		caller(const accessor_type &a_):a(a_){}
	};
	template <typename I>
	static inline boost::function<column_value_type (const I&)> getter(accessor_type a) { return caller<I>(a); }
	accessor_type a;
	column_accessor_handler(accessor_type a_):a(a_){}
	template <typename I>
	inline column_value_type get(const I& r) const { return get(a, r); }
};
template <typename FunctionObject>
struct column_accessor_handler<FunctionObject>
{
	typedef typename FunctionObject::result_type column_value_type;
	//TODO: define row_value_type again for boost::function and boost::bind
	//typedef typename FunctionObject::a1_ row_value_type;
	typedef FunctionObject accessor_type;
	template <typename I>
	static inline column_value_type get(accessor_type a, const I &r) { return a(r); }
	template <typename I>
	static inline boost::function<column_value_type (const I&)> getter(accessor_type a) { return a; }
	accessor_type a;
	column_accessor_handler(accessor_type a_):a(a_){}
	template <typename I>
	inline column_value_type get(const I& r) const { return get(a, r); }
};
namespace t 
{
	template <typename Value>
	inline void to_string(const Value &v, tstring &c)
	{
		//c=boost::lexical_cast<tstring>(v);
		c=litwindow::make_const_accessor(v).to_string();
	}
	template <typename ValueType, typename MemberType>
	inline const MemberType &to_member(const ValueType &v, MemberType (ValueType::*ptr_to_member))
	{
		return v.*ptr_to_member;
	}
	//template <typename ValueType, typename MemberType>
	//inline const MemberType &to_member(const ValueType &v, MemberType (ValueType::*ptr)() const)
	//{
	//	return (v.*ptr)();
	//}
	template <typename MemberType, typename ValueType>
	inline typename basic_column_descriptor<ValueType>::text_renderer_type make_text_renderer(MemberType (ValueType::*ptr_to_member))
	{
		return boost::bind(&to_string<MemberType>,
			boost::bind(&to_member<ValueType, MemberType>, _1, ptr_to_member),
			_2);
	}
	template <typename MemberType, typename ValueType, typename Formatter>
	inline typename basic_column_descriptor<ValueType>::text_renderer_type make_text_renderer(MemberType (ValueType::*ptr_to_member), Formatter fmt)
	{
		using boost::bind;
		return bind(&to_string<typename Formatter::result_type>,
			bind(fmt, bind(&to_member<ValueType, MemberType>, _1, ptr_to_member) ),
			_2);
	}
}

struct my_test
{
	int a;
	int b() const { return a+1; }
};
int c(const my_test &x)
{
	return x.a+2;
}

template <typename T, typename Enabled = void>
struct column_value_handler {};

template <typename T>
struct column_value_handler<T, typename boost::enable_if<boost::is_member_function_pointer<T> >::type>
{
	typedef typename boost::result_of<T()>::type value_type;
	template <typename V>
	value_type get(T t, V v) const { return (v.*t)(); }
};

template <typename T>
struct column_value_handler<T, typename boost::enable_if<boost::is_member_object_pointer<T> >::type>
{
	// boost::type_traits is still missing member_object_pointer_traits to determine the type
	// of a class and its member object. The following code was inspired by http://nealabq.com/blog/2008/09/
	template <typename MEMBER_OBJECT_POINTER_TRAITS> struct member_object_pointer_traits {};
	template <typename T, typename C> struct member_object_pointer_traits<T (C::*)> {
		typedef C class_type;
		typedef T value_type;
	};
	typedef typename member_object_pointer_traits<T>::value_type value_type;
	template <typename V>
	value_type get(T t, V v) const { return v.*t; }
};
template <typename T>
struct column_value_handler<T, typename boost::enable_if<boost::is_function<typename boost::remove_pointer<T>::type > >::type>
{
	typedef typename boost::remove_pointer<T>::type function_type;
	typedef typename boost::function_traits<function_type>::result_type value_type;
	template <typename V>
	value_type get(T t, V v) const { return (*t)(v); }
};
template <typename R, typename T1>
struct column_value_handler< boost::function1<R, T1>, typename boost::enable_if_c<true>::type >
{
	typedef boost::function1<R, T1> T;
	typedef R value_type;
	template <typename V>
	value_type get(T t, V v) const { return t(v); }
};
#ifdef not
template <typename T>
struct column_value_handler<T, typename boost::lazy_enable_if<typename T::result_type >::type>
{
	typedef typename T::result_type value_type;
	template <typename V>
	value_type get(T t, V v) const { return t(v); }
};
#endif // not

template <typename T, typename InputValue>
typename column_value_handler<T>::value_type test_this_and_that(T t, InputValue i)
{
	typedef column_value_handler<T> CVH;
	CVH cvh;
	CVH::value_type v=cvh.get(t, i);
	return v;
}

int test_full(const my_test &a, int add)
{
	return a.a+add;
}

template <typename T, typename V >
int test2(T t, const V& v, boost::function<int /*typename column_accessor_handler<typename T>::column_value_type*/ (const V&)> &g)
{
	typedef column_accessor_handler<T> INPUT;
	INPUT t0(t);
	typedef INPUT::column_value_type cvt;
	cvt r;
	r=t0.get(t,v);
	r=INPUT(t).get<V>(t, v);
	g=INPUT::getter<V>(t);
	//(boost::bind(&INPUT::get<V>, INPUT(), t, v))();
	//if (g)
	//	g=t0.get(t);
	return r;
}
void compile_tests()
{
	my_test v;
	v.a=10;

	typedef boost::function<int (const my_test&)> get_type;

	typedef column_accessor_handler<int (my_test::*)> cah;
	get_type g;
	test2(&my_test::a, v, g);
	boost::function1<int, const my_test&> f2;
	f2=boost::bind(&my_test::b, _1);
	test2(f2, v, g);
	test2(boost::bind(&my_test::b, _1), v, g);
	g(v);
#ifdef not
	test2(&my_test::b, v, g);

	test_this_and_that(&my_test::a, v);
	test_this_and_that(&my_test::b, v);
	test_this_and_that(&c, v);
#endif // not
	typedef boost::function1 <int, const my_test&> f_type;
	//f_type f(boost::bind(&test_full, v, 3));
	//f_type::result_type r;
	//column_value_handler< boost::function1<int, my_test> >::value_type cvh_type;
	////test_this_and_that(boost::bind(&test_full, v, 3), v);
	//typedef int (*fnc_type)(const my_test&);
	//if (boost::is_pointer< f_type >::value) {
	//	v.a=0;
	//}
	//if (boost::is_function< boost::remove_pointer<f_type >::type >::value) {
	//	v.a=1;
	//}
}
