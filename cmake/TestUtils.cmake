#[[
    get_test_name(TEST_NAME TEST_SOURCE)

    Derives a test name from a source-file path by combining the directory
    and the filename (without extension) with a "/" separator.

    Example:
        unit-tests/tokenizer.c  ->  unit-tests/tokenizer
        parser.c                ->  parser
]]
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
