/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: basictests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include <litwindow/dataadapter.h>
#include <litwindow/expr.h>
#include "fixtures.h"
#include <stdexcept>
#include <iostream>
using ::std::runtime_error;
using namespace litwindow;
using namespace std;

using namespace litwindow;

void experimentWithStuff()
{
}

struct compare_count
{
    int equal;
    int less;
    int greater;
    int total;
    compare_count()
    { reset (); }
    void operator += (const compare_count &b)
    {
        equal+=b.equal;
        less+=b.less;
        greater+=b.greater;
        total+=b.total;
    }
    void reset()
    {
        equal = less = greater = total = 0;
    }
    void assert_invariant()
    {
        BOOST_TEST(total == equal+less+greater);
    }
};

struct BasicTestsFixture
{
    Fix1 *f1;
    int anInteger;

    BasicTestsFixture()
    {
        f1=new Fix1;
        anInteger=10;
    }
    
    ~BasicTestsFixture()
    {
        delete f1;
    }

    bool testAccessorsEqual(bool expected, const accessor &one, const accessor &two)
    {
        BOOST_TEST(one.is_alias_of(two) == expected);
        BOOST_TEST(two.is_alias_of(one) == expected);
        BOOST_TEST((one==two) == expected);
        BOOST_TEST((two==one) == expected);
        BOOST_TEST((one!=two) == !expected);
        BOOST_TEST((two!=one) == !expected);
            // if they are expected to be unequal, either one<two or two<one must be true
        BOOST_TEST(((one<two) || (two<one)) == !expected);
        if (expected==false) {
            // if one != two, then either one<two or two<one, but not both
            BOOST_TEST((one < two) == !(two < one));
        } else {
            BOOST_TEST((one < two) == false);
            BOOST_TEST((two < one) == false);
        }
        return true;
    }

    /// iterate over all child accessors of @p a and return the number of accessors that are equal to @p b
    compare_count &countAccessorsEqual(const const_accessor &a, const const_accessor &b, compare_count &rc) const
    {
        BOOST_TEST(a.is_valid() == true);
        BOOST_TEST(b.is_valid() == true);
        if (a<b)
            ++rc.less;
        if (a==b)
            ++rc.equal;
        if (a>b)
            ++rc.greater;
        ++rc.total;
        const_aggregate ga=a.get_aggregate();
        if (ga.is_valid()) {
            const_aggregate::const_iterator i;
            for (i=ga.begin(); i!=ga.end(); ++i) {
                countAccessorsEqual(*i, b, rc);
            }
        }
        return rc;
    }

    void basicTestConstAccessorWithSimpleDatatype(const const_accessor &a, const tstring &value, const string &type, prop_t propType)
    {
            // has no class
        BOOST_TEST(a.class_name() == string());

            // cannot call 'get_aggregate'
        const_aggregate ag=a.get_aggregate();
        BOOST_TEST(ag.is_valid() == false);
            // cannot call 'get_container'
        try {
            const_container ac=a.get_container();
            BOOST_FAIL("should have thrown an exception");
        }
        catch (std::runtime_error &) { /* expected exception has been thrown */ }

            // is not an aggregate
        BOOST_TEST(a.is_aggregate() == false);

            // b should be an alias of a and vice versa
        const_accessor b=a;
        BOOST_TEST(a.is_alias_of(b) == true);
        BOOST_TEST(b.is_alias_of(a) == true);

            // is not a container
        BOOST_TEST(a.is_container() == false);

            // is of type 
        BOOST_TEST(a.type_name() == type);
        BOOST_TEST((a.get_type_name()==a.type_name()));
    
            // is type
        BOOST_TEST(a.is_type(propType) == true);

            // should have 'this' as a name.
        BOOST_TEST(string(a.get_name()) == string("this"));
        BOOST_TEST((a.name()==a.get_name()));

            // check to_string / to_int access
        if (a.is_int()) {
            int intValue;
            accessor intA=make_accessor(intValue);
            intA.from_string(a.to_string());
            BOOST_TEST((intValue==a.to_int()));
        }
            // check expected value
        BOOST_TEST(a.to_string() == value);
    }
    
