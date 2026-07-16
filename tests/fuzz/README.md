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

### A trailer that is an indirect object is cast to a dictionary

Reachable from the `parser` target; SEGV on a near-null address:

    SUMMARY: AddressSanitizer: SEGV in cos_dict_hash_ CosDict.c
      #1 cos_dict_get
      #2 cos_dict_obj_node_get_value_with_string
      #3 cos_parser_parse_xref_and_trailer_ CosParser.c

`cos_parser_parse_xref_and_trailer_()` guards the cast with
`cos_obj_node_is_dict(trailer_obj)`, but that predicate reports the type of the
*referenced* object: `cos_obj_node_get_value_type()` forwards an
`CosObjNodeType_Indirect` node to `cos_indirect_obj_node_get_type()`. So an
indirect object wrapping a dictionary answers true, and the following
`(CosDictObjNode *)` cast reinterprets a `CosIndirectObjNode`. Reading
`->value` off it yields the object ID: for `1 0 obj` the two `unsigned int`s
read back as the pointer `0x1`.

The `is_*` predicates are a trap for any caller that casts on the answer. Worth
checking the other call sites, and considering whether "is or references a
dict" and "is a dict" should be separate questions.

### The obj-parser target fuzzes slowly

Roughly 4 exec/s once the corpus is seeded, against ~50k/s for the tokenizer,
so it explores very little. Individual seeds parse in single-digit
milliseconds, so it is specific inputs the mutator finds -- a large `/Length`
is the obvious suspect. `-max_len` bounds it somewhat. Unrelated to
correctness, but it limits what this target can find.

## Should assertions be on while fuzzing?

Currently no: `COS_BUILD_FOR_FUZZING` also defines `COS_DISABLE_ASSERTIONS=1`,
making every `COS_IMPL_PARAM_CHECK` a no-op, so only ASan-visible faults are
reported.

This is worth revisiting, because the SQLite precedent it appears to follow
actually points the other way. SQLite disables assertions for its *coverage*
build -- which is what `src/common/Assert.h` cites SQLite for, and models
correctly. For *fuzzing*, SQLite does the opposite: its OSS-Fuzz build script
compiles with `-DSQLITE_DEBUG=1` and never defines `NDEBUG`, so assertions are
on. Their rationale, from "How SQLite Is Tested": fuzzcheck "is looking for
crashes, assertion faults, and/or memory leaks", and "most of the findings from
AFL have been assert() statements where the conditional was false under obscure
circumstances".
