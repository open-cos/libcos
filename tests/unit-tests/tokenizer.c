/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/common/CosError.h>
#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>
#include <libcos/syntax/CosLimits.h>
#include <libcos/syntax/tokenizer/CosToken.h>
#include <libcos/syntax/tokenizer/CosTokenValue.h>
#include <libcos/syntax/tokenizer/CosTokenizer.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// MARK: - Helpers

/**
 * Creates a read-only memory stream and tokenizer over @p input, reads the
 * first @p count tokens into @p out_tokens, then destroys the tokenizer and
 * closes the stream.
 *
 * Returns @c true on success.  The caller is responsible for calling
 * @c cos_token_reset() on each populated token when finished with it.
 */
static bool
get_tokens_(const char *input,
            CosToken *out_tokens,
            size_t count)
{
    CosMemoryStream *stream = NULL;
    CosTokenizer *tokenizer = NULL;
    bool ok = false;

    stream = cos_memory_stream_create_readonly(input, strlen(input));
    if (!stream) {
        goto cleanup;
    }

    tokenizer = cos_tokenizer_create((CosStream *)stream, NULL);
    if (!tokenizer) {
        goto cleanup;
    }

    for (size_t i = 0; i < count; i++) {
        out_tokens[i] = (CosToken){0};
        if (!cos_tokenizer_get_next_token(tokenizer, &out_tokens[i], NULL)) {
            goto cleanup;
        }
    }

    ok = true;

cleanup:
    if (tokenizer) {
        cos_tokenizer_destroy(tokenizer);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
    return ok;
}

// MARK: - Token type tests

static int
tokenize_integer_HasCorrectTypeAndValue(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("42", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Integer);
    int value = 0;
    TEST_EXPECT(cos_token_get_integer_value(&tok, &value));
    TEST_EXPECT(value == 42);
    return EXIT_SUCCESS;
}

static int
tokenize_negativeInteger_HasCorrectValue(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("-7", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Integer);
    int value = 0;
    TEST_EXPECT(cos_token_get_integer_value(&tok, &value));
    TEST_EXPECT(value == -7);
    return EXIT_SUCCESS;
}

static int
tokenize_positiveSignedInteger_HasCorrectValue(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("+3", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Integer);
    int value = 0;
    TEST_EXPECT(cos_token_get_integer_value(&tok, &value));
    TEST_EXPECT(value == 3);
    return EXIT_SUCCESS;
}

static int
tokenize_negativeReal_HasCorrectValue(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("-1.5", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Real);
    double value = 0.0;
    TEST_EXPECT(cos_token_value_get_real_number(&tok.value, &value));
    TEST_EXPECT(value == -1.5);
    return EXIT_SUCCESS;
}

static int
tokenize_name_HasCorrectType(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("/Type", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Name);
    cos_token_reset(&tok);
    return EXIT_SUCCESS;
}

static int
tokenize_literalString_HasCorrectType(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("(hello)", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Literal_String);
    cos_token_reset(&tok);
    return EXIT_SUCCESS;
}

static int
tokenize_hexString_HasCorrectType(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("<48656C6C6F>", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Hex_String);
    cos_token_reset(&tok);
    return EXIT_SUCCESS;
}

static int
tokenize_arrayDelimiters_RecognizedCorrectly(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("[]", tokens, 2));
    TEST_EXPECT(tokens[0].type == CosToken_Type_ArrayStart);
    TEST_EXPECT(tokens[1].type == CosToken_Type_ArrayEnd);
    return EXIT_SUCCESS;
}

static int
tokenize_dictionaryDelimiters_RecognizedCorrectly(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("<<>>", tokens, 2));
    TEST_EXPECT(tokens[0].type == CosToken_Type_DictionaryStart);
    TEST_EXPECT(tokens[1].type == CosToken_Type_DictionaryEnd);
    return EXIT_SUCCESS;
}

static int
tokenize_trueKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("true", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_True);
    return EXIT_SUCCESS;
}

static int
tokenize_falseKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("false", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_False);
    return EXIT_SUCCESS;
}

static int
tokenize_nullKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("null", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Null);
    return EXIT_SUCCESS;
}

