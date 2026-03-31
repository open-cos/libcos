include(TestUtils)

set(LIBCOS_EXAMPLES
    libcos/objects/iterate-objects.c
    libcos/objects/traverse-document.c
)

create_test_sourcelist(LIBCOS_EXAMPLE_SOURCES
    # The name of the generated test-driver source file.
    libcos-examples.c
    # The list of example source files to be added to the test-driver executable.
    ${LIBCOS_EXAMPLES}
)

add_executable(libcos-examples
    ${LIBCOS_EXAMPLE_SOURCES}
    CosExample.h
)

foreach (EXAMPLE_SOURCE ${LIBCOS_EXAMPLES})

    get_test_name(EXAMPLE_NAME ${EXAMPLE_SOURCE})

    string(MAKE_C_IDENTIFIER ${EXAMPLE_NAME} EXAMPLE_FUNCTION_NAME)

    set_property(SOURCE ${EXAMPLE_SOURCE}
        PROPERTY COMPILE_DEFINITIONS
        # Define the test name.
        TEST_NAME=${EXAMPLE_FUNCTION_NAME}
    )

    add_test(NAME examples/${EXAMPLE_NAME}
        # Command to run the example - the name is passed as an argument.
        COMMAND libcos-examples ${EXAMPLE_NAME}
    )
endforeach ()

target_include_directories(libcos-examples
    PUBLIC ${PROJECT_SOURCE_DIR}/include/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(libcos-examples PRIVATE
    ${COS_COMPILE_DEFINITIONS}
    $<$<COMPILE_LANGUAGE:C>:${COS_C_COMPILE_DEFINITIONS}>
    $<$<COMPILE_LANGUAGE:CXX>:${COS_CXX_COMPILE_DEFINITIONS}>
)

target_compile_options(libcos-examples PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${COS_C_FLAGS_WARNINGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${COS_CXX_FLAGS_WARNINGS}>
)

target_link_libraries(libcos-examples
    PUBLIC libcos
)
