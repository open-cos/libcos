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
    COS_STRICT_GROUP_COUNT = 5,
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
} CosStrictGroup;

/**
 * Controls how strictly the parser judges its input.
 *
 * Every function that accepts these options copies them, so an options value
 * does not need to outlive the parser or tokenizer it is passed to.
 */
typedef struct CosParserOptions {
    /**
     * The level for each group, indexed by @c CosStrictGroup .
     */
    CosStrictLevel strict_levels[COS_STRICT_GROUP_COUNT];
} CosParserOptions;

/**
 * Returns the default options, which report every group at
 * @c CosStrictLevel_Warn .
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

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_PARSE_COS_PARSER_OPTIONS_H */
