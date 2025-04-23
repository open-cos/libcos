
include(CMakePushCheckState)
include(CheckFunctionExists)
include(CheckSymbolExists)

macro(check_strerror)
    message(CHECK_START "Checking for POSIX strerror_r()")
    
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_QUIET ON)
    set(CMAKE_REQUIRED_DEFINITIONS
        -U_GNU_SOURCE
        -D_POSIX_C_SOURCE=200112L
    )
    check_symbol_exists(strerror_r "string.h" HAVE_POSIX_STRERROR_R)
    cmake_pop_check_state()
    
    if (HAVE_POSIX_STRERROR_R)
        message(CHECK_PASS "found")
    else ()
        # Ensure that the variable is defined, even if the check failed.
        set(HAVE_POSIX_STRERROR_R 0)
        
        message(CHECK_FAIL "not found")
    endif ()
endmacro()

macro(check_strlcpy)
    message(CHECK_START "Checking for strlcpy()")
    
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_QUIET ON)
    check_symbol_exists(strlcpy "string.h" HAVE_STRLCPY)
    cmake_pop_check_state()
    
    if (HAVE_STRLCPY)
        message(CHECK_PASS "found")
    else ()
        # Ensure that the variable is defined, even if the check failed.
        set(HAVE_STRLCPY 0)
        
        message(CHECK_FAIL "not found")
    endif ()
endmacro()


macro(_check_64bit_file_offset_support)
    message(CHECK_START "Checking for 64-bit file offset support")
    
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_QUIET ON)
    set(CMAKE_REQUIRED_DEFINITIONS
        -D_FILE_OFFSET_BITS=64
    )
    check_symbol_exists(fseeko "stdio.h" HAVE_64BIT_FILE_OFFSET_SUPPORT)
    cmake_pop_check_state()
    
    if (HAVE_64BIT_FILE_OFFSET_SUPPORT)
        # Record the required compile definitions for fseeko.
        set(LFS_COMPILE_DEFINITIONS
            -D_FILE_OFFSET_BITS=64
        )
        
        message(CHECK_PASS "found")
    else ()
        # Ensure that the variable is defined, even if the check failed.
        set(HAVE_64BIT_FILE_OFFSET_SUPPORT 0)
        
        message(CHECK_FAIL "not found")
    endif ()
endmacro()

# Defining the _LARGEFILE_SOURCE feature test macro is the obsolete way to enable large file support.
macro(_check_legacy_large_file_support)
    message(CHECK_START "Checking for legacy large file support")
    
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_QUIET ON)
    set(CMAKE_REQUIRED_DEFINITIONS
        -D_LARGEFILE_SOURCE
    )
    check_symbol_exists(fseeko "stdio.h" HAVE_LEGACY_LARGE_FILE_SUPPORT)
    cmake_pop_check_state()
    
    if (HAVE_LEGACY_LARGE_FILE_SUPPORT)
        # Record the required compile definitions for fseeko.
        set(LFS_COMPILE_DEFINITIONS
            -D_LARGEFILE_SOURCE
        )
        
        message(CHECK_PASS "found")
    else ()
        # Ensure that the variable is defined, even if the check failed.
        set(HAVE_LEGACY_LARGE_FILE_SUPPORT 0)
        
        message(CHECK_FAIL "not found")
    endif ()
endmacro()

function(check_large_file_support)
    # Check if there is a cache variable for this check.
    if (DEFINED CACHE{HAVE_LARGE_FILE_SUPPORT}) # NOTE: CACHE{VAR} requires CMake 3.14
        # Using the cache variable.
        return()
    endif ()
    
    message(CHECK_START "Checking for fseeko() and large file support")
    
    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    
    _check_64bit_file_offset_support()
    if (NOT HAVE_64BIT_FILE_OFFSET_SUPPORT)
        _check_legacy_large_file_support()
    endif ()
    
    list(POP_BACK CMAKE_MESSAGE_INDENT)
    
    if (HAVE_64BIT_FILE_OFFSET_SUPPORT OR HAVE_LEGACY_LARGE_FILE_SUPPORT)
        set(HAVE_LARGE_FILE_SUPPORT 1)
        
        message(CHECK_PASS "found")
    else ()
        set(HAVE_LARGE_FILE_SUPPORT 0)
        
        message(CHECK_FAIL "not found")
    endif ()
    
    # Cache the result of the check.
    set(HAVE_LARGE_FILE_SUPPORT ${HAVE_LARGE_FILE_SUPPORT}
        CACHE INTERNAL
        "Have large file support"
    )
    
    if (HAVE_LARGE_FILE_SUPPORT)
        # Set the other cache variables for large file support.
        set(LFS_COMPILE_DEFINITIONS ${LFS_COMPILE_DEFINITIONS}
            CACHE INTERNAL
            "Extra compile definitions for large file support"
        )
        set(LFS_HEADERS ${LFS_HEADERS}
            CACHE INTERNAL
            "Extra headers for large file support"
        )
    endif ()
endfunction()
