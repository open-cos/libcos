#!/usr/bin/env bash
#
# End-to-end harness for public_api_fixer.
#
# Usage:
#     harness.sh <path-to-public_api_fixer> <path-to-fixture.h>
#
# Exits 0 on success, 1 on assertion failure, 2 on bad arguments.

set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "usage: $0 <tool> <fixture>" >&2
    exit 2
fi

TOOL=$1
FIXTURE=$2

if [[ ! -x $TOOL ]]; then
    echo "harness: tool '$TOOL' is not executable" >&2
    exit 2
fi
if [[ ! -f $FIXTURE ]]; then
    echo "harness: fixture '$FIXTURE' not found" >&2
    exit 2
fi

# Globals populated by run_tool().
OUTPUT=""
EXIT=0

# Run the tool with the supplied args, capturing combined stdout/stderr into
# OUTPUT and the exit status into EXIT.
run_tool() {
    set +e
    OUTPUT=$("$TOOL" "$@" 2>&1)
    EXIT=$?
    set -e
}

assert_eq() {
    local expected=$1 actual=$2 context=$3
    if [[ $expected -ne $actual ]]; then
        printf 'FAIL: %s: expected exit %s, got %s\n----- output -----\n%s\n------------------\n' \
            "$context" "$expected" "$actual" "$OUTPUT" >&2
        exit 1
    fi
}

assert_contains() {
    local needle=$1 context=$2
    if [[ $OUTPUT != *"$needle"* ]]; then
        printf 'FAIL: %s: expected to find "%s" in output\n----- output -----\n%s\n------------------\n' \
            "$context" "$needle" "$OUTPUT" >&2
        exit 1
    fi
}

assert_not_contains() {
    local needle=$1 context=$2
    if [[ $OUTPUT == *"$needle"* ]]; then
        printf 'FAIL: %s: did NOT expect to find "%s" in output\n----- output -----\n%s\n------------------\n' \
            "$context" "$needle" "$OUTPUT" >&2
        exit 1
    fi
}

# ---------------------------------------------------------------------------
# Case 1: default behavior - flag unannotated decls, accept annotated ones.
# ---------------------------------------------------------------------------
run_tool \
    --annotation=PUBLIC_API_TEST \
    --header-filter='.*/fixture\.h$' \
    "$FIXTURE" \
    -- \
    -x c-header

assert_eq 1 "$EXIT" "case 1: warnings should produce exit 1"

for name in unannotated_function unannotated_var; do
    assert_contains "declaration $name is" "case 1"
done

for name in \
    annotated_function \
    multiline_annotated_function \
    doc_annotated_function \
    static_function \
    static_inline_helper \
    annotated_var
do
    assert_not_contains "declaration $name is" "case 1"
done

# ---------------------------------------------------------------------------
# Case 2: header-filter matching no header - exit cleanly with no warnings.
# ---------------------------------------------------------------------------
run_tool \
    --annotation=PUBLIC_API_TEST \
    --header-filter='.*/no_such_header_anywhere\.h$' \
    "$FIXTURE" \
    -- \
    -x c-header

assert_eq 0 "$EXIT" "case 2: non-matching filter should exit 0"
assert_not_contains "[public-api-fixer]" "case 2"

# ---------------------------------------------------------------------------
# Case 3: --annotation set to a name no symbol uses - every public decl is
# now reported, including ones the default run accepted.
# ---------------------------------------------------------------------------
run_tool \
    --annotation=NEVER_USED_MACRO \
    --header-filter='.*/fixture\.h$' \
    "$FIXTURE" \
    -- \
    -x c-header

assert_eq 1 "$EXIT" "case 3: warnings should produce exit 1"
assert_contains "declaration annotated_function is" "case 3"
assert_not_contains "declaration static_function is" "case 3"
assert_not_contains "declaration static_inline_helper is" "case 3"

# ---------------------------------------------------------------------------
# Case 4: --fix rewrites the header and a subsequent run is clean.
# ---------------------------------------------------------------------------
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cp "$FIXTURE" "$WORK/fixture.h"

# Apply fixes; expect a clean exit and a one-line summary instead of
# per-decl warnings.
run_tool \
    --annotation=PUBLIC_API_TEST \
    --header-filter='.*/fixture\.h$' \
    --fix \
    "$WORK/fixture.h" \
    -- \
    -x c-header
assert_eq 0 "$EXIT" "case 4: --fix should exit 0 on success"
assert_contains "applied 2 annotation" "case 4: --fix should print summary"
assert_not_contains "warning: public declaration" "case 4: --fix should suppress per-decl warnings"

# The previously-flagged decls should now wear the annotation.
if ! grep -q '^PUBLIC_API_TEST int unannotated_function' "$WORK/fixture.h"; then
    echo 'FAIL: case 4: PUBLIC_API_TEST not inserted before unannotated_function' >&2
    cat "$WORK/fixture.h" >&2
    exit 1
fi
if ! grep -q '^PUBLIC_API_TEST extern int unannotated_var' "$WORK/fixture.h"; then
    echo 'FAIL: case 4: PUBLIC_API_TEST not inserted before unannotated_var' >&2
    cat "$WORK/fixture.h" >&2
    exit 1
fi

# Re-run on the modified file: should now be clean.
run_tool \
    --annotation=PUBLIC_API_TEST \
    --header-filter='.*/fixture\.h$' \
    "$WORK/fixture.h" \
    -- \
    -x c-header
assert_eq 0 "$EXIT" "case 4: post-fix run should be clean"

# ---------------------------------------------------------------------------
# Case 5: --annotation-header inserts the supplying header once and is
# idempotent across re-runs.
# ---------------------------------------------------------------------------
WORK5=$(mktemp -d)
trap 'rm -rf "$WORK" "$WORK5"' EXIT
cp "$FIXTURE" "$WORK5/fixture.h"
# Stub header so the post-fix file still parses on re-run.
echo '/* stub */' > "$WORK5/test_annotation.h"

run_tool \
    --annotation=PUBLIC_API_TEST \
    --header-filter='.*/fixture\.h$' \
    --fix \
    --annotation-header=test_annotation.h \
    --include-style=quoted \
    "$WORK5/fixture.h" \
    -- \
    -x c-header \
    -I "$WORK5"
assert_eq 0 "$EXIT" "case 5: --fix with --annotation-header should exit 0"

count=$(grep -c '^#include "test_annotation.h"' "$WORK5/fixture.h")
if [[ $count -ne 1 ]]; then
    echo "FAIL: case 5: expected exactly one '#include \"test_annotation.h\"', got $count" >&2
    cat "$WORK5/fixture.h" >&2
    exit 1
fi

# Re-run with the same args; HeaderIncludes::insert returns nullopt when
# the spelling is already present, so no second include line should appear.
run_tool \
    --annotation=PUBLIC_API_TEST \
    --header-filter='.*/fixture\.h$' \
    --fix \
    --annotation-header=test_annotation.h \
    --include-style=quoted \
    "$WORK5/fixture.h" \
    -- \
    -x c-header \
    -I "$WORK5"
assert_eq 0 "$EXIT" "case 5: re-run should exit 0"

count=$(grep -c '^#include "test_annotation.h"' "$WORK5/fixture.h")
if [[ $count -ne 1 ]]; then
    echo "FAIL: case 5: re-run should not add a second include; got $count" >&2
    cat "$WORK5/fixture.h" >&2
    exit 1
fi

echo "public_api_fixer tests passed."
