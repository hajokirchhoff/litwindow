/*
 * Copyright 2004-2005, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library
 * distribution, file LICENCE.TXT
 * $Id: dataadapterimp.cpp,v 1.11 2007/12/18 11:09:35 Merry\Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include "litwindow/dataadapter.h"
#include "litwindow/logging.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
using namespace std;
using namespace litwindow;
//TODO: use boost::mpl
//TODO: modify dataadapter into a header-lib, get rid of .cpp files


//#pragma warning(disable: 4518) /* disable '__declspec(dllimport)' unexpected here */

IMPLEMENT_ADAPTER_TYPE(int8_t)
IMPLEMENT_ADAPTER_TYPE(int)
IMPLEMENT_ADAPTER_TYPE(short)
IMPLEMENT_ADAPTER_TYPE(float)
IMPLEMENT_ADAPTER_TYPE(bool)
IMPLEMENT_ADAPTER_TYPE(long)
IMPLEMENT_ADAPTER_TYPE(double)
IMPLEMENT_ADAPTER_TYPE(char)
IMPLEMENT_ADAPTER_TYPE(unsigned int)
IMPLEMENT_ADAPTER_TYPE(unsigned short)
IMPLEMENT_ADAPTER_TYPE(unsigned char)
IMPLEMENT_ADAPTER_TYPE(unsigned long)
//IMPLEMENT_ADAPTER_TYPE(litwindow::tstring)
IMPLEMENT_ADAPTER_TYPE(std::wstring)
IMPLEMENT_ADAPTER_TYPE(std::string)
IMPLEMENT_ADAPTER_TYPE(litwindow::container)
//IMPLEMENT_ADAPTER_TYPE(litwindow::const_container)
IMPLEMENT_ADAPTER_TYPE(litwindow::aggregate)
IMPLEMENT_ADAPTER_TYPE(litwindow::const_aggregate)

#ifdef _WIN64
IMPLEMENT_ADAPTER_TYPE(unsigned __int64)
IMPLEMENT_ADAPTER_TYPE(signed __int64)
#else
IMPLEMENT_ADAPTER_TYPE(boost::uint64_t)
#endif

#define HAS_BOOST_UUID
#ifdef HAS_BOOST_UUID
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

using boost::uuids::uuid;

template <>
litwindow::tstring litwindow::converter<uuid>::to_string(const uuid &v)
{
    basic_stringstream<TCHAR> out;
    out << v;
    return out.str();
}
template <>
size_t litwindow::converter<uuid>::from_string(const litwindow::tstring &newValue, uuid &v)
{
    basic_stringstream<TCHAR> in(newValue);
    in >> v;
    return sizeof(v);
}
LWL_IMPLEMENT_ACCESSOR(uuid)
#endif

#define HAS_BOOST_TRIBOOL
#ifdef HAS_BOOST_TRIBOOL
#include "boost/logic/tribool.hpp"
template <>
litwindow::tstring litwindow::converter<boost::tribool>::to_string(const boost::tribool& t)
{
    if (t == true) return _T("true");
    if (t == false) return _T("false");
    return _T("indeterminate");
}
template <>
size_t litwindow::converter<boost::tribool>::from_string(const litwindow::tstring& newValue, boost::tribool& v)
{
    if (newValue == _T("true"))
        v = true;
    else if (newValue == _T("false"))
        v = false;
    else if (newValue == _T("indeterminate"))
        v = boost::indeterminate;
    else
        throw litwindow::lwbase_error("invalid value for tribool");
    return sizeof(v);
}
LWL_IMPLEMENT_ACCESSOR(boost::tribool)
#endif


#ifdef _NATIVE_WCHAR_T_DEFINED
IMPLEMENT_ADAPTER_TYPE(wchar_t)
#endif

using namespace litwindow;
using namespace std;

/** @internal
*/
#ifndef DOXYGEN_INVOKED
template <>
bool converter<accessor>::has_schema() const
{
    return true;
}

