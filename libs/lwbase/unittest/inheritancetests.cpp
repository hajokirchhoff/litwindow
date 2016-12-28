/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: inheritancetests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <litwindow/dataadapter.h>
#include "fixtures.h"
#include <stdexcept>
#include <iostream>
using ::std::runtime_error;
using namespace litwindow;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class InheritanceTests:public CppUnit::TestFixture 
{
public:
    CPPUNIT_TEST_SUITE(InheritanceTests);
        CPPUNIT_TEST(testSimpleInheritance);
        CPPUNIT_TEST(testSimpleInheritanceTwoLevels);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void testSimpleInheritance()
    {
        simpleInheritance si;
        accessor a=make_accessor(si);
        CPPUNIT_ASSERT_EQUAL(true, a.is_aggregate());
        aggregate ag=a.get_aggregate();
        accessor test=ag["siString"];
        CPPUNIT_ASSERT_EQUAL(tstring(_T("simpleInheritanceString")), test.to_string());
        // find the inherited member "WithGetterSetter"
        test=ag["WithGetterSetter"];
        CPPUNIT_ASSERT_EQUAL(true, test.is_aggregate());
        aggregate wg=test.get_aggregate();
        // test the members of the inherited member
        accessor test2=wg["_i"];
        CPPUNIT_ASSERT_EQUAL(tstring(_T("500")), test2.to_string());
        test2=wg["AnotherInt"];
        CPPUNIT_ASSERT_EQUAL(tstring(_T("600")), test2.to_string());

        // now test these members starting from the base aggregate specifying the full path
        test2=ag["WithGetterSetter._i"];
        CPPUNIT_ASSERT_EQUAL(tstring(_T("500")), test2.to_string());
        test2=ag["WithGetterSetter.AnotherInt"];
        CPPUNIT_ASSERT_EQUAL(tstring(_T("600")), test2.to_string());

        // now test these members starting from the base aggregate with the abbreviated path following C++ namespace rules
        test2=ag["_i"];
        CPPUNIT_ASSERT_EQUAL(tstring(_T("500")), test2.to_string());
        test2=ag["AnotherInt"];
        CPPUNIT_ASSERT_EQUAL(tstring(_T("600")), test2.to_string());

    }

    void testSimpleInheritanceTwoLevels()
    {
        simpleInheritanceTwoLevels si;
        const_aggregate a=make_const_aggregate(si);
        // test member access to a parent class two levels up
        CPPUNIT_ASSERT_EQUAL(tstring(_T("500")), a["_i"].to_string());
        CPPUNIT_ASSERT_EQUAL(tstring(_T("600")), a["AnotherInt"].to_string());
        // test member access using the full identifier
        CPPUNIT_ASSERT_EQUAL(tstring(_T("500")), a["simpleInheritance.WithGetterSetter._i"].to_string());
        CPPUNIT_ASSERT_EQUAL(tstring(_T("600")), a["simpleInheritance.WithGetterSetter.AnotherInt"].to_string());
        // test member access using a partial identifier
        CPPUNIT_ASSERT_EQUAL(tstring(_T("600")), a["WithGetterSetter.AnotherInt"].to_string());
        CPPUNIT_ASSERT_EQUAL(tstring(_T("500")), a["WithGetterSetter._i"].to_string());
        // test member in the parent class one level up
        CPPUNIT_ASSERT_EQUAL(tstring(_T("simpleInheritanceString")), a["simpleInheritance.siString"].to_string());
        CPPUNIT_ASSERT_EQUAL(tstring(_T("simpleInheritanceString")), a["siString"].to_string());
        // test member in the derived class
        CPPUNIT_ASSERT_EQUAL(tstring(_T("siTwo")), a["siTwo"].to_string());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(InheritanceTests);
