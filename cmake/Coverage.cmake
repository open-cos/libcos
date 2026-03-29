
option(COS_BUILD_COVERAGE "Build with code coverage instrumentation" OFF)

if (COS_BUILD_COVERAGE AND COS_BUILD_FOR_FUZZING)
    message(FATAL_ERROR "Coverage and fuzzing cannot be enabled at the same time.")
endif ()

function(add_coverage TARGET)
    if (NOT COS_BUILD_COVERAGE)
        return()
    endif ()

    target_compile_definitions(${TARGET} PRIVATE COS_BUILD_COVERAGE=1)

    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        message(STATUS "Enabling code coverage (GCC) for target ${TARGET}")
        target_compile_options(${TARGET} PRIVATE --coverage)
        target_link_options(${TARGET} PUBLIC --coverage)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Enabling code coverage (Clang) for target ${TARGET}")
        target_compile_options(${TARGET} PRIVATE
            -fprofile-instr-generate
            -fcoverage-mapping
        )
        target_link_options(${TARGET} PUBLIC
            -fprofile-instr-generate
            -fcoverage-mapping
        )
    else ()
        message(FATAL_ERROR "Code coverage is only supported with GCC or Clang.")
    endif ()
endfunction()

if (COS_BUILD_COVERAGE)
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        find_program(LCOV_PROGRAM lcov)
        find_program(GENHTML_PROGRAM genhtml)

        if (LCOV_PROGRAM AND GENHTML_PROGRAM)
            add_custom_target(coverage
                COMMENT "Generating code coverage report"
                COMMAND ${LCOV_PROGRAM} --capture
                    --directory ${CMAKE_BINARY_DIR}
                    --output-file ${CMAKE_BINARY_DIR}/coverage.info
                    --rc branch_coverage=1
                COMMAND ${LCOV_PROGRAM}
                    --remove ${CMAKE_BINARY_DIR}/coverage.info
                    "*/tests/*"
                    --output-file ${CMAKE_BINARY_DIR}/coverage.info
                    --rc branch_coverage=1
                COMMAND ${GENHTML_PROGRAM}
                    ${CMAKE_BINARY_DIR}/coverage.info
                    --branch-coverage
                    --output-directory ${CMAKE_BINARY_DIR}/coverage-report
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
            message(STATUS "Coverage report target available: 'make coverage'")
        else ()
            message(WARNING "lcov/genhtml not found; 'coverage' target will not be available.")
        endif ()

    elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        find_program(LLVM_PROFDATA_PROGRAM llvm-profdata)
        find_program(LLVM_COV_PROGRAM llvm-cov)

        if (LLVM_PROFDATA_PROGRAM AND LLVM_COV_PROGRAM)
            add_custom_target(coverage
                COMMENT "Generating code coverage report"
                COMMAND ${LLVM_PROFDATA_PROGRAM} merge
                    -sparse ${CMAKE_BINARY_DIR}/default.profraw
                    -o ${CMAKE_BINARY_DIR}/default.profdata
                COMMAND ${LLVM_COV_PROGRAM} show
                    $<TARGET_FILE:libcos-test>
                    -instr-profile=${CMAKE_BINARY_DIR}/default.profdata
                    -format=html
                    -output-dir=${CMAKE_BINARY_DIR}/coverage-report
                    -object $<TARGET_FILE:libcos>
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
            message(STATUS "Coverage report target available: 'make coverage'")
        else ()
            message(WARNING "llvm-profdata/llvm-cov not found; 'coverage' target will not be available.")
        endif ()
    endif ()
endif ()
