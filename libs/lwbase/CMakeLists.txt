# CMakeList.txt : CMake project for litwindow, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("lwbase")

# Option to build shared or static (defaults to STATIC)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# Find Boost (format and bind are header-only)
find_package(Boost REQUIRED)

# Gather all .cpp files in this directory. Also glob logger/*.cpp explicitly: file(GLOB)
# is non-recursive and would otherwise silently skip logger/default_sink.cpp, whose
# explicit dllexport template instantiations are required for shared (DLL) builds.
file(GLOB SOURCES "*.cpp" "logger/*.cpp")

# Add library target (type controlled by BUILD_SHARED_LIBS)
add_library(lwbase ${SOURCES})

# Add compile definitions (only for shared builds)
target_compile_definitions(lwbase 
    PUBLIC
        # On Windows, wchar_t (16-bit) matches the platform's native wide-char
        # ABI, so build in Unicode mode. On Linux, wchar_t is 32-bit (UCS-4)
        # and does not match ODBC's 16-bit SQLWCHAR, so build narrow (char)
        # to avoid silent string corruption and MSVC-only wchar_t code paths.
        $<$<BOOL:${WIN32}>:UNICODE>
        $<$<BOOL:${WIN32}>:_UNICODE>
        # CMake's target_link_libraries already links lwbase explicitly, so
        # suppress litwindow's boost-style auto-link #pragma comment(lib, ...)
        # which otherwise looks for a lib named after the (VS-project-only)
        # auto_link naming convention and fails to link (LNK1104).
        LWL_LWBASE_NO_LIB
        # Enables __declspec(dllexport)/dllimport via litwindow's LWBASE_API/
        # LWBASE_DLL_EXPORT macros (see lw_pragma_import.h). Without this, those
        # macros expand to nothing on MSVC shared builds and explicitly
        # instantiated templates (e.g. get_prop_type_data_adapter_mechanism<T>)
        # are compiled into lwbase.dll/odbc.dll but never added to their export
        # table, causing LNK2019 unresolved externals in consumers. No effect
        # on non-MSVC compilers (see lw_pragma_import.h).
        $<$<BOOL:${BUILD_SHARED_LIBS}>:LITWINDOW_ALL_DYN_LINK>
        # LWBASE_DYN_LINK (enabled above via LITWINDOW_ALL_DYN_LINK) requires
        # BOOST_ALL_DYN_LINK on MSVC (see lw_pragma_import.h's #error check), and
        # since LITWINDOW_ALL_DYN_LINK is now PUBLIC and propagates to consumers,
        # this must be PUBLIC too so their compiles satisfy that check as well.
        $<$<BOOL:${BUILD_SHARED_LIBS}>:BOOST_ALL_DYN_LINK>
    PRIVATE 
        $<$<BOOL:${BUILD_SHARED_LIBS}>:LWBASE_EXPORTS>
)

# Link Boost headers
target_link_libraries(lwbase PUBLIC Boost::boost)

# Add include directories
target_include_directories(lwbase PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../..")
# logger/default_sink.cpp includes "stdafx.h" via a relative (same-directory-first)
# include, which otherwise only resolves for sources directly in this directory.
target_include_directories(lwbase PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET lwbase PROPERTY CXX_STANDARD 20)
endif()

# Add unit tests (optional, controlled by BUILD_TESTING)
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(unittest)
endif()
