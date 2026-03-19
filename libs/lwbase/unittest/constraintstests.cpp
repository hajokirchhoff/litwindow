/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: constraintstests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include <litwindow/dataadapter.h>
#include <litwindow/constraints.h>
#include "fixtures.h"
#include <stdexcept>
#include <iostream>
using ::std::runtime_error;
#include <functional>

using namespace std;
using namespace litwindow;

#ifdef NOT
class ConstraintsParserTests:public CppUnit::TestFixture
{
public:
	CPPUNIT_TEST_SUITE(ConstraintsParserTests);
		CPPUNIT_TEST(simpleParse);
	CPPUNIT_TEST_SUITE_END();

	void simpleParse()
	{
		parse_rules(_T("rulename : { a=b; c=d; }"));
	}
};
CPPUNIT_TEST_SUITE_REGISTRATION(ConstraintsParserTests);
#endif

struct ConstraintsTestsFixture
{
	int intValue1;
	int intValue2;
	Fix1 *fixValue1;
	constraint_solver *solver;

	ConstraintsTestsFixture()
	{
		intValue1=901;
		intValue2=902;
		fixValue1=new Fix1;
		fixValue1->i1=903;
		solver=new constraint_solver;
	}

	~ConstraintsTestsFixture()
	{
		delete fixValue1;
		delete solver;
	}
};

BOOST_FIXTURE_TEST_SUITE(ConstraintsTests, ConstraintsTestsFixture)

BOOST_AUTO_TEST_CASE(experimentWithStuff)
{
//        new int;
}

BOOST_AUTO_TEST_CASE(testIfElseExpression)
{
	bool test=false;
	int v1=10;
	int v2=12;
	expression<int> e=if_else(make_expr<bool>(make_const_accessor(test)), make_expr<int>(make_const_accessor(v1)), make_expr<int>(make_const_accessor(v2)) + 50);
	BOOST_TEST(e.evaluate(0) == 62);
	test=true;
	BOOST_TEST(e.evaluate(0) == 10);
	test=false;
	v1=20;
	v2=100;
	BOOST_TEST(e.evaluate(0) == 150);
}

BOOST_AUTO_TEST_CASE(testVariablesExpressions)
{
	bool wasThrown=false;
	struct l:public symbol_table_interface {
		ConstraintsTestsFixture &r;
		map<string, accessor> m_symbols;
		l(ConstraintsTestsFixture &_r)
			:r(_r)
		{
			m_symbols.insert(make_pair(string("two"), make_accessor(r.intValue2)));
			m_symbols.insert(make_pair(string("one"), make_accessor(r.intValue1)));
			m_symbols.insert(make_pair(string("fixValue1"), make_accessor(*r.fixValue1)));
		}
		accessor lookup_variable(const string &name)
		{
			accessor rc=m_symbols.insert(make_pair(name, accessor())).first->second;
			return rc;
		}
	} myL(*this);

	// next test 'not set' and 'invalid type' exception
	expression<int> i=_c(0)+_v("fixValue1");
	wasThrown=false;
	try {
		i.evaluate(&myL);
	}
	catch (std::runtime_error &e) {
		if (string(e.what())!="Variable fixValue1 has type Fix1 but type int expected.")
			throw;
		wasThrown=true;
	}
	BOOST_TEST(wasThrown == true);
	// finally test variable not found
	i=_c(100)+_v("noSuchVariable");
	wasThrown=false;
	try {
		i.evaluate(&myL);
	}
	catch (std::runtime_error &e) {
		if (string(e.what())!="Variable noSuchVariable not found")
			throw;
		wasThrown=true;
	}
	BOOST_TEST(wasThrown == true);
}

BOOST_AUTO_TEST_CASE(testRulesWithExpressions)
{
	accessor i1(make_accessor(intValue1));
	accessor i2(make_accessor(intValue2));
		// i1 = i2+4
	solver->add(make_rule(i1, make_expr<int>(i2)+4));
		// values must remain unchanged by just adding a rule
	BOOST_TEST(intValue1 == 901);
	BOOST_TEST(intValue2 == 902);
	solver->assign_value(i2, _T("10"));
	BOOST_TEST(intValue1 == 901);
	BOOST_TEST(intValue2 == 10);
	solver->solve();
	BOOST_TEST(intValue1 == 14);
	BOOST_TEST(intValue2 == 10);
}

