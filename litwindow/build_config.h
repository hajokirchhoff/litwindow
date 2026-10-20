#ifndef _LITWINDOW_BUILD_CONFIG_H
#define _LITWINDOW_BUILD_CONFIG_H

// Default (in-tree) build configuration: intentionally empty.
//
// Builds inside the litwindow source tree (Visual Studio projects, bjam, and
// the CMake targets, which define what they need as compile definitions) take
// all their settings from the project files, and keep using the boost-style
// auto link naming.
//
// `cmake --install` (and therefore the vcpkg port) replaces this file in the
// install tree with a version generated from cmake/build_config.h.in, which
// describes how the installed libraries were built. See there.

#endif