static int
tokenize_objKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("obj", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Obj);
    return EXIT_SUCCESS;
}

static int
tokenize_endObjKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("endobj", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_EndObj);
    return EXIT_SUCCESS;
}

static int
tokenize_rKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("R", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_R);
    return EXIT_SUCCESS;
}

static int
tokenize_xrefKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("xref", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_XRef);
    return EXIT_SUCCESS;
}

static int
tokenize_trailerKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("trailer", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Trailer);
    return EXIT_SUCCESS;
}

static int
tokenize_startxrefKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("startxref", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_StartXRef);
    return EXIT_SUCCESS;
}

static int
tokenize_nKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("n", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_N);
    return EXIT_SUCCESS;
}

static int
tokenize_fKeyword_RecognizedCorrectly(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("f", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_F);
    return EXIT_SUCCESS;
}

// MARK: - Integer range tests

static int
tokenize_intMax_IsInteger(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("2147483647", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Integer);
    int value = 0;
    TEST_EXPECT(cos_token_get_integer_value(&tok, &value));
    TEST_EXPECT(value == COS_INT_MAX);
    return EXIT_SUCCESS;
}

static int
tokenize_intMin_IsInteger(void)
{
    /*
     * The magnitude 2147483648 does not fit an int, so it is accumulated
     * unsigned and negated only once the sign is known.
     */
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("-2147483648", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Integer);
    int value = 0;
    TEST_EXPECT(cos_token_get_integer_value(&tok, &value));
    TEST_EXPECT(value == COS_INT_MIN);
    return EXIT_SUCCESS;
}

static int
tokenize_pastIntMax_IsLongInteger(void)
{
    /* Past COS_INT_MAX the token stays an integer; only the value widens. */
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("2147483648", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Integer);

    int narrow = 0;
    TEST_EXPECT(!cos_token_get_integer_value(&tok, &narrow));

    long long value = 0;
    TEST_EXPECT(cos_token_value_get_long_integer_number(&tok.value, &value));
    TEST_EXPECT(value == 2147483648LL);
    return EXIT_SUCCESS;
}

static int
tokenize_negativePastIntMin_IsLongInteger(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("-2147483649", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Integer);
    long long value = 0;
    TEST_EXPECT(cos_token_value_get_long_integer_number(&tok.value, &value));
    TEST_EXPECT(value == -2147483649LL);
    return EXIT_SUCCESS;
}

static int
tokenize_longLongMax_IsLongInteger(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("9223372036854775807", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Integer);
    long long value = 0;
    TEST_EXPECT(cos_token_value_get_long_integer_number(&tok.value, &value));
    TEST_EXPECT(value == 9223372036854775807LL);
    return EXIT_SUCCESS;
}

static int
tokenize_pastLongLongMax_IsReal(void)
{
    /*
     * Beyond the widest integer the value is reported as a real rather than
     * wrapping.  Precision is lost, which is why the comparison is
     * approximate.
     */
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("99999999999999999999", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Real);
    double value = 0.0;
    TEST_EXPECT(cos_token_value_get_real_number(&tok.value, &value));
    TEST_EXPECT(value > 9.9e19 && value < 1.1e20);
    return EXIT_SUCCESS;
}

static int
tokenize_negativePastLongLongMax_IsReal(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("-99999999999999999999", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Real);
    double value = 0.0;
    TEST_EXPECT(cos_token_value_get_real_number(&tok.value, &value));
    TEST_EXPECT(value < -9.9e19 && value > -1.1e20);
    return EXIT_SUCCESS;
}

static int
tokenize_leadingZeros_DoNotOverflow(void)
{
    /* Many digits, small value: digit count alone must not force a real. */
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("00000000000000000042", &tok, 1));
    TEST_EXPECT(tok.type == CosToken_Type_Integer);
    int value = 0;
    TEST_EXPECT(cos_token_get_integer_value(&tok, &value));
    TEST_EXPECT(value == 42);
    return EXIT_SUCCESS;
}

// MARK: - Bare delimiter tests