BOOST_AUTO_TEST_CASE(testAccessorExpressions)
{
	accessor one(make_accessor(intValue1));
	const_accessor two(make_accessor(intValue2));
	expression<int> sum=(make_expr<int>(one)+two)*two+15;
	BOOST_TEST(sum.evaluate(0) == (intValue1+intValue2)*intValue2+15);
	// Test the dependency mechanism.

	BOOST_TEST(sum.is_dependent_on(make_const_accessor(intValue1), 0) == static_dependency);
	BOOST_TEST(sum.is_dependent_on(make_const_accessor(intValue2), 0) == static_dependency);
	int notDependent;
	BOOST_TEST(sum.is_dependent_on(make_const_accessor(notDependent), 0) == no_dependency);
	// Test lazy evaluation. 
	// Change intValue2. The sum object is unchanged but evaluate should reflect the new value.
	intValue2=-14;
	BOOST_TEST(sum.evaluate(0) == (intValue1+intValue2)*intValue2+15);

	bool aBool=false;
	expression<bool> aBoolExpr=!make_const<bool>(aBool);
	BOOST_TEST(aBoolExpr.evaluate(0) == true);
}

BOOST_AUTO_TEST_CASE(testExpressions)
{
	// test operators
	BOOST_TEST(make_expression(make_const(0)+8).evaluate(0) == 0+8);
	BOOST_TEST(make_expression(make_const(7.0f)-3.0f).evaluate(0) == 7.0f-3.0f);
	BOOST_TEST(make_expression(make_const(15)* -78).evaluate(0) == 15*-78);
	BOOST_TEST(make_expression(make_const(800.0) / 254.0).evaluate(0) == 800.0/254.0);
	BOOST_TEST(make_expression(make_const(1) == 2).evaluate(0) == bool(1==2));
	BOOST_TEST(make_expression(make_const(55l) > 77l).evaluate(0) == bool(55l>77l));
	BOOST_TEST(make_expression(make_const(1234.02) >= 1234.02).evaluate(0) == bool(1234.02>=1234.02));
	BOOST_TEST(make_expression(make_const(1234) >= 1233).evaluate(0) == bool(1234>=1233));
	BOOST_TEST(make_expression(make_const(1234) >= 1235).evaluate(0) == bool(1234>=1235));
	BOOST_TEST(make_expression(make_const(1234) <= 1234).evaluate(0) == bool(1234<=1234));
	BOOST_TEST(make_expression(make_const(1234) <= 1233).evaluate(0) == bool(1234<=1233));
	BOOST_TEST(make_expression(make_const(1234) <= 1235).evaluate(0) == bool(1234<=1235));
	BOOST_TEST(make_expression(make_const(99.01) < 99.01).evaluate(0) == bool(99.01 < 99.01));
	BOOST_TEST(make_expression(make_const(99.01) < 100.01).evaluate(0) == bool(99.01 < 100.01));

	BOOST_TEST(make_expression(make_const(true) || false).evaluate(0) == bool(true || false));
	BOOST_TEST(make_expression(make_const(true) && false).evaluate(0) == bool(true && false));
	BOOST_TEST(make_expression(make_const(true) && (make_const(1) == 2)).evaluate(0) == bool(true && (1==2)));
	BOOST_TEST(make_expression(make_const(true) && (make_const(1)==2) || true).evaluate(0) == bool(true && (1==2) || true));

	BOOST_TEST(make_expression(make_const(string("abcd"))!=string("defg")).evaluate(0) == (string("abcd")!=string("defg")));


	// test expression copy constructor and assignment
	expression<bool> test2=make_const(15) < 20;
	expression<bool> test3(test2);
	BOOST_TEST(test2.evaluate(0) == true);
	BOOST_TEST(test3.evaluate(0) == true);
	test2= ( make_const(100) > 500 );
	BOOST_TEST(test2.evaluate(0) == false);
}

BOOST_AUTO_TEST_CASE(addSimpleRules)
{
	// add the following rules
	//  intValue1 = intValue2
	//  intValue2 = fixValue1->i1
	(*solver)   << ( target(intValue1) = intValue2 )
				<< ( target(intValue2) = fixValue1->i1 );
	//solver->add_rule(new rule_assign(make_accessor(intValue1), make_accessor(intValue2)));
	//solver->add_rule(new rule_assign(make_accessor(intValue2), make_accessor(fixValue1->i1)));

	// set initial values
	intValue1=901;
	intValue2=902;
	fixValue1->i1=903;

	// test assign without propagation
	solver->assign_value(new value_assign_string(make_accessor(intValue1), _T("15")));
	solver->solve();
	BOOST_TEST(intValue1 == 15); BOOST_TEST(intValue2 == 902); BOOST_TEST(fixValue1->i1 == 903);

	// test assign with one propagation
	solver->assign_value(new value_assign_string(make_accessor(intValue2), _T("-789")));
	solver->solve();
	BOOST_TEST(intValue1 == -789); BOOST_TEST(intValue2 == -789); BOOST_TEST(fixValue1->i1 == 903);

	// set initial values
	intValue1=901;
	intValue2=902;
	fixValue1->i1=903;

	// test assign with two propagations
	solver->assign_value(new value_assign_string(make_accessor(fixValue1->i1), _T("88442200")));
	solver->solve();
	solver->solve();
	BOOST_TEST(intValue1 == 88442200); BOOST_TEST(intValue2 == 88442200); BOOST_TEST(fixValue1->i1 == 88442200);
}

