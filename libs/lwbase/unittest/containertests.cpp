/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: containertests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <litwindow/dataadapter.h>
#include "fixtures.h"
#include <list>
#include <map>
using std::list;
using std::map;

typedef std::map<litwindow::tstring, int> the_map;

LWL_BEGIN_AGGREGATE_NO_COPY(the_map::value_type)
    PROP(first)
    PROP(second)
LWL_END_AGGREGATE()

IMPLEMENT_ADAPTER_CONTAINER(the_map)

using namespace litwindow;

template <class Container>
class ContainerTestTemplate:public CppUnit::TestFixture
{
public:
    Container *thecontainer;
    accessor a;
    const_accessor ca;
    typename Container::value_type v1, v2, v3;
    ContainerTestTemplate(typename Container::value_type _v1, typename Container::value_type _v2, typename Container::value_type _v3)
        :v1(_v1), v2(_v2), v3(_v3)
    {}
    void setUp()
    {
        thecontainer=new Container;
        a=make_accessor(*thecontainer);
        ca=make_const_accessor(*thecontainer);
    }
    void tearDown()
    {
        delete thecontainer;
    }

    int get_count(const const_container &c) const
    {
        int count=0;
        const_container::iterator i;
        for (i=c.begin(); i!=c.end(); ++i, ++count)
            ;
        return count;
    }

    void insertThreeObjects()
    {
        CPPUNIT_ASSERT_EQUAL(true, a.is_container());
        CPPUNIT_ASSERT_EQUAL(true, ca.is_container());
        const_container cc=ca.get_container();
        container c=a.get_container();
        int count=get_count(cc);
        CPPUNIT_ASSERT_EQUAL(count, get_count(c));
        container::iterator i=c.begin();
        CPPUNIT_ASSERT(i==c.begin());
        CPPUNIT_ASSERT_EQUAL(true, c.insert(i, make_accessor(v1)));

        CPPUNIT_ASSERT_EQUAL(count+1, get_count(c));
        CPPUNIT_ASSERT_EQUAL(count+1, get_count(cc));
        CPPUNIT_ASSERT(i!=c.begin());
        CPPUNIT_ASSERT(++c.begin()==i);

        CPPUNIT_ASSERT_EQUAL(true, c.insert(c.end(), make_accessor(v2)));
        CPPUNIT_ASSERT_EQUAL(count+2, get_count(c));
        CPPUNIT_ASSERT_EQUAL(count+2, get_count(cc));

        CPPUNIT_ASSERT_EQUAL(true, c.insert(c.end(), make_accessor(v3)));
        CPPUNIT_ASSERT_EQUAL(count+3, get_count(c));
        CPPUNIT_ASSERT_EQUAL(count+3, get_count(cc));

        i=c.begin();
        ++i;
        CPPUNIT_ASSERT_EQUAL(true, c.erase(i));
        CPPUNIT_ASSERT_EQUAL(count+2, get_count(c));
        CPPUNIT_ASSERT(++c.begin()==i);
    }
};

class ContainerTestTemplateString:public ContainerTestTemplate<vector<tstring> >
{
public:
    ContainerTestTemplateString():ContainerTestTemplate<vector<tstring> >(tstring(_T("_a")), tstring(_T("_b")), tstring(_T("_c"))) {}
    CPPUNIT_TEST_SUITE(ContainerTestTemplateString);
        CPPUNIT_TEST(insertThreeObjects);
    CPPUNIT_TEST_SUITE_END();
};

class ContainerTestTemplateInt:public ContainerTestTemplate<list<int> >
{
public:
    ContainerTestTemplateInt():ContainerTestTemplate<list<int> >(-8989, INT_MAX, INT_MIN) {}
    CPPUNIT_TEST_SUITE(ContainerTestTemplateInt);
        CPPUNIT_TEST(insertThreeObjects);
    CPPUNIT_TEST_SUITE_END();
};

