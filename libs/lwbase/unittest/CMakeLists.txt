# CMakeLists.txt for lwbase unit tests
#
cmake_minimum_required(VERSION 3.10)

# Find Boost Test framework
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

# Gather all test source files
file(GLOB TEST_SOURCES "*.cpp")

# Create test executable
add_executable(lwbase_tests ${TEST_SOURCES})

# Link against lwbase and Boost.Test
target_link_libraries(lwbase_tests 
    PRIVATE 
        lwbase 
        Boost::unit_test_framework
)

# Add compile definitions
target_compile_definitions(lwbase_tests 
    PRIVATE 
        $<$<BOOL:${BUILD_SHARED_LIBS}>:BOOST_TEST_DYN_LINK>
)

# Set C++ standard
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET lwbase_tests PROPERTY CXX_STANDARD 20)
endif()

# Register tests with CTest
add_test(NAME lwbase_tests COMMAND lwbase_tests)

# Optional: Set test working directory
set_tests_properties(lwbase_tests PROPERTIES 
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)