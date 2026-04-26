# Only build the public-api-fixer tool on Unix.
if (NOT UNIX OR EMSCRIPTEN)
    return()
endif ()

include(CMakePushCheckState)

cmake_push_check_state()
message(CHECK_START "Checking for Clang")
find_package(Clang QUIET REQUIRED CONFIG)
if (Clang_FOUND)
    message(CHECK_PASS "found")
else ()
    message(CHECK_FAIL "not found")
    message(FATAL_ERROR "Please install Clang or set the CMAKE_PREFIX_PATH to the Clang installation directory.")
endif ()
cmake_pop_check_state()

add_subdirectory(common)
add_subdirectory(public-api-fixer)
