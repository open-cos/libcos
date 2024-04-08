cmake_minimum_required(VERSION 3.13)
project(libcos
    VERSION 0.0.0
    DESCRIPTION "The Carousel Object System (COS) library."
    HOMEPAGE_URL "https://github.com/open-cos/libcos"
    LANGUAGES C)

# CMake modules.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(CheckLanguage)
include(CMakeDependentOption)

include(Common)

option(COS_BUILD_DOCUMENTATION "Build the documentation" OFF)

option(COS_BUILD_FOR_FUZZING "Build for fuzzing" OFF)

cmake_dependent_option(COS_DETERMINISTIC_FUZZING "Enable deterministic fuzzing" OFF
    "COS_BUILD_FOR_FUZZING" OFF
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

check_language(CXX)
if (CMAKE_CXX_COMPILER)
    enable_language(CXX)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()

set(COS_COMPILE_DEFINITIONS "")

set(COS_C_FLAGS "")
set(COS_C_FLAGS_WARNINGS "")
set(COS_C_COMPILE_DEFINITIONS "")

set(COS_CXX_FLAGS "")
set(COS_CXX_FLAGS_WARNINGS "")
set(COS_CXX_COMPILE_DEFINITIONS "")

set(COS_LINKER_FLAGS "")

if (COS_BUILD_FOR_FUZZING)
    
    if (NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR "Fuzzing is only supported with Clang.")
    else ()
        message(STATUS "Building for fuzzing with LibFuzzer.")
    endif ()
    
    if (CMAKE_C_COMPILER_VERSION VERSION_LESS "6.0.0")
        message(FATAL_ERROR "Clang ≥ 6.0.0 is required for fuzzing with LibFuzzer.")
    endif ()
    
    add_flags(COS_COMPILE_DEFINITIONS
        COS_BUILD_FOR_FUZZING=1
        COS_DISABLE_ASSERTIONS=1
    )
    
    if (COS_DETERMINISTIC_FUZZING)
        add_flags(COS_COMPILE_DEFINITIONS
            COS_DETERMINISTIC_FUZZING=1
            FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION=1
        )
    endif ()
    
    add_flags(COS_C_FLAGS
        -fsanitize=address,fuzzer-no-link
        -fsanitize-coverage=edge,indirect-calls
    )
    add_flags(COS_CXX_FLAGS
        -fsanitize=address,fuzzer-no-link
        -fsanitize-coverage=edge,indirect-calls
    )
    
    add_flags(COS_LINKER_FLAGS
        -fsanitize=fuzzer
    )

endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(libcos SHARED
    src/dummy.c

)

target_include_directories(libcos
    PUBLIC include/
    PRIVATE src/
)

target_sources(libcos PRIVATE
    src/CosDoc.c
    src/common/Assert.c
    src/common/Assert.h
    src/common/CharacterSet.c
    src/common/CharacterSet.h
    src/common/CosArray.c
    src/common/CosData.c
    src/common/CosDataRef.c
    src/common/CosDiagnostic.c
    src/common/CosDiagnosticHandler.c
    src/common/CosDict.c
    src/common/CosError.c
    src/common/CosList.c
    src/common/CosLog.c
    src/common/CosRingBuffer.c
    src/common/CosScanner.c
    src/common/CosString.c
    src/common/CosTypedArray.h
    src/common/CosUtils.c
    src/common/CosUtils.h
    src/common/CosVector.c
    src/common/CosVector.h
    src/common/memory-support.c
    src/common/memory-support.h
    src/common/memory/CosAllocator.c
    src/common/memory/CosMemory.c
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
    src/parse/CosTokenizer.c
    src/parse/CosTokenizer.h
    src/parse/tokenizer/CosTokenEntry.c
    src/parse/tokenizer/CosTokenEntry.h
    src/parse/tokenizer/CosTokenValue.c
    src/parse/tokenizer/CosTokenValue.h
    src/syntax/CosKeywords.c
    src/xref/CosXrefEntry.c
    src/xref/CosXrefSection.c
    src/xref/CosXrefSubsection.c
    src/xref/CosXrefTable.c
    src/xref/CosXrefTableParser.c
)

target_sources(libcos PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include/libcos
    FILES
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
    include/libcos/common/CosMacros.h
    include/libcos/common/CosRingBuffer.h
    include/libcos/common/CosScanner.h
    include/libcos/common/CosString.h
    include/libcos/common/CosTypes.h
    include/libcos/common/memory/CosAllocator.h
    include/libcos/common/memory/CosMemory.h
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
    include/libcos/syntax/CosKeywords.h
    include/libcos/syntax/CosLimits.h
    include/libcos/xref/CosXrefEntry.h
    include/libcos/xref/CosXrefSection.h
    include/libcos/xref/CosXrefSubsection.h
    include/libcos/xref/CosXrefTable.h
    include/libcos/xref/CosXrefTableParser.h
)

include(Sanitizers)
add_sanitizers(libcos)

# libcos++

add_library(libcos++ INTERFACE
    include/libcos++/objects/ArrayObj.hpp
    include/libcos++/impl/objects/ArrayObj.inl
)

set_target_properties(libcos++ PROPERTIES
    FOLDER include/libcos++
)

target_include_directories(libcos++ INTERFACE
    include/
)

target_sources(libcos++ INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include/libcos++
    FILES
    include/libcos++/Doc.hpp
    include/libcos++/common/Exceptions.hpp
    include/libcos++/impl/Doc.inl
    include/libcos++/impl/common/Exceptions.inl
    include/libcos++/impl/objects/NameObj.inl
    include/libcos++/impl/objects/Obj.inl
    include/libcos++/impl/objects/StreamObj.inl
    include/libcos++/objects/NameObj.hpp
    include/libcos++/objects/Obj.hpp
    include/libcos++/objects/StreamObj.hpp
)

add_library(cbind INTERFACE
    include/cbind/common/unique.hpp)

target_sources(cbind INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include/cbind
    FILES
    include/cbind/ownership/Ownable.hpp
)

set_target_properties(cbind PROPERTIES
    FOLDER include/cbind
)

target_include_directories(cbind INTERFACE
    include/
)

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

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    list(APPEND COS_C_FLAGS_WARNINGS
        -Wall -Wextra
        -Wno-unknown-pragmas
        -Werror=missing-declarations
        -Werror=missing-prototypes
        -Werror=implicit-function-declaration
        -Werror=return-type -Werror=incompatible-pointer-types
        -fanalyzer
    )
    
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 3.4)
        list(APPEND COS_C_FLAGS_WARNINGS
            -ftrapv
        )
    endif ()
    
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
        -Wno-declaration-after-statement
        -Wno-padded -Wno-unused-parameter -Wno-unused-variable -Wno-nullability-extension
        -Wno-gcc-compat
        -Werror=implicit-int -Werror=implicit-function-declaration
        -Werror=enum-conversion
        -Werror=strict-prototypes
        -Werror=missing-declarations -Werror=missing-variable-declarations
        -Werror=incompatible-pointer-types -Werror=missing-prototypes -Werror=return-type
        -Werror=undefined-internal
        -Werror=incompatible-function-pointer-types-strict
        #-Werror=incompatible-function-pointer-types
        -Werror=nullability
        
        #        -g -O1 -fsanitize=address -fno-omit-frame-pointer
        
        -g
        
