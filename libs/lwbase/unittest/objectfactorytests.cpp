/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: objectfactorytests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <boost/test/unit_test.hpp>
#include <litwindow/dataadapter.h>
#include "fixtures.h"

using namespace litwindow;

#define new DEBUG_NEW

struct ObjectFactoryTestFixture
{
    int anInt;
    accessor anIntAccessor;

    ObjectFactoryTestFixture()
    {
        anInt = 789;
        anIntAccessor = make_accessor(anInt);
    }

    ~ObjectFactoryTestFixture()
    {
    }
};

BOOST_FIXTURE_TEST_SUITE(ObjectFactoryTests, ObjectFactoryTestFixture)

BOOST_AUTO_TEST_CASE(testCreateNewObject)
{
    accessor anIntAccessor = make_accessor(anInt);
    accessor a = create_object(anIntAccessor.get_type());
    BOOST_TEST(a.is_int() == true);
    BOOST_TEST(a.is_aggregate() == false);
    BOOST_TEST(a.is_container() == false);
    a.from_int(anIntAccessor.to_int());
    BOOST_TEST(a.to_int() == 789);
    destroy_object(a);
}

BOOST_AUTO_TEST_CASE(testCloneObjects)
{
    accessor a = anIntAccessor.clone();
    BOOST_TEST(a.to_int() == 789);
    a.from_int(59);
    BOOST_TEST(a.to_int() == 59);
    BOOST_TEST(anIntAccessor.to_int() == 789);
    a.destroy();
}

BOOST_AUTO_TEST_SUITE_END()
