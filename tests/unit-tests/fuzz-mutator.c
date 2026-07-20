/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "CosTest.h"

#include <CosMutator.h>
#include <CosMutatorLex.h>

#include <libcos/CosDoc.h>
#include <libcos/CosParser.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

/**
 * Inputs that between them cover every branch of the lexer, including the ones
 * a well-formed file never reaches.
 */
static const char * COS_Nonnull const test_inputs_[] = {
    "",
    "x",
    "%PDF-1.7\n1 0 obj\n<< /Type /Catalog >>\nendobj\n",
    "<< /A 1 /B [2 3] /C (str) /D <AB> >>",
    "1 0 obj\n<< /Length 5 >>\nstream\nHELLO\nendstream\nendobj\n",
    "xref\n0 2\n0000000000 65535 f \n0000000009 00000 n \ntrailer\n<< >>\n",
    "startxref\n186\n%%EOF\n",
    "(unterminated",
    "(nested (parens (deep))) rest",
    "(escaped \\) paren)",
    "(trailing backslash \\",
    "<abcdef",
    "<>",
    "<<<<<<<<<<",
    ">>>>>>>>>>",
    ">",
    "/",
    "/#41#42",
    "/name-with-#-escape",
    "[[[[[[[[[[",
    "]]]]]]]]]]",
    "{}{}{}",
    "12345 -678 +9 .5 1.2.3 -- ++",
    "true false null R obj endobj n f",
    "stream\nno terminator here",
    "stream",
    "%comment with no newline",
    "%comment\nafter",
    "\r\n\t\f \0 mixed whitespace",
    "\xff\xfe\xfd\xfc",
    "trailer<</Size 4/Root 1 0 R>>startxref 0 %%EOF",
};

#define TEST_INPUT_COUNT (sizeof(test_inputs_) / sizeof(test_inputs_[0]))

/**
 * Asserts the tiling invariant: spans are contiguous, non-empty, strictly
 * increasing, and together cover exactly the input.
 *
 * Every operator indexes the buffer through these offsets, so a violation here
 * is an out-of-bounds access waiting to happen -- and one that would be
 * reported against libcos rather than against the mutator.
 */
static int
lex_spansTileInput_HoldsForAllInputs(void)
{
    static CosMutSpan spans[COS_MUT_MAX_SPANS];

    for (size_t i = 0; i < TEST_INPUT_COUNT; i++) {
        const unsigned char * const data =
            (const unsigned char *)test_inputs_[i];
        const size_t size = strlen(test_inputs_[i]);

        const size_t count = cos_mut_lex_scan_(data, size, spans,
                                               COS_MUT_MAX_SPANS);

        if (size == 0) {
            TEST_EXPECT(count == 0);
            continue;
        }

        TEST_EXPECT(count > 0);
        TEST_EXPECT(spans[0].offset == 0);

        size_t total = 0;
        for (size_t j = 0; j < count; j++) {
            TEST_EXPECT(spans[j].length > 0);
            TEST_EXPECT(spans[j].offset == total);
            TEST_EXPECT((size_t)spans[j].offset + spans[j].length <= size);
            total += spans[j].length;
        }

        TEST_EXPECT(total == size);
    }

    return EXIT_SUCCESS;
}

/**
 * A tiny span array must still tile, via the reserved trailing Garbage span.
 */
static int
lex_spanArrayTooSmall_StillTilesInput(void)
{
    CosMutSpan spans[4];

    const char * const input = "1 0 obj << /A /B /C /D >> endobj";
    const size_t size = strlen(input);

    const size_t count = cos_mut_lex_scan_((const unsigned char *)input,
                                           size,
                                           spans,
                                           sizeof(spans) / sizeof(spans[0]));

    TEST_EXPECT(count > 0);
    TEST_EXPECT(count <= (sizeof(spans) / sizeof(spans[0])));

    size_t total = 0;
    for (size_t i = 0; i < count; i++) {
        TEST_EXPECT(spans[i].length > 0);
        TEST_EXPECT(spans[i].offset == total);
        total += spans[i].length;
    }

    TEST_EXPECT(total == size);

    return EXIT_SUCCESS;
}

/**
 * The same seed and input must give the same mutant, every time, in any
 * process. Without this, a crash found at -seed=N cannot be reproduced.
 */
static int
mutate_sameSeed_ProducesIdenticalOutput(void)
{
    CosMutator * const first = cos_mutator_create(CosMutatorTarget_Parser, 1);
    CosMutator * const second = cos_mutator_create(CosMutatorTarget_Parser, 1);

    TEST_EXPECT(first != NULL);
    TEST_EXPECT(second != NULL);

    const char * const input = test_inputs_[2];
    const size_t size = strlen(input);

    int result = EXIT_SUCCESS;

    for (unsigned int seed = 1; seed <= 200; seed++) {
        cos_mutator_reseed(first, seed);
        cos_mutator_reseed(second, seed);

        size_t first_size = 0;
        size_t second_size = 0;

        const unsigned char * const a =
            cos_mutator_mutate(first, (const unsigned char *)input, size,
                               NULL, 0, 4096, &first_size);
        const unsigned char * const b =
            cos_mutator_mutate(second, (const unsigned char *)input, size,
                               NULL, 0, 4096, &second_size);

        if ((a == NULL) != (b == NULL) || first_size != second_size) {
            result = EXIT_FAILURE;
            break;
        }
        if (a && b && memcmp(a, b, first_size) != 0) {
            result = EXIT_FAILURE;
            break;
        }
    }

    cos_mutator_destroy(first);
    cos_mutator_destroy(second);

    return result;
}

