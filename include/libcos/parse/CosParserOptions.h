/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_PARSE_COS_PARSER_OPTIONS_H
#define LIBCOS_PARSE_COS_PARSER_OPTIONS_H

#include <libcos/common/CosAPI.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * The number of strict-mode groups.
 *
 * Declared separately from @c CosStrictGroup so that the enumeration contains
 * only values that are themselves valid groups.
 */
enum {
    COS_STRICT_GROUP_COUNT = 10,
};

/**
 * How a deviation from the specification is treated.
 */
typedef enum CosStrictLevel {
    /**
     * The check does not run and nothing is reported.
     */
    CosStrictLevel_Off,

    /**
     * A @c CosDiagnosticLevel_Warning is emitted and parsing continues.
     */
    CosStrictLevel_Warn,

    /**
     * A @c CosDiagnosticLevel_Error is emitted and the parse fails with
     * @c COS_ERROR_SYNTAX .
     */
    CosStrictLevel_Error,
} CosStrictLevel;

/**
 * A category of specification deviation, controlled independently of the others.
 */
typedef enum CosStrictGroup {
    /**
     * Numeric syntax, such as the number of significant fractional digits.
     */
    CosStrictGroup_NumberSyntax,

    /**
     * The whitespace separating the tokens of "N G R" and "N G obj".
     */
    CosStrictGroup_ObjHeaderSpacing,

    /**
     * The end-of-line markers required around @c stream , @c endstream and
     * @c endobj .
     */
    CosStrictGroup_EolMarkers,

    /**
     * A keyword that the specification requires is absent from the input.
     */
    CosStrictGroup_RequiredKeywords,

    /**
     * An indirect reference that does not resolve to an object.
     */
    CosStrictGroup_UndefinedRefs,

    /**
     * A minus sign in the middle of a number, such as "1.2-3" or "-12.-1".
     *
     * At @c CosStrictLevel_Off and @c CosStrictLevel_Warn the interior signs
     * are ignored and the digits are read as one number, matching Adobe and
     * PDFBox; at @c CosStrictLevel_Error the number is rejected.
     */
    CosStrictGroup_NumberSigns,

    /**
     * A "%PDF-" header that does not start at byte zero.
     *
     * Adobe searches the first 1024 bytes for the header rather than
     * requiring it at the start of the file.
     */
    CosStrictGroup_HeaderPosition,

    /**
     * A missing "%%EOF" marker at the end of the file.
     *
     * Adobe does not appear to require it; the startxref keyword is located
     * directly when it is absent.
     */
    CosStrictGroup_EofMarker,

    /**
     * A non-zero generation number on a reference to a compressed object.
     *
     * Objects in an object stream always have generation zero, and Adobe
     * ignores the generation number of a reference to one.
     */
    CosStrictGroup_CompressedObjGen,

    /**
     * A stream whose /Length is missing, non-integer, or does not agree with
     * the position of the endstream keyword.
     *
     * The specification requires /Length to give the exact byte count, but
     * Adobe and pdfium treat it as a hint and recover the true extent from the
     * endstream keyword. @c CosStreamLengthBehaviour selects when that recovery
     * runs; this group's level decides how the disagreement is handled: silently
     * fixed at @c CosStrictLevel_Off , adjusted with a warning at
     * @c CosStrictLevel_Warn , and rejected at @c CosStrictLevel_Error .
     */
    CosStrictGroup_StreamLength,
} CosStrictGroup;

/*
 * Behaviour selectors.
 *
 * A strict level answers "how loudly should this be reported"; it cannot say
 * what the parser should actually do when a construct has more than one
 * defensible reading. Where real implementations disagree, the reading is
 * chosen by a selector of its own, independent of the level:
 *
 *   - the selector decides which value the parser produces;
 *   - the group's @c CosStrictLevel decides whether that is silent, reported,
 *     or rejected outright.
 *
 * At @c CosStrictLevel_Error the construct is rejected whichever reading is
 * selected, so the two axes stay orthogonal.
 */

/**
 * How a sign that appears after a number's digits have started is read.
 *
 * Governed for reporting purposes by @c CosStrictGroup_NumberSigns .
 */
typedef enum CosInteriorSignBehaviour {
    /**
     * The number ends at the sign, and the remaining input is tokenized
     * separately: "1.2-3" is 1.2 followed by -3.
     *
     * This is what Ghostscript's @c pdfi_read_num does, and what libcos did
     * before the reading became selectable.
     */
    CosInteriorSignBehaviour_Terminate,

    /**
     * The sign is dropped and the digits run together, so "1.2-3" is 1.23 and
     * "-12.-1" is -12.1. If only zeros precede the sign it is taken as the
     * number's sign instead, so "0.00000-33917698" is negative and "--16.33"
     * is -16.33.
     *
     * This is what PDFBox's @c COSFloat does.
     */
    CosInteriorSignBehaviour_Merge,
} CosInteriorSignBehaviour;

/**
 * What the parser does with a stream's declared /Length.
 *
 * Governed for reporting purposes by @c CosStrictGroup_StreamLength .
 */
