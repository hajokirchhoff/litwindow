#ifndef _TEST_DATA_H_060318LWL
#define _TEST_DATA_H_060318LWL

#include <litwindow/tstring.hpp>

class test_data_class_simple
{
public:
    int i1;
    float f1;
    enum E1 {
        a, b, c
    } e1;
    //DECLARE_ADAPTER(test_data_class_simple)

    test_data_class_simple(int _i=100, float _f=100.1F, E1 _e=a);
	TCHAR c_str_100[100];
};

//DECLARE_ADAPTER_TYPE(test_data_class_simple::E1, /*UNITTESTDLL_API*/)

#ifdef _UNICODE
inline std::ostream &operator<<(std::ostream &o, litwindow::tstring const &str)
{
	o << litwindow::t2string(str);
	return o;
}
#endif


#ifdef NOT
class FixWithAggregateMembers
{
public:
    litwindow::tstring  aString;
    test_data_class_simple        anAggregateMember;
    int         anInt;

    FixWithAggregateMembers()
        :aString(_T("thisIsAStringValue"))
        ,anAggregateMember(3, 4.234F, test_data_class_simple::b)
        ,anInt(77)
    { }
};

class WithGetterSetter
{
public:
    int _i;
    int _another_int;
    const int &getTestIntAccess() const
    {
        return _i;
    }
    void setTestIntAccess(const int &i)
    {
        _i=i;
    }

    int GetAnotherInt() const
    {
        return _another_int;
    }
    int SetAnotherInt(int i)    // and again a different signature
    {
        return _another_int=i;
    }
    FixWithAggregateMembers aFixWithAggregateMembers;
    const FixWithAggregateMembers &get_aFixWithAggregateMembers() const
    {
        return aFixWithAggregateMembers;
    }
    void    set_aFixWithAggregateMembers(const FixWithAggregateMembers &aFix)
    {
        aFixWithAggregateMembers=aFix;
    }
    FixWithAggregateMembers anotherFix;
    FixWithAggregateMembers get_anotherFix() const
    {
        return anotherFix;
    }
    void  set_anotherFix(const FixWithAggregateMembers &aFix)
    {
        anotherFix=aFix;
    }

    FixWithAggregateMembers thirdFix;
    FixWithAggregateMembers getThirdFix() const
    {
        return thirdFix;
    }
    void setThirdFix(const FixWithAggregateMembers &aFix)
    {
        thirdFix=aFix;
    }
};

class UNITTESTDLL_API BooleanVector
{
public:
#pragma warning(disable: 4251)
    vector<litwindow::tstring> m_string_vector;
    bitset<4> m_fourBools;
    vector<bool> get_fourBools() const;
    void set_fourBools(const vector<bool> &v);

};

//DECLARE_ADAPTER_CONTAINER(vector<bool>, UNITTESTDLL_API)
DECLARE_ADAPTER_CONTAINER(vector<litwindow::tstring>, UNITTESTDLL_API)
DECLARE_ADAPTER_CONTAINER(std::list<int>, UNITTESTDLL_API)

/// Test class for simple inheritance tests.

class simpleInheritance:public WithGetterSetter
{
public:
    litwindow::tstring siString;
    simpleInheritance()
        :siString(_T("simpleInheritanceString"))
    {
        _i=500;
        _another_int=600;
    }
};

class simpleInheritanceTwoLevels:public simpleInheritance
{
public:
    litwindow::tstring siTwo;
    simpleInheritanceTwoLevels()
        :siTwo(_T("siTwo"))
    {
    }
};

#ifdef _UNICODE
template <>
std::string CppUnit::assertion_traits<wstring>::toString(const wstring &o)
{
	return litwindow::t2string(o);
}
#endif


class ExternalAccessorTest:public test_data_class_simple
{
public:
    int m_externalAccessorElement1;
};

class CoObjectTest:public test_data_class_simple
{
public:
    litwindow::tstring  m_someString;
};

#endif

#if defined(_MSC_VER)
#pragma once
#endif

#endif