    void basicTestAccessorWithSimpleDatatype(accessor &a, const tstring &original_value, const tstring &new_value)
    {
            // test from_string.
        BOOST_TEST(a.to_string() == original_value);
            // assign a new value using 'from_string'
        a.from_string(new_value);
            // the new value should match the 'to_string' return value
        BOOST_TEST(a.to_string() == new_value);
            // test 'int' capabilities
        if (a.is_int()) {
            a.from_int(15);
            BOOST_TEST(a.to_int() == 15);
            a.from_int(-89);
            BOOST_TEST(a.to_int() == -89);
                // try invalid format
            try {
                a.from_string(_T("pi"));
            }
            catch (std::runtime_error&) { /* should raise a runtime_exception since the format is not an integer */ }
        }
            // reassign the old value
        a.from_string(original_value);
        BOOST_TEST(a.to_string() == original_value);
    }

public:
    void setUp()
    {
        f1=new Fix1;
        anInteger=10;
    }
    void tearDown()
    {
        delete f1;
    }
};

BOOST_FIXTURE_TEST_SUITE(BasicTests, BasicTestsFixture)

BOOST_AUTO_TEST_CASE(experimentWithStuff)
{
    ::experimentWithStuff();
}

/// Test PROP_CSTR - access to char[]
BOOST_AUTO_TEST_CASE(testCStrAccess)
{
    aggregate a=make_aggregate(*f1);
    BOOST_TEST(a["c_str_100"].to_string() == tstring(_T("ThisIsA100Str")));
    accessor direct(a["c_str_100"]);
    /// To obtain a pointer to the char[] buffer, use a typed_const_accessor<char>.
    typed_const_accessor<TCHAR> b=dynamic_cast_accessor<TCHAR>(direct);
    /// Get the pointer to the buffer itself.
    const TCHAR *p=b.get_ptr();
    BOOST_TEST((*p==_T('T')));
    BOOST_TEST(tstring(p) == tstring(_T("ThisIsA100Str")));
    /// Get the size of the buffer.
    size_t bufferSize=b.get_sizeof();
    size_t charCount=bufferSize/sizeof(TCHAR);
    BOOST_TEST(bufferSize == sizeof(TCHAR)*100);
    BOOST_TEST(charCount == size_t(100));
    /// Copy data to the buffer.
    _tcscpy(const_cast<TCHAR*>(p), _T("A different String"));
    /// And it appears in the aggregate.
    BOOST_TEST(direct.to_string() == tstring(_T("A different String")));

    /// set a new value via accessor
    direct.from_string(tstring(_T("Yet another string")));
    /// must appear in the buffer
    BOOST_TEST(tstring(p) == tstring(_T("Yet another string")));

    /// Test length validation. A string of 99 chars should be okay...
    tstring this_string_fits_barely(99, _T('X'));
    direct.from_string(this_string_fits_barely);
    BOOST_TEST(tstring(p) == this_string_fits_barely);

    bool was_thrown=false;
    tstring this_string_is_too_long(100, _T('Q'));
    try {
        /// this should throw an 'out_of_range' error since the string is too long.
        direct.from_string(this_string_is_too_long);
    }
    catch (std::out_of_range &e) {
        /// catch the error
        /// NOTE: b.type_name() could either be 'char', 'wchar_t' or 'unsigned short' depending on whether the compiler recognized wchar_t as a builtin type
        string converterFunction(string("converter<")+b.type_name()+string(">::from_string input too long"));
        BOOST_TEST(string(e.what()) == converterFunction);
        was_thrown=true;
    }
    catch (...) {
        throw;   /// ops, this should not happen
    }
    BOOST_TEST((was_thrown==true));
    /// Also the original value should be left untouched
    BOOST_TEST(tstring(p) == this_string_fits_barely);
}

BOOST_AUTO_TEST_CASE(testWithGetSetFunction)
{
    WithGetterSetter w;
    aggregate a=make_aggregate(w);
    a["_i"].from_int(20);
    BOOST_TEST(a["TestIntAccess"].to_string() == tstring(_T("20")));
    w._another_int=987;
    BOOST_TEST(a["AnotherInt"].to_string() == tstring(_T("987")));
    a["AnotherInt"].from_string(_T("-5588"));
    BOOST_TEST(w._another_int == -5588);
}

