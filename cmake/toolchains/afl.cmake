# CMake toolchain file for building the fuzz harnesses with AFL++ instrumentation.
#
# AFL++ ships compiler wrappers (not a CMake toolchain), so this shim points
# CMake at them, preferring the LTO wrappers and falling back to the fast ones.
#
# afl-clang-lto gives the best instrumentation (collision-free coverage), but it
# hard-requires LLVM's lld linker: it drives the compile with --ld-path=<ld.lld>
# and fails to link if lld is absent. A wrapper being present on disk does not
# mean it can link, so gate the LTO preference on lld actually being installed;
# otherwise fall back to afl-clang-fast, which has no such dependency.
find_program(AFL_LLD NAMES ld.lld lld)

if (AFL_LLD)
    find_program(AFL_C_COMPILER   NAMES afl-clang-lto afl-clang-fast afl-cc)
    find_program(AFL_CXX_COMPILER NAMES afl-clang-lto++ afl-clang-fast++ afl-c++)
else ()
    find_program(AFL_C_COMPILER   NAMES afl-clang-fast afl-cc)
    find_program(AFL_CXX_COMPILER NAMES afl-clang-fast++ afl-c++)
endif ()

if (NOT AFL_C_COMPILER)
    message(FATAL_ERROR "AFL++ compiler (afl-clang-lto/afl-clang-fast) not found.")
endif ()

set(CMAKE_C_COMPILER   "${AFL_C_COMPILER}")
set(CMAKE_CXX_COMPILER "${AFL_CXX_COMPILER}")