template <>
tstring converter<accessor>::to_string(const schema_entry *e, const_prop_ptr member_ptr)
{
    return member(member_ptr).to_string();
}

IMPLEMENT_ADAPTER_TYPE(litwindow::accessor)
IMPLEMENT_ADAPTER_TYPE(litwindow::const_accessor)
#endif

//template<>
//tstring litwindow::converter<int>::to_string(const schema_entry *entry, const_prop_ptr member_ptr)
//{
//    ::std::tostringstream str;
//    str << member(member_ptr);
//    return str.str();
//}

namespace litwindow {

	void accessor::assign_value(const const_accessor &source)
	{
		if (is_int() && source.is_int())
			from_int(source.to_int());
		else if (is_type<double>() && source.is_type<float>())
			dynamic_cast_accessor<double>(*this).set(static_cast<double>(dynamic_cast_accessor<float>(source).get()));
		else if (is_type<float>() && source.is_type<double>())
			dynamic_cast_accessor<float>(*this).set(static_cast<float>(dynamic_cast_accessor<double>(source).get()));
		else
			from_string(source.to_string());
	}

	void const_accessor::query_value(const accessor &a)
	{
		if (is_int() && a.is_int())
			a.from_int(to_int());
		else if (is_type<double>() && a.is_type<float>())
			dynamic_cast_accessor<float>(a).set(static_cast<float>(dynamic_cast_accessor<double>(*this).get()));
		else if (is_type<float>() && a.is_type<double>())
			dynamic_cast_accessor<double>(a).set(static_cast<double>(dynamic_cast_accessor<float>(*this).get()));
		else
			a.from_string(to_string());
	}

    map<string, prop_t> &g_prop_type_map()
    {
        static map<string, prop_t> __prop_type_map;
        return __prop_type_map;
    }

    prop_t LWBASE_API get_prop_type_by_name(const char *name)
    {
        map<string, prop_t>::const_iterator i=g_prop_type_map().find(name);
        return i==g_prop_type_map().end() ? 0 : i->second;
    }

    bool LWBASE_API register_prop_type(prop_t type)
    {
        //litwindow::lw_log() << "registering " << type->get_type_name().c_str() << endl;
        return g_prop_type_map().insert(make_pair(type->get_type_name(), type)).second;
    }

