# CMake toolchain file for cross-compiling to Windows using MinGW.

# The name of the target operating system.
set(CMAKE_SYSTEM_NAME Windows)
# The processor architecture for which we are cross compiling.
set(CMAKE_SYSTEM_PROCESSOR i686)

set(WIN32 True)

set(TOOLCHAIN_PREFIX i686-w64-mingw32)

# cross compilers to use for C, C++
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

# target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# Modify default behavior of FIND_XXX() commands:

# Search programs in the host environment *only*.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search headers and libraries in the target environment *only*.
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Search for .pc files in the target environment.
set(PKG_CONFIG_EXECUTABLE ${TOOLCHAIN_PREFIX}-pkg-config)

find_program(WINE_EXECUTABLE wine)
if (WINE_EXECUTABLE)
    message(VERBOSE "Using Wine for cross-compiling emulator: ${WINE_EXECUTABLE}")
    
    set(CMAKE_CROSSCOMPILING_EMULATOR ${WINE_EXECUTABLE})
endif ()
