# The fuzz targets.
#
# By default these link a standalone main() (CosFuzzDriver.c) so that they build
# with any compiler and can replay a corpus. When COS_BUILD_FOR_FUZZING is ON,
# the selected engine (COS_FUZZING_ENGINE) supplies main() instead and the
# library is instrumented.

# Shared corpus/dictionary paths and the fuzz_<name> run targets.
include(FuzzTargets)

set(LIBCOS_FUZZ_TARGETS
    filter
    obj-parser
    parser
    tokenizer
)

foreach (FUZZ_TARGET ${LIBCOS_FUZZ_TARGETS})

    set(FUZZ_TARGET_NAME libcos-fuzz-${FUZZ_TARGET})

    add_executable(${FUZZ_TARGET_NAME}
        ${FUZZ_TARGET}.c
        CosFuzz.h
    )

    target_compile_features(${FUZZ_TARGET_NAME} PRIVATE
        c_std_99
    )

    target_include_directories(${FUZZ_TARGET_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include/
        PRIVATE ${PROJECT_SOURCE_DIR}/src/ ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_compile_definitions(${FUZZ_TARGET_NAME} PRIVATE
        ${COS_COMPILE_DEFINITIONS}
        $<$<COMPILE_LANGUAGE:C>:${COS_C_COMPILE_DEFINITIONS}>
    )

    target_compile_options(${FUZZ_TARGET_NAME} PRIVATE
        $<$<COMPILE_LANGUAGE:C>:${COS_C_FLAGS_WARNINGS}>
    )

    target_link_libraries(${FUZZ_TARGET_NAME} PRIVATE
        libcos
    )

    # Link the driver that provides main() -- the $LIB_FUZZING_ENGINE analog.
    if (COS_BUILD_FOR_FUZZING AND COS_FUZZING_ENGINE STREQUAL "libfuzzer")
        # libcos already supplies -fsanitize=fuzzer-no-link and the coverage
        # flags through its PUBLIC compile options; this links libFuzzer's main().
        target_link_options(${FUZZ_TARGET_NAME} PRIVATE -fsanitize=fuzzer)
    elseif (COS_BUILD_FOR_FUZZING AND COS_FUZZING_ENGINE STREQUAL "afl" AND AFL_DRIVER_LIBRARY)
        # AFL++'s persistent driver (fork-server main loop), when it is installed.
        target_link_libraries(${FUZZ_TARGET_NAME} PRIVATE ${AFL_DRIVER_LIBRARY})
    else ()
        # Standalone main(): corpus replay, and the AFL fallback -- it reads the
        # file AFL passes as @@.
        target_sources(${FUZZ_TARGET_NAME} PRIVATE CosFuzzDriver.c)
    endif ()

    # Replay the seed corpus as a regression test. This runs each seed exactly
    # once and exits; it is not a fuzzing session. Both the standalone driver
    # and libFuzzer accept the same file-argument form.
    #
    # CONFIGURE_DEPENDS re-globs when the directory changes, so that adding or
    # removing a seed does not leave the test running a stale file list.
    file(GLOB FUZZ_CORPUS_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/corpus/${FUZZ_TARGET}/*"
    )

    if (FUZZ_CORPUS_FILES)
        add_test(NAME fuzz/${FUZZ_TARGET}
            COMMAND ${FUZZ_TARGET_NAME} ${FUZZ_CORPUS_FILES}
        )
        set_tests_properties(fuzz/${FUZZ_TARGET} PROPERTIES TIMEOUT 30)
    else ()
        message(STATUS "No fuzz corpus for '${FUZZ_TARGET}'; skipping its replay test.")
    endif ()

    cos_add_fuzz_run_targets(${FUZZ_TARGET})

endforeach ()