    bool LWBASE_API unregister_prop_type(prop_t type)
    {
        return g_prop_type_map().erase(type->get_type_name())>0;
    }

LWBASE_API tstring accessor_as_debug(const const_accessor &e, bool show_type)
{
    tstring rc;
    try {
        if (!e.is_valid()) {
            tstringstream o;
            o << s2tstring(e.class_name()) << "::" << s2tstring(e.name()) << "==invalid";
            rc=o.str();
        } else {
            if (show_type) {
                rc.append(s2tstring(e.get_type_name())+_T(" : "));
            }
            if (e.is_aggregate()) {
                const_aggregate a(e.get_aggregate());
                tstringstream o;
                o << s2tstring(a.class_name()) << _T("{");
                const_aggregate::const_iterator i;
                for (i=a.begin(); i!=a.end(); ++i) {
                    if (i!=a.begin())
                        o << _T("; ");
                    if (i->is_aggregate()) {
                        o << accessor_as_debug(*i);
                    } else {
                        o << s2tstring(i->name()) << _T("=") << accessor_as_debug(*i);
                    }
                }
                o << _T("}");
                rc.append(o.str());
            } else {
                rc.append(e.to_string());
            }
        }
    }
    catch (std::exception &e) {
        tstringstream o;
        o << _T("(error: '") << e.what() << _T("')");
        rc=o.str();
    }
    return rc;
}

LWBASE_API tstring accessor_as_debug(const const_accessor &e)
{
    return accessor_as_debug(e, false);
}

//#region basic conversion from/to_string/int
void not_implemented(const string &what)
{
    string msg(what+" method not implemented");
      throw litwindow::lwbase_error(msg);
}

// the following code fragment shall be invisible to doxygen
#ifndef DOXYGEN_INVOKED

template<>
size_t converter<unsigned __int64>::from_string(const tstring &newValue, unsigned __int64 &v)
{
    using namespace boost;
    v=lexical_cast<unsigned __int64>(newValue);
    return sizeof(v);
}
template<>
litwindow::tstring converter<unsigned __int64>::to_string(const unsigned __int64 &v)
{
    using namespace boost;
    return lexical_cast<litwindow::tstring>(v);
}

template<>
size_t converter<signed __int64>::from_string(const tstring &newValue, signed __int64 &v)
{
	using namespace boost;
	v=lexical_cast<signed __int64>(newValue);
	return sizeof(v);
}
template<>
litwindow::tstring converter<signed __int64>::to_string(const signed __int64 &v)
{
	using namespace boost;
	return lexical_cast<litwindow::tstring>(v);
}

template <>
tstring converter<int>::to_string(const int &i)
{
    tostringstream str;
    str << i;
    return str.str();
}

template <>
tstring converter<long>::to_string(const long &i)
{
      tostringstream str;
      str << i;
      return str.str();
}

template <>
tstring converter<short>::to_string(const short &i)
{
    tostringstream str;
    str << i;
    return str.str();
}

template <>
tstring converter<unsigned int>::to_string(const unsigned int &i)
{
    tostringstream str;
    str << i;
    return str.str();
}

template <>
tstring converter<unsigned long>::to_string(const unsigned long &i)
{
    tostringstream str;
    str << i;
    return str.str();
}

template <>
tstring converter<double>::to_string(const double &i)
{
    tostringstream s;
    s << setiosflags(ios_base::fixed) << setw(100) << i;
    return s.str();
}

template <>
tstring converter<unsigned char>::to_string(const unsigned char &c)
{
    tostringstream s;
    s << (unsigned)c;
    return s.str();
}

template <>
tstring converter<TCHAR>::to_string(const TCHAR &c)
{
    tostringstream s;
    s << c;
    return s.str();
}

template <>
tstring converter<bool>::to_string(const bool &b)
{
    tostringstream str;
    str << (b ? _T("1") : _T("0"));
    return str.str();
}

template <>
tstring converter<uint16_t>::to_string(const uint16_t &ui)
{
	tostringstream str;
	str << ui;
	return str.str();
}

template <>
size_t converter<bool>::from_string(const tstring &newvalue, bool &member)
{
    if (newvalue==_T("1") || newvalue==_T("true"))
        member=true;
    else
        member=false;
    return sizeof(bool);
}

template <>
tstring converter<tstring>::to_string(const tstring &s)
{
    return s;
}
#ifdef _UNICODE
template <>
tstring converter<string>::to_string(const string &s)
{
	return litwindow::s2tstring(s);
}
#else
template<>
tstring converter<wstring>::to_string(const wstring &s)
{
	return litwindow::w2tstring(s);
}
#endif
template <>
int converter<unsigned __int64>::to_int(const unsigned __int64 &i)
{
	return static_cast<int>(i);
}
template <>
int converter<signed __int64>::to_int(const signed __int64 &i)
{
	return static_cast<int>(i);
}
template <>
int converter<int>::to_int(const int &i)
{
    return i;
}
template <>
int converter<unsigned int>::to_int(const unsigned int &i)
{
	return i;
}

template <>
int converter<short>::to_int(const short &i)
{
    return i;
}

template <>
int converter<long>::to_int(const long &i)
{
      return i;
}

template <>
void converter<long>::from_int(int value, long &member)
{
      member=value;
}

template <>
void converter<unsigned __int64>::from_int(int value, unsigned __int64 &member)
{
	member=value;
}

template <>
void converter<int>::from_int(int value, int &member)
{
    member=value;
}

template <>
void converter<short>::from_int(int value, short &member)
{
    member=value;
}
template <>
void converter<unsigned int>::from_int(int value, unsigned int &member)
{
	member=value;
}
template <>
size_t converter<string>::from_string(const tstring &p, string &member)
{
    member=t2string(p);
    return member.length();
}

template <>
size_t converter<wstring>::from_string(const tstring &p, wstring &member)
{
    member=t2wstring(p);
    return member.length();
}

template <>
size_t converter<int>::from_string(const tstring &p, int &member)
{
    tistringstream str(p);
    str >> member;
    if (str.fail())
        throw lwbase_error("invalid from_string<int> input. not a number.");
    return sizeof(member);
}

template <>
size_t converter<long>::from_string(const tstring &p, long &member)
{
      tistringstream str(p);
      str >> member;
      if (str.fail())
            throw lwbase_error("invalid from_string<long> input. not a number.");
      return sizeof (member);
}

template<>
size_t converter<short>::from_string(const tstring &p, short &member)
{
    tistringstream str(p);
    str >> member;
    if (str.fail())
        throw lwbase_error("invalid from_string<short> input. not a number.");
    return sizeof(member);
}

#if defined(_NATIVE_WCHAR_T_DEFINED) && defined(UNICODE)
template<>
size_t converter<wchar_t>::from_string(const tstring &p, wchar_t &member)
{
	if (p.length()>1)
		throw lwbase_error("invalid from_string<wchar_t> input. string too long (more than 1 char)");
	else if (p.length()==1)
		member=p[0];
	else
		member=_T('\0');
	return sizeof(member);
}
#endif

template <>
size_t converter<unsigned int>::from_string(const tstring &p, unsigned int &member)
{
    tistringstream s(p);
    s >> member;
    if (s.fail())
        throw lwbase_error("invalid from_string<unsigned int> input. not a number.");
    return sizeof (member);
}

template <>
size_t converter<unsigned long>::from_string(const tstring &p, unsigned long &member)
{
    tistringstream s(p);
    s >> member;
    if (s.fail())
        throw lwbase_error("invalid from_string<unsigned long> input. not a number.");
    return sizeof (member);
}

template <class V>
tstring infinityAsString()
{
    static tstring i;
    static bool needsInit=true;
    if (needsInit) {
        if (numeric_limits<V>::has_infinity) {
            tostringstream o;
            o << numeric_limits<V>::infinity();
            i=o.str();
        }
        needsInit=false;
    }
    return i;
}

template <>
size_t converter<double>::from_string(const tstring &p, double &member)
{
    if (p==infinityAsString<double>()) {
        member=numeric_limits<double>::infinity();
    } else {
        tistringstream s(p);
        s >> member;
        if (s.fail())
            throw lwbase_error("invalid from_string<double> input. not a number.");
    }
    return sizeof (member);
}

template <>
size_t converter<float>::from_string(const tstring &p, float &member)
{
    if (p==infinityAsString<float>()) {
        member=numeric_limits<float>::infinity();
    } else {
        tistringstream s(p);
        s >> member;
        if (s.fail())
            throw lwbase_error("invalid from_string<float> input. not a number.");
    }
    return sizeof (member);
}

template <>
size_t converter<unsigned char>::from_string(const tstring &p, unsigned char &member)
{
    unsigned long l;
    tistringstream s(p);
    s >> l;
    if (l>numeric_limits<unsigned char>::max())
        throw lwbase_error("invalid from_string<unsigned char> input. number too big.");
    if (s.fail())
        throw lwbase_error("invalid from_string<unsigned char> input. not a number.");
    member=(unsigned char)l;
    return sizeof(member);
}

template <>
bool converter<short>::is_int() const
{
	return true;
}
template <>
bool converter<unsigned __int64>::is_int() const { return true; }
template <>
bool converter<signed __int64>::is_int() const { return true; }
template <>
bool converter<int>::is_int() const
{
    return true;
}

template <>
bool converter<long>::is_int() const
{
      return true;
}

template <>
tstring converter<float>::to_string(const float &f)
{
    tostringstream str;
    str << f;
    return str.str();
}

#endif // DOXYGEN_INVOKED

//#endregion

//!\if developers
void schema_base::init(const schema_entry *staticArray, const char *aClassName)
{
    classname=aClassName;
    while (staticArray->m_name) {
            // First insert this schema entry into the 'this_schema' vector used for simple iterators
        this_schema.push_back(*staticArray);
        this_schema.back().m_class_name=classname;
        this_schema.back().m_name=strdup(this_schema.back().m_name);

        //@todo add a check to ensure that the element is unique
        staticArray++;
    }
}

template <class Aggregate>
pair<typename Aggregate::_iter, bool> find_identifier(const Aggregate &a, const string &propName, bool search_member_aggregates)
{
    Aggregate::_iter rc=a.find(propName);
    if (rc==a.end()) {
        Aggregate::_iter k;
        for (k=a.begin(); k!=a.end(); ++k) {
            if (k->is_nested() || search_member_aggregates && k->is_aggregate()) {
                Aggregate nested=(*k).get_aggregate();
                pair<Aggregate::_iter, bool> nested_rc=nested.find_scope(propName, search_member_aggregates);
                if (nested_rc.second)
                    return nested_rc;
            }
        }
    }
    return make_pair(rc, rc!=a.end());
}

template <class Aggregate>
pair<typename Aggregate::_iter, bool> find_scope(const Aggregate &a, string propName, bool search_member_aggregates)
{
    pair<Aggregate::_iter, bool> found_rc;
    Aggregate currentAggregate=a;
    string identifier;
    while (propName.length()) {
        string::size_type endIdentifier=propName.find('.');
        if (endIdentifier!=string::npos) {
            identifier=propName.substr(0, endIdentifier);
            propName.erase(0, endIdentifier+1);
        } else {
            identifier=propName;
            propName.erase();
        }
        found_rc=find_identifier(currentAggregate, identifier, search_member_aggregates);
        if (found_rc.second==false)
            return found_rc; // not found
        if (propName.length()) {    // some more identifier left
            if (found_rc.first->is_aggregate())
                currentAggregate=found_rc.first->get_aggregate();
            else if (is_type<accessor>(*found_rc.first)) {
                accessor nested_accessor=dynamic_cast_accessor<accessor>(*found_rc.first).get();
                if (nested_accessor.is_valid() && nested_accessor.is_aggregate())
                    currentAggregate=nested_accessor.get_aggregate();
                else // cannot search any deeper
                    return make_pair(a.end(), false);
            } else
                return make_pair(a.end(), false);
        }
    }
    return found_rc;
}

pair<const_aggregate::const_iterator, bool> LWBASE_API const_aggregate::find_scope(const string &propName, bool search_member_aggregates) const
{
    return litwindow::find_scope<const_aggregate>(*this, propName, search_member_aggregates);
}

pair<aggregate::iterator, bool> LWBASE_API aggregate::find_scope(const string &propName, bool search_member_aggregates) const
{
    return litwindow::find_scope<aggregate>(*this, propName, search_member_aggregates);
}

void schema_base::free_strings()
{
    const_iterator i;
    for (i=this_schema.begin(); i!=this_schema.end(); ++i)
        free(const_cast<char*>(i->m_name));
}

//!\endif