class ContainerTestTemplateMapStringInt:public ContainerTestTemplate<the_map >
{
public:
    ContainerTestTemplateMapStringInt():ContainerTestTemplate<the_map >(make_pair(_T("one"), 1), make_pair(_T("two"), 2), make_pair(_T("three"), 3)) {}
    CPPUNIT_TEST_SUITE(ContainerTestTemplateMapStringInt);
        CPPUNIT_TEST(insertThreeObjects);
    CPPUNIT_TEST_SUITE_END();
};

class ContainerTests:public CppUnit::TestFixture
{
public:

    void simpleContainerTest_const()
    {
        const_accessor a=make_accessor(*bv);
        CPPUNIT_ASSERT_EQUAL(true, a.is_aggregate());
        const_aggregate ag(a.get_aggregate());
        const_accessor boolVector=ag["m_string_vector"];
        CPPUNIT_ASSERT_EQUAL(true, boolVector.is_container());
        const_container c(boolVector.get_container());
        const_container::iterator i;
        size_t count=0;
        for (i=c.begin(); i!=c.end(); ++i, ++count) {
            switch (count) {
                case 0:
                    CPPUNIT_ASSERT_EQUAL(tstring(_T("one")), (*i).to_string());
                    break;
                case 1:
                    CPPUNIT_ASSERT_EQUAL(tstring(_T("two")), (*i).to_string());
                    break;
                case 2:
                    CPPUNIT_ASSERT_EQUAL(tstring(_T("three")), i->to_string());
                    break;
            }
        }
        CPPUNIT_ASSERT_EQUAL(3, int(count));
#if 0
        const_accessor fourBools=ag["fourBools"];
        CPPUNIT_ASSERT_EQUAL(string("4:[0000]"), fourBools.to_string());
#endif

        //const_container c=a.get_container();
    }

    void simpleContainerTest()
    {
        accessor a=make_accessor(*bv);
        CPPUNIT_ASSERT_EQUAL(true, a.is_aggregate());
        aggregate ag(a.get_aggregate());
        accessor boolVector=ag["m_string_vector"];
        CPPUNIT_ASSERT_EQUAL(true, boolVector.is_container());
        container c(boolVector.get_container());
        container::iterator i;
        size_t count=0;
        for (i=c.begin(); i!=c.end(); ++i, ++count) {
            switch (count) {
                case 0:
                    CPPUNIT_ASSERT_EQUAL(tstring(_T("one")), (*i).to_string());
                    break;
                case 1:
                    CPPUNIT_ASSERT_EQUAL(tstring(_T("two")), (*i).to_string());
                    break;
                case 2:
                    CPPUNIT_ASSERT_EQUAL(tstring(_T("three")), i->to_string());
                    break;
            }
        }
        CPPUNIT_ASSERT_EQUAL(3, int(count));
#if 0
        accessor fourBools=ag["fourBools"];
        CPPUNIT_ASSERT_EQUAL(string("4:[0000]"), fourBools.to_string());
#endif

        //container c=a.get_container();
    }

    CPPUNIT_TEST_SUITE(ContainerTests);
        CPPUNIT_TEST(simpleContainerTest);
		CPPUNIT_TEST(simpleContainerTest_const);
    CPPUNIT_TEST_SUITE_END();

public:
    BooleanVector *bv;
    void setUp()
    {
        bv=new BooleanVector;
        bv->m_string_vector.push_back(_T("one"));
        bv->m_string_vector.push_back(_T("two"));
        bv->m_string_vector.push_back(_T("three"));
    }
    void tearDown()
    {
        delete bv;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ContainerTests);

CPPUNIT_TEST_SUITE_REGISTRATION(ContainerTestTemplateString);
CPPUNIT_TEST_SUITE_REGISTRATION(ContainerTestTemplateInt);
CPPUNIT_TEST_SUITE_REGISTRATION(ContainerTestTemplateMapStringInt);
