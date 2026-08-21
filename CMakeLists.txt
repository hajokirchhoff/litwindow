# CMakeList.txt : CMake project for litwindow, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("litwindow")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set(CMAKE_CXX_STANDARD 20)
endif()

# Enable CTest at the root so Visual Studio's Test Explorer (which discovers
# tests starting from the top-level build directory) can find tests
# registered by subdirectories (e.g. libs/lwbase/unittest).
include(CTest)
enable_testing()

# Include subprojects
add_subdirectory("libs/lwbase")

# TODO: Add tests and install targets if needed.
