# litwindow

## How to build

This workspace now builds with CMake and vcpkg manifest mode.

## Requirements

- CMake 3.10 or newer
- A C++20-capable compiler
- Ninja or another supported CMake generator
- vcpkg available to CMake/Visual Studio for the dependencies declared in `vcpkg.json` (set the `VCPKG_ROOT` environment variable to your vcpkg checkout)

## Optional components (odbc / lwwx)

litwindow's CMake build has two optional components, each controlled by a CMake option and a matching `vcpkg.json` feature:

| Component | CMake option | CMake option default | vcpkg feature |
|---|---|---|---|
| `libs/odbc` (ODBC data-access library) | `LITWINDOW_BUILD_ODBC` | ON | `odbc` |
| `libs/lwwx` (wxWidgets UI integration library) | `LITWINDOW_BUILD_LWWX` | OFF | `wx` |

Neither vcpkg feature is a default feature, so building this repo directly (e.g. via the CMake presets below) is governed purely by the CMake option defaults above: `libs/odbc` builds by default (matches historical behaviour, and needs no extra vcpkg packages - see below), `libs/lwwx` does not. The vcpkg features only matter when litwindow is consumed as a vcpkg *port* by another project - there, both are opt-in and must be requested explicitly, e.g. `litwindow[odbc,wx]`. To build `lwwx` locally, enable both the CMake option and the vcpkg feature, e.g.:

- `cmake -S . -B out/build/lwwx -G Ninja -DLITWINDOW_BUILD_LWWX=ON -DVCPKG_MANIFEST_FEATURES=wx -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake`

Building `wxwidgets` from source via vcpkg the first time can take several minutes.

## Testing vcpkg consumption

litwindow is installable (`cmake --install`) and exports proper CMake package config (`litwindowConfig.cmake` + `litwindowTargets.cmake`), so downstream projects can do `find_package(litwindow REQUIRED)` and link `litwindow::lwbase`, `litwindow::odbc`, `litwindow::lwwx`. This is exercised end-to-end without publishing anything, using two extra pieces in this repo:

- `vcpkg-overlay/ports/litwindow/` - a local vcpkg port (`vcpkg.json` + `portfile.cmake`) that builds directly from this checkout (not a fetched release archive) and maps the `odbc`/`wx` vcpkg features to `LITWINDOW_BUILD_ODBC`/`LITWINDOW_BUILD_LWWX` via `vcpkg_check_features`.
- `examples/vcpkg-consumer/` - a tiny, completely separate CMake project (its own `vcpkg.json` depending on `litwindow[odbc]`) that only knows about litwindow through `find_package`, proving the port actually works for a consumer.

To run it yourself:

```powershell
cmake -S examples/vcpkg-consumer -B examples/vcpkg-consumer/build -G Ninja `
    -DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake `
    -DVCPKG_OVERLAY_PORTS=(Resolve-Path vcpkg-overlay/ports)
cmake --build examples/vcpkg-consumer/build
```

vcpkg's binary cache keys builds off `portfile.cmake`/`vcpkg.json`, not the actual source contents (since `SOURCE_PATH` here points straight at this checkout instead of a versioned archive). If you change litwindow's sources and want the consumer example to pick them up, force a clean rebuild of the cached port, e.g. delete `examples/vcpkg-consumer/build` and set `$env:VCPKG_FEATURE_FLAGS = "-binarycaching"` before reconfiguring.

To turn `vcpkg-overlay/ports/litwindow` into a real, publishable port (e.g. for a PR to the vcpkg community registry), replace its `SOURCE_PATH` line with a `vcpkg_from_github()`/`vcpkg_from_git()` call pinned to a tagged release.

## Linux-specific ODBC requirements

The `libs/odbc` library uses the system ODBC driver manager on Linux.
Do not use a vcpkg-built unixODBC for this project.

Install these packages before configuring the build:

### Debian/Ubuntu

