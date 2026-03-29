#[[
    get_test_name(OUT_VAR TEST_SOURCE)

    Derives a test name from a source-file path by combining the directory
    and the filename (without extension) with a "/" separator.

    Example:
        unit-tests/tokenizer.c  ->  unit-tests/tokenizer
        parser.c                ->  parser
]]
macro(get_test_name OUT_VAR TEST_SOURCE)
    get_filename_component(_GTN_DIRNAME
        ${TEST_SOURCE}
        # Directory of the test source file.
        DIRECTORY
    )

    get_filename_component(_GTN_BASENAME
        ${TEST_SOURCE}
        # File name without directory and last extension.
        NAME_WLE
    )

    if (_GTN_DIRNAME)
        set(${OUT_VAR} "${_GTN_DIRNAME}/${_GTN_BASENAME}")
    else ()
        set(${OUT_VAR} "${_GTN_BASENAME}")
    endif ()
endmacro()