BOOST_AUTO_TEST_CASE(testCircularRules)
{
	(*solver)   << (target(intValue1) = intValue2)
				<< (target(intValue2) = fixValue1->i1)
				<< (target(fixValue1->i1) = intValue1);

	BOOST_TEST(intValue1 == 901); BOOST_TEST(intValue2 == 902); BOOST_TEST(fixValue1->i1 == 903);
	solver->assign_value(make_accessor(intValue2), _T("781"));
	solver->solve();
	BOOST_TEST(intValue1 == 781); BOOST_TEST(intValue2 == 781); BOOST_TEST(fixValue1->i1 == 781);
	solver->assign_value(make_accessor(fixValue1->i1), _T("1415"));
	solver->solve();
	BOOST_TEST(intValue1 == 1415); BOOST_TEST(intValue2 == 1415); BOOST_TEST(fixValue1->i1 == 1415);
	// set initial values
	intValue1=901;
	intValue2=902;
	fixValue1->i1=903;
	solver->assign_value(make_accessor(intValue1), _T("-882134"));
	solver->solve();
	BOOST_TEST(intValue1 == -882134); BOOST_TEST(intValue2 == -882134); BOOST_TEST(fixValue1->i1 == -882134);
}

BOOST_AUTO_TEST_CASE(testNestedChanged)
{
	BOOST_TEST((make_accessor(fixValue1->i1) != make_accessor(*fixValue1)));
	BOOST_TEST((make_aggregate(*fixValue1)["i1"] == make_accessor(fixValue1->i1)));
	(*solver)   << (target(intValue1) = fixValue1->i1);
	BOOST_TEST(intValue1 == 901);
	fixValue1->i1=10;
	solver->mark_value_changed(make_accessor(*fixValue1), true);
	solver->solve();
	BOOST_TEST(intValue1 == 10);
}

BOOST_AUTO_TEST_CASE(testConflictException)
{
	(*solver)   << ( target(intValue1)=intValue2 )
				<< ( target(intValue1)=fixValue1->i1);

	// test empty solver. nothing should happen.
	solver->solve();

	// assign a value to 'intValue2'
	solver->assign_value(make_accessor(intValue2), _T("1615"));
	solver->solve();
	BOOST_TEST(intValue1 == 1615); BOOST_TEST(intValue2 == 1615); BOOST_TEST(fixValue1->i1 == 903);

	// assign similar values
	solver->assign_value(make_accessor(intValue2), _T("1234"));
	solver->assign_value(make_accessor(fixValue1->i1), _T("1234"));
	// should work.
	solver->solve();
	BOOST_TEST(intValue1 == 1234); BOOST_TEST(intValue2 == 1234); BOOST_TEST(fixValue1->i1 == 1234);

	// set initial values
	intValue1=901;
	intValue2=902;
	fixValue1->i1=903;
	// assign conflicting values
	solver->assign_value(make_accessor(intValue2), _T("4321"));
	solver->assign_value(make_accessor(fixValue1->i1), _T("-1234"));
	// enable_undo should be on by default
	BOOST_TEST(solver->is_undo() == true);
	// should throw an exception -> conflicting values
	try {
		solver->solve();
	}
	catch (constraint_solver::rules_conflict &e) {
		/* exception occurred. good. */
		string msg=e.what();
	}
	// and the original values should be left unchanged!!!
	BOOST_TEST(intValue1 == 901); BOOST_TEST(intValue2 == 902); BOOST_TEST(fixValue1->i1 == 903);

	// the solver will still be in 'solving' state. need to reset it.
	solver->reset();

	// now test conflict without undo
	// set initial values
	intValue1=901;
	intValue2=902;
	fixValue1->i1=903;
	// set enable_undo to false
	solver->enable_undo(false);
	// assign conflicting values
	solver->assign_value(make_accessor(intValue2), _T("4321"));
	solver->assign_value(make_accessor(fixValue1->i1), _T("-1234"));
	// should throw an exception -> conflicting values
	try {
		solver->solve();
	}
	catch (constraint_solver::rules_conflict &e) {
		/* exception occurred. good. */
		string msg=e.what();
	}
	// And the values should have been partially changed. They will reflect the state _before_ the conflicting assignment.
	BOOST_TEST(intValue1 == 4321); 
	BOOST_TEST(intValue2 == 4321); 
	BOOST_TEST(fixValue1->i1 == -1234);
}

BOOST_AUTO_TEST_SUITE_END()