typedef enum CosStreamLengthBehaviour {
    /**
     * A present and non-negative /Length is used as-is. Recovery from the
     * endstream keyword runs only when /Length is missing or non-integer.
     *
     * This is what libcos did before recovery became selectable.
     */
    CosStreamLengthBehaviour_Trust,

    /**
     * The stream extent is always located from the endstream keyword, and a
     * present /Length that disagrees with it is overridden.
     *
     * This is what pdfium does; it catches a /Length that is present but wrong.
     */
    CosStreamLengthBehaviour_Verify,
} CosStreamLengthBehaviour;

/**
 * What the parser produces for an integer literal whose magnitude exceeds the
 * @c long @c long accumulator, i.e. is too large for any integer object.
 *
 * Governed for reporting purposes by @c CosStrictGroup_NumberSyntax .
 */
typedef enum CosIntOverflowBehaviour {
    /**
     * The value becomes a real, carrying on in floating point. Precision is
     * lost but no digits are dropped.
     *
     * This is what Adobe and poppler do, and what libcos did before the reading
     * became selectable.
     */
    CosIntOverflowBehaviour_PromoteToReal,

    /**
     * The value is clamped to the integer object range and stays an integer.
     */
    CosIntOverflowBehaviour_Clamp,
} CosIntOverflowBehaviour;

/**
 * Controls how strictly the parser judges its input, and which reading it
 * takes where implementations legitimately differ.
 *
 * Every function that accepts these options copies them, so an options value
 * does not need to outlive the parser or tokenizer it is passed to.
 */
typedef struct CosParserOptions {
    /**
     * The level for each group, indexed by @c CosStrictGroup .
     */
    CosStrictLevel strict_levels[COS_STRICT_GROUP_COUNT];

    /**
     * How a sign in the middle of a number is read.
     */
    CosInteriorSignBehaviour interior_sign_behaviour;

    /**
     * What the parser does with a stream's declared /Length.
     */
    CosStreamLengthBehaviour stream_length_behaviour;

    /**
     * What the parser produces for an integer that overflows the accumulator.
     */
    CosIntOverflowBehaviour int_overflow_behaviour;
} CosParserOptions;

/**
 * Returns the default options: every group at @c CosStrictLevel_Warn ,
 * @c CosInteriorSignBehaviour_Merge , @c CosStreamLengthBehaviour_Trust , and
 * @c CosIntOverflowBehaviour_PromoteToReal .
 *
 * These are the options used when a @c NULL options pointer is passed.
 *
 * @return The default options.
 */
COS_API CosParserOptions
cos_parser_options_make_default(void);

/**
 * Sets the level for a single group.
 *
 * @param options The options to modify.
 * @param group The group to set the level of.
 * @param level The level to apply to @p group .
 */
COS_API void
cos_parser_options_set_strict_level(CosParserOptions *options,
                                    CosStrictGroup group,
                                    CosStrictLevel level);

/**
 * Gets the level for a single group.
 *
 * @param options The options to query.
 * @param group The group to get the level of.
 *
 * @return The level of @p group, or @c CosStrictLevel_Off if @p group is not a
 * valid group.
 */
COS_API CosStrictLevel
cos_parser_options_get_strict_level(const CosParserOptions *options,
                                    CosStrictGroup group);

// MARK: - Behaviour selectors

/**
 * Sets how a sign in the middle of a number is read.
 *
 * @param options The options to modify.
 * @param behaviour The reading to take.
 */
COS_API void
cos_parser_options_set_interior_sign_behaviour(CosParserOptions *options,
                                               CosInteriorSignBehaviour behaviour);

/**
 * Gets how a sign in the middle of a number is read.
 *
 * @param options The options to query.
 *
 * @return The selected reading, or the default if @p options is @c NULL .
 */
COS_API CosInteriorSignBehaviour
cos_parser_options_get_interior_sign_behaviour(const CosParserOptions *options);

/**
 * Sets what the parser does with a stream's declared /Length.
 *
 * @param options The options to modify.
 * @param behaviour The behaviour to take.
 */
COS_API void
cos_parser_options_set_stream_length_behaviour(CosParserOptions *options,
                                               CosStreamLengthBehaviour behaviour);

/**
 * Gets what the parser does with a stream's declared /Length.
 *
 * @param options The options to query.
 *
 * @return The selected behaviour, or the default if @p options is @c NULL .
 */
COS_API CosStreamLengthBehaviour
cos_parser_options_get_stream_length_behaviour(const CosParserOptions *options);

/**
 * Sets what the parser produces for an integer that overflows the accumulator.
 *
 * @param options The options to modify.
 * @param behaviour The behaviour to take.
 */
COS_API void
cos_parser_options_set_int_overflow_behaviour(CosParserOptions *options,
                                              CosIntOverflowBehaviour behaviour);

/**
 * Gets what the parser produces for an integer that overflows the accumulator.
 *
 * @param options The options to query.
 *
 * @return The selected behaviour, or the default if @p options is @c NULL .
 */
COS_API CosIntOverflowBehaviour
cos_parser_options_get_int_overflow_behaviour(const CosParserOptions *options);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PARSE_COS_PARSER_OPTIONS_H */
