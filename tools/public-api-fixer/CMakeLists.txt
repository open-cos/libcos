cmake_minimum_required(VERSION 3.13)

# The LLVM C++ libraries require C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(public_api_fixer)

target_sources(public_api_fixer PUBLIC
    main.cpp
)

target_include_directories(public_api_fixer PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(public_api_fixer PRIVATE
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangLex
    clangTooling
)
