
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

