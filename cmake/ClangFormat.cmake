# Aggregate targets are always defined so they can always be invoked.
add_custom_target(format
    COMMENT "Format all opted-in targets with clang-format"
)
add_custom_target(format-check
    COMMENT "Check formatting of all opted-in targets with clang-format"
)

find_program(CLANG_FORMAT_EXECUTABLE
    NAMES clang-format
    DOC "Path to clang-format executable")

if (NOT CLANG_FORMAT_EXECUTABLE)
    message(STATUS "clang-format not found; 'format' and 'format-check' targets will have no effect")
endif ()

function(add_clang_format TARGET_NAME)
    if (NOT CLANG_FORMAT_EXECUTABLE)
        return()
    endif ()
    
    # Private sources (.c and private .h files listed in target_sources PRIVATE).
    get_target_property(_private_sources ${TARGET_NAME} SOURCES)
    if (NOT _private_sources)
        set(_private_sources "")
    endif ()
    get_target_property(_target_source_dir ${TARGET_NAME} SOURCE_DIR)

    # Public headers from FILE_SET HEADERS (CMake 3.23+).
    get_target_property(_public_headers ${TARGET_NAME} HEADER_SET_HEADERS)
    if (NOT _public_headers)
        set(_public_headers "")
    endif ()

    set(_candidates ${_private_sources} ${_public_headers})
    list(FILTER _candidates INCLUDE REGEX "\\.(c|h)$")

    # Resolve to absolute paths; exclude files outside the source tree or
    # inside the binary directory (e.g. the generated test driver produced
    # by create_test_sourcelist), and skip files that do not exist on disk.
    set(_files "")
    foreach (_f IN LISTS _candidates)
        get_filename_component(_abs "${_f}" ABSOLUTE BASE_DIR "${_target_source_dir}")
        string(FIND "${_abs}" "${PROJECT_SOURCE_DIR}/" _src_pos)
        string(FIND "${_abs}" "${PROJECT_BINARY_DIR}/" _bin_pos)
        if (_src_pos EQUAL 0 AND NOT _bin_pos EQUAL 0 AND EXISTS "${_abs}")
            list(APPEND _files "${_abs}")
        endif ()
    endforeach ()
    
    if (NOT _files)
        message(WARNING "add_clang_format(${TARGET_NAME}): no formattable sources found")
        return()
    endif ()
    
    add_custom_target(format-${TARGET_NAME}
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${_files}
        COMMENT "Formatting ${TARGET_NAME} with clang-format"
        VERBATIM
    )
    add_custom_target(format-check-${TARGET_NAME}
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror ${_files}
        COMMENT "Checking ${TARGET_NAME} formatting with clang-format"
        VERBATIM
    )
    
    add_dependencies(format format-${TARGET_NAME})
    add_dependencies(format-check format-check-${TARGET_NAME})
endfunction()