static int
tokenize_bareCurlyBrackets_MakeProgress(void)
{
    /*
     * "{" and "}" begin no token, but the tokenizer must still consume them:
     * a zero-length token would leave the stream position unchanged and hang
     * any caller that reads until EOF.
     */
    CosToken tokens[3] = {{0}, {0}, {0}};
    TEST_EXPECT(get_tokens_("{}", tokens, 3));
    TEST_EXPECT(tokens[0].type == CosToken_Type_Unknown);
    TEST_EXPECT(tokens[0].length == 1);
    TEST_EXPECT(tokens[1].type == CosToken_Type_Unknown);
    TEST_EXPECT(tokens[1].length == 1);
    TEST_EXPECT(tokens[2].type == CosToken_Type_EOF);
    return EXIT_SUCCESS;
}

static int
tokenize_bareRightParenthesis_MakesProgress(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_(")", tokens, 2));
    TEST_EXPECT(tokens[0].type == CosToken_Type_Unknown);
    TEST_EXPECT(tokens[0].length == 1);
    TEST_EXPECT(tokens[1].type == CosToken_Type_EOF);
    return EXIT_SUCCESS;
}

static int
tokenize_curlyBracketsAroundToken_Recognized(void)
{
    CosToken tokens[3] = {{0}, {0}, {0}};
    TEST_EXPECT(get_tokens_("{ 1 }", tokens, 3));
    TEST_EXPECT(tokens[0].type == CosToken_Type_Unknown);
    TEST_EXPECT(tokens[1].type == CosToken_Type_Integer);
    int value = 0;
    TEST_EXPECT(cos_token_get_integer_value(&tokens[1], &value));
    TEST_EXPECT(value == 1);
    TEST_EXPECT(tokens[2].type == CosToken_Type_Unknown);
    return EXIT_SUCCESS;
}

// MARK: - Offset and length tests

static int
tokenize_singleToken_OffsetIsZero(void)
{
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("99", &tok, 1));
    TEST_EXPECT(tok.offset == 0);
    TEST_EXPECT(tok.length == 2);
    return EXIT_SUCCESS;
}

static int
tokenize_secondToken_OffsetAccountsForWhitespace(void)
{
    /* "42 7": "42" is 2 bytes at offset 0; "7" starts at offset 3. */
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("42 7", tokens, 2));
    TEST_EXPECT(tokens[0].offset == 0);
    TEST_EXPECT(tokens[0].length == 2);
    TEST_EXPECT(tokens[1].offset == 3);
    TEST_EXPECT(tokens[1].length == 1);
    return EXIT_SUCCESS;
}

static int
tokenize_nameToken_OffsetAndLengthCorrect(void)
{
    /*
     * "/AB" — name terminated by EOF; length is exactly 3 (solidus + 2
     * letters), and the offset is 0.
     */
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("/AB", &tok, 1));
    TEST_EXPECT(tok.offset == 0);
    TEST_EXPECT(tok.length == 3);
    cos_token_reset(&tok);
    return EXIT_SUCCESS;
}

// MARK: - Leading-whitespace: basic counts

static int
whitespace_firstToken_ZeroCount(void)
{
    /* No bytes precede the first token. */
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("42", &tok, 1));
    TEST_EXPECT(tok.leading_whitespace.char_count == 0);
    TEST_EXPECT(!tok.leading_whitespace.has_comment);
    TEST_EXPECT(tok.leading_whitespace.bytes_count == 0);
    return EXIT_SUCCESS;
}

static int
whitespace_multipleSpaces_CharCountCorrect(void)
{
    /* Three spaces between the two tokens. */
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1   2", tokens, 2));
    TEST_EXPECT(tokens[1].leading_whitespace.char_count == 3);
    TEST_EXPECT(tokens[1].leading_whitespace.bytes_count == 3);
    return EXIT_SUCCESS;
}

static int
whitespace_bytesCapAtMax(void)
{
    /*
     * Ten spaces between tokens.  char_count reflects the true count (10),
     * but bytes[] only stores the first COS_TOKEN_WHITESPACE_MAX_BYTES (8).
     */
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1          2", tokens, 2)); /* 10 spaces */
    TEST_EXPECT(tokens[1].leading_whitespace.char_count == 10);
    TEST_EXPECT(tokens[1].leading_whitespace.bytes_count == COS_TOKEN_WHITESPACE_MAX_BYTES);
    return EXIT_SUCCESS;
}

