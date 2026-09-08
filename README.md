# litwindow

## How to build

This workspace now builds with CMake and vcpkg manifest mode.

## Requirements

- CMake 3.10 or newer
- A C++20-capable compiler
- Ninja or another supported CMake generator
- vcpkg available to CMake/Visual Studio for the Boost dependencies declared in `vcpkg.json`

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

## Windows build

### Visual Studio

1. Open the folder in Visual Studio.
2. Let CMake configure the project.
3. Ensure vcpkg manifest mode is enabled so the Boost packages from `vcpkg.json` are restored.
4. Build the desired configuration.

### Command line example

- `cmake -S . -B out/build/windows -G Ninja`
- `cmake --build out/build/windows`

## Linux build

### Command line example

- `cmake -S . -B out/build/linux -G Ninja`
- `cmake --build out/build/linux`

## Running tests

After building, run:

- `ctest --test-dir out/build/windows --output-on-failure`
- or
- `ctest --test-dir out/build/linux --output-on-failure`

The ODBC test executable is registered as `odbc_tests` and expects the SQLite ODBC driver to be installed on Linux.
You can verify the driver registration with:

- `odbcinst -j`
- `odbcinst -q -d`

## Project layout

- `libs/lwbase` - core library
- `libs/odbc` - ODBC wrapper library
- `libs/odbc/odbc_unittest` - Boost.Test-based ODBC tests
