/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: objectfactorytests.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include <litwindow/dataadapter.h>
#include "fixtures.h"

using namespace litwindow;

#define new DEBUG_NEW

class ObjectFactoryTests:public CppUnit::TestFixture 
{
    void testCreateNewObject()
    {
        accessor anIntAccessor=make_accessor(anInt);
        accessor a=create_object(anIntAccessor.get_type());
        CPPUNIT_ASSERT_EQUAL(true, a.is_int());
        CPPUNIT_ASSERT_EQUAL(false, a.is_aggregate());
        CPPUNIT_ASSERT_EQUAL(false, a.is_container());
        a.from_int(anIntAccessor.to_int());
        CPPUNIT_ASSERT_EQUAL(789, a.to_int());
        destroy_object(a);
    }
    void testCloneObjects()
    {
        accessor a=anIntAccessor.clone();
        CPPUNIT_ASSERT_EQUAL(789, a.to_int());
        a.from_int(59);
        CPPUNIT_ASSERT_EQUAL(59, a.to_int());
        CPPUNIT_ASSERT_EQUAL(789, anIntAccessor.to_int());
        a.destroy();
    }
public:
    CPPUNIT_TEST_SUITE(ObjectFactoryTests);
        CPPUNIT_TEST(testCreateNewObject);
        CPPUNIT_TEST(testCloneObjects);
    CPPUNIT_TEST_SUITE_END();

public:
    int anInt;
    accessor anIntAccessor;

    void setUp()
    {
        anInt=789;
        anIntAccessor=make_accessor(anInt);
    }
    void tearDown()
    {
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ObjectFactoryTests);
