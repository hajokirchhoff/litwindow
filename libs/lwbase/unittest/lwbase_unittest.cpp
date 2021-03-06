#include "stdafx.h"
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#include "litwindow/logger.hpp"
#include "litwindow/logger/sink.hpp"
#include "boost/thread/thread.hpp"

#define new DEBUG_NEW

using namespace litwindow;
using namespace std;

BOOST_AUTO_TEST_CASE(simple_log_name)
{
    using namespace logger;
    tag one("one");
    tag two("two");
    tag one_other("one");
    tag three("three");
    tag two_other("two");
	tag two_third=two_other;
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
    tag two_point_three(two+three);
    tag two_point_three_direct("two/three");
    {
        BOOST_CHECK_EQUAL(two_point_three, two_point_three_direct);
    }
    {
        tag one_by_index(one.index());
        tag two_by_index(two.index());
        BOOST_CHECK_EQUAL(one, one_by_index);
        BOOST_CHECK_EQUAL(two, two_by_index);
        tag complex_by_index(two_point_three_direct.index());
        BOOST_CHECK_EQUAL(two_point_three, complex_by_index);
    }
    {
        struct excp {
            static void invalid_index()
            {
                size_t invalid_i=tag::end();
                tag x(invalid_i);
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
        BOOST_CHECK(rc==wstring(L"info\t\t\t0\tHello"));
    }
    {
        // test contl immediately after level,component,topic
		test && logger::warning && logger::contl;
        test && L"Ups";
        wstring rc=test.rdbuf()->str();
        BOOST_CHECK(rc==L"info\t\t\t0\tHellowarning\t\t\t0\tUps");
    }
    {
        // test contl after some text
        test && logger::error && L"some text - " && logger::contl;
        test && L"some more text";
        wstring rc=test.rdbuf()->str();
        BOOST_CHECK(rc==L"info\t\t\t0\tHellowarning\t\t\t0\tUpserror\t\t\t0\tsome text - some more text");
    }
}


BOOST_AUTO_TEST_CASE(simple_log_sink)
{
    using namespace logger;
	std::stringstream s;
    ostream_logsink sink(s);
	sink.format().timestamp=false;
	sink.format().level=false;
    events e("aComponent", "aTopic", logger::enabled, sink);
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
	threadsafe::wevents e;
	e.get_default().sink(&sink);
    e && debug && L"This is a test with number " && 800;
    e && warning && L"Some more tests.";
	struct run_in_thread_t 
	{
		threadsafe::wevents m_e;
		run_in_thread_t(threadsafe::wevents &e):m_e(e){}
		void operator()()
		{
			m_e && debug && L"This from inside the thread" && 900;
		}
	};
	run_in_thread_t call_rit(e);
	wstring rc_a(s.str());
	boost::thread trd(call_rit);
	trd.join();
	wstring rc(s.str());
	BOOST_CHECK(rc==wstring(L"\t\tThis is a test with number 800\n\t\tSome more tests.\n\t\tThis from inside the thread900\n"));
}

BOOST_AUTO_TEST_CASE(simple_stderr_log)
{
	using namespace logger;
	ostream_logsink s(std::cerr);
	events evt("Testkomponente", "Topic", logger::enabled, s);
	evt && "Zeile 1";
	evt << "Zeile 2";
}

template <class _Elem>
void simple_memory_sink_test(const _Elem *prefix1, const _Elem *prefix2)
{
	using namespace logger;
	basic_memory_logsink<_Elem, 1024> sink;
	const size_t first_count=200;
	const size_t second_count=900;
	const size_t max_length=377;
	{
		basic_events<_Elem> evt(sink);
		evt << prefix1 << 1;
		evt << prefix1 << 2;
		for (size_t i=5; i<first_count; ++i)
			evt << prefix1 << i;
		for (size_t i=0; i<second_count; ++i) {
			basic_ostringstream<_Elem> o;
			for (size_t j=0; j<i%max_length; ++j) {
				o << (j%10);
			}
			evt << prefix2 << o.str();
		}
	}
	{
		basic_memory_logsink<_Elem, 1024>::const_iterator i=sink.begin();
		size_t count=1;
		while (i!=sink.end() && count<first_count) {
			const basic_memory_logsink<_Elem, 1024>::entry &current(*i);
			basic_ostringstream<_Elem> str;
			str << prefix1 << count;
			BOOST_CHECK(str.str()==current.str());
			++count;
			if (count==3)
				count=5;
			++i;
		}
		count=0;
		while (i!=sink.end() && count<second_count) {
			basic_ostringstream<_Elem> o;
			o << prefix2;
			for (size_t j=0; j<count%max_length; ++j) {
				o << (j%10);
			}
			basic_string<_Elem> rc(i->str());
			BOOST_CHECK(rc==o.str());
			++i;
			++count;
		}
		BOOST_CHECK_EQUAL(count, second_count);
		BOOST_CHECK(i==sink.end());
	}
}

BOOST_AUTO_TEST_CASE(simple_memory_sink)
{
	simple_memory_sink_test("Event ", "long: ");
	simple_memory_sink_test(L"Eventl ", L"longl: ");
}