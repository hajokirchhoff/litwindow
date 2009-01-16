/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: wrapping_strstream_test.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"

#include "litwindow/logging.h"
#include "fixtures.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

USING_LITWINDOW_NS
using namespace std;

class WrappingTests:public CppUnit::TestFixture 
{
public:
    TCHAR ten_chars[11];
    void setUp()
    {
        _tcscpy(ten_chars, _T("0123456789"));
    }
    void tearDown()
    {
    }

    void basicTests()
    {
        wrapping_tostrstream test_out;
        test_out << _T("This is a test");
        tstring result;
        test_out.copy(result);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("This is a test")), result);
    }

    void wrapOnceTests()
    {
        wrapping_tostrstream test_out(32);
        test_out << _T('A') << ten_chars << _T('B') << ten_chars << _T('C') << ten_chars << _T('D') << ten_chars << _T('E');
        tstring result;
        test_out.copy(result);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("123456789C0123456789D0123456789E")), result);
    }

static tstring last_line;
static void printit(const TCHAR *line)
{
    last_line=line;
}

    void redirectTests()
    {
        wrapping_tostrstream test_out(32);
        static_redirect_tstreambuf rd(printit);
        rd.insert(test_out);
        last_line.erase();
        test_out << _T("Hallo") << endl;
        CPPUNIT_ASSERT_EQUAL(tstring(_T("Hallo")), last_line);
        tstring result;
        test_out.copy(result);
        CPPUNIT_ASSERT_EQUAL(tstring(_T("Hallo\n")), result);
    }

    void testTypeRegistration()
    {
        tstring result;
        lw_log().copy(result);
        lw_log().clear_buffer();
        tstring expected(_T("registering "));
        CPPUNIT_ASSERT_EQUAL(expected, result.substr(0, expected.length()));
        lw_log().copy(result);
        CPPUNIT_ASSERT_EQUAL(tstring(), result);
    }

    CPPUNIT_TEST_SUITE(WrappingTests);
        CPPUNIT_TEST(basicTests);
        CPPUNIT_TEST(wrapOnceTests);
        CPPUNIT_TEST(redirectTests);
        CPPUNIT_TEST(testTypeRegistration);
    CPPUNIT_TEST_SUITE_END();

};

tstring WrappingTests::last_line;

CPPUNIT_TEST_SUITE_REGISTRATION(WrappingTests);
