# CMakeList.txt : CMake project for litwindow, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("litwindow" VERSION 1.5.0 LANGUAGES CXX)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set(CMAKE_CXX_STANDARD 20)
endif()

# Enable CTest at the root so Visual Studio's Test Explorer (which discovers
# tests starting from the top-level build directory) can find tests
# registered by subdirectories (e.g. libs/lwbase/unittest).
include(CTest)
enable_testing()

# Needed for CMAKE_INSTALL_<dir> variables (used by every subdirectory's
# install(TARGETS ...) below) and for generating/installing the litwindow
# package config used by find_package(litwindow) / the vcpkg port.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Optional components. odbc is on by default (matches historical behaviour);
# lwwx (wxWidgets UI integration) is opt-in since it pulls in wxWidgets, a
# large dependency that most consumers of litwindow do not need.
option(LITWINDOW_BUILD_ODBC "Build the litwindow ODBC data-access library" ON)
option(LITWINDOW_BUILD_LWWX "Build the litwindow wxWidgets UI library (lwwx)" OFF)

# Include subprojects
add_subdirectory("libs/lwbase")
if(LITWINDOW_BUILD_ODBC)
	add_subdirectory("libs/odbc")
endif()
if(LITWINDOW_BUILD_LWWX)
	add_subdirectory("libs/lwwx")
endif()

# Install the litwindow/*.hpp headers shared by every library above (each
# target's INSTALL_INTERFACE include dir points at ${CMAKE_INSTALL_INCLUDEDIR},
# i.e. exactly where this lands).
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/litwindow"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
	FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hpp"
)

# Generate and install the litwindow package config, so consumers (including
# the vcpkg port under vcpkg-overlay/ports/litwindow) can do
# find_package(litwindow REQUIRED) and link litwindow::lwbase / ::odbc / ::lwwx.
# configure_package_config_file() only substitutes plain variables (no
# generator expressions), so resolve the booleans to a literal TRUE/FALSE now.
if(LITWINDOW_BUILD_ODBC)
	set(LITWINDOW_BUILD_ODBC TRUE)
else()
	set(LITWINDOW_BUILD_ODBC FALSE)
endif()
if(LITWINDOW_BUILD_LWWX)
	set(LITWINDOW_BUILD_LWWX TRUE)
else()
	set(LITWINDOW_BUILD_LWWX FALSE)
endif()

configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/litwindowConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/litwindowConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/litwindow"
)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/litwindowConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(FILES
		"${CMAKE_CURRENT_BINARY_DIR}/litwindowConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/litwindowConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/litwindow"
)
install(EXPORT litwindowTargets
	FILE litwindowTargets.cmake
	NAMESPACE litwindow::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/litwindow"
)
