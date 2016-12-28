/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: constraintstests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <litwindow/dataadapter.h>
#include <litwindow/constraints.h>
#include "fixtures.h"
#include <stdexcept>
#include <iostream>
using ::std::runtime_error;
#include <functional>

#define new DEBUG_NEW

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

class ConstraintsTests:public CppUnit::TestFixture 
{
public:
    void experimentWithStuff()
    {
//        new int;
    }

    void testIfElseExpression()
    {
        bool test=false;
        int v1=10;
        int v2=12;
        expression<int> e=if_else(make_expr<bool>(make_const_accessor(test)), make_expr<int>(make_const_accessor(v1)), make_expr<int>(make_const_accessor(v2)) + 50);
        CPPUNIT_ASSERT_EQUAL(62, e.evaluate(0));
        test=true;
        CPPUNIT_ASSERT_EQUAL(10, e.evaluate(0));
        test=false;
        v1=20;
        v2=100;
        CPPUNIT_ASSERT_EQUAL(150, e.evaluate(0));
    }

	void testVariablesExpressions()
	{
		bool wasThrown=false;
		struct l:public symbol_table_interface {
			ConstraintsTests &r;
            map<string, accessor> m_symbols;
			l(ConstraintsTests &_r)
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
		CPPUNIT_ASSERT_EQUAL(true, wasThrown);
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
		CPPUNIT_ASSERT_EQUAL(true, wasThrown);
	}

    void testRulesWithExpressions()
    {
        accessor i1(make_accessor(intValue1));
        accessor i2(make_accessor(intValue2));
            // i1 = i2+4
        solver->add(make_rule(i1, make_expr<int>(i2)+4));
            // values must remain unchanged by just adding a rule
        CPPUNIT_ASSERT_EQUAL(901, intValue1);
        CPPUNIT_ASSERT_EQUAL(902, intValue2);
        solver->assign_value(i2, _T("10"));
        CPPUNIT_ASSERT_EQUAL(901, intValue1);
        CPPUNIT_ASSERT_EQUAL(10, intValue2);
        solver->solve();
        CPPUNIT_ASSERT_EQUAL(14, intValue1);
        CPPUNIT_ASSERT_EQUAL(10, intValue2);
    }

    void testAccessorExpressions()
    {
        accessor one(make_accessor(intValue1));
        const_accessor two(make_accessor(intValue2));
        expression<int> sum=(make_expr<int>(one)+two)*two+15;
        CPPUNIT_ASSERT_EQUAL((intValue1+intValue2)*intValue2+15, sum.evaluate(0));
        // Test the dependency mechanism.

        CPPUNIT_ASSERT_EQUAL(static_dependency, sum.is_dependent_on(make_const_accessor(intValue1), 0));
        CPPUNIT_ASSERT_EQUAL(static_dependency, sum.is_dependent_on(make_const_accessor(intValue2), 0));
        int notDependent;
        CPPUNIT_ASSERT_EQUAL(no_dependency, sum.is_dependent_on(make_const_accessor(notDependent), 0));
        // Test lazy evaluation. 
        // Change intValue2. The sum object is unchanged but evaluate should reflect the new value.
        intValue2=-14;
        CPPUNIT_ASSERT_EQUAL((intValue1+intValue2)*intValue2+15, sum.evaluate(0));

        bool aBool=false;
        expression<bool> aBoolExpr=!make_const<bool>(aBool);
        CPPUNIT_ASSERT_EQUAL(true, aBoolExpr.evaluate(0));
    }
    void testExpressions()
    {
        // test operators
        CPPUNIT_ASSERT_EQUAL(0+8, make_expression(make_const(0)+8).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(7.0f-3.0f, make_expression(make_const(7.0f)-3.0f).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(15*-78, make_expression(make_const(15)* -78).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(800.0/254.0, make_expression(make_const(800.0) / 254.0).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(1==2), make_expression(make_const(1) == 2).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(55l>77l), make_expression(make_const(55l) > 77l).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(1234.02>=1234.02), make_expression(make_const(1234.02) >= 1234.02).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(1234>=1233), make_expression(make_const(1234) >= 1233).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(1234>=1235), make_expression(make_const(1234) >= 1235).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(1234<=1234), make_expression(make_const(1234) <= 1234).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(1234<=1233), make_expression(make_const(1234) <= 1233).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(1234<=1235), make_expression(make_const(1234) <= 1235).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(99.01 < 99.01), make_expression(make_const(99.01) < 99.01).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(99.01 < 100.01), make_expression(make_const(99.01) < 100.01).evaluate(0));

        CPPUNIT_ASSERT_EQUAL(bool(true || false), make_expression(make_const(true) || false).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(true && false), make_expression(make_const(true) && false).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(true && (1==2)), make_expression(make_const(true) && (make_const(1) == 2)).evaluate(0));
        CPPUNIT_ASSERT_EQUAL(bool(true && (1==2) || true), make_expression(make_const(true) && (make_const(1)==2) || true).evaluate(0));

        CPPUNIT_ASSERT_EQUAL(string("abcd")!=string("defg"), make_expression(make_const(string("abcd"))!=string("defg")).evaluate(0));


        // test expression copy constructor and assignment
        expression<bool> test2=make_const(15) < 20;
        expression<bool> test3(test2);
        CPPUNIT_ASSERT_EQUAL(true, test2.evaluate(0));
        CPPUNIT_ASSERT_EQUAL(true, test3.evaluate(0));
        test2= ( make_const(100) > 500 );
        CPPUNIT_ASSERT_EQUAL(false, test2.evaluate(0));
    }

    void addSimpleRules()
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
        CPPUNIT_ASSERT_EQUAL(15, intValue1); CPPUNIT_ASSERT_EQUAL(902, intValue2); CPPUNIT_ASSERT_EQUAL(903, fixValue1->i1);

        // test assign with one propagation
        solver->assign_value(new value_assign_string(make_accessor(intValue2), _T("-789")));
        solver->solve();
        CPPUNIT_ASSERT_EQUAL(-789, intValue1); CPPUNIT_ASSERT_EQUAL(-789, intValue2); CPPUNIT_ASSERT_EQUAL(903, fixValue1->i1);

        // set initial values
        intValue1=901;
        intValue2=902;
        fixValue1->i1=903;

        // test assign with two propagations
        solver->assign_value(new value_assign_string(make_accessor(fixValue1->i1), _T("88442200")));
        solver->solve();
        solver->solve();
        CPPUNIT_ASSERT_EQUAL(88442200, intValue1); CPPUNIT_ASSERT_EQUAL(88442200, intValue2); CPPUNIT_ASSERT_EQUAL(88442200, fixValue1->i1);
    }

    void testCircularRules()
    {
        (*solver)   << (target(intValue1) = intValue2)
                    << (target(intValue2) = fixValue1->i1)
                    << (target(fixValue1->i1) = intValue1);

        CPPUNIT_ASSERT_EQUAL(901, intValue1); CPPUNIT_ASSERT_EQUAL(902, intValue2); CPPUNIT_ASSERT_EQUAL(903, fixValue1->i1);
        solver->assign_value(make_accessor(intValue2), _T("781"));
        solver->solve();
        CPPUNIT_ASSERT_EQUAL(781, intValue1); CPPUNIT_ASSERT_EQUAL(781, intValue2); CPPUNIT_ASSERT_EQUAL(781, fixValue1->i1);
        solver->assign_value(make_accessor(fixValue1->i1), _T("1415"));
        solver->solve();
        CPPUNIT_ASSERT_EQUAL(1415, intValue1); CPPUNIT_ASSERT_EQUAL(1415, intValue2); CPPUNIT_ASSERT_EQUAL(1415, fixValue1->i1);
        // set initial values
        intValue1=901;
        intValue2=902;
        fixValue1->i1=903;
        solver->assign_value(make_accessor(intValue1), _T("-882134"));
        solver->solve();
        CPPUNIT_ASSERT_EQUAL(-882134, intValue1); CPPUNIT_ASSERT_EQUAL(-882134, intValue2); CPPUNIT_ASSERT_EQUAL(-882134, fixValue1->i1);
    }

    void testNestedChanged()
    {
        CPPUNIT_ASSERT(make_accessor(fixValue1->i1) != make_accessor(*fixValue1));
        CPPUNIT_ASSERT(make_aggregate(*fixValue1)["i1"] == make_accessor(fixValue1->i1));
        (*solver)   << (target(intValue1) = fixValue1->i1);
        CPPUNIT_ASSERT_EQUAL(901, intValue1);
        fixValue1->i1=10;
        solver->mark_value_changed(make_accessor(*fixValue1), true);
        solver->solve();
        CPPUNIT_ASSERT_EQUAL(10, intValue1);
    }

    void testConflictException()
    {
        (*solver)   << ( target(intValue1)=intValue2 )
                    << ( target(intValue1)=fixValue1->i1);

        // test empty solver. nothing should happen.
        solver->solve();

        // assign a value to 'intValue2'
        solver->assign_value(make_accessor(intValue2), _T("1615"));
        solver->solve();
        CPPUNIT_ASSERT_EQUAL(1615, intValue1); CPPUNIT_ASSERT_EQUAL(1615, intValue2); CPPUNIT_ASSERT_EQUAL(903, fixValue1->i1);

        // assign similar values
        solver->assign_value(make_accessor(intValue2), _T("1234"));
        solver->assign_value(make_accessor(fixValue1->i1), _T("1234"));
        // should work.
        solver->solve();
        CPPUNIT_ASSERT_EQUAL(1234, intValue1); CPPUNIT_ASSERT_EQUAL(1234, intValue2); CPPUNIT_ASSERT_EQUAL(1234, fixValue1->i1);

        // set initial values
        intValue1=901;
        intValue2=902;
        fixValue1->i1=903;
        // assign conflicting values
        solver->assign_value(make_accessor(intValue2), _T("4321"));
        solver->assign_value(make_accessor(fixValue1->i1), _T("-1234"));
        // enable_undo should be on by default
        CPPUNIT_ASSERT_EQUAL(true, solver->is_undo());
        // should throw an exception -> conflicting values
        try {
            solver->solve();
        }
        catch (constraint_solver::rules_conflict &e) {
            /* exception occurred. good. */
			string msg=e.what();
        }
        // and the original values should be left unchanged!!!
        CPPUNIT_ASSERT_EQUAL(901, intValue1); CPPUNIT_ASSERT_EQUAL(902, intValue2); CPPUNIT_ASSERT_EQUAL(903, fixValue1->i1);

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
        CPPUNIT_ASSERT_EQUAL(4321, intValue1); 
        CPPUNIT_ASSERT_EQUAL(4321, intValue2); 
        CPPUNIT_ASSERT_EQUAL(-1234, fixValue1->i1);
    }

    CPPUNIT_TEST_SUITE(ConstraintsTests);
        CPPUNIT_TEST(testExpressions);
        CPPUNIT_TEST(testAccessorExpressions);
        CPPUNIT_TEST(addSimpleRules);
        CPPUNIT_TEST(experimentWithStuff);
        CPPUNIT_TEST(testCircularRules);
        CPPUNIT_TEST(testConflictException);
        CPPUNIT_TEST(testRulesWithExpressions);
		CPPUNIT_TEST(testVariablesExpressions);
        CPPUNIT_TEST(testNestedChanged);
        CPPUNIT_TEST(testIfElseExpression);
    CPPUNIT_TEST_SUITE_END();
public:
    int intValue1;
    int intValue2;
    Fix1 *fixValue1;
    constraint_solver *solver;
public:
    void setUp()
    {
        intValue1=901;
        intValue2=902;
        fixValue1=new Fix1;
        fixValue1->i1=903;
        solver=new constraint_solver;
    }
    void tearDown()
    {
        delete fixValue1;
        delete solver;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConstraintsTests);
