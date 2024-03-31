
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Fuzzing with LibFuzzer requires a Clang compiler. Fuzzer will not be built.")
    return()
endif ()

add_executable(libcos-fuzz)

target_sources(libcos-fuzz PRIVATE
    fuzz.c
)

target_compile_features(libcos-fuzz PRIVATE
    c_std_99
)

target_compile_options(libcos-fuzz PRIVATE
    -fsanitize=fuzzer
)

target_link_options(libcos-fuzz PRIVATE
    -fsanitize=fuzzer
)

target_link_libraries(libcos-fuzz PRIVATE
    libcos
)
