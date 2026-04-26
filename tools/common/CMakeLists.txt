add_library(libcos_tooling STATIC
    HeaderCompilationDatabase.cpp
    MacroExpansionTracker.cpp
)

set_target_properties(libcos_tooling PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(libcos_tooling
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(libcos_tooling PUBLIC
    clangBasic
    clangLex
    clangTooling
)
