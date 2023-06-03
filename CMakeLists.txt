cmake_minimum_required(VERSION 3.25)
project(libcos C)

set(CMAKE_C_STANDARD 99)

add_library(libcos SHARED
        src/CosDocument.c
        include/libcos/CosDocument.h
        src/CosObject.c
        include/libcos/CosObject.h
        src/common/CosVector.c src/common/CosVector.h src/common/CosString.c src/common/CosString.h src/common/Assert.c src/common/Assert.h src/io/CosInputStream.c src/io/CosInputStream.h src/io/CosFileInputStream.c src/io/CosMemoryInputStream.c)

target_include_directories(libcos
        PUBLIC include/
        PRIVATE src/)

target_compile_options(libcos PRIVATE
        # clang/GCC warnings
        $<$<CXX_COMPILER_ID:Clang>:
        -Wall -Weverything -Wno-c++98-compat-pedantic -Wno-padded -Wno-unused-function>
        # disable noexcept type warning on GCC
        $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -Wno-noexcept-type>
        # MSVC warnings
        $<$<CXX_COMPILER_ID:MSVC>:
        /W4>)
