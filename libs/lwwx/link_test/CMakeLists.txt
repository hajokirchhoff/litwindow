# Link test: an executable that consumes lwwx must link without relying on the
# header-embedded #pragma comment(lib, ...) auto-link names (which don't match
# the CMake-built library names).
cmake_minimum_required(VERSION 3.10)

add_executable(lwwx_link_test link_test.cpp)
target_link_libraries(lwwx_link_test PRIVATE lwwx)
set_property(TARGET lwwx_link_test PROPERTY CXX_STANDARD 20)

# Copy lwbase/lwwx and the wxWidgets DLLs next to the executable (shared builds).
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET lwwx_link_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:lwwx_link_test>
            $<TARGET_FILE_DIR:lwwx_link_test>
        COMMAND_EXPAND_LISTS
    )
endif()

add_test(NAME lwwx_link_test COMMAND lwwx_link_test)
