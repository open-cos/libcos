cmake_minimum_required(VERSION 3.25)
project(libcos C)

set(CMAKE_C_STANDARD 99)

add_library(libcos SHARED
        include/libcos/CosDocument.h
        include/libcos/CosObject.h
        include/libcos/io/CosInputStream.h
        src/CosDocument.c
        src/CosObject.c
        src/common/Assert.c
        src/common/Assert.h
        src/common/CosString.c
        src/common/CosString.h
        src/common/CosVector.c
        src/common/CosVector.h
        src/io/CosFileInputStream.c
        src/io/CosInputStream.c
        src/io/CosMemoryInputStream.c
        src/io/string-support.c
        src/parse/CosTokenizer.c src/parse/CosTokenizer.h include/libcos/CosBasicTypes.h src/common/CosError.c include/libcos/common/CosError.h include/libcos/io/string-support.h include/libcos/common/CosDefines.h src/common/CharacterSet.h src/common/CharacterSet.c src/parse/CosParser.c src/parse/CosParser.h include/libcos/common/CosTypes.h src/objects/CosObjectStorage.c src/objects/CosObjectStorage.h src/parse/CosToken.c src/parse/tokenizer/CosTokenValue.c src/parse/tokenizer/CosTokenValue.h
        src/common/CosString-Impl.h
        src/common/CosData.c
        src/common/CosData.h
)

target_include_directories(libcos
        PUBLIC include/
        PRIVATE src/)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(libcos PRIVATE
            -Wall -Wextra -Werror)
elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(libcos PRIVATE
            -Wall
            -Wextra
            -Weverything
            -Wno-padded -Wno-unused-parameter -Wno-unused-variable -Wno-unsafe-buffer-usage
            -Werror=incompatible-pointer-types
            -Werror=return-type)
elseif (C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(libcos PRIVATE
            /W4 /WX)
endif ()

target_compile_options(libcos PRIVATE
        # clang/GCC warnings
        $<$<CXX_COMPILER_ID:Clang>:
        -Wall -Weverything -Wno-c++98-compat-pedantic -Wno-padded -Wno-unused-function -Werror>
        # disable noexcept type warning on GCC
        $<$<CXX_COMPILER_ID:GNU>: -Wall -Wextra -Wno-noexcept-type -Werror>
        # MSVC warnings
        $<$<CXX_COMPILER_ID:MSVC>:
        /W4>)

