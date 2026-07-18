# The engine-agnostic structure-aware mutator.
#
# The mutation logic lives here, free of any fuzzing-engine symbol, because the
# two engines take a custom mutator in incompatible ways: libFuzzer links
# LLVMFuzzerCustomMutator into the harness, while AFL++ dlopens a shared object
# exporting afl_custom_fuzz. Each engine gets a thin adapter under adapters/.
#
# This builds in every configuration, not just a fuzzing one, so that its unit
# test runs as part of the ordinary test suite.

add_library(libcos-fuzz-mutator STATIC
    CosMutator.c
    CosMutator.h
    CosMutator-Private.h
    CosMutatorBuffer.c
    CosMutatorBuffer.h
    CosMutatorLex.c
    CosMutatorLex.h
    CosMutatorOps.c
    CosMutatorRandom.c
    CosMutatorRandom.h
    CosMutatorRepair.c
    CosMutatorRepair.h
)

target_compile_features(libcos-fuzz-mutator PRIVATE
    c_std_99
)

# Deliberately NOT linked against libcos.
#
# libcos attaches its sanitizer and coverage flags PUBLIC, so linking it here
# would instrument the mutator too, and SanitizerCoverage would then count the
# mutator's own edges as target coverage -- filling the corpus with inputs that
# differ only in which mutation path ran. Keeping the dependency off also leaves
# the future AFL++ module loadable by a non-instrumented afl-fuzz.
#
# The include paths below are header-only: the mutator uses CosDefines.h for the
# project's attribute macros and Assert.h for the parameter checks. Those checks
# do call into libcos (cos_assertion_failure_), which resolves because every
# consumer -- the harnesses and libcos-test -- links libcos itself. A future
# standalone AFL module will have to supply its own definitions.
target_include_directories(libcos-fuzz-mutator
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${PROJECT_SOURCE_DIR}/include/ ${PROJECT_SOURCE_DIR}/src/
)

target_compile_definitions(libcos-fuzz-mutator PRIVATE
    ${COS_COMPILE_DEFINITIONS}
    $<$<COMPILE_LANGUAGE:C>:${COS_C_COMPILE_DEFINITIONS}>
)

target_compile_options(libcos-fuzz-mutator PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${COS_C_FLAGS_WARNINGS}>
)

# So that the AFL++ module, when it is added, can link this into a shared object.
set_target_properties(libcos-fuzz-mutator PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
