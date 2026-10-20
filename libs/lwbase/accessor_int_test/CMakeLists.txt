# Regression test: calling accessor::set<int> from a consumer of lwbase.
# With a shared lwbase on MSVC, this used to fail with LNK2019
# (unresolved __declspec(dllimport) accessor::assign_value(int const&)).
cmake_minimum_required(VERSION 3.10)

add_executable(lwbase_accessor_int_test accessor_int_test.cpp)
target_link_libraries(lwbase_accessor_int_test PRIVATE lwbase)
set_property(TARGET lwbase_accessor_int_test PROPERTY CXX_STANDARD 20)

if (BUILD_SHARED_LIBS)
	add_custom_command(TARGET lwbase_accessor_int_test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"$<TARGET_FILE:lwbase>"
			"$<TARGET_FILE_DIR:lwbase_accessor_int_test>"
	)
endif()

add_test(NAME lwbase_accessor_int_test COMMAND lwbase_accessor_int_test)
