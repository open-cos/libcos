cmake_minimum_required(VERSION 3.25)
project(libcos
        VERSION 0.0.0
        DESCRIPTION "The Carousel Object System (COS) library."
        HOMEPAGE_URL "https://github.com/open-cos/libcos"
        LANGUAGES C)

# CMake modules.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(libcos SHARED
        src/common/memory/CosRefCounter.c
        src/common/memory/CosRefCounter.h
        include/libcos/private/objects/CosNameObj-Impl.h
        src/common/CosClass.c
        include/libcos/common/CosClass.h
        src/common/CosArray.c
        include/libcos/common/CosArray.h
        include/libcos/common/CosObj.h
        src/common/CosObj.c
        include/libcos/private/common/CosObj-Impl.h
        include/libcos/private/common/CosClass-Impl.h
        include/libcos/common/CosDict.h
        src/common/CosDict.c
        src/common/CosDataRef.c
        include/libcos/common/CosDataRef.h)

target_sources(libcos PRIVATE
        src/CosDoc.c
        src/common/Assert.c
        src/common/Assert.h
        src/common/CharacterSet.c
        src/common/CharacterSet.h
        src/common/CosData.c
        include/libcos/common/CosData.h
        src/common/CosDataBuffer.c
        src/common/CosDataBuffer.h
        src/common/CosDiagnostic.c
        src/common/CosDiagnosticHandler.c
        src/common/CosError.c
        src/common/CosHashTable.c
        src/common/CosHashTable.h
        src/common/CosLog.c
        src/common/CosMacros.h
        src/common/CosString.c
        src/common/CosString.h
        src/common/CosVector.c
        src/common/CosVector.h
        src/common/memory-support.c
        src/common/memory-support.h
        src/io/CosBufferedInputStream.c
        src/io/CosBufferedInputStream.h
        src/io/CosFileInputStream.c
        src/io/CosInputStream.c
        src/io/CosInputStreamReader.c
        src/io/CosInputStreamReader.h
        src/io/CosMemoryInputStream.c
        src/io/string-support.c
        src/objects/CosArrayObj-Impl.h
        src/objects/CosArrayObj.c
        src/objects/CosBoolObj.c
        src/objects/CosDictObj.c
        src/objects/CosIndirectObj.c
        src/objects/CosIntegerObj.c
        src/objects/CosNameObj.c
        src/objects/CosNullObj.c
        src/objects/CosObj-Impl.h
        src/objects/CosBaseObj.c
        src/objects/CosObjID.c
        src/objects/CosObjRef.c
        src/objects/CosObjRef.h
        src/objects/CosObjectStorage.c
        src/objects/CosObjectStorage.h
        src/objects/CosRealObj.c
        src/objects/CosStringObj-Impl.h
        src/objects/CosStringObj.c
        src/parse/CosObjParser.c
        src/parse/CosObjParser.h
        src/parse/CosTokenizer.c
        src/parse/CosTokenizer.h
        src/parse/tokenizer/CosTokenValue.c
        src/parse/tokenizer/CosTokenValue.h
        src/syntax/CosSyntax.c
        src/syntax/CosSyntax.h
)

target_sources(libcos PUBLIC
        include/libcos/CosArrayObj.h
        include/libcos/CosBasicTypes.h
        include/libcos/CosBoolObj.h
        include/libcos/CosDoc.h
        include/libcos/CosIndirectObj.h
        include/libcos/CosIntegerObj.h
        include/libcos/CosNameObj.h
        include/libcos/CosNullObj.h
        include/libcos/CosBaseObj.h
        include/libcos/CosObjID.h
        include/libcos/CosRealObj.h
        include/libcos/CosStringObj.h
        include/libcos/common/CosDefines.h
        include/libcos/common/CosDiagnostic.h
        include/libcos/common/CosDiagnosticHandler.h
        include/libcos/common/CosError.h
        include/libcos/common/CosLog.h
        include/libcos/common/CosTypes.h
        include/libcos/io/CosFileInputStream.h
        include/libcos/io/CosInputStream.h
        include/libcos/io/string-support.h
        include/libcos/objects/CosDictObj.h
        include/libcos/private/objects/CosDictObj-Impl.h
        include/libcos/private/objects/CosBaseObj-Impl.h
)

target_include_directories(libcos
        PUBLIC include/
        PRIVATE src/)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(libcos PRIVATE
            -Wall -Wextra
            -Wno-unknown-pragmas
            -Werror=missing-declarations
            -Werror=return-type -Werror=incompatible-pointer-types
            -fanalyzer
    )

    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0)
        target_compile_options(libcos PRIVATE
                -Werror=enum-conversion
        )
    endif ()

elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(libcos PRIVATE
            -Wall
            -Wextra
            -Weverything
            -Wint-conversion
            -Wstrict-prototypes
#            -Werror=incompatible-function-pointer-types-strict
            -Wno-padded -Wno-unused-parameter -Wno-unused-variable -Wno-nullability-extension
            -Werror=implicit-int -Werror=implicit-function-declaration
            -Werror=strict-prototypes
            -Werror=missing-declarations -Werror=missing-variable-declarations
            -Werror=incompatible-pointer-types -Werror=missing-prototypes -Werror=return-type
#            -Werror=incompatible-function-pointer-types
            -Werror=nullability
    )
elseif (C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(libcos PRIVATE
            /W4 /WX)
endif ()

set_target_properties(libcos PROPERTIES
        PREFIX "" # Remove the "lib" prefix from the library name.
)

# Tests

enable_testing()

add_executable(libcos-tests
        tests/parse/CosTokenizer-Test.c
)

target_include_directories(libcos-tests
        PUBLIC include/
        PRIVATE src/
        PRIVATE tests/
)

target_link_libraries(libcos-tests libcos)

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
