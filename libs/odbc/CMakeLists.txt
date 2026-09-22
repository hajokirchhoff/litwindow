# CMakeLists.txt for the lwodbc (odbc) library
#
cmake_minimum_required (VERSION 3.10)

project ("odbc")

# Option to build shared or static (defaults to STATIC)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# Find Boost (format and bind are header-only)
find_package(Boost REQUIRED)

# Find the ODBC driver manager.
# On Windows this resolves to the odbc32 library from the platform SDK.
# On Linux the system-provided unixODBC is used (package "unixodbc-dev" on
# Debian/Ubuntu, "unixODBC-devel" on Fedora/RHEL). The system driver manager is
# deliberately preferred over a vcpkg-built one: distributions patch unixODBC
# with a multiarch driver search directory (e.g. /usr/lib/x86_64-linux-gnu/odbc)
# and the correct SYSTEM_FILE_PATH for /etc/odbcinst.ini, so drivers installed via
# the system package manager are found automatically. A vanilla vcpkg build has
# neither and would not see system-registered drivers.
find_package(ODBC REQUIRED)

# Gather all .cpp files in this directory
file(GLOB SOURCES "*.cpp")

# stdafx.cpp only exists to generate the (MSVC-only) precompiled header, not needed here
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp")

if (NOT WIN32)
	# dbms_ms_access.cpp / dbms_ms_excel.cpp use Windows-only headers (<odbcinst.h>, <io.h>)
	# to support the MS Access / MS Excel ODBC drivers, which do not exist on non-Windows platforms.
	list(REMOVE_ITEM SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/dbms_ms_access.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/dbms_ms_excel.cpp"
	)
endif()

# Add library target (type controlled by BUILD_SHARED_LIBS)
add_library(odbc ${SOURCES})

# Add compile definitions (only for shared builds)
target_compile_definitions(odbc
	PUBLIC
		# UNICODE/_UNICODE select TCHAR=wchar_t and are only meaningful on Windows,
		# where wchar_t is UTF-16. On Linux, wchar_t is 4 bytes and impractical for
		# ODBC/UTF-8 interop, so TCHAR=char (narrow, UTF-8 encoded) is used instead.
		$<$<BOOL:${WIN32}>:UNICODE>
		$<$<BOOL:${WIN32}>:_UNICODE>
		# CMake's target_link_libraries already links odbc explicitly, so
		# suppress litwindow's boost-style auto-link #pragma comment(lib, ...)
		# which otherwise looks for a lib named after the (VS-project-only)
		# auto_link naming convention and fails to link (LNK1104).
		LITWINDOW_ODBC_NO_LIB
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
		$<$<BOOL:${BUILD_SHARED_LIBS}>:LWODBC_EXPORTS>
)

# Link Boost headers, lwbase and the ODBC driver manager
target_link_libraries(odbc PUBLIC lwbase ODBC::ODBC Boost::boost)

# dbms.cpp calls SQLConfigDataSourceW / SQLInstallerErrorW, which are part of the
# Windows ODBC Installer API (odbccp32.lib), not the core driver manager import lib
# (odbc32.lib). This library only exists on Windows.
if (WIN32)
	target_link_libraries(odbc PUBLIC odbccp32)
	# odbccp32.lib references the legacy CRT symbol _vsnwprintf_s (via StringCchPrintfW),
	# which was removed from the default UCRT import libs. legacy_stdio_definitions.lib
	# re-exports it and must be linked explicitly on modern MSVC toolchains.
	target_link_libraries(odbc PUBLIC legacy_stdio_definitions)
endif()

# Add include directories
target_include_directories(odbc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../..")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET odbc PROPERTY CXX_STANDARD 20)
endif()

# Add unit tests (optional, controlled by BUILD_TESTING)
include(CTest)
if(BUILD_TESTING)
	add_subdirectory(odbc_unittest)
endif()
