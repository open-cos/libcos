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

# Apply fixes. Exit code is still 1 because warnings were emitted before
# being applied; we don't assert on it here.
run_tool \
    --annotation=PUBLIC_API_TEST \
    --header-filter='.*/fixture\.h$' \
    --fix \
    "$WORK/fixture.h" \
    -- \
    -x c-header

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

echo "public_api_fixer tests passed."