BOOST_AUTO_TEST_CASE(testConstAccessor)
{
    int i=99;
    const_accessor a=make_const_accessor(i);
    basicTestConstAccessorWithSimpleDatatype(a, _T("99"), "int", get_prop_type<int>());
    accessor b=make_accessor(i);
    basicTestConstAccessorWithSimpleDatatype(b, _T("99"), "int", get_prop_type<int>());
    basicTestAccessorWithSimpleDatatype(b, _T("99"), _T("-598"));
}

BOOST_AUTO_TEST_CASE(testDirectValueAccess)
{
    // test direct access via typed_accessor

    // create a string to access
    tstring aString(_T("onetwothreefour"));
    // create a generic accessor
    accessor a=make_accessor(aString);
    // get a typed_accessor<string> from the generic accessor using dynamic_cast_accessor
    typed_accessor<tstring> as=dynamic_cast_accessor<tstring>(a);
    // this accessor is expected to be valid. After all, 'a' points to a string.
    BOOST_TEST(as.is_valid() == true);
    // use 'get' to retrieve a string and compare it with the expected value.
    tstring s;
    as.get(s);
    BOOST_TEST(s == tstring(_T("onetwothreefour")));

    // try to get a typed_accessor<int> from the generic accessor
    typed_accessor<int> ai=dynamic_cast_accessor<int>(a);
    // this typed_accessor<int> is expected to be invalid. 'a' does not point to an int.
    BOOST_TEST(ai.is_valid() == false);

    // use the typed_accessor<string> to set a new value
    as.set(_T("xyz"));
    // verify that 'set' did indeed change the original string 'aString'
    BOOST_TEST(aString == tstring(_T("xyz")));

    // now test const_accessor and dynamic_cast_accessor with typed_const_accessor
    // first get a const_accessor from the accessor
    const_accessor ac(a);
    // now get a typed_const_accessor<string> from the const_accessor
    typed_const_accessor<tstring> acs(dynamic_cast_accessor<tstring>(ac));
    // the const_accessor should return the original string value
    BOOST_TEST(ac.to_string() == tstring(_T("xyz")));
    // and so should the typed_const_accessor<string>::get function.
    tstring newStringValue=*acs.get_ptr();
    BOOST_TEST(newStringValue == tstring(_T("xyz")));

    // test the reverse. get a const_accessor from the typed_const_accessor.
    const_accessor reverse(acs.get_const_accessor());
    BOOST_TEST(reverse.to_string() == tstring(_T("xyz")));
}

BOOST_AUTO_TEST_CASE(testAggregateMemberDirectValueAccess)
{
    FixWithAggregateMembers m;
    const_aggregate a(make_const_aggregate(m));
    BOOST_TEST(a.class_name() == string("FixWithAggregateMembers"));
    const_accessor aggregateMember=a["anAggregateMember"];
    BOOST_TEST((aggregateMember.is_aggregate()));

    tstring r=as_debug(m);
    BOOST_TEST(r == tstring(_T("FixWithAggregateMembers{aString=thisIsAStringValue; Fix1{i1=3; f1=4.234; e1=1; c_str_100=ThisIsA100Str}; anInt=77}")));

    typed_const_accessor<Fix1> access_fix1=dynamic_cast_accessor<Fix1>(a["anAggregateMember"]);
    BOOST_TEST(access_fix1.is_valid() == true);
    BOOST_TEST(access_fix1.get_ptr()->i1 == int(3));
    BOOST_TEST(access_fix1.get().f1 == float(4.234F));
    BOOST_TEST(access_fix1.get().e1 == Fix1::E1(Fix1::b));
    //access_fix1.get().e1=Fix1::c;

    typed_const_accessor<tstring> access_string=dynamic_cast_accessor<tstring>(a["aString"]);
    BOOST_TEST(access_string.is_valid() == true);
    tstring aValue=access_string.get();
    BOOST_TEST(aValue == tstring(_T("thisIsAStringValue")));
}

