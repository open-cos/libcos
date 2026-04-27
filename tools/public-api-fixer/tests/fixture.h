/*
 * Copyright (c) 2025 OpenCOS.
 *
 * Fixture exercised by tests/harness.sh. Each declaration is paired
 * with a comment stating whether the tool is expected to flag it.
 *
 * The annotation here expands to nothing on purpose — that matches the
 * default Unix expansion of COS_API and forces the tool to rely on
 * preprocessor-level expansion tracking rather than AST attributes.
 */

#ifndef PUBLIC_API_FIXER_FIXTURE_H
#define PUBLIC_API_FIXER_FIXTURE_H

#define PUBLIC_API_TEST

#ifdef __cplusplus
extern "C" {
#endif

/* Should be flagged: no annotation. */
int unannotated_function(int x);

/* Should NOT be flagged: annotation on the same line. */
PUBLIC_API_TEST int annotated_function(int x);

/* Should NOT be flagged: annotation on the line immediately above. */
PUBLIC_API_TEST
int multiline_annotated_function(int x);

/* Should NOT be flagged: annotation between docs and declaration. */
/** docs */
PUBLIC_API_TEST int doc_annotated_function(int x);

/* Should NOT be flagged: static linkage (excluded by the matcher). */
static int static_function(int x);

/* Should NOT be flagged: static inline definition. */
static inline int
static_inline_helper(int x)
{
    return x + 1;
}

/* Should be flagged: extern variable without annotation. */
extern int unannotated_var;

/* Should NOT be flagged: extern variable with annotation. */
PUBLIC_API_TEST extern int annotated_var;

#ifdef __cplusplus
}
#endif

#endif /* PUBLIC_API_FIXER_FIXTURE_H */
