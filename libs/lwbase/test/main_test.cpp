/** \file
	This file contains basic test cases. The test cases also serve as examples on how to use the lit window library.
	*/
#include "precompiled.h"
#include <litwindow/dataadapter.h>
#include "test_data.hpp"

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

/** Create accessor adapters from a simple class.
	This test/example creates an accessor for an object of a simple class without any inheritance or aggregate members.
	It demonstrates the use of various 'is_...' functions to test attributes of the accessor.
	*/
BOOST_AUTO_UNIT_TEST(test_accessors)
{
	// create an object of the test class
	test_data_class_simple test_object;

	// create a const accessor pointing to nothing
	litwindow::const_accessor const_a;

	// the accessor points to nothing, so it is not valid
	BOOST_CHECK(const_a.is_valid()==false);

	// now point the accessor to the object
	const_a=litwindow::make_const_accessor(test_object);

	// and verify attributes
	// the accessor should now be valid
	BOOST_CHECK(const_a.is_valid()==true);
	// should be pointing to an aggregate (a struct or a class)
	BOOST_CHECK(const_a.is_aggregate()==true);
	// but not to a container (such as std::vector or otherwise)
	BOOST_CHECK(const_a.is_container()==false);
	// not to a coobject (lit window library specialty)
	BOOST_CHECK(const_a.is_coobject()==false);
	// nor to a C type vector (such as    char test[256]; )
	BOOST_CHECK(const_a.is_c_vector()==false);
	// and the accessor does not point to the inherited part of a class
	BOOST_CHECK(const_a.is_inherited()==false);
	// nor to a nested aggregate
	BOOST_CHECK(const_a.is_nested()==false);
	// or an integer
	BOOST_CHECK(const_a.is_int()==false);

	// create a non-const accessor pointing to nothing
	litwindow::accessor a;
	BOOST_CHECK(a.is_valid()==false);
	
	// point it to the same test object
	a=litwindow::make_accessor(test_object);
	// the accessor @p a should point to the same object as the @ü const_a accessor. 
	// It is an alias to the const accessor.
	BOOST_CHECK(a.is_alias_of(const_a)==true);
}

#ifdef _MSC_VER
// disable 'unit_test_framework.hpp' warnings
#pragma warning(push, 2)
#endif

#include <boost/test/included/unit_test_framework.hpp>
