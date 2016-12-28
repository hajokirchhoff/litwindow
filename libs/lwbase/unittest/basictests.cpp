/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: basictests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <litwindow/dataadapter.h>
#include <litwindow/expr.h>
#include "fixtures.h"
#include <stdexcept>
#include <iostream>
using ::std::runtime_error;
using namespace litwindow;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace litwindow;

void experimentWithStuff()
{
}

class BasicTests:public CppUnit::TestFixture 
{
public:
    CPPUNIT_TEST_SUITE(BasicTests);
        CPPUNIT_TEST(experimentWithStuff);
        CPPUNIT_TEST(testBasicAccess);
        CPPUNIT_TEST(testDirectValueAccess);
        CPPUNIT_TEST(testAggregateAccess);
        CPPUNIT_TEST(testAggregateWithAggregateMemberAccess);
        CPPUNIT_TEST(testAggregateMemberDirectValueAccess);
        CPPUNIT_TEST(testWithGetSetFunction);
        CPPUNIT_TEST(testWithGetterSetterDirectValueAccess);
        CPPUNIT_TEST(testConstAccessor);
        CPPUNIT_TEST(testAssignFromAccessor);
        CPPUNIT_TEST(testAccessorAssignment);
        CPPUNIT_TEST_EXCEPTION(testNoSuchPropertyException, runtime_error);
        CPPUNIT_TEST_EXCEPTION(testNoToStringException, runtime_error);
        CPPUNIT_TEST(testEquality);
        CPPUNIT_TEST(testDynamicRuntimeTypeInformation);
        CPPUNIT_TEST(testExternalAccessorFunctions);
        CPPUNIT_TEST(testCoObjects);
		CPPUNIT_TEST(testCStrAccess);
    CPPUNIT_TEST_SUITE_END();

//#region helper stuff
    Fix1 *f1;
    int anInteger;

