
macro(get_test_name TEST_NAME TEST_SOURCE)
    get_filename_component(TEST_DIRNAME
        ${TEST_SOURCE}
        # Directory of the test source file.
        DIRECTORY
    )
    
    get_filename_component(TEST_BASENAME
        ${TEST_SOURCE}
        # File name without directory and last extension.
        NAME_WLE
    )
    
    if (TEST_DIRNAME)
        set(TEST_NAME "${TEST_DIRNAME}/${TEST_BASENAME}")
    else ()
        set(TEST_NAME "${TEST_BASENAME}")
    endif ()
endmacro()

set(LIBCOS_TESTS
    ObjectTest.cpp
    tokenizer.c
    parser.c
    filters/ascii85.c
    filters/ascii-hex.c
    unit/test-dict.c
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

target_include_directories(libcos-test
    PUBLIC ${PROJECT_SOURCE_DIR}/include/
    PRIVATE ${PROJECT_SOURCE_DIR}/src/ ${CMAKE_CURRENT_SOURCE_DIR}
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
