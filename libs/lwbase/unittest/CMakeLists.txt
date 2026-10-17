# CMakeLists.txt for lwbase unit tests
#
cmake_minimum_required(VERSION 3.10)

# Find Boost Test framework
find_package(Boost REQUIRED COMPONENTS unit_test_framework thread serialization)

# Gather all test source files
file(GLOB TEST_SOURCES "*.cpp")

# Create test executable
add_executable(lwbase_tests ${TEST_SOURCES})

# Link against lwbase and Boost.Test
# All test files (including lwbase_unittest.cpp) use the standard, non-included
# <boost/test/unit_test.hpp> header, so Boost::unit_test_framework must be linked to
# supply the implementation (and, via BOOST_TEST_MODULE in lwbase_unittest.cpp, the
# generated main()).
target_link_libraries(lwbase_tests 
    PRIVATE 
        lwbase 
        Boost::unit_test_framework
        Boost::thread
        Boost::serialization
)

# Note: no manual BOOST_TEST_DYN_LINK compile definition is needed here. vcpkg does
# not vary the Boost triplet based on this project's own BUILD_SHARED_LIBS setting,
# so the installed Boost::unit_test_framework's actual (static vs. dynamic) linkage
# can differ from this project's own linkage choice. Boost's own headers already
# detect this correctly via BOOST_ALL_DYN_LINK (set automatically by vcpkg's Boost
# packages when Boost itself is built as a DLL) and enable BOOST_TEST_DYN_LINK
# accordingly - tying it to BUILD_SHARED_LIBS instead caused a mismatch that broke
# the "static" presets.

# Set C++ standard
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET lwbase_tests PROPERTY CXX_STANDARD 20)
endif()

# When building lwbase as a shared library (BUILD_SHARED_LIBS=ON), lwbase.dll is not
# automatically copied next to lwbase_tests.exe (unlike the vcpkg-managed Boost DLLs,
# which vcpkg's applocal deployment already handles). Without this, running the test
# executable fails to find lwbase.dll at load time.
if (BUILD_SHARED_LIBS)
    add_custom_command(TARGET lwbase_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:lwbase>"
            "$<TARGET_FILE_DIR:lwbase_tests>"
    )
endif()

# Register tests with CTest
add_test(NAME lwbase_tests COMMAND lwbase_tests)

# Optional: Set test working directory
set_tests_properties(lwbase_tests PROPERTIES 
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)