BOOST_AUTO_TEST_CASE(testWithGetterSetterDirectValueAccess)
{
    // this tests calling dynamic_cast_accessor for properties of an aggregate that
    // are exposed through get/set functions rather than being member variables.
    WithGetterSetter w;
    aggregate a=make_aggregate(w);
    a["_i"].from_int(20);
    BOOST_TEST(a["TestIntAccess"].to_string() == tstring(_T("20")));
    w._another_int=987;
    BOOST_TEST(a["AnotherInt"].to_string() == tstring(_T("987")));
    a["AnotherInt"].from_string(_T("-5588"));
    BOOST_TEST(w._another_int == -5588);

    typed_accessor<int> a_i=dynamic_cast_accessor<int>(a["TestIntAccess"]);
    typed_const_accessor<int> ac_i=dynamic_cast_accessor<int>(a["TestIntAccess"]);
    BOOST_TEST(a_i.is_valid() == true);
    BOOST_TEST(ac_i.is_valid() == true);
    BOOST_TEST(a_i.get() == int(20));
    BOOST_TEST(ac_i.get() == int(20));
    a_i.set(-492);
    BOOST_TEST(ac_i.get() == int(-492));

    typed_accessor<FixWithAggregateMembers> a_fix=dynamic_cast_accessor<FixWithAggregateMembers>(a["aFixWithAggregateMembers"]);
    BOOST_TEST(a_fix.is_valid() == true);
    BOOST_TEST(a_fix.get().anInt == 77);
    BOOST_TEST(a_fix.get().aString == tstring(_T("thisIsAStringValue")));
    FixWithAggregateMembers mv(a_fix.get());
    mv.anAggregateMember.i1=10101010;
    a_fix.set(mv);
    BOOST_TEST(w.aFixWithAggregateMembers.anAggregateMember.i1 == 10101010);
    BOOST_TEST(a_fix.get().anAggregateMember.i1 == 10101010);
    w.aFixWithAggregateMembers.anAggregateMember.f1=-454545.121212F;
    BOOST_TEST(a_fix.get().anAggregateMember.f1 == -454545.121212F);
}

BOOST_AUTO_TEST_CASE(testBasicAccess)
{
    Fix1::E1 ee;
    prop_t type=get_prop_type(&ee);
    //beginexample testbasicaccess
        // define a test object
    int i1=10;
        // use 'make_const_accessor' to create a const_accessor for any kind of object
    const_accessor a(make_const_accessor(i1));

        // use 'to_string' to return the value as a string
    BOOST_TEST(a.to_string() == tstring(_T("10")));

        // 'is_int' returns true if the value is an integer
    BOOST_TEST((a.is_int()));
        // 'to_int' returns the value as an integer - provided is_int returns true
    BOOST_TEST((a.to_int()==10));

        // 'is_container' returns true if the value is a container
    BOOST_TEST((a.is_container()==false));

    Fix1 f;
        // 'const_accessor's can be assigned just like pointers
    a=make_const_accessor(f);
        // this would throw an exception because there is no 'to_string' method for f...
//        BOOST_TEST(a.to_string()=="100");
    BOOST_TEST((a.is_int()==false));
    BOOST_TEST((a.is_container()==false));
    //endexample

    a=make_const_accessor(i1);
    i1=15;
    BOOST_TEST((a.to_string()==_T("15")));
    BOOST_TEST((a.is_int()));
    BOOST_TEST((a.to_int()==15));

    BOOST_TEST((a.is_container()==false));
    BOOST_TEST((a.is_aggregate()==false));

    BOOST_TEST(as_debug(a) == tstring(_T("15")));
}

