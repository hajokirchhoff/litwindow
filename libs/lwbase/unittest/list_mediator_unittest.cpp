#include "stdafx.h"
#include <boost/test/auto_unit_test.hpp>
#include <boost/bind.hpp>
#include "litwindow/ui/list_mediator.hpp"

#define new DEBUG_NEW

using namespace litwindow;
using namespace std;
using namespace boost;
using namespace litwindow::ui;


struct TestData
{
	int integer;
	wstring name;
	bool calc() const { return 5.9*integer>integer; }
	void calc(const double &d) { integer=(int)ceil(d); }
	int calc2() const { return 2*integer;}
	TestData()
		:integer(7),name(L"name"){}
};

wstring TestDataAccess(const TestData &a, int col)
{
	return a.name+wstring(L"-TestDataAccess(int col)");
}
wstring TestDataAccess(const TestData &a)
{
	return a.name+wstring(L"-TestDataAccess");
}

template <typename RowValue, typename Accessor, typename Enabled = void>
struct column_access
{
	typedef boost::function<void(const RowValue&, wstring&)> text_renderer_t;
};

template <typename RowValue, typename ColValue>
struct column_access<RowValue, ColValue (RowValue::*)() const>
{
	typedef boost::function<void(const RowValue&, wstring&)> text_renderer_t;
	typedef ColValue (RowValue::*accessor_type)() const;
	void text(text_renderer_t &renderer, accessor_type a)
	{
		renderer=boost::bind(a, _1);
	}
};
template <typename RowValue, typename Accessor>
void testtemplate(Accessor a)
{
	typedef column_access<RowValue, Accessor>::text_renderer_t text_renderer_t;
	text_renderer_t t;
	column_access<RowValue>().text(t, a);
}
template <typename RowValue, typename ColValue>
void testtemplate(ColValue (RowValue::*a)() const)
{

}

void TestDataAccess_renderer(const TestData &a, wstring &r)
{
	r=a.name;
}

void StringDataFormatter(const wstring &in, wstring &out)
{
	out=in+L"--";
}

void DataFormatter(int i, wstring &r)
{
	r=lexical_cast<wstring>(i*3);
}

template <typename Value>
struct functor_accessor
{
	functor_accessor(const char* name, const Value &v) :m_name(name), m_v(v) {}
	Value operator()(const TestData &d) { return m_v; }
	std::string m_name;
	Value m_v;
};

struct functor_object
{
	functor_object(const char* name) :m_name(name) {}
	void operator()(const TestData& d, wstring& rc) {}
	std::string m_name;
};

void fmt_float(float f, wstring &rc)
{
	rc = L"test";
}

BOOST_AUTO_TEST_CASE(columns_descriptor_test_new)
{

	using float_functor_accessor = functor_accessor<float>;
	using bca_t = basic_columns_adapter<basic_column_descriptor<TestData> >;
	bca_t cols;
	cols.columns().emplace_back(L"end", 10);
	cols.columns().emplace_back(L"integer", -1, &TestData::integer);

	using Func = int(*)(const TestData&);

	if (boost::is_void <std::result_of<Func(const TestData&)>>::value) {

	}
	using Func2 = functor_accessor<float>;

	boost::function<void(const TestData&, tstring&)> fn = functor_object("hallo");

	using call_type = decltype(functor_object("hallo"));

	basic_column_descriptor<TestData> test_functor(L"testfunctor", 30, functor_object("hallo"));

	boost::function<float(const TestData&)> __fnc = float_functor_accessor("Hi", 4.f);

	using testdatacolumn = basic_column_descriptor<TestData>;

	testdatacolumn::text_renderer_type tr = functor_object("Test");

	boost::function<float(const TestData&)> accfnc = float_functor_accessor("test", 3.f);

	testdatacolumn test_float_functor(L"testfunctor", 30, float_functor_accessor("test", 3.f));

	testdatacolumn fmt_float_test(L"float_functor", 20, float_functor_accessor("test", 3.0f), &fmt_float );
	testdatacolumn free_fnc(L"freefnc", 20, boost::bind(&TestDataAccess, _1, 9));

	cols.columns() = {
		{ L"float_functor", 20, float_functor_accessor("test", 3.0f) },
		{L"float_functor", 20, float_functor_accessor("test", 3.0f), &fmt_float},
	};

	testdatacolumn t3(L"calc", 20, &TestData::calc);

	cols.columns() =
	{
	{L"integer", -1, &TestData::integer},
	{L"calc", 20, &TestData::calc},
	{L"calc2", 22, &TestData::calc2},
	{L"functor", 30, functor_object("hallo")},
	{L"freeFunction", 40, &TestDataAccess},
	{L"bind", 99, boost::bind(&TestDataAccess, _1, 9)},
	{L"free-renderer", 50, &TestDataAccess_renderer}
	};
}

