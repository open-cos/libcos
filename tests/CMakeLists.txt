

set(LIBCOS_TESTS
    ObjectTest.cpp
    tokenizer.c
    parser.c
)

create_test_sourcelist(LIBCOS_TEST_SOURCES
    # The name of the generated test-driver source file.
    libcos-test.c
    # The list of test source files to be added to the test-driver executable.
    ${LIBCOS_TESTS}
)

add_executable(libcos-test
    ${LIBCOS_TEST_SOURCES}
)

foreach (TEST_SOURCE ${LIBCOS_TESTS})
    get_filename_component(TEST_NAME
        ${TEST_SOURCE}
        # File name without directory and extension.
        NAME_WE
    )
    
    set_property(SOURCE ${TEST_SOURCE}
        PROPERTY COMPILE_DEFINITIONS
        # Define the test name.
        TEST_NAME=${TEST_NAME}
    )
    
    add_test(NAME ${TEST_NAME}
        # Command to run the test - the name of the test is passed as an argument.
        COMMAND libcos-test ${TEST_NAME}
    )
endforeach ()

target_include_directories(libcos-test
    # Find a better way to do this?
    PUBLIC ../include/
    PRIVATE ../src/
)

target_compile_options(libcos-test PRIVATE
    ${COS_C_FLAGS_WARNINGS}
    ${COS_CXX_FLAGS_WARNINGS}
)

target_link_libraries(libcos-test
    PUBLIC libcos libcos++
)

if (COS_BUILD_FOR_FUZZING)
    add_subdirectory(fuzz)
endif ()
