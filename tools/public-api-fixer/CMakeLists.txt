cmake_minimum_required(VERSION 3.13)

# The LLVM C++ libraries require C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(public_api_fixer
    AnnotationCheckAction.cpp
    MissingAnnotationCallback.cpp
    main.cpp
)

target_include_directories(public_api_fixer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(public_api_fixer PRIVATE
    libcos_tooling
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangLex
    clangTooling
)

add_test(
    NAME public_api_fixer_basic
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/harness.sh
            $<TARGET_FILE:public_api_fixer>
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixture.h
)

# add_public_api_check(<name>
#                      TARGET <target>
#                      [APPLY_FIXES]
#                      [ANNOTATION <macro>])
#
# Creates a custom target <name> that runs public_api_fixer over the
# default PUBLIC FILE_SET HEADERS of <target>. The custom target is not
# part of ALL; build it explicitly (e.g. `cmake --build . --target <name>`).
#
# Options:
#   APPLY_FIXES   Rewrite the headers in place to insert the missing
#                 annotation. Without this, the tool only reports.
#   ANNOTATION    Macro name considered the public-API marker. Defaults
#                 to whatever the tool itself defaults to (COS_API).
function(add_public_api_check name)
    cmake_parse_arguments(arg
        "APPLY_FIXES"
        "TARGET;ANNOTATION"
        ""
        ${ARGN})

    if (NOT arg_TARGET)
        message(FATAL_ERROR
            "add_public_api_check(${name}): TARGET is required.")
    endif ()
    if (NOT TARGET ${arg_TARGET})
        message(FATAL_ERROR
            "add_public_api_check(${name}): '${arg_TARGET}' is not a target.")
    endif ()

    # Read the default HEADERS file set. PUBLIC visibility populates both
    # HEADER_SET (on the target) and INTERFACE_HEADER_SET (for consumers);
    # try the former first, then fall back for INTERFACE-only targets.
    get_target_property(_headers ${arg_TARGET} HEADER_SET)
    if (NOT _headers)
        get_target_property(_headers ${arg_TARGET} INTERFACE_HEADER_SET)
    endif ()
    if (NOT _headers)
        message(FATAL_ERROR
            "add_public_api_check(${name}): target '${arg_TARGET}' has no "
            "PUBLIC FILE_SET HEADERS declaration.")
    endif ()

    set(_extra_args)
    if (arg_APPLY_FIXES)
        list(APPEND _extra_args --fix)
    endif ()
    if (arg_ANNOTATION)
        list(APPEND _extra_args "--annotation=${arg_ANNOTATION}")
    endif ()

    add_custom_target(${name}
        COMMAND $<TARGET_FILE:public_api_fixer>
                -p ${CMAKE_BINARY_DIR}
                ${_extra_args}
                ${_headers}
        DEPENDS public_api_fixer
        VERBATIM
        COMMENT
            "Running public-api-fixer on ${arg_TARGET}'s public headers")
endfunction()
