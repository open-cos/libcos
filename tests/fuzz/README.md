# Fuzz targets

Four targets, each a `LLVMFuzzerTestOneInput` harness:

| Target | Source | Exercises |
|---|---|---|
| `libcos-fuzz-tokenizer` | `tokenizer.c` | `CosTokenizer` token scanning |
| `libcos-fuzz-obj-parser` | `obj-parser.c` | `CosObjParser` object syntax |
| `libcos-fuzz-parser` | `parser.c` | `CosParser`: header, startxref, xref, trailer, `/Prev` chain |
| `libcos-fuzz-filter` | `filter.c` | Decode filters (first input byte selects the filter) |

## Two build modes

The harnesses **build by default**, with every ordinary build:

```sh
cmake -B cmake-build-claude -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-claude
```

In this mode they link `CosFuzzDriver.c`, a plain `main()` that runs each file
named on the command line through the target once. No Clang, sanitizers, or
libFuzzer required. This exists so the harnesses cannot silently rot: they are
compiled against the real API on every build.

To actually fuzz, configure with instrumentation (Clang only; applies ASan and
coverage to the whole library):

```sh
cmake --preset fuzz -B cmake-build-fuzz
cmake --build cmake-build-fuzz
mkdir -p /tmp/cos-fuzz/parser
cmake-build-fuzz/tests/fuzz/libcos-fuzz-parser \
    -dict=tests/fuzz/dictionaries/cos.dict \
    /tmp/cos-fuzz/parser tests/fuzz/corpus/parser/
```

Note the scratch directory listed **first**: LibFuzzer writes newly discovered
inputs into the first corpus directory it is given, and reads the rest. Passing
`tests/fuzz/corpus/parser/` alone would dump thousands of generated files into
the repository.

`-DCOS_BUILD_FUZZERS=OFF` skips the targets entirely.

## Run targets and engines

An instrumented build adds a `fuzz_<target>` target per harness that launches a
session with the corpus, dictionary, and artifact paths already wired. The
engine is chosen at configure time by `COS_FUZZING_ENGINE`; the harness is
otherwise identical (one `LLVMFuzzerTestOneInput`, the engine linked in like
OSS-Fuzz's `$LIB_FUZZING_ENGINE`):

```sh
cmake --preset fuzz -B cmake-build-fuzz          # libFuzzer (default engine)
cmake --build cmake-build-fuzz --target fuzz_parser
```

Findings land under the build tree, per engine and target:

```
<build>/fuzzing/
  corpus/libfuzzer/<target>/   # libFuzzer live corpus
  corpus/afl/<target>/         # AFL output dir (queue/ crashes/ hangs/)
  crashes/libfuzzer/<target>/  # libFuzzer crash/timeout artifacts
```

These are build-tree artifacts, not committed. Promoting interesting finds back
into `corpus/<target>/` (corpus minimization / merge) is a manual, future step.

### AFL++

The `afl` preset builds the same harnesses with AFL++ instrumentation, via
`cmake/toolchains/afl.cmake` (which selects `afl-clang-lto`/`afl-clang-fast`):

```sh
cmake --preset afl -B cmake-build-afl
cmake --build cmake-build-afl --target fuzz_parser          # fresh run
cmake --build cmake-build-afl --target fuzz_parser_resume   # resume
```

The AFL `fuzz_<target>` and `fuzz_<target>_resume` targets appear only when
`afl-fuzz` is found. AFL manages its own `queue/`, `crashes/`, and `hangs/`
inside its output dir; do not pre-create it.

### Tuning

`FUZZ_TIMEOUT_S` / `FUZZ_MAX_TOTAL_TIME` (libFuzzer), `FUZZ_TIMEOUT_MS` (AFL),
and `FUZZ_DICT_PATH` are cache variables, e.g. `-DFUZZ_MAX_TOTAL_TIME=60` caps a
CI run.

## Corpus

`corpus/<target>/` holds seed inputs, replayed by CTest as `fuzz/<target>`:

```sh
ctest --test-dir cmake-build-claude -R '^fuzz/' --output-on-failure
```

This runs each seed once and exits -- a deterministic regression check, not a
fuzzing session. It is not a substitute for fuzzing; it is what catches a
harness that compiles but no longer works.

When fuzzing finds a crash, add the reproducer to the relevant corpus directory
so the replay test covers it from then on.

`dictionaries/cos.dict` lists COS keywords and common dictionary keys. Pass it
via `-dict=` as shown above; libFuzzer uses it to build inputs that reach past
the lexer.

## Open findings

Not yet fixed. Reproducers are deliberately **not** in `corpus/`, because the
replay test would fail. Add them when the fix lands.

None outstanding.

## Assertions while fuzzing

Yes: `COS_BUILD_FOR_FUZZING` leaves assertions on, so a failed
`COS_IMPL_PARAM_CHECK` aborts and libFuzzer reports it as a finding.

This follows SQLite. It disables assertions only for its *coverage* build --
which is what `src/common/Assert.h` cites SQLite for, and models correctly via
`COS_BUILD_COVERAGE`. For *fuzzing*, SQLite does the opposite: its OSS-Fuzz build
script compiles with `-DSQLITE_DEBUG=1` and never defines `NDEBUG`, so assertions
are on. Their rationale, from "How SQLite Is Tested": fuzzcheck "is looking for
crashes, assertion faults, and/or memory leaks", and "most of the findings from
AFL have been assert() statements where the conditional was false under obscure
circumstances".

Both `COS_ASSERT` (internal invariants) and `COS_IMPL_PARAM_CHECK` (internal
preconditions) abort, so libFuzzer surfaces either directly. Only
`COS_API_PARAM_CHECK` -- public-API argument validation -- logs and continues,
since the public function reports the error to its caller instead.
