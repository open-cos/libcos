cmake_minimum_required(VERSION 3.25)
project(libcos C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(libcos SHARED
        include/libcos/CosDocument.h
        include/libcos/CosObj.h
        include/libcos/io/CosInputStream.h
        src/CosDocument.c
        src/objects/CosObj.c
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
        src/parse/CosTokenizer.c
        src/parse/CosTokenizer.h
        include/libcos/CosBasicTypes.h
        src/common/CosError.c
        include/libcos/common/CosError.h
        include/libcos/io/string-support.h
        include/libcos/common/CosDefines.h
        src/common/CharacterSet.h
        src/common/CharacterSet.c
        src/parse/CosParser.c
        src/parse/CosParser.h
        include/libcos/common/CosTypes.h
        src/objects/CosObjectStorage.c
        src/objects/CosObjectStorage.h
        src/parse/tokenizer/CosTokenValue.h
        src/common/CosString-Impl.h
        src/common/CosData.c
        src/common/CosData.h
        src/common/CosDataBuffer.c
        src/common/CosDataBuffer.h
        src/common/memory-support.c
        src/common/memory-support.h
        src/objects/CosObjRef.c
        src/objects/CosObjRef.h
        include/libcos/CosBoolObj.h
        src/objects/CosBoolObj.c
        src/objects/CosArrayObj.c
        include/libcos/CosArrayObj.h
        src/objects/CosObj-Impl.h
        src/objects/CosArrayObj-Impl.h
        src/objects/CosBoolObj-Impl.h
        include/libcos/CosNameObj.h
        src/objects/CosNameObj.c
        src/objects/CosNameObj-Impl.h
        src/objects/CosStringObj-Impl.h
        include/libcos/CosStringObj.h
        src/objects/CosStringObj.c
        src/common/CosHashTable.c
        src/common/CosHashTable.h
        src/lex/CosLexer.c
        src/lex/CosLexer.h
        src/io/CosInputStreamReader.c
        src/io/CosInputStreamReader.h
)

target_include_directories(libcos
        PUBLIC include/
        PRIVATE src/)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(libcos PRIVATE
            -Wall -Wextra
            -Werror=return-type -Werror=incompatible-pointer-types
            -fsanitize=address
            -fsanitize=undefined
            -fno-sanitize-recover=all
            -fsanitize=float-divide-by-zero
            -fsanitize=float-cast-overflow
            -fsanitize=null
            -fsanitize=alignment
    )
elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(libcos PRIVATE
            -Wall
            -Wextra
            -Weverything
            -Wint-conversion
            -Wno-padded -Wno-unused-parameter -Wno-unused-variable -Wno-nullability-extension
            -Werror=incompatible-pointer-types
            -Werror=return-type)
elseif (C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(libcos PRIVATE
            /W4 /WX)
endif ()

set_target_properties(libcos PROPERTIES
    PREFIX "" # Remove the "lib" prefix from the library name.
)

# Doxygen

find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs/api)

    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(libcos-docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)

endif (DOXYGEN_FOUND)
