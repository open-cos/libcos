
include(Common)

option(SANITIZE_ADDRESS "Enable address sanitizer" OFF)
option(SANITIZE_MEMORY "Enable memory sanitizer" OFF)
option(SANITIZE_THREAD "Enable thread sanitizer" OFF)
option(SANITIZE_UNDEFINED "Enable undefined behavior sanitizer" OFF)

include(CMakeDependentOption)

cmake_dependent_option(UBSAN_CHECK_ALIGNMENT
    "Use of a misaligned pointer or creation of a misaligned reference" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_BOOL
    "Load of a bool value which is neither true nor false" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_BUILTIN
    "Passing invalid values to compiler builtins" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_BOUNDS
    "Out of bounds array indexing, in cases where the array bound can be statically determined" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_ENUM
    "Load of a value of an enumerated type which is not in the range of representable values for that enumerated type." OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_FLOAT_CAST_OVERFLOW
    "Conversion to, from, or between floating-point types which would overflow the destination." OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_FLOAT_DIVIDE_BY_ZERO
    "Division of a floating-point value by zero" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_FUNCTION
    "Indirect call of a function through a function pointer of the wrong type" OFF
    "SANITIZE_UNDEFINED" OFF
)


cmake_dependent_option(UBSAN_CHECK_IMPLICIT_SIGNED_INTEGER_TRUNCATION
    "Implicit conversion from signed integer of larger bit width to smaller bit width" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_IMPLICIT_UNSIGNED_INTEGER_TRUNCATION
    "Implicit conversion from unsigned integer of larger bit width to smaller bit width" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_IMPLICIT_INTEGER_TRUNCATION
    "Implicit conversion from integer of larger bit width to smaller bit width" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_IMPLICIT_INTEGER_SIGN_CHANGE
    "Implicit conversion between integer types, if that changes the sign of the value" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_INTEGER_DIVISION_BY_ZERO
    "Detect integer division by zero" OFF
    "SANITIZE_UNDEFINED" OFF
)

if (UBSAN_CHECK_IMPLICIT_INTEGER_TRUNCATION OR (
    UBSAN_CHECK_IMPLICIT_SIGNED_INTEGER_TRUNCATION AND
    UBSAN_CHECK_IMPLICIT_UNSIGNED_INTEGER_TRUNCATION))
    message(DEBUG "Adding implicit-integer-truncation check group (UBSAN)")
    add_flags(UBSAN_FLAGS -fsanitize=implicit-integer-truncation)
else ()
    if (UBSAN_CHECK_IMPLICIT_SIGNED_INTEGER_TRUNCATION)
        message(DEBUG "Adding implicit-signed-integer-truncation check (UBSAN)")
        add_flags(UBSAN_FLAGS -fsanitize=implicit-signed-integer-truncation)
    endif ()
    if (UBSAN_CHECK_IMPLICIT_UNSIGNED_INTEGER_TRUNCATION)
        message(DEBUG "Adding implicit-unsigned-integer-truncation check (UBSAN)")
        add_flags(UBSAN_FLAGS -fsanitize=implicit-unsigned-integer-truncation)
    endif ()
endif ()

cmake_dependent_option(UBSAN_CHECK_UNREACHABLE
    "Control flow reaches an unreachable program point" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_NULLABILITY_ARG
    "Passing null as a function parameter which is annotated with _Nonnull" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_NULLABILITY_ASSIGN
    "Assigning null to a variable which is annotated with _Nonnull" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_NULLABILITY_RETURN
    "Returning null from a function with a return type annotated with _Nonnull" OFF
    "SANITIZE_UNDEFINED" OFF
)

cmake_dependent_option(UBSAN_CHECK_NULLABILITY
    "Enable undefined behavior sanitizer nullability checks" OFF
    "SANITIZE_UNDEFINED" OFF
)

if (UBSAN_CHECK_UNREACHABLE)
    message(DEBUG "Adding unreachable check (UBSAN)")
    add_flags(UBSAN_FLAGS -fsanitize=unreachable)
endif ()

if (UBSAN_CHECK_NULLABILITY OR (
    UBSAN_CHECK_NULLABILITY_ARG AND
    UBSAN_CHECK_NULLABILITY_ASSIGN AND
    UBSAN_CHECK_NULLABILITY_RETURN))
    message(DEBUG "Adding nullability check group (UBSAN)")
    add_flags(UBSAN_FLAGS -fsanitize=nullability)