// MARK: - Leading-whitespace: predicates

static int
whitespace_singleSpace_IsSingleSpace(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1 2", tokens, 2));
    TEST_EXPECT(cos_token_whitespace_is_single_space(&tokens[1].leading_whitespace));
    return EXIT_SUCCESS;
}

static int
whitespace_singleLineFeed_IsEol(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1\n2", tokens, 2));
    TEST_EXPECT(cos_token_whitespace_is_eol(&tokens[1].leading_whitespace));
    return EXIT_SUCCESS;
}

static int
whitespace_crLf_IsEol(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1\r\n2", tokens, 2));
    TEST_EXPECT(cos_token_whitespace_is_eol(&tokens[1].leading_whitespace));
    return EXIT_SUCCESS;
}

static int
whitespace_bareCr_IsBareCr(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1\r2", tokens, 2));
    TEST_EXPECT(cos_token_whitespace_is_bare_cr(&tokens[1].leading_whitespace));
    return EXIT_SUCCESS;
}

// MARK: - Leading-whitespace: comments

static int
whitespace_commentOnly_SetsHasComment(void)
{
    /* '%' immediately after the first token; no whitespace before it. */
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1%comment\n2", tokens, 2));
    TEST_EXPECT(tokens[1].leading_whitespace.has_comment);
    TEST_EXPECT(tokens[1].leading_whitespace.char_count == 0);
    TEST_EXPECT(tokens[1].leading_whitespace.bytes_count == 0);
    return EXIT_SUCCESS;
}

static int
whitespace_spaceBeforeComment_BytesCapturedBeforeComment(void)
{
    /*
     * "1 %comment\n2": one SP precedes the '%', which is captured in bytes[].
     * The comment sets has_comment.  char_count counts only the SP (the EOL
     * that ends the comment is consumed inside skip_comment and not recorded).
     */
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1 %comment\n2", tokens, 2));
    TEST_EXPECT(tokens[1].leading_whitespace.has_comment);
    TEST_EXPECT(tokens[1].leading_whitespace.char_count == 1);
    TEST_EXPECT(tokens[1].leading_whitespace.bytes_count == 1);
    TEST_EXPECT(tokens[1].leading_whitespace.bytes[0] == 0x20);
    return EXIT_SUCCESS;
}

// MARK: - Leading-whitespace: predicate cross-checks

static int
whitespace_eolPredicate_ReturnsFalseForSingleSpace(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1 2", tokens, 2));
    TEST_EXPECT(!cos_token_whitespace_is_eol(&tokens[1].leading_whitespace));
    return EXIT_SUCCESS;
}

static int
whitespace_singleSpacePredicate_ReturnsFalseForLineFeed(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1\n2", tokens, 2));
    TEST_EXPECT(!cos_token_whitespace_is_single_space(&tokens[1].leading_whitespace));
    return EXIT_SUCCESS;
}

static int
whitespace_bareCrPredicate_ReturnsFalseForCrLf(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1\r\n2", tokens, 2));
    TEST_EXPECT(!cos_token_whitespace_is_bare_cr(&tokens[1].leading_whitespace));
    return EXIT_SUCCESS;
}

static int
whitespace_multipleSpaces_NotSingleSpace(void)
{
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("1   2", tokens, 2));
    TEST_EXPECT(!cos_token_whitespace_is_single_space(&tokens[1].leading_whitespace));
    return EXIT_SUCCESS;
}

// MARK: - Strict mode tests

/**
 * Tokenizes @p input with the number-syntax group set to @p level .
 *
 * A rejected number does not make @c cos_tokenizer_get_next_token() fail: the
 * tokenizer's convention is to yield an Unknown token and propagate the error,
 * so the token type is what distinguishes the levels.
 *
 * @param input The NUL-terminated input.
 * @param level The level to apply to @c CosStrictGroup_NumberSyntax .
 * @param out_type Receives the type of the first token.
 * @param out_had_error Receives whether an error was propagated.
 *
 * @return @c true if the fixture was built and a token was read.
 */