    bool testAccessorsEqual(bool expected, const accessor &one, const accessor &two)
    {
        CPPUNIT_ASSERT_EQUAL(expected, one.is_alias_of(two));
        CPPUNIT_ASSERT_EQUAL(expected, two.is_alias_of(one));
        CPPUNIT_ASSERT_EQUAL(expected, one==two);
        CPPUNIT_ASSERT_EQUAL(expected, two==one);
        CPPUNIT_ASSERT_EQUAL(!expected, one!=two);
        CPPUNIT_ASSERT_EQUAL(!expected, two!=one);
            // if they are expected to be unequal, either one<two or two<one must be true
        CPPUNIT_ASSERT_EQUAL(!expected, (one<two) || (two<one));
        if (expected==false) {
            // if one != two, then either one<two or two<one, but not both
            CPPUNIT_ASSERT_EQUAL(one < two, !(two < one));
        } else {
            CPPUNIT_ASSERT_EQUAL(false, one < two);
            CPPUNIT_ASSERT_EQUAL(false, two < one);
        }
        return true;
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
            CPPUNIT_ASSERT_EQUAL(total, equal+less+greater);
        }
    };
    /// iterate over all child accessors of @p a and return the number of accessors that are equal to @p b
    compare_count &countAccessorsEqual(const const_accessor &a, const const_accessor &b, compare_count &rc) const
    {
        CPPUNIT_ASSERT_EQUAL(true, a.is_valid());
        CPPUNIT_ASSERT_EQUAL(true, b.is_valid());
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
        CPPUNIT_ASSERT_EQUAL(string(), a.class_name());

            // cannot call 'get_aggregate'
        const_aggregate ag=a.get_aggregate();
        CPPUNIT_ASSERT_EQUAL(false, ag.is_valid());
            // cannot call 'get_container'
        try {
            const_container ac=a.get_container();
            CPPUNIT_FAIL("should have thrown an exception");
        }
        catch (std::runtime_error &) { /* expected exception has been thrown */ }

            // is not an aggregate
        CPPUNIT_ASSERT_EQUAL(false, a.is_aggregate());

            // b should be an alias of a and vice versa
        const_accessor b=a;
        CPPUNIT_ASSERT_EQUAL(true, a.is_alias_of(b));
        CPPUNIT_ASSERT_EQUAL(true, b.is_alias_of(a));

            // is not a container
        CPPUNIT_ASSERT_EQUAL(false, a.is_container());

            // is of type 
        CPPUNIT_ASSERT_EQUAL(type, a.type_name());
        CPPUNIT_ASSERT(a.get_type_name()==a.type_name());
    
            // is type
        CPPUNIT_ASSERT_EQUAL(true, a.is_type(propType));

            // should have 'this' as a name.
        CPPUNIT_ASSERT_EQUAL(string("this"), string(a.get_name()));
        CPPUNIT_ASSERT(a.name()==a.get_name());

            // check to_string / to_int access
        if (a.is_int()) {
            int intValue;
            accessor intA=make_accessor(intValue);
            intA.from_string(a.to_string());
            CPPUNIT_ASSERT(intValue==a.to_int());
        }
            // check expected value
        CPPUNIT_ASSERT_EQUAL(value, a.to_string());
    }
    void basicTestAccessorWithSimpleDatatype(accessor &a, const tstring &original_value, const tstring &new_value)
    {
            // test from_string.
        CPPUNIT_ASSERT_EQUAL(original_value, a.to_string());
            // assign a new value using 'from_string'
        a.from_string(new_value);
            // the new value should match the 'to_string' return value
        CPPUNIT_ASSERT_EQUAL(new_value, a.to_string());
            // test 'int' capabilities
        if (a.is_int()) {
            a.from_int(15);
            CPPUNIT_ASSERT_EQUAL(15, a.to_int());
            a.from_int(-89);
            CPPUNIT_ASSERT_EQUAL(-89, a.to_int());
                // try invalid format
            try {
                a.from_string(_T("pi"));
            }
            catch (std::runtime_error&) { /* should raise a runtime_exception since the format is not an integer */ }
        }
            // reassign the old value
        a.from_string(original_value);
        CPPUNIT_ASSERT_EQUAL(original_value, a.to_string());
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

	/// Test PROP_CSTR - access to char[]
	void testCStrAccess()
	{
		aggregate a=make_aggregate(*f1);
		CPPUNIT_ASSERT_EQUAL(tstring(_T("ThisIsA100Str")), a["c_str_100"].to_string());
		accessor direct(a["c_str_100"]);
        /// To obtain a pointer to the char[] buffer, use a typed_const_accessor<char>.
		typed_const_accessor<TCHAR> b=dynamic_cast_accessor<TCHAR>(direct);
        /// Get the pointer to the buffer itself.
        const TCHAR *p=b.get_ptr();
        CPPUNIT_ASSERT(*p==_T('T'));
		CPPUNIT_ASSERT_EQUAL(tstring(_T("ThisIsA100Str")), tstring(p));
        /// Get the size of the buffer.
        size_t bufferSize=b.get_sizeof();
        size_t charCount=bufferSize/sizeof(TCHAR);
        CPPUNIT_ASSERT_EQUAL(bufferSize, sizeof(TCHAR)*100);
        CPPUNIT_ASSERT_EQUAL(charCount, size_t(100));
        /// Copy data to the buffer.
        _tcscpy(const_cast<TCHAR*>(p), _T("A different String"));
        /// And it appears in the aggregate.
        CPPUNIT_ASSERT_EQUAL(tstring(_T("A different String")), direct.to_string());

        /// set a new value via accessor
        direct.from_string(tstring(_T("Yet another string")));
        /// must appear in the buffer
        CPPUNIT_ASSERT_EQUAL(tstring(_T("Yet another string")), tstring(p));

        /// Test length validation. A string of 99 chars should be okay...
        tstring this_string_fits_barely(99, _T('X'));
        direct.from_string(this_string_fits_barely);
        CPPUNIT_ASSERT_EQUAL(this_string_fits_barely, tstring(p));

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
            CPPUNIT_ASSERT_EQUAL(converterFunction, string(e.what()));
            was_thrown=true;
        }
        catch (...) {
            throw;   /// ops, this should not happen
        }
        CPPUNIT_ASSERT(was_thrown==true);
        /// Also the original value should be left untouched
        CPPUNIT_ASSERT_EQUAL(this_string_fits_barely, tstring(p));
	}

    void testWithGetSetFunction()
    {
        WithGetterSetter w;
        aggregate a=make_aggregate(w);
        a["_i"].from_int(20);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("20")), a["TestIntAccess"].to_string());
        w._another_int=987;
        CPPUNIT_ASSERT_EQUAL(tstring(_T("987")), a["AnotherInt"].to_string());
        a["AnotherInt"].from_string(_T("-5588"));
        CPPUNIT_ASSERT_EQUAL(-5588, w._another_int);
    }

    void testConstAccessor()
    {
        int i=99;
        const_accessor a=make_const_accessor(i);
        basicTestConstAccessorWithSimpleDatatype(a, _T("99"), "int", get_prop_type<int>());
        accessor b=make_accessor(i);
        basicTestConstAccessorWithSimpleDatatype(b, _T("99"), "int", get_prop_type<int>());
        basicTestAccessorWithSimpleDatatype(b, _T("99"), _T("-598"));
    }