else ()
    if (UBSAN_CHECK_NULLABILITY_ARG)
        message(DEBUG "Adding nullability-arg check (UBSAN)")
        add_flags(UBSAN_FLAGS -fsanitize=nullability-arg)
    endif ()
    if (UBSAN_CHECK_NULLABILITY_ASSIGN)
        message(DEBUG "Adding nullability-assign check (UBSAN)")
        add_flags(UBSAN_FLAGS -fsanitize=nullability-assign)
    endif ()
    if (UBSAN_CHECK_NULLABILITY_RETURN)
        message(DEBUG "Adding nullability-return check (UBSAN)")
        add_flags(UBSAN_FLAGS -fsanitize=nullability-return)
    endif ()
endif ()

cmake_dependent_option(UBSAN_TRAP
    "Enable undefined behavior sanitizer traps" OFF
    "SANITIZE_UNDEFINED" OFF
)

if (UBSAN_TRAP)
    message(DEBUG "Adding trap (UBSAN)")
    add_flags(UBSAN_FLAGS -fsanitize-trap)
endif ()

include(CheckCompilerFlag)
include(CheckLinkerFlag)

function(sanitizer_flag SANITIZER VAR)
    string(TOLOWER ${SANITIZER} SANITIZER_NAME)
    set(${VAR} "-fsanitize=${SANITIZER_NAME}" PARENT_SCOPE)
endfunction()

function(check_sanitizer LANG SANITIZER VAR)
    sanitizer_flag(${SANITIZER} SANITIZER_FLAG)
    
    set(CMAKE_REQUIRED_FLAGS "${SANITIZER_FLAG}")
    set(CMAKE_REQUIRED_QUIET ON)
    check_compiler_flag(${LANG} ${SANITIZER_FLAG} COMPILER_HAS_SANITIZER)
    check_linker_flag(${LANG} ${SANITIZER_FLAG} LINKER_HAS_SANITIZER)
    
    if (COMPILER_HAS_SANITIZER AND LINKER_HAS_SANITIZER)
        set(${VAR} TRUE PARENT_SCOPE)
    else ()
        set(${VAR} FALSE PARENT_SCOPE)
    endif ()
endfunction()

function(enable_sanitizer TARGET SANITIZER VAR)
    check_sanitizer(C ${SANITIZER} HAS_SANITIZER)
    if (HAS_SANITIZER)
        message(STATUS "Enabling ${SANITIZER} sanitizer")
        
        string(TOUPPER ${SANITIZER} SANITIZER_UPPER)
        sanitizer_flag(${SANITIZER} SANITIZER_FLAG)
        
        target_compile_definitions(${TARGET} PRIVATE SANITIZE_${SANITIZER_UPPER})
        target_compile_options(${TARGET} PRIVATE ${SANITIZER_FLAG})
        target_link_options(${TARGET} INTERFACE ${SANITIZER_FLAG})
    else ()
        message(WARNING "${SANITIZER} sanitizer is not supported by the compiler")
    endif ()
    set(${VAR} ${HAS_SANITIZER} PARENT_SCOPE)
endfunction()

function(enable_address_sanitizer TARGET VAR)
    enable_sanitizer(${TARGET} address ${VAR})
endfunction()

function(enable_memory_sanitizer TARGET VAR)
    enable_sanitizer(${TARGET} memory ${VAR})
endfunction()

function(enable_thread_sanitizer TARGET VAR)
    enable_sanitizer(${TARGET} thread ${VAR})
endfunction()

function(enable_undefined_sanitizer TARGET VAR)
    enable_sanitizer(${TARGET} undefined ${VAR})
endfunction()

if (SANITIZE_ADDRESS AND SANITIZE_MEMORY)
    message(FATAL_ERROR "Address and memory sanitizers cannot be enabled at the same time")
endif ()

# Add all enabled sanitizers to target.
function(add_sanitizers TARGET)
    if (SANITIZE_ADDRESS)
        enable_address_sanitizer(${TARGET} HAS_ADDRESS_SANITIZER)
    endif ()
    if (SANITIZE_MEMORY)
        enable_memory_sanitizer(${TARGET} HAS_MEMORY_SANITIZER)
    endif ()
    if (SANITIZE_THREAD)
        enable_thread_sanitizer(${TARGET} HAS_THREAD_SANITIZER)
    endif ()
    if (SANITIZE_UNDEFINED)
        enable_undefined_sanitizer(${TARGET} HAS_UNDEFINED_SANITIZER)
    endif ()
endfunction()
