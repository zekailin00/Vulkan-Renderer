# Vulkan Renderer

## Coordinates

- upward: +y
- forward: -z
- right: +x
- default camera looks at -z

## Vulkan fragment shader stage range

- x: [-1, 1] from left to right
- y: [-1, 1] from top to bottom
- z: [ 0, 1] from front to back

## Build setting
- The engine MUST be compiled by a 64 bit compiler. 
- For Visual Studio: x86_amd64 is good, but x86 does NOT work

## Localization
- Currently, all paths must be in English.
- Specifically, all characters must be in ASCII format. Each character is 1 byte.
- When conversion from path to string has non ASCII characters, exception is thrown


## V8 Engine Windows Setup
- https://v8.dev/docs/source-code

**Notes**
- Install depot_tools and Visual Studio
- depot_tools path needs to be added to "path" environment variables, and the path is better to be the first (above `python` and `git`)
-  DEPOT_TOOLS_WIN_TOOLCHAIN environment variable in the same way, and set it to 0 (use local toolchain instead of tools private to Google).
- depot tools have all executables needed to build V8, such as ninja.
- If Visual Studio is already install long time ago, might need to install another specific windows SDK version. At the moment this note is written, the required SDK is Windows SDK (10.0.22621) (with ALL features includes).
- If installing it from Visual Studio does not work, an installer from Windows Dev Center can be downloaded, and just select to install ALL components.

```
gclient             # update depot tools
mkdir <path>/v8
cd <path>/v8
fetch v8            # fetch V8 repository
cd v8
```

- https://v8.dev/docs/embed
- Compile v8 to a monolithic static library

```
tools/dev/v8gen.py x64.release.sample
ninja -C out.gn/x64.release.sample v8_monolith
```

**Notes**
- If `ninja` build fails, it is probably due to more recent C++ compiler deprecating some APIs that the V8 code base uses. V8 compiler setting treats all warnings as errors. To fix this:

```
tools/dev/v8gen.py x64.release.sample           # same

gn args out.gn/x64.release.sample
# This will open up the build configuration.
# Add a line: treat_warnings_as_errors = false
# Then save and retry ninja build

ninja -C out.gn/x64.release.sample v8_monolith  # same
```

### Windows Build Configuration
- is_debug = false for release build
- always static libraries
- built with clang
```
is_component_build = false
is_debug = true
is_clang = true
target_cpu = "x64"
use_custom_libcxx = false
v8_monolithic = true
v8_use_external_startup_data = false
treat_warnings_as_errors = false
```


## Windows Build Issues
1. needs to specify dynamic/static library, release/debug build
2. for debug build, optimization needs to be set to 0
3. V8 is built using Clang, linking v8 against an executable built with MSVC will lead to a linker error saying v8 lib is corrupted.
4. The compiler used for windows OS currently must be Clang (MSVC CLI) for MSVC downloaded from Visual Studio Installer
5. In addition to linking V8 to the intended executed, the platform part of V8, when the target is Windows, also uses dynamic libraries from Windows OS, so we also need to manually link all Windows DLLs to resolve those linking errors.
6. Currently, only Windows DLLs used are `Winmm` and `Dbghelp`. Linking those two name to the target in CMAKE is sufficient