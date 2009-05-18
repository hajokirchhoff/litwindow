#include "stdafx.h"
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include "litwindow/logger.hpp"

#define new DEBUG_NEW

using namespace litwindow;
using namespace std;

struct test_caller
{
    mutable size_t call_count;
    size_t dont_call_me() const { return ++call_count; }
    test_caller():call_count(0){}
};

BOOST_AUTO_TEST_CASE(logging_syntax_check)
{
    logger::basic_events<wchar_t, basic_stringstream<wchar_t> > test;
    {
        test_caller a;
        test && "Hello" && endl;
        wstring rc=test.rdbuf()->str();
        BOOST_CHECK(rc==wstring(L"Hello"));
    }
    {
        test && logger::warning;
        test && L"Ups" && std::endl;
        wstring rc=test.rdbuf()->str();
        BOOST_CHECK(rc==L"HelloUps");
    }
    //test << /*"Hello" << */logger::error << L"This is a test error. Value is " << 15 << endl;
    //test & logger::error; // & L"This error is." & endl;
    //test << 10;
    //test << L"This is a test";
    ////test << endl;

    //test & 10;
    //test & L"This is a test";
    //test & logger::error;
#ifdef not
    {
        test_caller a;
        test & logger::error & 10 & logger::warning & 15 & a.dont_call_me();
        BOOST_CHECK_EQUAL(a.call_count, 1);
    }
    {
        test_caller c;
        if (false && c.dont_call_me()==0) {

        }
        BOOST_CHECK_EQUAL(c.call_count, 0);
    }
    {
        test_caller b;
        test & logger::disable & logger::error & 10 & logger::warning & 15 & b.dont_call_me();
        //BOOST_CHECK_EQUAL(b.call_count, 0);
    }
    test & endl;
#endif
}

BOOST_AUTO_TEST_CASE(simple_log_streambuf)
{
    using namespace logger;
    basic_logstream<wchar_t> logstream;
    logstream << L"This is a test of " << 15 << " and numbers" << endl;
    std::wstring rc(logstream.rdbuf()->str());
    BOOST_CHECK(rc==std::wstring(L"This is a test of 15 and numbers\n"));
}

BOOST_AUTO_TEST_CASE(simple_log_level)
{
    using namespace logger;
    wevents e;
    e && debug && L"This is a test with number " && 800 && logger::endl;
}