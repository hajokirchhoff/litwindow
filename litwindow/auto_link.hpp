#ifdef BOOST_VERSION_HPP
#define BOOST_LIB_VERSION_ORG BOOST_LIB_VERSION
#undef BOOST_LIB_VERSION
#else
#define BOOST_VERSION_HPP
#endif
#define BOOST_LIB_VERSION LWL_LIB_VERSION_STRING
#include <boost/config/auto_link.hpp>
#undef BOOST_LIB_VERSION
#ifdef BOOST_LIB_VERSION_ORG
#define BOOST_LIB_VERSION BOOST_LIB_VERSION_ORG
#else
#undef BOOST_VERSION_HPP
#endif
#undef LWL_LIB_VERSION_STRING
