/*
 * Copyright (c) 2026 OpenCOS.
 */

#include "libcos/parse/CosParserOptions.h"

#include "common/Assert.h"
#include "parse/CosParserOptions-Private.h"

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosDiagnosticHandler.h>
#include <libcos/common/CosError.h>

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

/*
 * Pins the group count to the enumeration so that adding a group without
 * bumping the count (or the reverse) is a compile error rather than an
 * out-of-bounds read. C99 has no _Static_assert, hence the array-size idiom.
 */
typedef char cos_strict_group_count_matches_enum_
    [(COS_STRICT_GROUP_COUNT == (CosStrictGroup_CompressedObjGen + 1)) ? 1 : -1];

CosParserOptions
cos_parser_options_make_default(void)
{
    CosParserOptions options;

    for (unsigned int i = 0; i < COS_STRICT_GROUP_COUNT; i++) {
        options.strict_levels[i] = CosStrictLevel_Warn;
    }

    options.interior_sign_behaviour = CosInteriorSignBehaviour_Merge;

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

void
cos_parser_options_set_interior_sign_behaviour(CosParserOptions *options,
                                               CosInteriorSignBehaviour behaviour)
{
    COS_API_PARAM_CHECK(options != NULL);
    if (COS_UNLIKELY(!options)) {
        return;
    }

    options->interior_sign_behaviour = behaviour;
}

CosInteriorSignBehaviour
cos_parser_options_get_interior_sign_behaviour(const CosParserOptions *options)
{
    COS_API_PARAM_CHECK(options != NULL);
    if (COS_UNLIKELY(!options)) {
        return CosInteriorSignBehaviour_Merge;
    }

    return options->interior_sign_behaviour;
}

bool
cos_options_report_(const CosParserOptions *options,
                    CosDiagnosticHandler * COS_Nullable handler,
                    CosStrictGroup group,
                    const char *message,
                    CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(options != NULL);
    COS_IMPL_PARAM_CHECK(message != NULL);
    if (!options || !message) {
        return true;
    }

    const CosStrictLevel level =
        cos_parser_options_get_strict_level(options, group);
    if (level == CosStrictLevel_Off) {
        return true;
    }

    cos_diagnose(handler ? handler : cos_diagnostic_handler_get_default(),
                 (level == CosStrictLevel_Error) ? CosDiagnosticLevel_Error
                                                 : CosDiagnosticLevel_Warning,
                 message);

    if (level == CosStrictLevel_Error) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_SYNTAX, message),
                            out_error);
        return false;
    }

    return true;
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
