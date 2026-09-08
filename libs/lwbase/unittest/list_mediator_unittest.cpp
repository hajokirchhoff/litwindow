#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include <boost/bind/bind.hpp>
#include "litwindow/ui/list_mediator.hpp"

#define new DEBUG_NEW

using namespace litwindow;
using namespace std;
using namespace boost;
using namespace litwindow::ui;


struct TestData
{
	int integer;
	tstring name;
	bool calc() const { return 5.9*integer>integer; }
	void calc(const double &d) { integer=(int)ceil(d); }
	int calc2() const { return 2*integer;}
	TestData()
		:integer(7),name(_T("name")){}
};

tstring TestDataAccess(const TestData &a, int col)
{
	return a.name+tstring(_T("-TestDataAccess(int col)"));
}
tstring TestDataAccess(const TestData &a)
{
	return a.name+tstring(_T("-TestDataAccess"));
}

template <typename RowValue, typename Accessor, typename Enabled = void>
struct column_access
{
	typedef boost::function<void(const RowValue&, tstring&)> text_renderer_t;
};

template <typename RowValue, typename ColValue>
struct column_access<RowValue, ColValue (RowValue::*)() const>
{
	typedef boost::function<void(const RowValue&, tstring&)> text_renderer_t;
	typedef ColValue (RowValue::*accessor_type)() const;
	void text(text_renderer_t &renderer, accessor_type a)
	{
		renderer=boost::bind(a, boost::placeholders::_1);
	}
};
template <typename RowValue, typename Accessor>
void testtemplate(Accessor a)
{
	typedef typename column_access<RowValue, Accessor>::text_renderer_t text_renderer_t;
	text_renderer_t t;
	column_access<RowValue, Accessor>().text(t, a);
}
template <typename RowValue, typename ColValue>
void testtemplate(ColValue (RowValue::*a)() const)
{

}

void TestDataAccess_renderer(const TestData &a, tstring &r)
{
	r=a.name;
}

void StringDataFormatter(const tstring &in, tstring &out)
{
	out=in+_T("--");
}

void DataFormatter(int i, tstring &r)
{
	r=lexical_cast<tstring>(i*3);
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
	void operator()(const TestData& d, tstring& rc) {}
	std::string m_name;
};

void fmt_float(float f, tstring &rc)
{
	rc = _T("test");
}

BOOST_AUTO_TEST_CASE(columns_descriptor_test_new)
{

	using float_functor_accessor = functor_accessor<float>;
	using bca_t = basic_columns_adapter<basic_column_descriptor<TestData> >;
	bca_t cols;
	cols.columns().emplace_back(_T("end"), 10);
	cols.columns().emplace_back(_T("integer"), -1, &TestData::integer);

	using Func = int(*)(const TestData&);

	if (boost::is_void <std::invoke_result<Func, const TestData&>>::value) {

	}
	using Func2 = functor_accessor<float>;

	boost::function<void(const TestData&, tstring&)> fn = functor_object("hallo");

	using call_type = decltype(functor_object("hallo"));

	basic_column_descriptor<TestData> test_functor(_T("testfunctor"), 30, functor_object("hallo"));

	boost::function<float(const TestData&)> __fnc = float_functor_accessor("Hi", 4.f);

	using testdatacolumn = basic_column_descriptor<TestData>;

	testdatacolumn::text_renderer_type tr = functor_object("Test");

	boost::function<float(const TestData&)> accfnc = float_functor_accessor("test", 3.f);

	testdatacolumn test_float_functor(_T("testfunctor"), 30, float_functor_accessor("test", 3.f));

	testdatacolumn fmt_float_test(_T("float_functor"), 20, float_functor_accessor("test", 3.0f), &fmt_float );
	testdatacolumn free_fnc(_T("freefnc"), 20, boost::bind(&TestDataAccess, boost::placeholders::_1, 9));

	cols.columns() = {
		{ _T("float_functor"), 20, float_functor_accessor("test", 3.0f) },
		{_T("float_functor"), 20, float_functor_accessor("test", 3.0f), &fmt_float},
	};

	testdatacolumn t3(_T("calc"), 20, &TestData::calc);

	cols.columns() =
	{
	{_T("integer"), -1, &TestData::integer},
	{_T("calc"), 20, &TestData::calc},
	{_T("calc2"), 22, &TestData::calc2},
	{_T("functor"), 30, functor_object("hallo")},
	{_T("freeFunction"), 40, &TestDataAccess},
	{_T("bind"), 99, boost::bind(&TestDataAccess, boost::placeholders::_1, 9)},
	{_T("free-renderer"), 50, &TestDataAccess_renderer}
	};
}