BOOST_AUTO_TEST_CASE(testAggregateAccess)
{
        // use 'make_const_aggregate' to create a const_aggregate for an aggregate object
    const_aggregate a(make_const_aggregate(*f1));
        // use 'find' to return an accessor to a member of a specific name.
        // 'type_name' returns the name of the member to which the accessor points to
    BOOST_TEST(a.find("i1")->type_name() == string("int"));
        // 'class_name' returns the name of the class of the member
    BOOST_TEST(string(a.find("i1")->class_name()) == string("Fix1"));
        // 'find' returns 'end()' if the member was not found. i1 should be found, so find!=end()
    BOOST_TEST((a.find("i1")!=a.end()));
    BOOST_TEST(a.find("i1")->to_string() == tstring(_T("100")));
    BOOST_TEST(a.find("i1")->is_int() == true);
        // operator[] is a shorthand for find. it throws an exception if the member does not exist.
    BOOST_TEST(a["i1"].to_int() == 100);

        // test accessor::get_aggregate
    const_accessor ac(make_const_accessor(*f1));
    BOOST_TEST(ac.is_aggregate() == true);
    const_aggregate ac2a(ac.get_aggregate());
    BOOST_TEST(ac2a.is_alias_of(a) == true);

    const_accessor f1a=ac2a["f1"];
    BOOST_TEST(f1a.type_name() == string("float"));
    BOOST_TEST(f1a.to_string() == tstring(_T("100.1")));
    BOOST_TEST(f1a.is_int() == false);

    tstring r=as_debug(*f1);
    BOOST_TEST(r == tstring(_T("Fix1{i1=100; f1=100.1; e1=0; c_str_100=ThisIsA100Str}")));
}

BOOST_AUTO_TEST_CASE(testNoSuchPropertyException)
{
    const_aggregate a(make_const_aggregate(*f1));
    BOOST_TEST((a.find("i2")==a.end()));
        // Fix1 does not have a member called 'i2', so operator[] should throw a runtime_error exception
    BOOST_CHECK_THROW(a["i2"].to_int(), runtime_error);
}

BOOST_AUTO_TEST_CASE(testNoToStringException)
{
    Fix1 f;
    const_accessor a(make_const_accessor(f));
        // The class 'Fix1' does not have a 'to_string' method implemented. Calling it throws an exception.
    BOOST_CHECK_THROW(a.to_string(), runtime_error);
}

BOOST_AUTO_TEST_CASE(testAggregateWithAggregateMemberAccess)
{
    FixWithAggregateMembers m;
    const_aggregate a(make_const_aggregate(m));
    BOOST_TEST(a.class_name() == string("FixWithAggregateMembers"));
    const_accessor aggregateMember=a["anAggregateMember"];
    BOOST_TEST((aggregateMember.is_aggregate()));

    tstring r=as_debug(m);
    BOOST_TEST(r == tstring(_T("FixWithAggregateMembers{aString=thisIsAStringValue; Fix1{i1=3; f1=4.234; e1=1; c_str_100=ThisIsA100Str}; anInt=77}")));

    const_accessor t=aggregateMember.get_aggregate()["e1"];
    BOOST_TEST(t.to_string() == tstring(_T("1")));
}

BOOST_AUTO_TEST_CASE(testAccessorAssignment)
{
    accessor a=make_accessor(*f1);
    accessor b=a;
    BOOST_TEST((accessor_as_debug(b)==accessor_as_debug(a)));
}

BOOST_AUTO_TEST_CASE(testAssignFromAccessor)
{
    accessor a=make_accessor(*f1);
    Fix1 two;
    accessor b=make_accessor(two);
    f1->i1=9999;
    f1->f1=8888.8f;
    BOOST_TEST((two.i1!=9999));
    BOOST_TEST((two.f1!=8888.8f));
    b.from_accessor(a);
    BOOST_TEST(two.i1 == 9999);
    BOOST_TEST(two.f1 == 8888.8f);
    a=make_accessor(anInteger);
    try {
        b.from_accessor(a);
    }
    catch (std::runtime_error &e) {
        BOOST_TEST(string(e.what()) == string("type mismatch"));
    }
}

BOOST_AUTO_TEST_CASE(testCoObjects)
{
    CoObjectTest aTest;
    aggregate a=make_aggregate(aTest);
    // f1 is a member variable of 'aTest'
    BOOST_TEST(a["f1"].to_string() == tstring(_T("100.1")));
    // but 'm_coobjectstring' is not. It is a member of the coobject.
    BOOST_TEST(a["m_coobjectstring"].to_string() == tstring(_T("This is the coobject!")));
}

