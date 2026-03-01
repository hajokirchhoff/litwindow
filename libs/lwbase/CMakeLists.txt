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

# Gather all .cpp files in this directory
file(GLOB SOURCES "*.cpp")

# Add library target (type controlled by BUILD_SHARED_LIBS)
add_library(lwbase ${SOURCES})

# Add compile definitions (only for shared builds)
target_compile_definitions(lwbase 
    PRIVATE 
        $<$<BOOL:${BUILD_SHARED_LIBS}>:LWBASE_EXPORTS>
        $<$<BOOL:${BUILD_SHARED_LIBS}>:BOOST_ALL_DYN_LINK>
)

# Link Boost headers
target_link_libraries(lwbase PUBLIC Boost::boost)

# Add include directories
target_include_directories(lwbase PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../..")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET lwbase PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