BOOST_AUTO_TEST_CASE(columns_descriptor_test)
{
	//te(bind(&TestDataAccess_renderer, boost::placeholders::_1, boost::placeholders::_2));
	//te(&TestDataAccess);
	using bca_t = basic_columns_adapter<basic_column_descriptor<TestData> >;
	bca_t d;
	d.add(_T("integer"), 10, &TestData::integer);	// ptr to member
	d.add(_T("calc"), 20, &TestData::calc);			// ptr to member_function with overload
	d.add(_T("calc2"), 100, &TestData::calc2);	// ptr to member without overload
	d.add(_T("TestDataAccess"),	200,	&TestDataAccess)	// ptr to free function
		;
	d.add
		(_T("bind"),		100,	boost::bind(&TestDataAccess, boost::placeholders::_1, 9)) // bind
		(_T("name"),		100,	&TestData::name)	// ptr to member
		;
	bca_t::text_renderer_type renderer;
	renderer=boost::bind(&TestDataAccess_renderer, boost::placeholders::_1, boost::placeholders::_2);
	d.add
		(_T("function-renderer"), 10,	renderer)
		(_T("free-renderer"), 10,	&TestDataAccess_renderer)
		(_T("bind-renderer"), 10, boost::bind(&TestDataAccess_renderer, boost::placeholders::_1, boost::placeholders::_2))
		;

	TestData t;
	tstring r;
	d.render_element_at(0, r, t);
	BOOST_CHECK(r == _T("7"));
	d.render_element_at(1, r, t);
	BOOST_CHECK(r == _T("1"));
	d.render_element_at(2, r, t);
	BOOST_CHECK(r == _T("14"));
	d.render_element_at(3, r, t);
	BOOST_CHECK(r == _T("name-TestDataAccess"));

	d.render_element_at(_T("calc2"), r, t);
	BOOST_CHECK(r == _T("14"));

	d.render_element_at(4, r, t);
	BOOST_CHECK(r == _T("name-TestDataAccess(int col)"));
	d.render_element_at(5, r, t);
	BOOST_CHECK(r == _T("name"));

	r.clear();
	d.render_element_at(_T("name"), r, t);	// should be column 6
	BOOST_CHECK(r == _T("name"));
	r.clear();
	d.render_element_at(_T("function-renderer"), r, t);
	BOOST_CHECK(r == _T("name"));
	r.clear();
	d.render_element_at(_T("free-renderer"), r, t);
	BOOST_CHECK(r == _T("name"));
	r.clear();
	d.render_element_at(_T("bind-renderer"), r, t);
	BOOST_CHECK(r == _T("name"));

	d.add
		(_T("formatter-renderer"), 10, &TestData::integer, &DataFormatter)
		(_T("formatter-renderer-2"), 10, &TestData::calc2, &DataFormatter)
		(_T("bind-formatter-renderer"), 10, boost::bind(&TestDataAccess, boost::placeholders::_1, 9), &StringDataFormatter)
		(_T("free-function-formatter"), 10, &TestDataAccess, &StringDataFormatter)
		;
	r.clear();
	d.render_element_at(_T("formatter-renderer"), r, t);
	BOOST_CHECK(r == _T("21"));
	r.clear();
	d.render_element_at(_T("formatter-renderer-2"), r, t);
	BOOST_CHECK(r == _T("42"));
	r.clear();
	d.render_element_at(_T("bind-formatter-renderer"), r, t);
	BOOST_CHECK(r == _T("name-TestDataAccess(int col)--"));
	r.clear();
	d.render_element_at(_T("free-function-formatter"), r, t);
	BOOST_CHECK(r == _T("name-TestDataAccess--"));
}

BOOST_AUTO_TEST_CASE(column_values_test)
{
	TestData t;
	BOOST_CHECK_EQUAL(t.integer, 7);
	boost::function<int(const TestData&)> f0=boost::bind(&TestData::integer, boost::placeholders::_1);
	boost::function<bool(const TestData&)> f1=boost::bind(&TestData::calc, boost::placeholders::_1);
	testtemplate<TestData>(&TestData::calc);
	boost::function<tstring(const TestData&)> f2=boost::bind(&TestDataAccess, boost::placeholders::_1);
	boost::function<tstring(const TestData&)> f3=boost::bind(f2, boost::placeholders::_1);
	BOOST_CHECK_EQUAL(f0(t), 7);
	BOOST_CHECK_EQUAL(f1(t), true);
	BOOST_CHECK(f2(t)==tstring(_T("name-TestDataAccess")));
	BOOST_CHECK(f3(t)==tstring(_T("name-TestDataAccess")));

	boost::function<int(const TestData&)> f4=boost::bind(&TestData::calc2, boost::placeholders::_1);
	boost::function<bool(const TestData&, const TestData&)> c0=boost::bind(f0, t) < boost::bind(f4, t);
	BOOST_CHECK(c0(t, t));
	tstring rc;
	to_string(f0(t), rc);
	BOOST_CHECK(rc==_T("7"));
	boost::function<void(const TestData&, tstring&)> strg=boost::bind<void>(&to_string<int>, boost::bind(f0, boost::placeholders::_1), boost::placeholders::_2);
}

