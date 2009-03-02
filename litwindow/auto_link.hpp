// boost/auto_link unfortunately includes boost/version.hpp, which sets BOOST_LIB_VERSION
// which is then coded into the auto_link information

#include <boost/version.hpp>
// replace BOOST_LIB_VERSION with our own LWL_LIB_VERSION_STRING
// but remember the original setting so it can be restored later on
#define BOOST_LIB_VERSION_ORG BOOST_LIB_VERSION
#undef BOOST_LIB_VERSION

#define BOOST_LIB_VERSION LWL_LIB_VERSION_STRING

#include <boost/config/auto_link.hpp>

// restore the original version
#undef BOOST_LIB_VERSION
#define BOOST_LIB_VERSION BOOST_LIB_VERSION_ORG
#undef BOOST_VERSION_HPP
#undef BOOST_LIB_VERSION

#undef LWL_LIB_VERSION_STRING