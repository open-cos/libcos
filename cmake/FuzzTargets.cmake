# Fuzzing run targets and shared corpus/dictionary paths.
#
# Included from tests/fuzz/CMakeLists.txt. Defines cos_add_fuzz_run_targets(),
# which is called once per harness inside the fuzz-target loop. Run targets are
# created only for an instrumented fuzzing build (COS_BUILD_FOR_FUZZING=ON); the
# engine (COS_FUZZING_ENGINE) decides the launch command. All findings land
# under ${CMAKE_BINARY_DIR}/fuzzing/, which is ephemeral (the build dir is
# gitignored). Promoting finds back into the committed seeds is a future step.

# Tunables (overridable from the command line / CI).
set(FUZZ_TIMEOUT_MS "5000" CACHE STRING
    "AFL per-run timeout in milliseconds (afl-fuzz -t).")
set(FUZZ_TIMEOUT_S "5" CACHE STRING
    "libFuzzer per-run timeout in seconds (-timeout).")
set(FUZZ_MAX_TOTAL_TIME "0" CACHE STRING
    "libFuzzer total run time in seconds; 0 = unbounded (-max_total_time).")
set(FUZZ_DICT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dictionaries/cos.dict" CACHE FILEPATH
    "Keyword dictionary passed to both engines.")

# Committed, read-only seed inputs.
set(FUZZ_SEEDS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/corpus")

# Build-tree output roots.
set(FUZZ_CORPUS_DIR  "${CMAKE_BINARY_DIR}/fuzzing/corpus")
set(FUZZ_CRASHES_DIR "${CMAKE_BINARY_DIR}/fuzzing/crashes")

# AFL tooling (optional). afl-fuzz gates the afl run targets; AFL_DRIVER_LIBRARY,
# when present, gives persistent-mode driving (otherwise the standalone
# CosFuzzDriver.c main() is used, fed by AFL's @@ file argument).
find_program(AFL_FUZZ afl-fuzz)
find_library(AFL_DRIVER_LIBRARY NAMES AFLDriver aflpp_driver afl_driver)

# cos_mutator_target_enum(<out_var> <fuzz-target-name>) maps a harness name onto
# the CosMutatorTarget enumerator that selects its grammar surface.
function(cos_mutator_target_enum OUT_VAR FUZZ_TARGET)
    if (FUZZ_TARGET STREQUAL "parser")
        set(${OUT_VAR} "CosMutatorTarget_Parser" PARENT_SCOPE)
    elseif (FUZZ_TARGET STREQUAL "obj-parser")
        set(${OUT_VAR} "CosMutatorTarget_ObjParser" PARENT_SCOPE)
    elseif (FUZZ_TARGET STREQUAL "tokenizer")
        set(${OUT_VAR} "CosMutatorTarget_Tokenizer" PARENT_SCOPE)
    else ()
        set(${OUT_VAR} "CosMutatorTarget_Generic" PARENT_SCOPE)
    endif ()
endfunction()

# cos_add_fuzz_run_targets(<name>) defines fuzz_<name> (and, for AFL,
# fuzz_<name>_resume) for the harness libcos-fuzz-<name>.
function(cos_add_fuzz_run_targets FUZZ_TARGET)
    if (NOT COS_BUILD_FOR_FUZZING)
        return()
    endif ()

    set(target_name "libcos-fuzz-${FUZZ_TARGET}")
    set(seeds "${FUZZ_SEEDS_DIR}/${FUZZ_TARGET}")

    if (COS_FUZZING_ENGINE STREQUAL "libfuzzer")
        set(live_corpus "${FUZZ_CORPUS_DIR}/libfuzzer/${FUZZ_TARGET}")
        set(crashes "${FUZZ_CRASHES_DIR}/libfuzzer/${FUZZ_TARGET}")
        # libFuzzer tolerates pre-existing directories. It writes new inputs into
        # the first corpus dir and reads the seeds from the rest. The trailing
        # slash on -artifact_prefix makes it a directory, not a filename prefix.
        add_custom_target(fuzz_${FUZZ_TARGET}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${live_corpus}" "${crashes}"
            COMMAND $<TARGET_FILE:${target_name}>
                "${live_corpus}"
                "${seeds}"
                -dict=${FUZZ_DICT_PATH}
                -artifact_prefix=${crashes}/
                -timeout=${FUZZ_TIMEOUT_S}
                -max_total_time=${FUZZ_MAX_TOTAL_TIME}
                # libFuzzer turns length control off as soon as it finds a
                # custom mutator. That is the wrong default here: without it
                # inputs balloon (a 625Kb corpus against 233Kb) and throughput
                # drops roughly fourfold for slightly worse coverage. Harmless
                # when no custom mutator is linked.
                -len_control=100
            DEPENDS ${target_name}
            USES_TERMINAL
            VERBATIM
            COMMENT "Fuzzing ${target_name} with libFuzzer"
        )
    elseif (COS_FUZZING_ENGINE STREQUAL "afl")
        if (NOT AFL_FUZZ)
            message(STATUS "afl-fuzz not found; skipping run targets for ${target_name}.")
            return()
        endif ()
        set(afl_out "${FUZZ_CORPUS_DIR}/afl/${FUZZ_TARGET}")
        # Fresh run: AFL refuses to start if its -o dir already exists, so create
        # only the parent. AFL manages queue/ crashes/ hangs/ inside -o itself.
        add_custom_target(fuzz_${FUZZ_TARGET}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${FUZZ_CORPUS_DIR}/afl"
            COMMAND ${AFL_FUZZ}
                -i "${seeds}"
                -o "${afl_out}"
                -x ${FUZZ_DICT_PATH}
                -t ${FUZZ_TIMEOUT_MS}
                -- $<TARGET_FILE:${target_name}> @@
            DEPENDS ${target_name}
            USES_TERMINAL
            VERBATIM
            COMMENT "Fuzzing ${target_name} with AFL++ (fresh run)"
        )
        add_custom_target(fuzz_${FUZZ_TARGET}_resume
            COMMAND ${AFL_FUZZ}
                -i -
                -o "${afl_out}"
                -x ${FUZZ_DICT_PATH}
                -t ${FUZZ_TIMEOUT_MS}
                -- $<TARGET_FILE:${target_name}> @@
            DEPENDS ${target_name}
            USES_TERMINAL
            VERBATIM
            COMMENT "Resuming AFL++ fuzzing of ${target_name}"
        )
    endif ()
endfunction()
