/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: inheritancetests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <boost/test/unit_test.hpp>
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

BOOST_AUTO_TEST_SUITE(InheritanceTests)

BOOST_AUTO_TEST_CASE(testSimpleInheritance)
{
    simpleInheritance si;
    accessor a = make_accessor(si);
    BOOST_TEST(a.is_aggregate() == true);
    aggregate ag = a.get_aggregate();
    accessor test = ag["siString"];
    BOOST_TEST(test.to_string() == tstring(_T("simpleInheritanceString")));
    // find the inherited member "WithGetterSetter"
    test = ag["WithGetterSetter"];
    BOOST_TEST(test.is_aggregate() == true);
    aggregate wg = test.get_aggregate();
    // test the members of the inherited member
    accessor test2 = wg["_i"];
    BOOST_TEST(test2.to_string() == tstring(_T("500")));
    test2 = wg["AnotherInt"];
    BOOST_TEST(test2.to_string() == tstring(_T("600")));

    // now test these members starting from the base aggregate specifying the full path
    test2 = ag["WithGetterSetter._i"];
    BOOST_TEST(test2.to_string() == tstring(_T("500")));
    test2 = ag["WithGetterSetter.AnotherInt"];
    BOOST_TEST(test2.to_string() == tstring(_T("600")));

    // now test these members starting from the base aggregate with the abbreviated path following C++ namespace rules
    test2 = ag["_i"];
    BOOST_TEST(test2.to_string() == tstring(_T("500")));
    test2 = ag["AnotherInt"];
    BOOST_TEST(test2.to_string() == tstring(_T("600")));
}

BOOST_AUTO_TEST_CASE(testSimpleInheritanceTwoLevels)
{
    simpleInheritanceTwoLevels si;
    const_aggregate a = make_const_aggregate(si);
    // test member access to a parent class two levels up
    BOOST_TEST(a["_i"].to_string() == tstring(_T("500")));
    BOOST_TEST(a["AnotherInt"].to_string() == tstring(_T("600")));
    // test member access using the full identifier
    BOOST_TEST(a["simpleInheritance.WithGetterSetter._i"].to_string() == tstring(_T("500")));
    BOOST_TEST(a["simpleInheritance.WithGetterSetter.AnotherInt"].to_string() == tstring(_T("600")));
    // test member access using a partial identifier
    BOOST_TEST(a["WithGetterSetter.AnotherInt"].to_string() == tstring(_T("600")));
    BOOST_TEST(a["WithGetterSetter._i"].to_string() == tstring(_T("500")));
    // test member in the parent class one level up
    BOOST_TEST(a["simpleInheritance.siString"].to_string() == tstring(_T("simpleInheritanceString")));
    BOOST_TEST(a["siString"].to_string() == tstring(_T("simpleInheritanceString")));
    // test member in the derived class
    BOOST_TEST(a["siTwo"].to_string() == tstring(_T("siTwo")));
}

BOOST_AUTO_TEST_SUITE_END()