- `sudo apt-get update`
- `sudo apt-get install cmake ninja-build g++ unixodbc unixodbc-dev libsqliteodbc`

### Fedora/RHEL

- `sudo dnf install cmake ninja-build gcc-c++ unixODBC unixODBC-devel libsqliteodbc`

### Notes

- `unixodbc` provides the runtime driver manager.
- `unixodbc-dev` / `unixODBC-devel` provides headers and link libraries needed to build `libs/odbc`.
- `libsqliteodbc` installs the SQLite ODBC driver used by `odbc_unittest`.
- On Debian/Ubuntu, the driver registration usually uses `Driver=libsqlite3odbc.so` instead of an absolute path. That is normal: the system unixODBC package knows its multiarch driver directory and resolves the relative name automatically.

## CMake presets

`CMakePresets.json` declares 8 ready-to-use configure presets, covering every combination of platform, build type and linkage:

- `windows-debug-shared`, `windows-release-shared`, `windows-debug-static`, `windows-release-static`
- `linux-debug-shared`, `linux-release-shared`, `linux-debug-static`, `linux-release-static`
- (plus equivalent `macos-*` presets)

Each preset builds to its own directory under `out/build/<preset>`, so switching between them (even Windows vs. Linux) never requires cleaning a previous build.

## Windows build

### Visual Studio

1. Open the folder in Visual Studio.
2. Select a `windows-*` CMake preset from the configuration dropdown.
3. Let CMake configure the project (vcpkg manifest mode restores the packages from `vcpkg.json` automatically).
4. Build the desired configuration.

### Command line example

- `cmake --preset windows-debug-shared`
- `cmake --build out/build/windows-debug-shared`

(Run these from a Visual Studio Developer Command Prompt / after `VsDevCmd.bat`, so `cl.exe` is on `PATH`.)

## Linux build

### Command line example

- `cmake --preset linux-debug-shared`
- `cmake --build out/build/linux-debug-shared`

## Building all variants at once (Windows host, incl. Linux via WSL)

`build_all_variants.ps1` configures and builds all 8 Windows/Linux x Debug/Release x Shared/Static presets in one go from a Windows machine:

- Windows presets are built natively (it locates Visual Studio via `vswhere` and uses `VsDevCmd.bat`).
- Linux presets are built inside WSL (Windows Subsystem for Linux). The script checks whether WSL is installed and a distribution is registered; if not, it prints an error and skips the Linux presets while still building the Windows ones.
- It never touches Visual Studio's own CMake integration, so it can safely run alongside an open Visual Studio instance without triggering a solution reload.

```powershell
.\build_all_variants.ps1                 # build everything
.\build_all_variants.ps1 -SkipLinux      # Windows presets only
.\build_all_variants.ps1 -SkipWindows    # Linux presets only (requires WSL)
```

Each preset's configure+build output is logged to `out\build-<preset>.log`, and a pass/fail/skipped summary is printed at the end.

## Running tests

After building, run:

- `ctest --test-dir out/build/windows-debug-shared --output-on-failure`
- or
- `ctest --test-dir out/build/linux-debug-shared --output-on-failure`

(substitute whichever preset directory you built)

The ODBC test executable is registered as `odbc_tests` and expects the SQLite ODBC driver to be installed on Linux.
You can verify the driver registration with:

- `odbcinst -j`
- `odbcinst -q -d`

## Project layout

- `libs/lwbase` - core library
- `libs/odbc` - ODBC wrapper library (optional, `LITWINDOW_BUILD_ODBC`, on by default)
- `libs/odbc/odbc_unittest` - Boost.Test-based ODBC tests
- `libs/lwwx` - wxWidgets UI integration library (optional, `LITWINDOW_BUILD_LWWX`, off by default)
- `vcpkg-overlay/ports/litwindow` - local/dev vcpkg port used to test litwindow's vcpkg consumption (see "Testing vcpkg consumption" above)
- `examples/vcpkg-consumer` - standalone sample project that consumes litwindow purely via vcpkg + `find_package`
