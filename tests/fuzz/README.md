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

These are correctness bugs rather than crashes, so no fuzz target will catch
them: the parser quietly returns the wrong answer instead of falling over.
Both were found while fixing the obj-parser hang.

### Literal and hex strings never parse

`cos_parse_string_()` (`src/parse/CosObjParser.c`) guards with

    token->type != CosToken_Type_Literal_String ||
    token->type != CosToken_Type_Hex_String

which is true for every token, since no token is both. The operator wants to
be `&&`. Every string fails to parse, so `[ (s) ]` yields an array of no
elements rather than one.

### Three context checks are inverted

`cos_handle_bool_()`, `cos_handle_null_()` and `cos_handle_real_()` reject the
object when the context *allows* it:

    if (cos_parser_context_allows_(context, CosObjParserFlag_RealObj)) {
        ... "Invalid real object" ...
    }

`cos_handle_indirect_ref_()` and `cos_handle_indirect_def_()` have the `!` that
these are missing. The effect is silent data loss wherever the type is legal:
an array element context allows all the direct types, so `[ 3.14 ]` and
`[ null ]` both come back empty.

Worth settling what the context flags are for at the same time. They are only
consulted by five of the handlers -- dictionaries, arrays, names and integers
ignore them entirely -- so the top-level context of `CosObjParserFlag_Indirect-
ObjDef` is not enforced today. Correcting the polarity without deciding that
question would start rejecting bare top-level values that currently parse.

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