/**
 * The engines' max_size is a hard contract: exceeding it corrupts the fuzzer's
 * own buffer. Sweep sizes well below the input length, where the shrink path
 * makes size accounting easy to get wrong.
 */
static int
mutate_anyMaxSize_NeverExceedsIt(void)
{
    CosMutator * const mutator = cos_mutator_create(CosMutatorTarget_Parser, 7);
    TEST_EXPECT(mutator != NULL);

    int result = EXIT_SUCCESS;

    for (size_t i = 0; i < TEST_INPUT_COUNT && result == EXIT_SUCCESS; i++) {
        const unsigned char * const data =
            (const unsigned char *)test_inputs_[i];
        const size_t size = strlen(test_inputs_[i]);

        for (size_t max_size = 0; max_size <= (size * 2) + 8; max_size++) {
            for (unsigned int seed = 1; seed <= 8; seed++) {
                cos_mutator_reseed(mutator, seed);

                size_t out_size = 12345;
                const unsigned char * const mutant =
                    cos_mutator_mutate(mutator, data, size,
                                       NULL, 0, max_size, &out_size);

                if (out_size > max_size) {
                    result = EXIT_FAILURE;
                    break;
                }
                if (!mutant && out_size != 0) {
                    result = EXIT_FAILURE;
                    break;
                }
            }
            if (result != EXIT_SUCCESS) {
                break;
            }
        }
    }

    cos_mutator_destroy(mutator);

    return result;
}

/**
 * Degenerate and adversarial inputs must not crash the mutator. Under the fuzz
 * preset this test binary is sanitized, so this also serves as the
 * out-of-bounds check.
 */
static int
mutate_degenerateInput_DoesNotCrash(void)
{
    CosMutator * const mutator = cos_mutator_create(CosMutatorTarget_Parser, 3);
    TEST_EXPECT(mutator != NULL);

    /* Oversized and repetitive inputs the literal table cannot express. */
    static unsigned char big[64 * 1024];

    int result = EXIT_SUCCESS;

    for (unsigned int variant = 0; variant < 4 && result == EXIT_SUCCESS; variant++) {
        switch (variant) {
            case 0:
                memset(big, 0x00, sizeof(big));
                break;
            case 1:
                memset(big, 0xFF, sizeof(big));
                break;
            case 2:
                memset(big, '9', sizeof(big));
                break;
            default:
                memset(big, '[', sizeof(big));
                break;
        }

        for (unsigned int seed = 1; seed <= 20; seed++) {
            cos_mutator_reseed(mutator, seed);

            size_t out_size = 0;
            const unsigned char * const mutant =
                cos_mutator_mutate(mutator, big, sizeof(big),
                                   NULL, 0, sizeof(big) * 2, &out_size);

            if (mutant && out_size > (sizeof(big) * 2)) {
                result = EXIT_FAILURE;
                break;
            }
        }
    }

    /* Every literal input, including the empty one. */
    for (size_t i = 0; i < TEST_INPUT_COUNT && result == EXIT_SUCCESS; i++) {
        for (unsigned int seed = 1; seed <= 20; seed++) {
            cos_mutator_reseed(mutator, seed);

            size_t out_size = 0;
            (void)cos_mutator_mutate(mutator,
                                     (const unsigned char *)test_inputs_[i],
                                     strlen(test_inputs_[i]),
                                     NULL, 0, 8192, &out_size);
        }
    }

    cos_mutator_destroy(mutator);

    return result;
}

/**
 * Reads a seed from the committed fuzz corpus.
 */
static size_t
read_corpus_file_(const char *relative_path,
                  unsigned char *out_buffer,
                  size_t buffer_size)
{
    char path[1024];

    const int written = snprintf(path, sizeof(path), "%s/%s",
                                 COS_FUZZ_CORPUS_DIR, relative_path);
    if (written <= 0 || (size_t)written >= sizeof(path)) {
        return 0;
    }

    FILE * const file = fopen(path, "rb");
    if (!file) {
        return 0;
    }

    const size_t bytes_read = fread(out_buffer, 1, buffer_size, file);

    (void)fclose(file);

    return bytes_read;
}

/**
 * Runs the real parser over a mutant.
 */
static bool
parse_bytes_(const unsigned char *data,
             size_t size)
{
    CosMemoryStream * const stream = cos_memory_stream_create_readonly(data, size);
    if (!stream) {
        return false;
    }

    CosDoc * const doc = cos_doc_create();
    if (!doc) {
        cos_stream_close((CosStream *)stream);
        return false;
    }

    bool result = false;

    /* The document owns the parser; the parser only borrows the stream. */
    CosParser * const parser = cos_parser_create(doc, (CosStream *)stream, NULL);
    if (parser) {
        result = cos_parser_parse(parser, NULL);
    }

    cos_doc_destroy(doc);
    cos_stream_close((CosStream *)stream);

    return result;
}