//#endregion
//#region finished_tests
    void testDirectValueAccess()
    {
        // test direct access via typed_accessor

        // create a string to access
        tstring aString(_T("onetwothreefour"));
        // create a generic accessor
        accessor a=make_accessor(aString);
        // get a typed_accessor<string> from the generic accessor using dynamic_cast_accessor
        typed_accessor<tstring> as=dynamic_cast_accessor<tstring>(a);
        // this accessor is expected to be valid. After all, 'a' points to a string.
        CPPUNIT_ASSERT_EQUAL(true, as.is_valid());
        // use 'get' to retrieve a string and compare it with the expected value.
        tstring s;
        as.get(s);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("onetwothreefour")), s);

        // try to get a typed_accessor<int> from the generic accessor
        typed_accessor<int> ai=dynamic_cast_accessor<int>(a);
        // this typed_accessor<int> is expected to be invalid. 'a' does not point to an int.
        CPPUNIT_ASSERT_EQUAL(false, ai.is_valid());

        // use the typed_accessor<string> to set a new value
        as.set(_T("xyz"));
        // verify that 'set' did indeed change the original string 'aString'
        CPPUNIT_ASSERT_EQUAL(tstring(_T("xyz")), aString);

        // now test const_accessor and dynamic_cast_accessor with typed_const_accessor
        // first get a const_accessor from the accessor
        const_accessor ac(a);
        // now get a typed_const_accessor<string> from the const_accessor
        typed_const_accessor<tstring> acs(dynamic_cast_accessor<tstring>(ac));
        // the const_accessor should return the original string value
        CPPUNIT_ASSERT_EQUAL(tstring(_T("xyz")), ac.to_string());
        // and so should the typed_const_accessor<string>::get function.
        tstring newStringValue=*acs.get_ptr();
        CPPUNIT_ASSERT_EQUAL(tstring(_T("xyz")), newStringValue);

        // test the reverse. get a const_accessor from the typed_const_accessor.
        const_accessor reverse(acs.get_const_accessor());
        CPPUNIT_ASSERT_EQUAL(tstring(_T("xyz")), reverse.to_string());
    }
    void testAggregateMemberDirectValueAccess()
    {
        FixWithAggregateMembers m;
        const_aggregate a(make_const_aggregate(m));
        CPPUNIT_ASSERT_EQUAL(string("FixWithAggregateMembers"), a.class_name());
        const_accessor aggregateMember=a["anAggregateMember"];
        CPPUNIT_ASSERT(aggregateMember.is_aggregate());

        tstring r=as_debug(m);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("FixWithAggregateMembers{aString=thisIsAStringValue; Fix1{i1=3; f1=4.234; e1=1; c_str_100=ThisIsA100Str}; anInt=77}")), r);

        typed_const_accessor<Fix1> access_fix1=dynamic_cast_accessor<Fix1>(a["anAggregateMember"]);
        CPPUNIT_ASSERT_EQUAL(true, access_fix1.is_valid());
        CPPUNIT_ASSERT_EQUAL(int(3), access_fix1.get_ptr()->i1);
        CPPUNIT_ASSERT_EQUAL(float(4.234F), access_fix1.get().f1);
        CPPUNIT_ASSERT_EQUAL(Fix1::E1(Fix1::b), access_fix1.get().e1);
        //access_fix1.get().e1=Fix1::c;

        typed_const_accessor<tstring> access_string=dynamic_cast_accessor<tstring>(a["aString"]);
        CPPUNIT_ASSERT_EQUAL(true, access_string.is_valid());
        tstring aValue=access_string.get();
        CPPUNIT_ASSERT_EQUAL(tstring(_T("thisIsAStringValue")), aValue);
    }
    void testWithGetterSetterDirectValueAccess()
    {
        // this tests calling dynamic_cast_accessor for properties of an aggregate that
        // are exposed through get/set functions rather than being member variables.
        WithGetterSetter w;
        aggregate a=make_aggregate(w);
        a["_i"].from_int(20);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("20")), a["TestIntAccess"].to_string());
        w._another_int=987;
        CPPUNIT_ASSERT_EQUAL(tstring(_T("987")), a["AnotherInt"].to_string());
        a["AnotherInt"].from_string(_T("-5588"));
        CPPUNIT_ASSERT_EQUAL(-5588, w._another_int);

        typed_accessor<int> a_i=dynamic_cast_accessor<int>(a["TestIntAccess"]);
        typed_const_accessor<int> ac_i=dynamic_cast_accessor<int>(a["TestIntAccess"]);
        CPPUNIT_ASSERT_EQUAL(true, a_i.is_valid());
        CPPUNIT_ASSERT_EQUAL(true, ac_i.is_valid());
        CPPUNIT_ASSERT_EQUAL(int(20), a_i.get());
        CPPUNIT_ASSERT_EQUAL(int(20), ac_i.get());
        a_i.set(-492);
        CPPUNIT_ASSERT_EQUAL(int(-492), ac_i.get());

        typed_accessor<FixWithAggregateMembers> a_fix=dynamic_cast_accessor<FixWithAggregateMembers>(a["aFixWithAggregateMembers"]);
        CPPUNIT_ASSERT_EQUAL(true, a_fix.is_valid());
        CPPUNIT_ASSERT_EQUAL(77, a_fix.get().anInt);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("thisIsAStringValue")), a_fix.get().aString);
        FixWithAggregateMembers mv(a_fix.get());
        mv.anAggregateMember.i1=10101010;
        a_fix.set(mv);
        CPPUNIT_ASSERT_EQUAL(10101010, w.aFixWithAggregateMembers.anAggregateMember.i1);
        CPPUNIT_ASSERT_EQUAL(10101010, a_fix.get().anAggregateMember.i1);
        w.aFixWithAggregateMembers.anAggregateMember.f1=-454545.121212F;
        CPPUNIT_ASSERT_EQUAL(-454545.121212F, a_fix.get().anAggregateMember.f1);
    }
    void testBasicAccess()
    {
        Fix1::E1 ee;
        prop_t type=get_prop_type(&ee);
        //beginexample testbasicaccess
            // define a test object
        int i1=10;
            // use 'make_const_accessor' to create a const_accessor for any kind of object
        const_accessor a(make_const_accessor(i1));

            // use 'to_string' to return the value as a string
        CPPUNIT_ASSERT_EQUAL(tstring(_T("10")), a.to_string());

            // 'is_int' returns true if the value is an integer
        CPPUNIT_ASSERT(a.is_int());
            // 'to_int' returns the value as an integer - provided is_int returns true
        CPPUNIT_ASSERT(a.to_int()==10);

            // 'is_container' returns true if the value is a container
        CPPUNIT_ASSERT(a.is_container()==false);

        Fix1 f;
            // 'const_accessor's can be assigned just like pointers
        a=make_const_accessor(f);
            // this would throw an exception because there is no 'to_string' method for f...
//        CPPUNIT_ASSERT(a.to_string()=="100");
        CPPUNIT_ASSERT(a.is_int()==false);
        CPPUNIT_ASSERT(a.is_container()==false);
        //endexample

        a=make_const_accessor(i1);
        i1=15;
        CPPUNIT_ASSERT(a.to_string()==_T("15"));
        CPPUNIT_ASSERT(a.is_int());
        CPPUNIT_ASSERT(a.to_int()==15);

        CPPUNIT_ASSERT(a.is_container()==false);
        CPPUNIT_ASSERT(a.is_aggregate()==false);

        CPPUNIT_ASSERT_EQUAL(tstring(_T("15")), as_debug(a));
    }

    void testAggregateAccess()
    {
            // use 'make_const_aggregate' to create a const_aggregate for an aggregate object
        const_aggregate a(make_const_aggregate(*f1));
            // use 'find' to return an accessor to a member of a specific name.
            // 'type_name' returns the name of the member to which the accessor points to
        CPPUNIT_ASSERT_EQUAL(string("int"), a.find("i1")->type_name());
            // 'class_name' returns the name of the class of the member
        CPPUNIT_ASSERT_EQUAL(string("Fix1"), string(a.find("i1")->class_name()));
            // 'find' returns 'end()' if the member was not found. i1 should be found, so find!=end()
        CPPUNIT_ASSERT(a.find("i1")!=a.end());
        CPPUNIT_ASSERT_EQUAL(tstring(_T("100")), a.find("i1")->to_string());
        CPPUNIT_ASSERT_EQUAL(true, a.find("i1")->is_int());
            // operator[] is a shorthand for find. it throws an exception if the member does not exist.
        CPPUNIT_ASSERT_EQUAL(100, a["i1"].to_int());

            // test accessor::get_aggregate
        const_accessor ac(make_const_accessor(*f1));
        CPPUNIT_ASSERT_EQUAL(true, ac.is_aggregate());
        const_aggregate ac2a(ac.get_aggregate());
        CPPUNIT_ASSERT_EQUAL(true, ac2a.is_alias_of(a));

        const_accessor f1a=ac2a["f1"];
        CPPUNIT_ASSERT_EQUAL(string("float"), f1a.type_name());
        CPPUNIT_ASSERT_EQUAL(tstring(_T("100.1")), f1a.to_string());
        CPPUNIT_ASSERT_EQUAL(false, f1a.is_int());

        tstring r=as_debug(*f1);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("Fix1{i1=100; f1=100.1; e1=0; c_str_100=ThisIsA100Str}")), r);
    }
    void testNoSuchPropertyException()
    {
        const_aggregate a(make_const_aggregate(*f1));
        CPPUNIT_ASSERT(a.find("i2")==a.end());
            // Fix1 does not have a member called 'i2', so operator[] should throw a runtime_error exception
        CPPUNIT_ASSERT_EQUAL(20, a["i2"].to_int());
    }
    void testNoToStringException()
    {
        Fix1 f;
        const_accessor a(make_const_accessor(f));
            // The class 'Fix1' does not have a 'to_string' method implemented. Calling it throws an exception.
        CPPUNIT_ASSERT(a.to_string()==_T("100"));   // should throw an exception instead
    }
    void testAggregateWithAggregateMemberAccess()
    {
        FixWithAggregateMembers m;
        const_aggregate a(make_const_aggregate(m));
        CPPUNIT_ASSERT_EQUAL(string("FixWithAggregateMembers"), a.class_name());
        const_accessor aggregateMember=a["anAggregateMember"];
        CPPUNIT_ASSERT(aggregateMember.is_aggregate());

        tstring r=as_debug(m);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("FixWithAggregateMembers{aString=thisIsAStringValue; Fix1{i1=3; f1=4.234; e1=1; c_str_100=ThisIsA100Str}; anInt=77}")), r);

        const_accessor t=aggregateMember.get_aggregate()["e1"];
        CPPUNIT_ASSERT_EQUAL(tstring(_T("1")), t.to_string());
    }