BOOST_AUTO_TEST_CASE(testExternalAccessorFunctions)
{
    ExternalAccessorTest aTest;
    aTest.m_externalAccessorElement1=-1561;
    aggregate a=make_aggregate(aTest);
    BOOST_TEST(a["m_element2"].to_string() == tstring(_T("-1561")));
    BOOST_TEST(a["m_element2"].is_int() == true);
    BOOST_TEST(a["m_element2"].to_int() == -1561);
    a["m_element2"].from_int(848491);
    BOOST_TEST(aTest.m_externalAccessorElement1 == 848491);
    typed_accessor<int> aInt=dynamic_cast_accessor<int>(a["m_element2"]);

    BOOST_TEST(aInt.is_valid() == true);
    BOOST_TEST(aInt.get() == 848491);
    aInt.set(1);
    BOOST_TEST(aInt.get() == 1);
    BOOST_TEST(aTest.m_externalAccessorElement1 == 1);
}

BOOST_AUTO_TEST_CASE(testEquality)
{
    Fix1 _f2;
    accessor one(make_accessor(*f1));
    accessor two(make_accessor(_f2));
    testAccessorsEqual(false, one, two);

    WithGetterSetter g, h;
    aggregate agg=make_aggregate(g);
    one=agg["TestIntAccess"];
    two=make_aggregate(g)["TestIntAccess"];
    testAccessorsEqual(true, one, two);

    two=make_aggregate(h)["TestIntAccess"];
    testAccessorsEqual(false, one, two);

    one=agg["aFixWithAggregateMembers"];
    testAccessorsEqual(false, one, two);

    two=make_aggregate(g)["aFixWithAggregateMembers"];
    testAccessorsEqual(true, one, two);

    one=agg["anotherFix"];
    testAccessorsEqual(false, one, two);

    two=agg["aFixWithAggregateMembers"];
    testAccessorsEqual(false, one, two);

    two=agg["anotherFix"];
    testAccessorsEqual(true, one, two);

    // now the same with inheritance and getter/setter functions

    compare_count c;
    simpleInheritanceTwoLevels si2;
    aggregate ga(make_aggregate(si2));
    accessor b(make_accessor(si2.siString));
    testAccessorsEqual(true, ga["siString"], b);
    accessor m=ga.get_accessor();

    // assert different ways of getting an accessor to si2
    BOOST_TEST((m==ga.get_accessor()));
    BOOST_TEST((make_aggregate(si2)==make_accessor(si2).get_aggregate()));
    BOOST_TEST((make_accessor(si2).get_aggregate().get_accessor()==make_accessor(si2)));
    BOOST_TEST((ga==make_accessor(si2).get_aggregate()));
    BOOST_TEST((ga.get_accessor()==make_accessor(si2)));
    BOOST_TEST((make_accessor(si2).get_aggregate().get_accessor()==m));
    BOOST_TEST((m==make_accessor(si2)));

    // compare accessors
    countAccessorsEqual(m, b, c);
    BOOST_TEST(c.equal == 1);
    c.assert_invariant();

    // compare to an accessor that is not part of the object
    b=make_accessor(anInteger);
    c.reset();
    countAccessorsEqual(m, b, c);
    BOOST_TEST(c.equal == 0);
    c.assert_invariant();

    // compare to an accessor that is a get/set property
    b=ga["anotherFix"];
    c.reset();
    countAccessorsEqual(m, b, c);
    BOOST_TEST(c.equal == 1);
    c.assert_invariant();
}

BOOST_AUTO_TEST_CASE(testDynamicRuntimeTypeInformation)
{
    accessor a(make_accessor(anInteger));
    prop_t intType=get_prop_type_by_name("int");
    BOOST_TEST((intType!=0) == true);
    BOOST_TEST(a.is_type(intType) == true);
    tstring aString;
    a=make_accessor(aString);
#ifdef _UNICODE
#define tstringname "std::wstring"
#else
#define tstringname "std::string"
#endif
    prop_t stringType=get_prop_type_by_name(tstringname);
    BOOST_TEST((stringType!=0) == true);
    BOOST_TEST(a.is_type(stringType) == true);
}

BOOST_AUTO_TEST_SUITE_END()

