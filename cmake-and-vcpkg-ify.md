# CMake- and vcpkg-ifying litwindow

This document records how litwindow grew from a Visual Studio/MSBuild-oriented project into a CMake build that can be configured across Windows, Linux, and macOS, and installed and consumed as a vcpkg package.

The migration did not start by deleting every Visual Studio project file. The repository still contains historical `.sln`/`.vcxproj` files, but the documented build and package path is now CMake. CMake also keeps Visual Studio as a supported IDE: open the repository folder and let Visual Studio configure it as a CMake project.

## Starting point and goals

The original build description was distributed across Visual Studio projects, property sheets, per-project Boost package references, precompiled-header settings, and platform-specific linker configuration. That worked for the Windows/MSVC workflow, but it did not provide a common way to build all libraries and tests on Linux or to consume litwindow from an unrelated CMake project.

The migration aimed to:

- Describe the libraries, dependencies, compile definitions, and tests in CMake rather than rely on Visual Studio project settings.
- Preserve the existing Windows build behavior while making the code and build work with GCC/Clang on non-Windows platforms.
- Make dependencies reproducible through vcpkg manifest mode where vcpkg is appropriate.
- Install the libraries and headers and export namespaced CMake targets for downstream projects.
- Keep the wxWidgets UI integration optional, so users who only need the core library do not have to build a large GUI dependency.

## 1. Establish a CMake build

The first step was a root `CMakeLists.txt` and an initial `lwbase` target. The project declares the language and version in one place, sets C++20, and includes the library subdirectories instead of treating each `.vcxproj` as the build graph.

Each library has its own `CMakeLists.txt`. Targets describe their public include paths, compile definitions, and linked dependencies. In particular, include paths use separate build and install interfaces: source-tree consumers see the repository headers, while installed consumers see the installed `include` directory. This is important both for local development and for a real package.

The build uses `BUILD_SHARED_LIBS` to choose shared or static libraries, and CMake generator expressions to apply definitions only when relevant. Windows-specific definitions such as `UNICODE` are kept conditional rather than leaking into Linux builds. MSVC DLL export definitions are propagated through the library targets so both the library and its consumers agree on the shared-library ABI; the targets also disable litwindow's old auto-link pragmas where CMake already links the libraries explicitly.

## 2. Make the source portable, not just the build files

Creating CMake files exposed assumptions that Visual Studio had previously supplied implicitly. Some source changes were needed so the same targets could compile outside MSVC:

- MSVC precompiled-header translation units are not treated as portable library sources. The corresponding source lists omit files that only exist to generate a precompiled header.
- ODBC sources for Microsoft Access and Excel use Windows-only headers and drivers, so those files are excluded on non-Windows platforms.
- Windows uses its native ODBC manager; Linux uses the system unixODBC installation. The Linux driver manager is intentionally not taken from vcpkg, because the distribution package provides its driver search paths and registration behavior.
- Character handling is selected for the platform. Windows builds use its native UTF-16 `wchar_t` conventions; Linux builds use narrow character strings for ODBC interop rather than assuming Windows-sized wide characters.
- Compiler-specific linkage and DLL annotations are expressed as target properties and definitions instead of being available only through Visual Studio property sheets.

This work turned "CMake can generate a build" into "the code has a supported non-Windows build path." It also surfaced linking details that were previously hidden in project settings, such as Windows ODBC Installer libraries and shared-library symbol exports.

## 3. Add library and test targets

The CMake graph was expanded from the core `lwbase` library to include `odbc` and its tests. The Boost.Test-based tests are built as ordinary CMake executables and registered with CTest, so the same test command works independently of the IDE. Test targets request only the Boost components they use.

`include(CTest)` at the top level makes tests discoverable from the root build directory, including in Visual Studio's Test Explorer. `BUILD_TESTING` controls whether test subdirectories are included; package builds turn tests off.

The `lwwx` library was then brought into the CMake graph. It uses imported wxWidgets targets and excludes source files that were only for MSVC precompiled headers or were already excluded by the old project. Since wxWidgets is a substantial dependency and not needed by every user, the component is opt-in.

The root options now express that intent:

| Component | CMake option | Local build default | vcpkg feature |
|---|---|---:|---|
| Core `lwbase` | Always built | On | Always present |
| ODBC data access | `LITWINDOW_BUILD_ODBC` | On | `odbc` (opt-in for package consumers) |
| wxWidgets UI integration (`lwwx`) | `LITWINDOW_BUILD_LWWX` | Off | `wx` (opt-in) |

