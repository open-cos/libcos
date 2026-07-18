include(TestUtils)

# The mutator core, which libcos-test exercises directly. It builds in every
# configuration -- it has no libcos or fuzzing-engine dependency -- so it lives
# outside the COS_BUILD_FUZZERS guard that gates the harnesses below.
add_subdirectory(fuzz/mutator)

set(LIBCOS_TESTS
    parser.c
    filters/ascii85.c
    filters/ascii-hex.c
    filters/decode-pipeline.c
    filters/flate.c
    filters/predictor.c
    filters/run-length.c
    io/sub-stream.c
    unit-tests/checked-arith.c
    unit-tests/dict.c
    unit-tests/tokenizer.c
    unit-tests/xref-table.c
    unit-tests/xref-stream.c
    unit-tests/xref-cycle.c
    unit-tests/xref-hybrid.c
    unit-tests/encrypted-doc.c
    unit-tests/obj-stream.c
    unit-tests/trailer-chain.c
    unit-tests/file-structure.c
    unit-tests/indirect-obj.c
    unit-tests/obj.c
    unit-tests/obj-parser.c
    unit-tests/fuzz-mutator.c
)

create_test_sourcelist(LIBCOS_TEST_SOURCES
    # The name of the generated test-driver source file.
    libcos-test.c
    # The list of test source files to be added to the test-driver executable.
    ${LIBCOS_TESTS}
)

add_executable(libcos-test
    ${LIBCOS_TEST_SOURCES}
    CosTest.h

)

foreach (TEST_SOURCE ${LIBCOS_TESTS})
    
    get_test_name(TEST_NAME ${TEST_SOURCE})
    
    string(MAKE_C_IDENTIFIER ${TEST_NAME} TEST_FUNCTION_NAME)
    
    set_property(SOURCE ${TEST_SOURCE}
        PROPERTY COMPILE_DEFINITIONS
        # Define the test name.
        TEST_NAME=${TEST_FUNCTION_NAME}
    )
    
    add_test(NAME ${TEST_NAME}
        # Command to run the test - the name of the test is passed as an argument.
        COMMAND libcos-test ${TEST_NAME}
    )
endforeach ()

# A regression in /Prev cycle detection hangs instead of failing, so bound the run time.
set_tests_properties(unit-tests/xref-cycle PROPERTIES TIMEOUT 30)

# The mutator test reads seeds from the committed fuzz corpus. APPEND, because
# the loop above sets COMPILE_DEFINITIONS rather than appending to it, so a
# plain set_property here would drop TEST_NAME and break the test driver.
set_property(SOURCE unit-tests/fuzz-mutator.c APPEND
    PROPERTY COMPILE_DEFINITIONS
    COS_FUZZ_CORPUS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fuzz/corpus"
)

target_include_directories(libcos-test
    PUBLIC ${PROJECT_SOURCE_DIR}/include/
    PRIVATE ${PROJECT_SOURCE_DIR}/src/ ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(libcos-test PRIVATE
    ${COS_COMPILE_DEFINITIONS}
    $<$<COMPILE_LANGUAGE:C>:${COS_C_COMPILE_DEFINITIONS}>
    $<$<COMPILE_LANGUAGE:CXX>:${COS_CXX_COMPILE_DEFINITIONS}>
)

target_compile_options(libcos-test PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${COS_C_FLAGS_WARNINGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${COS_CXX_FLAGS_WARNINGS}>
)

target_link_libraries(libcos-test
    PUBLIC libcos libcos++
    PRIVATE libcos-fuzz-mutator
)

# The fuzz targets are built by default so that they cannot rot, but the fuzzer
# itself is never run by CTest. Building for fuzzing implies building them.
if (COS_BUILD_FUZZERS OR COS_BUILD_FOR_FUZZING)
    add_subdirectory(fuzz)
endif ()
