/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: fixtures.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#ifndef _FIXTURES_
#define _FIXTURES_
#pragma once

#include <bitset>
#include <list>
#include <boost/test/unit_test.hpp>
#include "litwindow/dataadapter.h"

#if _USRDLL

#ifdef UNITTESTDLL_EXPORTS
#define UNITTESTDLL_API _declspec(dllexport)
#else
#define UNITTESTDLL_API _declspec(dllimport)

#ifdef _DEBUG
#pragma comment(lib, "unittestdlld.lib")
#else
#pragma comment(lib, "unittestdll.lib")
#endif

#endif
#else
#define UNITTESTDLL_API

#endif

using namespace std;

// Under a Unicode build litwindow::tstring is std::wstring, which Boost.Test
// cannot print via the default operator<<(std::ostream&, ...). Provide a
// print_log_value specialization (as done in libs/lwbase/test/example1.cpp)
// so BOOST_TEST/BOOST_CHECK on tstring values work in all test translation
// units that include this header.
#ifdef _UNICODE
template<>
inline void boost::test_tools::tt_detail::print_log_value<litwindow::tstring>::operator ()(std::ostream &ostr, const litwindow::tstring &t)
{
	ostr << litwindow::t2string(t);
}
#endif

class Fix1
{
public:
    int i1;
    float f1;
    enum E1 {
        a, b, c
    } e1;
    //DECLARE_ADAPTER(Fix1)

    Fix1(int _i=100, float _f=100.1F, E1 _e=a)
        :i1(_i),f1(_f),e1(_e)
    {
		_tcscpy(c_str_100, _T("ThisIsA100Str"));
	}
	TCHAR c_str_100[100];
};

DECLARE_ADAPTER_TYPE(Fix1::E1, UNITTESTDLL_API)

class FixWithAggregateMembers
{
public:
    litwindow::tstring  aString;
    Fix1        anAggregateMember;
    int         anInt;

    FixWithAggregateMembers()
        :aString(_T("thisIsAStringValue"))
        ,anAggregateMember(3, 4.234F, Fix1::b)
        ,anInt(77)
    { }
};

class WithGetterSetter
{
public:
    int _i;
    int _another_int;
    const int &getTestIntAccess() const
    {
        return _i;
    }
    void setTestIntAccess(const int &i)
    {
        _i=i;
    }

    int GetAnotherInt() const
    {
        return _another_int;
    }
    int SetAnotherInt(int i)    // and again a different signature
    {
        return _another_int=i;
    }
    FixWithAggregateMembers aFixWithAggregateMembers;
    const FixWithAggregateMembers &get_aFixWithAggregateMembers() const
    {
        return aFixWithAggregateMembers;
    }
    void    set_aFixWithAggregateMembers(const FixWithAggregateMembers &aFix)
    {
        aFixWithAggregateMembers=aFix;
    }
    FixWithAggregateMembers anotherFix;
    FixWithAggregateMembers get_anotherFix() const
    {
        return anotherFix;
    }
    void  set_anotherFix(const FixWithAggregateMembers &aFix)
    {
        anotherFix=aFix;
    }

    FixWithAggregateMembers thirdFix;
    FixWithAggregateMembers getThirdFix() const
    {
        return thirdFix;
    }
    void setThirdFix(const FixWithAggregateMembers &aFix)
    {
        thirdFix=aFix;
    }
};

class UNITTESTDLL_API BooleanVector
{
public:
#pragma warning(disable: 4251)
    vector<litwindow::tstring> m_string_vector;
    bitset<4> m_fourBools;
    vector<bool> get_fourBools() const;
    void set_fourBools(const vector<bool> &v);

};

/// Test class for simple inheritance tests.

class simpleInheritance:public WithGetterSetter
{
public:
    litwindow::tstring siString;
    simpleInheritance()
        :siString(_T("simpleInheritanceString"))
    {
        _i=500;
        _another_int=600;
    }
};

class simpleInheritanceTwoLevels:public simpleInheritance
{
public:
    litwindow::tstring siTwo;
    simpleInheritanceTwoLevels()
        :siTwo(_T("siTwo"))
    {
    }
};

class ExternalAccessorTest:public Fix1
{
public:
    int m_externalAccessorElement1;
};

class CoObjectTest:public Fix1
{
public:
    litwindow::tstring  m_someString;
};

#endif
