
include(CMakePushCheckState)
include(CheckFunctionExists)

macro(check_strerror)
    message(CHECK_START "Checking for POSIX strerror_r()")
    
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_QUIET ON)
    set(CMAKE_REQUIRED_DEFINITIONS
        -U_GNU_SOURCE
        -D_POSIX_C_SOURCE=200112L
    )
    check_function_exists(strerror_r HAVE_POSIX_STRERROR_R)
    cmake_pop_check_state()
    
    if (HAVE_POSIX_STRERROR_R)
        message(CHECK_PASS "found")
    else ()
        message(CHECK_FAIL "not found")
    endif ()
endmacro()

