/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: fixtures2.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdafx.h"
#include "fixtures.h"
using namespace std;

BEGIN_ADAPTER(simpleInheritance)
    PROP_I(WithGetterSetter)
    PROP(siString)
END_ADAPTER()

BEGIN_ADAPTER(simpleInheritanceTwoLevels)
    PROP_I(simpleInheritance)
    PROP(siTwo)
END_ADAPTER()

const int &GetExternalElement(const ExternalAccessorTest &e)
{
    return e.m_externalAccessorElement1;
}

void SetExternalElement(ExternalAccessorTest &e, const int &newValue)
{
    e.m_externalAccessorElement1=newValue;
}

BEGIN_ADAPTER(ExternalAccessorTest)
    PROP_I(Fix1)
    PROP(m_externalAccessorElement1)
    PROP_RW_EXT(int, GetExternalElement, SetExternalElement, "m_element2")
END_ADAPTER()

using litwindow::tstring;
class TheCoObject
{
public:
    TheCoObject()
        :m_coobjectstring(_T("This is the coobject!"))
    {}
    CoObjectTest *m_originalObject;
    tstring m_coobjectstring;
    tstring GetOriginal() const
    {
        litwindow::const_accessor a=litwindow::make_const_accessor(*m_originalObject);
        return litwindow::accessor_as_debug(a);
    }
    void SetOriginal(const tstring &)
    {
        throw "cannot set original object";
    }
};

BEGIN_ADAPTER(TheCoObject)
    PROP(m_coobjectstring)
    PROP_GetSet(tstring, Original)
END_ADAPTER()

TheCoObject *GetCoObjectForCoObjectTest(CoObjectTest *o)
{
    static TheCoObject t;
    if (t.m_originalObject==0)
        t.m_originalObject=o;
    return &t;
}

BEGIN_ADAPTER(CoObjectTest)
    PROP_I(Fix1)
    PROP(m_someString)
    PROP_CO(TheCoObject, GetCoObjectForCoObjectTest)
END_ADAPTER()