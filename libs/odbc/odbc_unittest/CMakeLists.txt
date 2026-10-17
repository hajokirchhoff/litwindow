# CMakeLists.txt for odbc unit tests
#
cmake_minimum_required(VERSION 3.10)

# Find Boost Test framework
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

# Gather all test source files
file(GLOB TEST_SOURCES "*.cpp")

# stdafx.cpp only exists to generate the (MSVC-only) precompiled header, not needed here
list(REMOVE_ITEM TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp")

# Create test executable
add_executable(odbc_tests ${TEST_SOURCES})

# Link against odbc and Boost.Test
target_link_libraries(odbc_tests
	PRIVATE
		odbc
		Boost::unit_test_framework
)

# Set C++ standard
if (CMAKE_VERSION VERSION_GREATER 3.12)
	set_property(TARGET odbc_tests PROPERTY CXX_STANDARD 20)
endif()

# When building odbc/lwbase as shared libraries (BUILD_SHARED_LIBS=ON), the DLLs are not
# automatically copied next to odbc_tests.exe. Without this, running the test executable
# fails to find odbc.dll / lwbase.dll at load time.
if (BUILD_SHARED_LIBS)
	add_custom_command(TARGET odbc_tests POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"$<TARGET_FILE:odbc>"
			"$<TARGET_FILE_DIR:odbc_tests>"
	)
	add_custom_command(TARGET odbc_tests POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"$<TARGET_FILE:lwbase>"
			"$<TARGET_FILE_DIR:odbc_tests>"
	)
endif()

# Register tests with CTest
add_test(NAME odbc_tests COMMAND odbc_tests)

# Optional: Set test working directory
set_tests_properties(odbc_tests PROPERTIES
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