#        -fsanitize=address
#
#        -fsanitize=undefined
#        -fsanitize=float-divide-by-zero,unsigned-integer-overflow,implicit-conversion,local-bounds,nullability
        
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
    )
    add_flags(COS_LINKER_FLAGS
#        -fsanitize=address,undefined
    )
elseif (C_COMPILER_ID STREQUAL "MSVC")
    list(APPEND COS_C_FLAGS_WARNINGS
        /W4 /WX)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND COS_CXX_FLAGS_WARNINGS
    
    )

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND COS_CXX_FLAGS_WARNINGS
        -Wno-gcc-compat
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND COS_CXX_FLAGS_WARNINGS
        /W4 /WX)
endif ()

target_compile_definitions(libcos PRIVATE
    ${COS_COMPILE_DEFINITIONS}
    ${COS_C_COMPILE_DEFINITIONS}
)
target_compile_options(libcos PRIVATE
    ${COS_C_FLAGS_WARNINGS}
)
target_link_options(libcos PUBLIC
    ${COS_LINKER_FLAGS}
)

target_compile_definitions(cbind INTERFACE
    ${COS_COMPILE_DEFINITIONS}
    ${COS_C_COMPILE_DEFINITIONS}
    ${COS_CXX_COMPILE_DEFINITIONS}
)

target_compile_options(cbind INTERFACE
    ${COS_C_FLAGS_WARNINGS}
    ${COS_CXX_FLAGS_WARNINGS}
)

target_compile_definitions(libcos++ INTERFACE
    ${COS_COMPILE_DEFINITIONS}
    ${COS_CXX_COMPILE_DEFINITIONS}
)
target_compile_options(libcos++ INTERFACE
    ${COS_C_FLAGS_WARNINGS}
    ${COS_CXX_FLAGS_WARNINGS}
)

set_target_properties(libcos cbind libcos++ PROPERTIES
    PREFIX "" # Remove the "lib" prefix from the library name.
    
    VERIFY_INTERFACE_HEADER_SETS TRUE
)

if (MSVC)
    set_target_properties(libcos PROPERTIES
        # This is required to create a symbol file (.lib) in addition to the
        # shared library (.dll).
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    )
endif ()

target_link_libraries(libcos++ INTERFACE
    libcos cbind
)

add_subdirectory(tests)

if (COS_BUILD_DOCUMENTATION)
    add_subdirectory(docs)
endif (COS_BUILD_DOCUMENTATION)
