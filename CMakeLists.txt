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

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(libcos SHARED)

target_sources(libcos PRIVATE
        src/CosDoc.c
        src/common/Assert.c
        src/common/Assert.h
        src/common/CharacterSet.c
        src/common/CharacterSet.h
        src/common/CosArray.c
        src/common/CosData.c
        src/common/CosDataBuffer.c
        src/common/CosDataBuffer.h
        src/common/CosDataRef.c
        src/common/CosDiagnostic.c
        src/common/CosDiagnosticHandler.c
        src/common/CosDict.c
        src/common/CosError.c
        src/common/CosHashTable.c
        src/common/CosHashTable.h
        src/common/CosList.c
        src/common/CosLog.c
        src/common/CosMacros.h
        src/common/CosString.c
        src/common/CosUtils.c
        src/common/CosUtils.h
        src/common/CosVector.c
        src/common/CosVector.h
        src/common/memory-support.c
        src/common/memory-support.h
        src/common/memory/CosRefCounter.c
        src/common/memory/CosRefCounter.h
        src/io/CosBufferedInputStream.c
        src/io/CosBufferedInputStream.h
        src/io/CosFileInputStream.c
        src/io/CosInputStream.c
        src/io/CosInputStreamReader.c
        src/io/CosInputStreamReader.h
        src/io/CosMemoryInputStream.c
        src/io/string-support.c
        src/objects/CosArrayObj.c
        src/objects/CosBoolObj.c
        src/objects/CosDictObj.c
        src/objects/CosIndirectObj.c
        src/objects/CosIntObj.c
        src/objects/CosNameObj.c
        src/objects/CosNullObj.c
        src/objects/CosObj.c
        src/objects/CosObjID.c
        src/objects/CosRealObj.c
        src/objects/CosReferenceObj.c
        src/objects/CosStreamObj.c
        src/objects/CosStringObj.c
        src/parse/CosObjParser.c
        src/parse/CosObjParser.h
        src/parse/CosToken.c
        src/parse/CosTokenizer.c
        src/parse/CosTokenizer.h
        src/parse/tokenizer/CosTokenValue.c
        src/parse/tokenizer/CosTokenValue.h
        src/syntax/CosSyntax.c
        src/syntax/CosSyntax.h
)

target_sources(libcos PUBLIC
        include/libcos/CosBasicTypes.h
        include/libcos/CosDoc.h
        include/libcos/CosObjID.h
        include/libcos/CosObject.h
        include/libcos/common/CosArray.h
        include/libcos/common/CosData.h
        include/libcos/common/CosDataRef.h
        include/libcos/common/CosDefines.h
        include/libcos/common/CosDiagnostic.h
        include/libcos/common/CosDiagnosticHandler.h
        include/libcos/common/CosDict.h
        include/libcos/common/CosError.h
        include/libcos/common/CosList.h
        include/libcos/common/CosLog.h
        include/libcos/common/CosString.h
        include/libcos/common/CosTypes.h
        include/libcos/io/CosFileInputStream.h
        include/libcos/io/CosInputStream.h
        include/libcos/io/string-support.h
        include/libcos/objects/CosArrayObj.h
        include/libcos/objects/CosBoolObj.h
        include/libcos/objects/CosDictObj.h
        include/libcos/objects/CosIndirectObj.h
        include/libcos/objects/CosIntObj.h
        include/libcos/objects/CosNameObj.h
        include/libcos/objects/CosNullObj.h
        include/libcos/objects/CosObj.h
        include/libcos/objects/CosObjectTypes.h
        include/libcos/objects/CosRealObj.h
        include/libcos/objects/CosReferenceObj.h
        include/libcos/objects/CosStreamObj.h
        include/libcos/objects/CosStringObj.h
)

target_include_directories(libcos
        PUBLIC include/
        PRIVATE src/)

set(COS_C_FLAGS_WARNINGS "")
set(COS_COMPILE_DEFINITIONS "")

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    list(APPEND COS_C_FLAGS_WARNINGS
            -Wall -Wextra
            -Wno-unknown-pragmas
            -Werror=missing-declarations
            -Werror=implicit-function-declaration
            -Werror=return-type -Werror=incompatible-pointer-types
            -fanalyzer
    )

    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0)
        list(APPEND COS_C_FLAGS_WARNINGS
                -Werror=enum-conversion
        )
    endif ()

elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    list(APPEND COS_C_FLAGS_WARNINGS
            -Wall
            -Wextra
            -Weverything
            -Wint-conversion
            -Wstrict-prototypes
            #            -Werror=incompatible-function-pointer-types-strict
            -Wno-declaration-after-statement
            -Wno-padded -Wno-unused-parameter -Wno-unused-variable -Wno-nullability-extension
            -Wno-gcc-compat
            -Werror=implicit-int -Werror=implicit-function-declaration
            -Werror=enum-conversion
            -Werror=strict-prototypes
            -Werror=missing-declarations -Werror=missing-variable-declarations
            -Werror=incompatible-pointer-types -Werror=missing-prototypes -Werror=return-type
            -Werror=undefined-internal
            #            -Werror=incompatible-function-pointer-types
            -Werror=nullability
    )
elseif (C_COMPILER_ID STREQUAL "MSVC")
    list(APPEND COS_C_FLAGS_WARNINGS
            /W4 /WX)
endif ()

if (NOT MSVC)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND COS_COMPILE_DEFINITIONS
                _FORTIFY_SOURCE=1
        )
    else ()
        list(APPEND COS_COMPILE_DEFINITIONS
                _FORTIFY_SOURCE=2
        )
    endif ()
endif ()

target_compile_options(libcos PRIVATE ${COS_C_FLAGS_WARNINGS})
target_compile_definitions(libcos PRIVATE ${COS_COMPILE_DEFINITIONS})

set_target_properties(libcos PROPERTIES
        PREFIX "" # Remove the "lib" prefix from the library name.

        VERIFY_INTERFACE_HEADER_SETS TRUE
)

# Tests

enable_testing()

add_executable(libcos-tests
        tests/parse/CosTokenizer-Test.c
        tests/parse/CosObjParser-Test.c
)

target_include_directories(libcos-tests
        PUBLIC include/
        PRIVATE src/
        PRIVATE tests/
)

target_compile_options(libcos-tests PRIVATE ${COS_C_FLAGS_WARNINGS})

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
