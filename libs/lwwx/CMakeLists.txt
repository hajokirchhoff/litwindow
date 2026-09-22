# CMakeLists.txt for the lwwx (wxWidgets UI integration) library
#
# This is an optional component (see LITWINDOW_BUILD_LWWX in the root
# CMakeLists.txt / the "wx" vcpkg feature). It is only added to the build
# when explicitly requested, since it pulls in wxWidgets, a large GUI
# dependency most consumers of litwindow do not need.
#
cmake_minimum_required (VERSION 3.10)

project ("lwwx")

# Option to build shared or static (defaults to STATIC), same knob as the
# other litwindow libraries.
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

find_package(Boost REQUIRED)

# wxWidgets components actually used by the library sources below:
#   base/core  - wx/frame.h, wx/textctrl.h, wx/sizer.h, wx/event.h, ...
#   html       - wx/html/htmlwin.h
#   adv        - wx/calctrl.h, wx/datectrl.h
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base html adv)

# Gather all .cpp files in this directory (library sources only; the
# samples/ subdirectory has its own, separate demo applications and is not
# part of the library).
file(GLOB SOURCES "*.cpp")

# stdwx.cpp only exists to generate the (MSVC-only) precompiled header, not needed here.
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/stdwx.cpp")

# action_objects.cpp is excluded from the build in every configuration of the
# original lwwx.vcxproj (dead/superseded code kept for reference).
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/action_objects.cpp")

add_library(lwwx ${SOURCES})

target_compile_definitions(lwwx
	PUBLIC
		# Tells litwindow/wx/lwwx.h (and wxWidgets itself) that wxWidgets was
		# built/linked as a shared library. Mirrors how the vcpkg triplet
		# decides wxWidgets' own linkage; kept in sync with BUILD_SHARED_LIBS
		# for simplicity since both are normally chosen together.
		$<$<BOOL:${BUILD_SHARED_LIBS}>:WXUSINGDLL>
		# See libs/odbc/CMakeLists.txt for why these two are PUBLIC.
		$<$<BOOL:${BUILD_SHARED_LIBS}>:LITWINDOW_ALL_DYN_LINK>
		$<$<BOOL:${BUILD_SHARED_LIBS}>:BOOST_ALL_DYN_LINK>
	PRIVATE
		$<$<BOOL:${BUILD_SHARED_LIBS}>:LWWX_EXPORTS>
)

target_link_libraries(lwwx PUBLIC lwbase Boost::boost wx::core wx::base wx::html wx::adv)

target_include_directories(lwwx PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../..")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET lwwx PROPERTY CXX_STANDARD 20)
endif()
