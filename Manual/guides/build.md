# Building the Core {#guide_build}

The emulator core is located in the `VACore` directory of the repository. It is a plain C++20 library with a CMake build. The macOS application uses the same sources through the Xcode project.

## Requirements

- A C++20 compiler: GCC, Clang, or Visual Studio (MSVC)
- CMake

If CMake finds zlib, the core is built with support for compressed files (gzip). Without zlib, the build still succeeds and the compressed formats are unavailable.

## Targets

`VACore/CMakeLists.txt` defines two targets and pulls in the libraries they depend on:

| Target | Description |
|--------|-------------|
| `VACore` | The emulator core as a library. It links the utility library `utlib`, and the retro vault library `rvlib`. |
| `VAHeadless` | A small command-line app performing smoke tests (see @ref guide_headless "Headless App"). |

With no build type set, CMake builds the Release configuration.

## Building on Linux and macOS

Configure, build, and run the self tests from the root of the repository:

```
cmake -B build -S VACore
cmake --build build
ctest --test-dir build --verbose
```

To build a Debug version, pass `-DCMAKE_BUILD_TYPE=Debug` to the first command. To choose a compiler, pass `-DCMAKE_CXX_COMPILER=clang++` (or `g++`) and the matching `-DCMAKE_C_COMPILER`.

## Building on Windows

With Visual Studio, use the compiler detected by CMake and select the configuration when building:

```
cmake -B build -S VACore
cmake --build build --config Release
ctest --test-dir build --build-config Release --verbose
```

With MinGW (MSYS2), use the Ninja generator:

```
cmake -G Ninja -B build -S VACore -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The headless app additionally links `ws2_32` on Windows. CMake takes care of that.

## Self tests

`ctest` runs the headless app three times:

| Test | Command | What it does |
|------|---------|--------------|
| SelfTest1 | `VAHeadless --verbose --footprint` | Reports the size of the core objects. |
| SelfTest2 | `VAHeadless --verbose --smoke` | Runs a RetroShell script that exercises the emulator. |
| SelfTest3 | `VAHeadless --verbose --diagnose` | Launches the emulator thread and runs the self-diagnosis script. |

The same three commands run on every push on GitHub, for Linux, Windows (MSVC), and MinGW, in the Release and Debug configurations.

## Using the core in your project

The easiest way to use the core is to add it as a subdirectory of your own CMake project:

```
add_subdirectory(path/to/VACore)
add_executable(myapp main.cpp)
target_link_libraries(myapp VACore)
```

Linking `VACore` brings in the other libraries and adds the `VACore` directory to the include path, so the API header is found with:

```cpp
#include "VAmiga.h"
```

The class [vamiga::VAmiga](@ref vamiga::VAmiga) is the entry point of the API. The following program creates an emulator instance, configures it, and runs it. The emulator computes frames in a thread of its own, so the main thread is free to do other work.

```cpp
#include "VAmiga.h"

using namespace vamiga;

// Receives all messages sent by the emulator. Note that the function is
// called on the emulator thread.
static void process(const void *listener, Message msg, std::exception_ptr error)
{
    // Forward the message to the user thread (not shown)
}

int main()
{
    // Create an emulator instance
    VAmiga vamiga;

    // Launch the emulator thread
    vamiga.launch(nullptr, process);

    // Install a Kickstart ROM
    vamiga.mem.loadRom("kick13.rom");

    // Configure the emulator
    vamiga.set(Opt::AGNUS_REVISION, long(AgnusRevision::ECS_1MB));

    // Insert a disk
    vamiga.df0.insert("game.adf", false);

    // Start the emulator
    vamiga.run();

    // ...
}
```

The chapter @ref guide_headless "Headless App" shows a complete program that is used for smoke tests.