static bool
tokenize_number_at_level_(const char *input,
                          CosStrictLevel level,
                          CosToken_Type *out_type,
                          bool *out_had_error)
{
    CosMemoryStream *stream = NULL;
    CosTokenizer *tokenizer = NULL;
    bool built = false;

    *out_type = CosToken_Type_Unknown;
    *out_had_error = false;

    stream = cos_memory_stream_create_readonly(input, strlen(input));
    if (!stream) {
        goto cleanup;
    }

    CosParserOptions options = cos_parser_options_make_default();
    cos_parser_options_set_strict_level(&options,
                                        CosStrictGroup_NumberSyntax,
                                        level);

    tokenizer = cos_tokenizer_create((CosStream *)stream, &options);
    if (!tokenizer) {
        goto cleanup;
    }

    CosToken token = {0};
    CosError error = cos_error_none();
    if (!cos_tokenizer_get_next_token(tokenizer, &token, &error)) {
        goto cleanup;
    }

    *out_type = token.type;
    *out_had_error = (error.code != COS_ERROR_NONE);
    cos_token_reset(&token);
    built = true;

cleanup:
    if (tokenizer) {
        cos_tokenizer_destroy(tokenizer);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
    return built;
}

/**
 * A real with more fractional digits than COS_REAL_MAX_SIG_FRAC_DIG.
 *
 * This branch was unreachable before the strict-mode options existed, so its
 * behaviour is verified here rather than assumed.
 */
static int
strict_tooManyFractionalDigits_HonoursLevel(void)
{
    // COS_REAL_MAX_SIG_FRAC_DIG is 5; this has 8.
    const char * const input = "1.12345678";
    CosToken_Type type = CosToken_Type_Unknown;
    bool had_error = false;

    // Off and Warn both accept the number; only the reporting differs.
    TEST_EXPECT(tokenize_number_at_level_(input, CosStrictLevel_Off, &type, &had_error));
    TEST_EXPECT(type == CosToken_Type_Real);
    TEST_EXPECT(!had_error);

    TEST_EXPECT(tokenize_number_at_level_(input, CosStrictLevel_Warn, &type, &had_error));
    TEST_EXPECT(type == CosToken_Type_Real);
    TEST_EXPECT(!had_error);

    // Error rejects it: the number does not become a Real token.
    TEST_EXPECT(tokenize_number_at_level_(input, CosStrictLevel_Error, &type, &had_error));
    TEST_EXPECT(type == CosToken_Type_Unknown);
    TEST_EXPECT(had_error);

    return EXIT_SUCCESS;
}

/**
 * A real within the digit limit must not trip the check at any level.
 */
static int
strict_fractionalDigitsWithinLimit_AlwaysAccepted(void)
{
    const char * const input = "1.12345";
    CosToken_Type type = CosToken_Type_Unknown;
    bool had_error = false;

    TEST_EXPECT(tokenize_number_at_level_(input, CosStrictLevel_Error, &type, &had_error));
    TEST_EXPECT(type == CosToken_Type_Real);
    TEST_EXPECT(!had_error);

    return EXIT_SUCCESS;
}

/**
 * Reads the first token of @p input with the number-signs group at @p level .
 */
static bool
tokenize_signs_at_level_(const char *input,
                         CosStrictLevel level,
                         CosToken_Type *out_type,
                         double *out_real,
                         int *out_int)
{
    CosMemoryStream *stream = NULL;
    CosTokenizer *tokenizer = NULL;
    bool built = false;

    *out_type = CosToken_Type_Unknown;
    *out_real = 0.0;
    *out_int = 0;

    stream = cos_memory_stream_create_readonly(input, strlen(input));
    if (!stream) {
        goto cleanup;
    }

    CosParserOptions options = cos_parser_options_make_default();
    cos_parser_options_set_strict_level(&options,
                                        CosStrictGroup_NumberSigns,
                                        level);
    // Keep the fractional-digit check out of the way of these inputs.
    cos_parser_options_set_strict_level(&options,
                                        CosStrictGroup_NumberSyntax,
                                        CosStrictLevel_Off);

    tokenizer = cos_tokenizer_create((CosStream *)stream, &options);
    if (!tokenizer) {
        goto cleanup;
    }

    CosToken token = {0};
    if (!cos_tokenizer_get_next_token(tokenizer, &token, NULL)) {
        goto cleanup;
    }

    *out_type = token.type;
    (void)cos_token_value_get_real_number(&token.value, out_real);
    (void)cos_token_get_integer_value(&token, out_int);
    cos_token_reset(&token);
    built = true;

cleanup:
    if (tokenizer) {
        cos_tokenizer_destroy(tokenizer);
    }
    if (stream) {
        cos_stream_close((CosStream *)stream);
    }
    return built;
}

/**
 * An interior sign is skipped and the digits run together.
 *
 * Matches PDFBox, which special-cases these exact shapes (PDFBOX-5829 has
 * "-12.-1"). Note this differs from Ghostscript, which stops at the sign.
 */
static int
signs_interiorSign_DigitsRunTogether(void)
{
    CosToken_Type type = CosToken_Type_Unknown;
    double real = 0.0;
    int integer = 0;

    TEST_EXPECT(tokenize_signs_at_level_("1.2-3", CosStrictLevel_Off, &type, &real, &integer));
    TEST_EXPECT(type == CosToken_Type_Real);
    TEST_EXPECT(real > 1.2299 && real < 1.2301);

    TEST_EXPECT(tokenize_signs_at_level_("-12.-1", CosStrictLevel_Off, &type, &real, &integer));
    TEST_EXPECT(type == CosToken_Type_Real);
    TEST_EXPECT(real < -12.0999 && real > -12.1001);

    return EXIT_SUCCESS;
}

/**
 * With nothing but zeros before it, the interior sign is the number's sign.
 *
 * PDFBOX-2990 has "0.00000-33917698", which means a small negative number.
 */
static int
signs_beforeSignificantDigit_SetsTheSign(void)
{
    CosToken_Type type = CosToken_Type_Unknown;
    double real = 0.0;
    int integer = 0;

    TEST_EXPECT(tokenize_signs_at_level_("0.00000-33917698",
                                         CosStrictLevel_Off,
                                         &type, &real, &integer));
    TEST_EXPECT(type == CosToken_Type_Real);
    TEST_EXPECT(real < 0.0);

    // A repeated leading sign, as in PDFBOX-4289's "--16.33".
    TEST_EXPECT(tokenize_signs_at_level_("--16.33", CosStrictLevel_Off, &type, &real, &integer));
    TEST_EXPECT(type == CosToken_Type_Real);
    TEST_EXPECT(real < -16.32 && real > -16.34);

    return EXIT_SUCCESS;
}

/**
 * At Error the sign terminates the number, which is the pre-existing behaviour.
 */
static int
signs_strictLevel_TerminatesTheNumber(void)
{
    CosToken_Type type = CosToken_Type_Unknown;
    double real = 0.0;
    int integer = 0;

    TEST_EXPECT(tokenize_signs_at_level_("1.2-3", CosStrictLevel_Error, &type, &real, &integer));
    TEST_EXPECT(type == CosToken_Type_Real);
    TEST_EXPECT(real > 1.1999 && real < 1.2001);

    return EXIT_SUCCESS;
}

/**
 * Well-formed numbers are unaffected at every level.
 */
static int
signs_wellFormedNumbers_Unaffected(void)
{
    CosToken_Type type = CosToken_Type_Unknown;
    double real = 0.0;
    int integer = 0;

    for (int level = CosStrictLevel_Off; level <= CosStrictLevel_Error; level++) {
        TEST_EXPECT(tokenize_signs_at_level_("-7", (CosStrictLevel)level, &type, &real, &integer));
        TEST_EXPECT(type == CosToken_Type_Integer);
        TEST_EXPECT(integer == -7);

        TEST_EXPECT(tokenize_signs_at_level_("-1.5", (CosStrictLevel)level, &type, &real, &integer));
        TEST_EXPECT(type == CosToken_Type_Real);
        TEST_EXPECT(real < -1.49 && real > -1.51);
    }

    return EXIT_SUCCESS;
}

// MARK: - Test driver

TEST_MAIN()
{
    /* Token type tests */
    TEST_EXPECT(tokenize_integer_HasCorrectTypeAndValue() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_negativeInteger_HasCorrectValue() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_positiveSignedInteger_HasCorrectValue() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_negativeReal_HasCorrectValue() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_name_HasCorrectType() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_literalString_HasCorrectType() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_hexString_HasCorrectType() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_arrayDelimiters_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_dictionaryDelimiters_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_trueKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_falseKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_nullKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_objKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_endObjKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_rKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_xrefKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_trailerKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_startxrefKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_nKeyword_RecognizedCorrectly() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_fKeyword_RecognizedCorrectly() == EXIT_SUCCESS);

    /* Integer range tests */
    TEST_EXPECT(tokenize_intMax_IsInteger() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_intMin_IsInteger() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_pastIntMax_IsLongInteger() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_negativePastIntMin_IsLongInteger() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_longLongMax_IsLongInteger() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_pastLongLongMax_IsReal() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_negativePastLongLongMax_IsReal() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_leadingZeros_DoNotOverflow() == EXIT_SUCCESS);

    /* Bare delimiter tests */
    TEST_EXPECT(tokenize_bareCurlyBrackets_MakeProgress() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_bareRightParenthesis_MakesProgress() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_curlyBracketsAroundToken_Recognized() == EXIT_SUCCESS);

    /* Offset and length tests */
    TEST_EXPECT(tokenize_singleToken_OffsetIsZero() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_secondToken_OffsetAccountsForWhitespace() == EXIT_SUCCESS);
    TEST_EXPECT(tokenize_nameToken_OffsetAndLengthCorrect() == EXIT_SUCCESS);

    /* Leading-whitespace: basic counts */
    TEST_EXPECT(whitespace_firstToken_ZeroCount() == EXIT_SUCCESS);
    TEST_EXPECT(whitespace_multipleSpaces_CharCountCorrect() == EXIT_SUCCESS);
    TEST_EXPECT(whitespace_bytesCapAtMax() == EXIT_SUCCESS);

    /* Leading-whitespace: predicates */
    TEST_EXPECT(whitespace_singleSpace_IsSingleSpace() == EXIT_SUCCESS);
    TEST_EXPECT(whitespace_singleLineFeed_IsEol() == EXIT_SUCCESS);
    TEST_EXPECT(whitespace_crLf_IsEol() == EXIT_SUCCESS);
    TEST_EXPECT(whitespace_bareCr_IsBareCr() == EXIT_SUCCESS);

    /* Leading-whitespace: comments */
    TEST_EXPECT(whitespace_commentOnly_SetsHasComment() == EXIT_SUCCESS);
    TEST_EXPECT(whitespace_spaceBeforeComment_BytesCapturedBeforeComment() == EXIT_SUCCESS);

    /* Leading-whitespace: predicate cross-checks */
    TEST_EXPECT(whitespace_eolPredicate_ReturnsFalseForSingleSpace() == EXIT_SUCCESS);
    TEST_EXPECT(whitespace_singleSpacePredicate_ReturnsFalseForLineFeed() == EXIT_SUCCESS);
    TEST_EXPECT(whitespace_bareCrPredicate_ReturnsFalseForCrLf() == EXIT_SUCCESS);
    TEST_EXPECT(whitespace_multipleSpaces_NotSingleSpace() == EXIT_SUCCESS);

    /* Strict mode */
    TEST_EXPECT(strict_tooManyFractionalDigits_HonoursLevel() == EXIT_SUCCESS);
    TEST_EXPECT(strict_fractionalDigitsWithinLimit_AlwaysAccepted() == EXIT_SUCCESS);

    /* Interior signs */
    TEST_EXPECT(signs_interiorSign_DigitsRunTogether() == EXIT_SUCCESS);
    TEST_EXPECT(signs_beforeSignificantDigit_SetsTheSign() == EXIT_SUCCESS);
    TEST_EXPECT(signs_strictLevel_TerminatesTheNumber() == EXIT_SUCCESS);
    TEST_EXPECT(signs_wellFormedNumbers_Unaffected() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