BOOST_AUTO_TEST_CASE(columns_descriptor_test)
{
	//te(bind(&TestDataAccess_renderer, _1, _2));
	//te(&TestDataAccess);
	using bca_t = basic_columns_adapter<basic_column_descriptor<TestData> >;
	bca_t d;
	d.add(L"integer", 10, &TestData::integer);	// ptr to member
	d.add(L"calc", 20, &TestData::calc);			// ptr to member_function with overload
	d.add(L"calc2", 100, &TestData::calc2);	// ptr to member without overload
	d.add(L"TestDataAccess",	200,	&TestDataAccess)	// ptr to free function
		;
	d.add
		(L"bind",		100,	boost::bind(&TestDataAccess, _1, 9)) // bind
		(L"name",		100,	&TestData::name)	// ptr to member
		;
	bca_t::text_renderer_type renderer;
	renderer=boost::bind(&TestDataAccess_renderer, _1, _2);
	d.add
		(L"function-renderer", 10,	renderer)
		(L"free-renderer", 10,	&TestDataAccess_renderer)
		(L"bind-renderer", 10, boost::bind(&TestDataAccess_renderer, _1, _2))
		;

	TestData t;
	wstring r;
	d.render_element_at(0, r, t);
	BOOST_CHECK(r == L"7");
	d.render_element_at(1, r, t);
	BOOST_CHECK(r == L"1");
	d.render_element_at(2, r, t);
	BOOST_CHECK(r == L"14");
	d.render_element_at(3, r, t);
	BOOST_CHECK(r == L"name-TestDataAccess");

	d.render_element_at(L"calc2", r, t);
	BOOST_CHECK(r == L"14");

	d.render_element_at(4, r, t);
	BOOST_CHECK(r == L"name-TestDataAccess(int col)");
	d.render_element_at(5, r, t);
	BOOST_CHECK(r == L"name");

	r.clear();
	d.render_element_at(L"name", r, t);	// should be column 6
	BOOST_CHECK(r == L"name");
	r.clear();
	d.render_element_at(L"function-renderer", r, t);
	BOOST_CHECK(r == L"name");
	r.clear();
	d.render_element_at(L"free-renderer", r, t);
	BOOST_CHECK(r == L"name");
	r.clear();
	d.render_element_at(L"bind-renderer", r, t);
	BOOST_CHECK(r == L"name");

	d.add
		(L"formatter-renderer", 10, &TestData::integer, &DataFormatter)
		(L"formatter-renderer-2", 10, &TestData::calc2, &DataFormatter)
		(L"bind-formatter-renderer", 10, boost::bind(&TestDataAccess, _1, 9), &StringDataFormatter)
		(L"free-function-formatter", 10, &TestDataAccess, &StringDataFormatter)
		;
	r.clear();
	d.render_element_at(L"formatter-renderer", r, t);
	BOOST_CHECK(r == L"21");
	r.clear();
	d.render_element_at(L"formatter-renderer-2", r, t);
	BOOST_CHECK(r == L"42");
	r.clear();
	d.render_element_at(L"bind-formatter-renderer", r, t);
	BOOST_CHECK(r == L"name-TestDataAccess(int col)--");
	r.clear();
	d.render_element_at(L"free-function-formatter", r, t);
	BOOST_CHECK(r == L"name-TestDataAccess--");
}

BOOST_AUTO_TEST_CASE(column_values_test)
{
	TestData t;
	BOOST_CHECK_EQUAL(t.integer, 7);
	boost::function<int(const TestData&)> f0=boost::bind(&TestData::integer, _1);
	boost::function<bool(const TestData&)> f1=boost::bind(&TestData::calc, _1);
	testtemplate<TestData>(&TestData::calc);
	boost::function<wstring(const TestData&)> f2=boost::bind(&TestDataAccess, _1);
	boost::function<wstring(const TestData&)> f3=boost::bind(f2, _1);
	BOOST_CHECK_EQUAL(f0(t), 7);
	BOOST_CHECK_EQUAL(f1(t), true);
	BOOST_CHECK(f2(t)==wstring(L"name-TestDataAccess"));
	BOOST_CHECK(f3(t)==wstring(L"name-TestDataAccess"));

	boost::function<int(const TestData&)> f4=boost::bind(&TestData::calc2, _1);
	boost::function<bool(const TestData&, const TestData&)> c0=boost::bind(f0, t) < boost::bind(f4, t);
	BOOST_CHECK(c0(t, t));
	wstring rc;
	to_string(f0(t), rc);
	BOOST_CHECK(rc==L"7");
	boost::function<void(const TestData&, wstring&)> strg=boost::bind<void>(&to_string<int>, boost::bind(f0, _1), _2);
}

