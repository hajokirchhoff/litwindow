#include "stdafx.h"
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include "litwindow/logger.hpp"
#include "litwindow/logger/sink.hpp"

#define new DEBUG_NEW

using namespace litwindow;
using namespace std;

BOOST_AUTO_TEST_CASE(simple_log_name)
{
    using namespace logger;
    name one("one");
    name two("two");
    name one_other("one");
    name three("three");
    name two_other("two");
    {
        BOOST_CHECK_EQUAL(one.index(), one_other.index());
        BOOST_CHECK_EQUAL(two.index(), two_other.index());
        BOOST_CHECK_NE(one.index(), two.index());
        BOOST_CHECK_NE(two.index(), three.index());

        BOOST_CHECK_EQUAL(one, one_other);
        BOOST_CHECK_GE(one, one_other);
        BOOST_CHECK(!(one>one_other));
        BOOST_CHECK(!(one<one_other));
        BOOST_CHECK(!(one!=one_other));
        BOOST_CHECK_EQUAL(two, two_other);
        BOOST_CHECK_LE(two, two_other);

        // compare strings - alphabetical ordering
        BOOST_CHECK_GE(two, one);
        BOOST_CHECK_GT(two, one);
        BOOST_CHECK_GE(two, three);
        BOOST_CHECK_GT(two, three);

        BOOST_CHECK_LE(one, two);
        BOOST_CHECK_LT(one, two);
    }
    name two_point_three(two+three);
    name two_point_three_direct("two/three");
    {
        BOOST_CHECK_EQUAL(two_point_three, two_point_three_direct);
    }
    {
        name one_by_index(one.index());
        name two_by_index(two.index());
        BOOST_CHECK_EQUAL(one, one_by_index);
        BOOST_CHECK_EQUAL(two, two_by_index);
        name complex_by_index(two_point_three_direct.index());
        BOOST_CHECK_EQUAL(two_point_three, complex_by_index);
    }
    {
        struct excp {
            static void invalid_index()
            {
                size_t invalid_i=name::end();
                name x(invalid_i);
            }
        };
        BOOST_CHECK_THROW(excp::invalid_index(), std::out_of_range);
    }
}

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
        test && "Hello";
        wstring rc=test.rdbuf()->str();
        BOOST_CHECK(rc==wstring(L"\t\tHello"));
    }
    {
		test && logger::warning && logger::contl;
        test && L"Ups";
        wstring rc=test.rdbuf()->str();
        BOOST_CHECK(rc==L"\t\tHello\t\tUps");
    }
}


BOOST_AUTO_TEST_CASE(simple_log_sink)
{
    using namespace logger;
	std::stringstream s;
    ostream_logsink sink(s);
	sink.format().timestamp=false;
	sink.format().level=false;
    events e("aComponent", "aTopic", &sink);
    e && "Test 1";
	string rc(s.str());
	BOOST_CHECK_EQUAL(rc, string("aComponent\taTopic\tTest 1\n"));
}

BOOST_AUTO_TEST_CASE(simple_log_level)
{
    using namespace logger;
	std::wstringstream s;
	wostream_logsink sink(s);
	sink.format().timestamp=false;
	sink.format().level=false;
    wevents e;
	e.sink(&sink);
    e && debug && L"This is a test with number " && 800;
    e && warning && L"Some more tests.";
	wstring rc(s.str());
	BOOST_CHECK(rc==wstring(L"\t\tThis is a test with number 800\n\t\tSome more tests.\n"));
}