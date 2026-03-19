/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: wrapping_strstream_test.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include "litwindow/logging.h"
#include "fixtures.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace litwindow;
using namespace std;

static tstring last_line;

static void printit(const TCHAR *line)
{
    last_line=line;
}

struct WrappingTestsFixture
{
    TCHAR ten_chars[11];

    WrappingTestsFixture()
    {
        _tcscpy(ten_chars, _T("0123456789"));
    }

    ~WrappingTestsFixture()
    {
    }
};

BOOST_FIXTURE_TEST_SUITE(WrappingTests, WrappingTestsFixture)

BOOST_AUTO_TEST_CASE(basicTests)
{
    wrapping_tostrstream test_out;
    test_out << _T("This is a test");
    tstring result;
    test_out.copy(result);
    BOOST_TEST(result == tstring(_T("This is a test")));
}

BOOST_AUTO_TEST_CASE(wrapOnceTests)
{
    wrapping_tostrstream test_out(32);
    test_out << _T('A') << ten_chars << _T('B') << ten_chars << _T('C') << ten_chars << _T('D') << ten_chars << _T('E');
    tstring result;
    test_out.copy(result);
    BOOST_TEST(result == tstring(_T("123456789C0123456789D0123456789E")));
}

BOOST_AUTO_TEST_CASE(redirectTests)
{
    wrapping_tostrstream test_out(32);
    static_redirect_tstreambuf rd(printit);
    rd.insert(test_out);
    last_line.erase();
    test_out << _T("Hallo") << endl;
    BOOST_TEST(last_line == tstring(_T("Hallo")));
    tstring result;
    test_out.copy(result);
    BOOST_TEST(result == tstring(_T("Hallo\n")));
}

BOOST_AUTO_TEST_CASE(testTypeRegistration)
{
    tstring result;
    lw_log().copy(result);
    lw_log().clear_buffer();
    tstring expected(_T("registering "));
    BOOST_TEST(result.substr(0, expected.length()) == expected);
    lw_log().copy(result);
    BOOST_TEST(result == tstring());
}

BOOST_AUTO_TEST_SUITE_END()
