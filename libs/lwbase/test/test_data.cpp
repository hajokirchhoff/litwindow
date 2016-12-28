#include "precompiled.h"
#include <litwindow/dataadapter.h>
#include "test_data.hpp"
#include <sstream>
using namespace std;
using litwindow::tstring;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

test_data_class_simple::test_data_class_simple(int _i, float _f, E1 _e)
:i1(_i),f1(_f),e1(_e)
{
	_tcscpy(c_str_100, _T("ThisIsA100Str"));
}

BEGIN_ADAPTER(test_data_class_simple)
PROP(i1)
PROP(f1)
PROP(e1)
PROP_CSTR(c_str_100)
END_ADAPTER()

IMPLEMENT_ADAPTER_TYPE(test_data_class_simple::E1)    // Prepare enumeration E1 for use with data adapters.

#ifdef NOT

#ifndef DOXYGEN_INVOKED
template <>
tstring litwindow::converter<test_data_class_simple::E1>::to_string(const test_data_class_simple::E1 &e)
{
	tstringstream o;
	o << e;
	return o.str();
}
#endif


IMPLEMENT_ADAPTER_CONTAINER(vector<tstring>)
IMPLEMENT_ADAPTER_CONTAINER(list<int>)

BEGIN_ADAPTER(FixWithAggregateMembers)
PROP(aString)
PROP(anAggregateMember)
PROP(anInt)
END_ADAPTER()

BEGIN_ADAPTER(WithGetterSetter)
PROP(_i)
PROP_getset(int, TestIntAccess)
PROP_GetSet(int, AnotherInt)
PROP_get_set_(FixWithAggregateMembers, aFixWithAggregateMembers)
PROP_get_set_(FixWithAggregateMembers, anotherFix)
PROP_getset(FixWithAggregateMembers, ThirdFix)
END_ADAPTER()

#if 0
template <> 
tstring litwindow::converter<vector<int> >::to_string(const schema_entry *entry, const_prop_ptr member_ptr) 
{ 
	return "vector"; 
} 

template <> 
litwindow::prop_t ADAPTER_DLL_EXPORT litwindow::prop_type_object<vector<int> >::get(const vector<int> *t) 
{ 
	static container_converter<vector<int> > theConverter("vector<int>"); 
	return &theConverter; 
}

template <>
const litwindow::schema_entry ADAPTER_DLL_EXPORT & litwindow::prop_schema_entry<vector<int> >::get(const vector<int> *)
{
	static schema_entry e(0, get_prop_type<vector<int> >(), "this");
	return e;
}
#endif

#if 0
const ::litwindow::schema_base &::litwindow::schema<vector<int> >::get_schema() 
{ 
	static schema_base *schema_ptr=_init_schema();
	return *schema_ptr; 
}

#endif

BEGIN_ADAPTER(BooleanVector)
PROP(m_string_vector)
//    PROP_get_set_(vector<int>, fourBools)
END_ADAPTER()

using namespace litwindow;
/*
template <>
tstring converter<vector<int> >::to_string(const vector<int> &b) 
{
ostringstream out;
out << b.size() << ":[";
vector<int>::const_iterator i;
for (i=b.begin(); i!=b.end(); ++i) {
out << (*i) ? '1' : '0';
}
out << "]";
return out.str();
}
*/
vector<bool> BooleanVector::get_fourBools() const
{
	vector<bool> rc;
	size_t i;
	for (i=0; i<m_fourBools.size(); ++i) {
		rc.push_back(m_fourBools[i]);
	}
	return rc;
}

void BooleanVector::set_fourBools(const vector<bool> &v)
{
	size_t i;
	for (i=0; i<m_fourBools.size() && i<v.size(); ++i) {
		m_fourBools[i]=v[i];
	}
}

#endif
