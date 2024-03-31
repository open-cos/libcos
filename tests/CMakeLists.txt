
enable_testing()

add_executable(libcos-tests
    libcos++/ObjectTest.cpp
    parse/CosTokenizer-Test.c
    parse/CosObjParser-Test.c
)

target_include_directories(libcos-tests
    # Find a better way to do this?
    PUBLIC ../include/
    PRIVATE ../src/
)

target_compile_options(libcos-tests PRIVATE
    ${COS_C_FLAGS_WARNINGS}
    ${COS_CXX_FLAGS_WARNINGS}
)

target_link_libraries(libcos-tests
    PUBLIC libcos libcos++
)

if (COS_BUILD_FOR_FUZZING)
    add_subdirectory(fuzz)
endif ()
