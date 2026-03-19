/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"
#include "syntax/tokenizer/CosToken.h"
#include "syntax/tokenizer/CosTokenizer.h"

#include <libcos/io/CosMemoryStream.h>
#include <libcos/io/CosStream.h>

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

    tokenizer = cos_tokenizer_create((CosStream *)stream);
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

// MARK: - Offset and length tests

static int
tokenize_singleToken_OffsetIsZero(void)
{
    /*
     * "99 " — trailing space terminates the number cleanly (the space is
     * ungot), so length is 2 and offset is 0.
     */
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("99 ", &tok, 1));
    TEST_EXPECT(tok.offset == 0);
    TEST_EXPECT(tok.length == 2);
    return EXIT_SUCCESS;
}

static int
tokenize_secondToken_OffsetAccountsForWhitespace(void)
{
    /*
     * "42 7 ": "42" is 2 bytes at offset 0; "7" starts at offset 3.
     * The trailing space ensures "7" is terminated by unget rather than EOF,
     * giving it the correct length of 1.
     */
    CosToken tokens[2] = {{0}, {0}};
    TEST_EXPECT(get_tokens_("42 7 ", tokens, 2));
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
     * "/AB " — 4 bytes, name terminated by the trailing space.
     * The space is ungetted so the token length is exactly 3 (solidus + 2
     * letters), and the offset is 0.
     */
    CosToken tok = {0};
    TEST_EXPECT(get_tokens_("/AB ", &tok, 1));
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

// MARK: - Test driver

TEST_MAIN()
{
    /* Token type tests */
    TEST_EXPECT(tokenize_integer_HasCorrectTypeAndValue() == EXIT_SUCCESS);
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

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
