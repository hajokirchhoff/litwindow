# Minimal standalone project that consumes litwindow purely as a vcpkg
# package (via find_package(litwindow)), completely separate from
# litwindow's own build. Used to verify the litwindow vcpkg port/features
# actually work for a downstream consumer. See the top-level README's
# "Testing vcpkg consumption" section for how to configure/build this.
cmake_minimum_required(VERSION 3.10)
project(litwindow_consumer_example CXX)

find_package(litwindow REQUIRED)

add_executable(litwindow_consumer_example main.cpp)
target_link_libraries(litwindow_consumer_example PRIVATE litwindow::lwbase)

if(litwindow_ODBC_FOUND)
	target_link_libraries(litwindow_consumer_example PRIVATE litwindow::odbc)
	target_compile_definitions(litwindow_consumer_example PRIVATE LITWINDOW_CONSUMER_HAS_ODBC)
endif()
