/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: containertests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <boost/test/unit_test.hpp>
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
struct ContainerTestTemplate
{
    Container *thecontainer;
    accessor a;
    const_accessor ca;
    typename Container::value_type v1, v2, v3;
    
    ContainerTestTemplate(typename Container::value_type _v1, typename Container::value_type _v2, typename Container::value_type _v3)
        :v1(_v1), v2(_v2), v3(_v3)
    {
        thecontainer = new Container;
        a = make_accessor(*thecontainer);
        ca = make_const_accessor(*thecontainer);
    }
    
    ~ContainerTestTemplate()
    {
        delete thecontainer;
    }

    int get_count(const const_container &c) const
    {
        int count = 0;
        const_container::iterator i;
        for (i = c.begin(); i != c.end(); ++i, ++count)
            ;
        return count;
    }

    void insertThreeObjects()
    {
        BOOST_TEST(a.is_container() == true);
        BOOST_TEST(ca.is_container() == true);
        const_container cc = ca.get_container();
        container c = a.get_container();
        int count = get_count(cc);
        BOOST_TEST(get_count(c) == count);
        container::iterator i = c.begin();
        BOOST_TEST((i == c.begin()));
        BOOST_TEST(c.insert(i, make_accessor(v1)) == true);

        BOOST_TEST(get_count(c) == count + 1);
        BOOST_TEST(get_count(cc) == count + 1);
        BOOST_TEST((i != c.begin()));
        BOOST_TEST((++c.begin() == i));

        BOOST_TEST(c.insert(c.end(), make_accessor(v2)) == true);
        BOOST_TEST(get_count(c) == count + 2);
        BOOST_TEST(get_count(cc) == count + 2);

        BOOST_TEST(c.insert(c.end(), make_accessor(v3)) == true);
        BOOST_TEST(get_count(c) == count + 3);
        BOOST_TEST(get_count(cc) == count + 3);

        i = c.begin();
        ++i;
        BOOST_TEST(c.erase(i) == true);
        BOOST_TEST(get_count(c) == count + 2);
        BOOST_TEST((++c.begin() == i));
    }
};

struct ContainerTestTemplateString : ContainerTestTemplate<vector<tstring>>
{
    ContainerTestTemplateString() : ContainerTestTemplate<vector<tstring>>(tstring(_T("_a")), tstring(_T("_b")), tstring(_T("_c"))) {}
};

BOOST_FIXTURE_TEST_SUITE(ContainerTestTemplateStringTests, ContainerTestTemplateString)

BOOST_AUTO_TEST_CASE(insertThreeObjects)
{
    ContainerTestTemplateString::insertThreeObjects();
}

BOOST_AUTO_TEST_SUITE_END()

struct ContainerTestTemplateInt : ContainerTestTemplate<list<int>>
{
    ContainerTestTemplateInt() : ContainerTestTemplate<list<int>>(-8989, INT_MAX, INT_MIN) {}
};

BOOST_FIXTURE_TEST_SUITE(ContainerTestTemplateIntTests, ContainerTestTemplateInt)

BOOST_AUTO_TEST_CASE(insertThreeObjects)
{
    ContainerTestTemplateInt::insertThreeObjects();
}

BOOST_AUTO_TEST_SUITE_END()

struct ContainerTestTemplateMapStringInt : ContainerTestTemplate<the_map>
{
    ContainerTestTemplateMapStringInt() : ContainerTestTemplate<the_map>(make_pair(_T("one"), 1), make_pair(_T("two"), 2), make_pair(_T("three"), 3)) {}
};

BOOST_FIXTURE_TEST_SUITE(ContainerTestTemplateMapStringIntTests, ContainerTestTemplateMapStringInt)

BOOST_AUTO_TEST_CASE(insertThreeObjects)
{
    ContainerTestTemplateMapStringInt::insertThreeObjects();
}

BOOST_AUTO_TEST_SUITE_END()

struct ContainerTestsFixture
{
    BooleanVector *bv;
    
    ContainerTestsFixture()
    {
        bv = new BooleanVector;
        bv->m_string_vector.push_back(_T("one"));
        bv->m_string_vector.push_back(_T("two"));
        bv->m_string_vector.push_back(_T("three"));
    }
    
    ~ContainerTestsFixture()
    {
        delete bv;
    }
};

BOOST_FIXTURE_TEST_SUITE(ContainerTests, ContainerTestsFixture)

BOOST_AUTO_TEST_CASE(simpleContainerTest_const)
{
    const_accessor a = make_accessor(*bv);
    BOOST_TEST(a.is_aggregate() == true);
    const_aggregate ag(a.get_aggregate());
    const_accessor boolVector = ag["m_string_vector"];
    BOOST_TEST(boolVector.is_container() == true);
    const_container c(boolVector.get_container());
    const_container::iterator i;
    size_t count = 0;
    for (i = c.begin(); i != c.end(); ++i, ++count) {
        switch (count) {
            case 0:
                BOOST_TEST((*i).to_string() == tstring(_T("one")));
                break;
            case 1:
                BOOST_TEST((*i).to_string() == tstring(_T("two")));
                break;
            case 2:
                BOOST_TEST(i->to_string() == tstring(_T("three")));
                break;
        }
    }
    BOOST_TEST(int(count) == 3);
#if 0
    const_accessor fourBools = ag["fourBools"];
    BOOST_TEST(fourBools.to_string() == string("4:[0000]"));
#endif
}

BOOST_AUTO_TEST_CASE(simpleContainerTest)
{
    accessor a = make_accessor(*bv);
    BOOST_TEST(a.is_aggregate() == true);
    aggregate ag(a.get_aggregate());
    accessor boolVector = ag["m_string_vector"];
    BOOST_TEST(boolVector.is_container() == true);
    container c(boolVector.get_container());
    container::iterator i;
    size_t count = 0;
    for (i = c.begin(); i != c.end(); ++i, ++count) {
        switch (count) {
            case 0:
                BOOST_TEST((*i).to_string() == tstring(_T("one")));
                break;
            case 1:
                BOOST_TEST((*i).to_string() == tstring(_T("two")));
                break;
            case 2:
                BOOST_TEST(i->to_string() == tstring(_T("three")));
                break;
        }
    }
    BOOST_TEST(int(count) == 3);
#if 0
    accessor fourBools = ag["fourBools"];
    BOOST_TEST(fourBools.to_string() == string("4:[0000]"));
#endif
}

BOOST_AUTO_TEST_SUITE_END()
