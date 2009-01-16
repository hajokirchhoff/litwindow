#include "stdafx.h"
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include "litwindow/logger.hpp"

using namespace litwindow;
using namespace std;

BOOST_AUTO_TEST_CASE(basic_logging)
{
    struct test_caller
    {
        mutable size_t call_count;
        size_t dont_call_me() const { return ++call_count; }
        test_caller():call_count(0){}
    };
    logger::basic_events<wchar_t, basic_stringbuf<wchar_t> > test;
    //test << /*"Hello" << */logger::error << L"This is a test error. Value is " << 15 << endl;
    //test & logger::error; // & L"This error is." & endl;
    //test << 10;
    //test << L"This is a test";
    ////test << endl;

    //test & 10;
    //test & L"This is a test";
    //test & logger::error;
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
}