/**
 * The claim this whole design rests on: with repair forced on, most mutants of
 * a well-formed file still get through all three phases of cos_parser_parse,
 * rather than dying at the startxref scan the way a byte-mutated file does.
 *
 * Asserted as a ratio rather than per-seed, so that retuning the operator
 * weights does not cause a spurious failure.
 */
static int
repair_forcedOnValidFile_MostMutantsStillParse(void)
{
    static unsigned char seed_data[8192];

    const size_t seed_size = read_corpus_file_("parser/xref-table.pdf",
                                               seed_data,
                                               sizeof(seed_data));
    TEST_EXPECT(seed_size > 0);

    /* The seed itself must parse, or the ratio below measures nothing. */
    TEST_EXPECT(parse_bytes_(seed_data, seed_size));

    unsigned int parsed[2] = {0, 0};
    unsigned int total[2] = {0, 0};

    for (unsigned int mode = 0; mode < 2; mode++) {
        CosMutator * const mutator = cos_mutator_create(CosMutatorTarget_Parser, 11);
        TEST_EXPECT(mutator != NULL);

        cos_mutator_set_repair_percent(mutator, (mode == 0) ? 0 : 100);

        for (unsigned int seed = 1; seed <= 200; seed++) {
            cos_mutator_reseed(mutator, seed);

            size_t out_size = 0;
            const unsigned char * const mutant =
                cos_mutator_mutate(mutator, seed_data, seed_size,
                                   NULL, 0, sizeof(seed_data), &out_size);
            if (!mutant || out_size == 0) {
                continue;
            }

            total[mode]++;
            if (parse_bytes_(mutant, out_size)) {
                parsed[mode]++;
            }
        }

        cos_mutator_destroy(mutator);
    }

    TEST_EXPECT(total[0] > 0);
    TEST_EXPECT(total[1] > 0);

    /*
     * The thresholds are deliberately loose. What is being asserted is the
     * design claim -- that repairing the tail is what gets mutants past phase 2
     * of cos_parser_parse -- not a particular operator mix, so retuning the
     * weights should not fail this test. At the time of writing the rates are
     * about 18% without repair and about 55% with it.
     */
    const bool repair_helps_a_lot = (parsed[1] >= (parsed[0] * 2));
    const bool repair_meets_floor = ((parsed[1] * 100u) >= (total[1] * 40u));

    if (!repair_helps_a_lot || !repair_meets_floor) {
        (void)fprintf(stderr,
                      "repair off: %u/%u parsed; repair on: %u/%u parsed "
                      "(want on >= 2x off, and on >= 40%%)\n",
                      parsed[0], total[0], parsed[1], total[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * With almost no headroom, repair must decline and restore rather than emit a
 * truncated file whose offsets no longer mean anything.
 */
static int
repair_noHeadroom_StaysWithinMaxSize(void)
{
    static unsigned char seed_data[8192];

    const size_t seed_size = read_corpus_file_("parser/xref-table.pdf",
                                               seed_data,
                                               sizeof(seed_data));
    TEST_EXPECT(seed_size > 0);

    CosMutator * const mutator = cos_mutator_create(CosMutatorTarget_Parser, 13);
    TEST_EXPECT(mutator != NULL);

    cos_mutator_set_repair_percent(mutator, 100);

    int result = EXIT_SUCCESS;

    for (size_t slack = 0; slack <= 8; slack++) {
        const size_t max_size = seed_size + slack;

        for (unsigned int seed = 1; seed <= 50; seed++) {
            cos_mutator_reseed(mutator, seed);

            size_t out_size = 0;
            (void)cos_mutator_mutate(mutator, seed_data, seed_size,
                                     NULL, 0, max_size, &out_size);

            if (out_size > max_size) {
                result = EXIT_FAILURE;
                break;
            }
        }

        if (result != EXIT_SUCCESS) {
            break;
        }
    }

    cos_mutator_destroy(mutator);

    return result;
}

TEST_MAIN()
{
    TEST_EXPECT(lex_spansTileInput_HoldsForAllInputs() == EXIT_SUCCESS);
    TEST_EXPECT(lex_spanArrayTooSmall_StillTilesInput() == EXIT_SUCCESS);

    TEST_EXPECT(mutate_sameSeed_ProducesIdenticalOutput() == EXIT_SUCCESS);
    TEST_EXPECT(mutate_anyMaxSize_NeverExceedsIt() == EXIT_SUCCESS);
    TEST_EXPECT(mutate_degenerateInput_DoesNotCrash() == EXIT_SUCCESS);

    TEST_EXPECT(repair_forcedOnValidFile_MostMutantsStillParse() == EXIT_SUCCESS);
    TEST_EXPECT(repair_noHeadroom_StaysWithinMaxSize() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
