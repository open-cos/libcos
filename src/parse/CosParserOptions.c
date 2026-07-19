/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "libcos/parse/CosParserOptions.h"

#include "common/Assert.h"
#include "parse/CosParserOptions-Private.h"

#include <libcos/common/CosDefines.h>

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

/*
 * Pins the group count to the enumeration so that adding a group without
 * bumping the count (or the reverse) is a compile error rather than an
 * out-of-bounds read. C99 has no _Static_assert, hence the array-size idiom.
 */
typedef char cos_strict_group_count_matches_enum_
    [(COS_STRICT_GROUP_COUNT == (CosStrictGroup_UndefinedRefs + 1)) ? 1 : -1];

CosParserOptions
cos_parser_options_make_default(void)
{
    CosParserOptions options;

    for (unsigned int i = 0; i < COS_STRICT_GROUP_COUNT; i++) {
        options.strict_levels[i] = CosStrictLevel_Warn;
    }

    return options;
}

void
cos_parser_options_set_strict_level(CosParserOptions *options,
                                    CosStrictGroup group,
                                    CosStrictLevel level)
{
    COS_API_PARAM_CHECK(options != NULL);
    if (COS_UNLIKELY(!options)) {
        return;
    }

    // The enumeration is public, so an out-of-range group is caller error
    // rather than a broken internal invariant. The cast folds a negative
    // value into the same check.
    COS_API_PARAM_CHECK((unsigned int)group < (unsigned int)COS_STRICT_GROUP_COUNT);
    if (COS_UNLIKELY((unsigned int)group >= (unsigned int)COS_STRICT_GROUP_COUNT)) {
        return;
    }

    options->strict_levels[(unsigned int)group] = level;
}

CosStrictLevel
cos_parser_options_get_strict_level(const CosParserOptions *options,
                                    CosStrictGroup group)
{
    COS_API_PARAM_CHECK(options != NULL);
    if (COS_UNLIKELY(!options)) {
        return CosStrictLevel_Off;
    }

    COS_API_PARAM_CHECK((unsigned int)group < (unsigned int)COS_STRICT_GROUP_COUNT);
    if (COS_UNLIKELY((unsigned int)group >= (unsigned int)COS_STRICT_GROUP_COUNT)) {
        return CosStrictLevel_Off;
    }

    return options->strict_levels[(unsigned int)group];
}

CosParserOptions
cos_parser_options_resolve_(const CosParserOptions * COS_Nullable options)
{
    if (!options) {
        return cos_parser_options_make_default();
    }

    return *options;
}

COS_ASSUME_NONNULL_END