      //-----------------------------------------------------------------------------------------------------------//
/** @defgroup data_adapters_warnings_errors Data Adapter Compiler & Linker Warnings and Errors

    @section intro Introduction
    The Data Adapter mechanism relies heavily on templates. Some mistakes generate rather obscure
    error messages and warnings. This section helps you solve these problems. The message numbers
    here refer to the Microsoft Visual C++ compiler.

    @section compiler_messages Compiler messages

    @section linker_messages Linker messages

      @subsection LNK1179 LNK1179 - xyz.obj: invalid or corrupt file: duplicate comdat
      @subsubsection Cause
            VC++ 6.0, Debug Build and using get/set methods PROP_GetSet/PROP_RW_FUNC etc...
            The VC 6.0 compiler is broken in many places, especially when it comes to templates.
            It duplicates the template instantiation make_getter_setter<...> when it should not.
      @subsubsection Solution
            Change the build settings for the offending object file. Goto 'xyz.cpp' and make the following
            changes for the debug build: On the C++ tab in the 'General' category set 'Debug info' to
            'Program Database' instead of 'Program Database for Edit and Continue'. In the 'Optimizations'
            category set 'Inline function expansion' to 'only __inline' instead of 'Disable'. This causes
            the offending template to be inlined and the error will disappear.

    @subsection LNK4049 LNK4049 - locally defined symbol "public: static struct litwindow::..." imported
    @subsubsection Cause
            You are using DECLARE_ADAPTER_TYPE(type, importexportspec) in an include file, using
        IMPLEMENT_ADAPTER_TYPE(type) in a source file and the 'importexportspec' is defined as
        "_declspec(dllimport)".

        IMPLEMENT_ADAPTER_TYPE(type) will always export _declspec(dllexport) the code that is being
        generated. Verify that the import/export specification you pass to DECLARE_ADAPTER_TYPE is
        correctly defined and resolves to _declspec(dllexport) when compiling the DLL/EXE.

            You are using VC 6.0, DECLARE_ADAPTER_TYPE(type, importexportspec) in an include file, using
        IMPLEMENT_ADAPTER_TYPE(type) in one source file and BEGIN_ADAPTER/END_ADAPTER in another source file.
        importexportspec is correctly defined.

        DECLARE_ADAPTER_TYPE declares an explicit specialization of a static function of a template class.
        data_adapter.h declares the generic template with _declspec(dllimport). The compiler will quietly
        change this to _declspec(dllexport) when it sees IMPLEMENT_ADAPTER_TYPE. But if you are using
        BEGIN_ADAPTER/END_ADAPTER in a different source file, that source file does not see the
        IMPLEMENT_ADAPTER_TYPE definition and thus the compiler assumes _declspec(dllimport) here.

    @subsubsection Solution
        This warning can safely be ignored. Pass "/WARN:1" to the linker to suppress this and all
        other warnings of level 2 and above. Future versions of the Lit Window Library may find a way
        to solve this problem.


    */

/** @defgroup data_adapters_howto Data Adapters Howto
    @section Overview
    Data adapters allow access to objects when the class declaration is not available at compile time.
    Use data adapters to create a library of algorithms for common issues such as
    -   loading/saving settings
    -   data exchange for dialogs
    -   printing debug information

    Use this library with objects of any class, even if the class declaration did not exist when you
    wrote the library. Data adapters provide the bridge between your class declaration and the
    algorithm in the library.

    @section quick_summary Quick Summary
    To prepare your program for data adapters, add...
    -#  IMPLEMENT_ADAPTER_TYPE(name) for simple data types and enumerations.
    -#  BEGIN_ADAPTER(name) / END_ADAPTER() tables for struct or class definitions.
    -#  IMPLEMENT_CONTAINER(name) for container types.

    Choose which adapter to use...
    -#  Use litwindow::accessor to access the values of simple data types and enumerations.
    -#  Use litwindow::aggregate to access members of struct and class definitions.
    -#  Use litwindow::container to access the elements in a container.

    To convert the basic adapter litwindow::accessor into the other forms...
    -#  Use litwindow::accessor::is_aggregate to determine if the adapter has an an aggregate adapter.
    -#  Use litwindow::accessor::get_aggregate to get the aggregate adapter for the object.
    -#  Use litwindow::accessor::is_container to determine if the adapter has a container adapter.
    -#  Use litwindow::accessor::get_container to get the container adapter for the object.

    @section adapters Three different kinds of data adapters.
    Data adapters come in three varieties.
        -   litwindow::accessor
        -   litwindow::aggregate
        -   litwindow::container

    Each kind has a const and a non const version.
    -   litwindow::accessor adapters allow access to the value of an object. The object itself is considered opaque.
        It has only one value and can be accessed only in its entirety.
    -   litwindow::aggregate adapters allow access to individual members of aggregates - struct or class type objects.
        They access one member at a time. aggregate adapters define an iterator that iterates over all member variables
        of the aggregate, including its superclasses if any.
    -   litwindow::container adapters allow access to the objects in a container. They define an iterator that iterates
        over all objects in the container, as well as common iterator operations such as begin, end, insert, delete.

    You can create accessor adapters from any kind of object, including aggregates and containers. Aggregate adapters
    can only be created from aggregate data types and container adapters only from container data types.

    @subsection accessor accessor and const_accessor.
    The most basic form of an adapter is the litwindow::accessor. Think of an accessor as a pointer. Like an
    untyped pointer it can point to any type of object, but it also contains the type information of the
    object it points to.
    @subsubsection implement_adapter_type   Preparing to use accessors
    Before you can use an accessor for a data type, you must make this data type available to the accessor adapter
    logic. Use IMPLEMENT_ADAPTER_TYPE(name) to make a data type available for accessor adapters. Include this macro anywhere
    in your sourcecode. You can use this macro even for data types you did not write yourself.
    @note You will probably use this macro only for enumerations. The basic data types char, int, float etc... are
    included in the litwindow library. Aggregate data types (see next section) are defined differently.

    @dontinclude fixtures.cpp
    @skipline IMPLEMENT_ADAPTER_TYPE

    @subsubsection using_accessors Using accessors
    -   Use litwindow::make_accessor or litwindow::make_const_accessor to create an accessor adapter from an object.
    -   Use @link litwindow::const_accessor::to_string to_string @endlink to get a string representation of the value.
    -   Use is_container or is_aggregate to find out if the object is a container or an aggregate

    @dontinclude basictests.cpp
    @skip  //beginexample testbasicaccess
    @until //endexample

    Use from_string and pass a string representation of the value to set the object value.

    @subsection aggregate aggregate and const_aggregate
    aggregate adapters allow access to individual members of an aggregate data type (struct or class). It is
    a collection of accessors. Each accessor represents a member pointer.
    */

}

#include <malloc.h>

namespace litwindow {

string LWBASE_API w2sstring(const wstring &w)
{
    locale loc;
    const wchar_t *next_in;
    char *next_out;
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    const codecvt<wchar_t, char, mbstate_t> &cvt( use_facet<codecvt<wchar_t, char, mbstate_t> >( loc ));
    size_t buffer_length=cvt.max_length()*w.length();
    char *buffer=(char*)_alloca(buffer_length*sizeof(char));
    cvt.out(state,
        w.data(), w.data()+w.length(), next_in,
        buffer, buffer+buffer_length, next_out);
    return string(buffer, next_out-buffer);
}

wstring LWBASE_API s2wstring(const string &s)
{
    locale loc;
    const char *next_in;
    wchar_t *next_out;
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    const codecvt<wchar_t, char, mbstate_t> &cvt(use_facet<codecvt<wchar_t, char, mbstate_t> >( loc ));
    size_t buffer_length=s.length();
    wchar_t *buffer=(wchar_t*)_alloca(buffer_length*sizeof(wchar_t));
    cvt.in(state,
        s.data(), s.data()+s.length(), next_in,
        buffer, buffer+buffer_length, next_out);
    return wstring(buffer, next_out-buffer);
}

};