The local ODBC default preserves the historical build behavior. The separate vcpkg features let downstream users install only the optional components they need. For example, a vcpkg consumer requests `litwindow[odbc]` or `litwindow[wx]`; enabling `lwwx` in a direct checkout also requires the `wx` manifest feature so vcpkg restores wxWidgets.

## 4. Move dependency management to vcpkg manifests

A root `vcpkg.json` declares the Boost packages needed by litwindow and its tests. The `wx` feature adds wxWidgets. The `odbc` feature deliberately adds no unixODBC package: on Linux that dependency is supplied by the system package manager, and on Windows the SDK supplies the native ODBC library.

With the vcpkg toolchain configured, manifest mode restores these dependencies during CMake configuration. This replaces the old per-Visual-Studio-project Boost package setup for the CMake path. `VCPKG_ROOT` identifies the vcpkg checkout; the CMake presets pass its toolchain file to CMake.

## 5. Make the build installable and consumable

Building a library is not enough for vcpkg: a downstream project needs installed headers, libraries, dependency information, and stable target names. The CMake targets therefore gained install rules and share a `litwindowTargets` export. The root project installs the public `litwindow` headers and generates `litwindowConfig.cmake` plus a same-major-compatible version file.

The installed target names use the `litwindow::` namespace: `litwindow::lwbase`, `litwindow::lwodbc`, and, when included, `litwindow::lwwx`. The package config loads the exported targets and calls `find_dependency()` for Boost and whichever optional dependencies were used to build that package. Consumers can consequently use the normal CMake pattern:

```cmake
find_package(litwindow REQUIRED)
target_link_libraries(my_app PRIVATE litwindow::lwbase)
```

The local development port under `vcpkg-overlay/ports/litwindow/` connects vcpkg features to CMake options using `vcpkg_check_features`, configures and installs the project with the vcpkg CMake helpers, fixes up the package config, and installs the license. It builds directly from this checkout, which is convenient for development and testing but is not yet a registry-ready source recipe. A publishable port should fetch a tagged source revision and pin its archive hash instead.

A separate project under `examples/vcpkg-consumer/` tests the package as an actual consumer would: it has its own manifest, runs `find_package(litwindow)`, and links only exported targets. This catches install/export or dependency propagation mistakes that an in-tree build cannot catch.

## 6. Provide repeatable platform and linkage configurations

`CMakePresets.json` provides distinct build directories for Windows, Linux, and macOS, with Debug and RelWithDebInfo configurations and shared/static variants. The Windows presets use Ninja and MSVC; the Linux/macOS presets use the toolchain and compiler available on those hosts. Separate build directories mean changing a preset does not reuse an incompatible cache from another platform or linkage mode.

The presets are usable from the command line and from Visual Studio's CMake folder workflow. A Windows command-line build can be run from a Developer Command Prompt:

```powershell
cmake --preset windows-debug-shared
cmake --build out/build/windows-debug-shared
ctest --test-dir out/build/windows-debug-shared --output-on-failure
```

A native Linux build follows the same pattern with `linux-debug-shared`. The repository's `build_all_variants.ps1` automates the Windows and Linux preset matrix from Windows, building Windows natively and Linux through WSL when available. macOS presets are available for builds on macOS; the Windows-hosted script does not attempt to build macOS.

## 7. Verify the package boundary

The standalone consumer is the end-to-end check for the vcpkg contract. From PowerShell, configure it with the local overlay port and the vcpkg toolchain, then build it:

```powershell
cmake -S examples/vcpkg-consumer -B examples/vcpkg-consumer/build -G Ninja `
	-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake `
	-DVCPKG_OVERLAY_PORTS=(Resolve-Path vcpkg-overlay/ports)
cmake --build examples/vcpkg-consumer/build
```

This builds litwindow through its port, installs and exports its targets into vcpkg's package tree, and then compiles a separate executable against those targets. Because this development port uses the working tree as its source, vcpkg binary caching can reuse an old package after source edits; clear the consumer build/cache or disable binary caching when validating fresh source changes.

## Result and remaining boundary

The result is one CMake target graph for local builds, tests, installation, and downstream package consumption, with vcpkg manifest mode supplying portable dependencies where appropriate. Windows remains supported, while Linux and macOS have platform-specific presets and the source-level adaptations needed to avoid depending on MSBuild behavior.

The old Visual Studio project files are still present in the repository; this migration made CMake the cross-platform build and vcpkg consumption path rather than deleting those historical files. The overlay port is likewise a local development/test port, not a published vcpkg registry entry. Publishing it requires replacing the checkout-relative source path with a pinned release fetch.
