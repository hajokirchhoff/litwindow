/** \file
	This file contains test cases that also serve as examples.
	*/
#include "precompiled.h"
#include <litwindow/dataadapter.h>
#include "test_data.hpp"

#include <boost/test/auto_unit_test.hpp>

/** Sample @p struct showing how a user can provide a dataadapter to a library.
	*/
struct sample_struct
{
	int							member1;
	litwindow::tstring	member2;
	sample_struct()
		:member1(150967)
		,member2(_T("sample_struct-member2"))
	{}
};

/** Define the dataadapter */
LWL_BEGIN_AGGREGATE(sample_struct)
	PROP(member1)
	PROP(member2)
LWL_END_AGGREGATE()

template<>
void boost::test_tools::tt_detail::print_log_value<litwindow::tstring>::operator ()(std::ostream &ostr, const litwindow::tstring &t)
{
	ostr << litwindow::t2string(t);
}
/** Example showing how to create a dataadapter. */
BOOST_AUTO_UNIT_TEST(example_providing_dataadapter)
{
	sample_struct example;
	// create a dataadapter for 'sample_struct'
	litwindow::accessor a=litwindow::make_accessor(example);

	// dump the contents to a tstring
	litwindow::tstring result=litwindow::accessor_as_debug(a);
	BOOST_CHECK_EQUAL(result, litwindow::tstring(_T("sample_struct{member1=150967; member2=sample_struct-member2}")));

}
