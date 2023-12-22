
option(SANITIZE_MEMORY "Enable memory sanitizer" ON)

set(MSAN_COMPILE_OPTIONS "-fsanitize=memory")
set(MSAN_LINK_OPTIONS "-fsanitize=memory")
set(MSAN_DEFINITIONS "SANITIZE_MEMORY")

include(CheckCCompilerFlag)

##set(CMAKE_REQUIRED_QUIET ON)
#check_c_compiler_flag("-fsanitize=address" HAS_MEMORY_SANITIZER)
#if (NOT HAS_MEMORY_SANITIZER)
#    message(WARNING "Memory sanitizer is not supported by the compiler")
#    set(SANITIZE_MEMORY OFF)
#endif()

if (SANITIZE_MEMORY)
    message(STATUS "Enabling memory sanitizer")

    message(STATUS "Memory sanitizer enabled")
endif()

# Add sanitizer to target.
function(add_sanitizer TARGET SANITIZER)
    target_compile_options(${TARGET} PRIVATE ${${SANITIZER}_COMPILE_OPTIONS})
    target_link_libraries(${TARGET} PRIVATE ${${SANITIZER}_LINK_OPTIONS})
    target_compile_definitions(${TARGET} PRIVATE ${${SANITIZER}_DEFINITIONS})
endfunction()

# Add all enabled sanitizers to target.
function(add_sanitizers TARGET)
    if (SANITIZE_MEMORY AND HAS_MEMORY_SANITIZER)
        add_sanitizer(${TARGET} MSAN)
    endif()
endfunction()