//#endregion
    void testAccessorAssignment()
    {
        accessor a=make_accessor(*f1);
        accessor b=a;
        CPPUNIT_ASSERT(accessor_as_debug(b)==accessor_as_debug(a));
    }
    void testAssignFromAccessor()
    {
        accessor a=make_accessor(*f1);
        Fix1 two;
        accessor b=make_accessor(two);
        f1->i1=9999;
        f1->f1=8888.8f;
        CPPUNIT_ASSERT(two.i1!=9999);
        CPPUNIT_ASSERT(two.f1!=8888.8f);
        b.from_accessor(a);
        CPPUNIT_ASSERT_EQUAL(9999, two.i1);
        CPPUNIT_ASSERT_EQUAL(8888.8f, two.f1);
        a=make_accessor(anInteger);
        try {
            b.from_accessor(a);
        }
        catch (std::runtime_error &e) {
            CPPUNIT_ASSERT_EQUAL(string("type mismatch"), string(e.what()));
        }
    }
    void testCoObjects()
    {
        CoObjectTest aTest;
        aggregate a=make_aggregate(aTest);
        // f1 is a member variable of 'aTest'
        CPPUNIT_ASSERT_EQUAL(tstring(_T("100.1")), a["f1"].to_string());
        // but 'm_coobjectstring' is not. It is a member of the coobject.
        CPPUNIT_ASSERT_EQUAL(tstring(_T("This is the coobject!")), a["m_coobjectstring"].to_string());
    }
    void testExternalAccessorFunctions()
    {
        ExternalAccessorTest aTest;
        aTest.m_externalAccessorElement1=-1561;
        aggregate a=make_aggregate(aTest);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("-1561")), a["m_element2"].to_string());
        CPPUNIT_ASSERT_EQUAL(true, a["m_element2"].is_int());
        CPPUNIT_ASSERT_EQUAL(-1561, a["m_element2"].to_int());
        a["m_element2"].from_int(848491);
        CPPUNIT_ASSERT_EQUAL(848491, aTest.m_externalAccessorElement1);
        typed_accessor<int> aInt=dynamic_cast_accessor<int>(a["m_element2"]);

        CPPUNIT_ASSERT_EQUAL(true, aInt.is_valid());
        CPPUNIT_ASSERT_EQUAL(848491, aInt.get());
        aInt.set(1);
        CPPUNIT_ASSERT_EQUAL(1, aInt.get());
        CPPUNIT_ASSERT_EQUAL(1, aTest.m_externalAccessorElement1);
    }
    void testEquality()
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
        CPPUNIT_ASSERT(m==ga.get_accessor());
        CPPUNIT_ASSERT(make_aggregate(si2)==make_accessor(si2).get_aggregate());
        CPPUNIT_ASSERT(make_accessor(si2).get_aggregate().get_accessor()==make_accessor(si2));
        CPPUNIT_ASSERT(ga==make_accessor(si2).get_aggregate());
        CPPUNIT_ASSERT(ga.get_accessor()==make_accessor(si2));
        CPPUNIT_ASSERT(make_accessor(si2).get_aggregate().get_accessor()==m);
        CPPUNIT_ASSERT(m==make_accessor(si2));

        // compare accessors
        countAccessorsEqual(m, b, c);
        CPPUNIT_ASSERT_EQUAL(1, c.equal);
        c.assert_invariant();

        // compare to an accessor that is not part of the object
        b=make_accessor(anInteger);
        c.reset();
        countAccessorsEqual(m, b, c);
        CPPUNIT_ASSERT_EQUAL(0, c.equal);
        c.assert_invariant();

        // compare to an accessor that is a get/set property
        b=ga["anotherFix"];
        c.reset();
        countAccessorsEqual(m, b, c);
        CPPUNIT_ASSERT_EQUAL(1, c.equal);
        c.assert_invariant();
    }
    void experimentWithStuff()
    {
        ::experimentWithStuff();
    }
    void testDynamicRuntimeTypeInformation()
    {
        accessor a(make_accessor(anInteger));
        prop_t intType=get_prop_type_by_name("int");
        CPPUNIT_ASSERT_EQUAL(true, intType!=0);
        CPPUNIT_ASSERT_EQUAL(true, a.is_type(intType));
        tstring aString;
        a=make_accessor(aString);
#ifdef _UNICODE
#define tstringname "std::wstring"
#else
#define tstringname "std::string"
#endif
        prop_t stringType=get_prop_type_by_name(tstringname);
        CPPUNIT_ASSERT_EQUAL(true, stringType!=0);
        CPPUNIT_ASSERT_EQUAL(true, a.is_type(stringType));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BasicTests);

