#ifndef _LITWINDOW_CONFIG_HPP
#define _LITWINDOW_CONFIG_HPP

/// Determine the way that property types are returned
/// 1 ... uses RTTI class names to look up property types (prop_t)
/// 0 ... uses template get_prop_type<>() to lookup property types
#define LITWINDOW_USES_RTTI_LOOKUP 0

/// major version number of the lwbase part of the library
#define LWBASE_VERSION_MAJOR 1
/// minor version number of the lwbase part of the library
#define LWBASE_VERSION_MINOR 0
/// build number of the lwbase part of the library
#define LWBASE_VERSION_BUILD 3
#define LWBASE_VERSION ((LWBASE_VERSION_MAJOR << 16) | LWBASE_VERSION_MINOR)
#define LWBASE_LIB_VERSION_STRING "1_0_0"

#include "./platform_toolset.hpp"

#endif
