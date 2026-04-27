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
