/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "filters/CosFilterFactory.h"

#include "common/Assert.h"

#include <libcos/common/CosError.h>
#include <libcos/common/CosString.h>
#include <libcos/filters/CosASCII85Filter.h>
#include <libcos/filters/CosASCIIHexFilter.h>
#include <libcos/filters/CosFlateFilter.h>
#include <libcos/filters/CosRunLengthFilter.h>

#include <string.h>

COS_ASSUME_NONNULL_BEGIN

// Private function prototypes

static bool
cos_filter_name_matches_(const CosString *name,
                         const char *full_name,
                         const char *abbreviation);

// Public function implementations

CosStream *
cos_filter_create_for_name_(const CosString *name,
                            const CosFilterOptions * COS_Nullable options,
                            CosError * COS_Nullable out_error)
{
    COS_IMPL_PARAM_CHECK(name != NULL);

    CosStream *filter = NULL;

    if (cos_filter_name_matches_(name, "FlateDecode", "Fl")) {
        filter = (CosStream *)cos_flate_filter_create();
    }
    else if (cos_filter_name_matches_(name, "ASCIIHexDecode", "AHx")) {
        filter = (CosStream *)cos_ascii_hex_filter_create();
    }
    else if (cos_filter_name_matches_(name, "ASCII85Decode", "A85")) {
        filter = (CosStream *)cos_ascii85_filter_create();
    }
    else if (cos_filter_name_matches_(name, "RunLengthDecode", "RL")) {
        filter = (CosStream *)cos_run_length_filter_create();
    }
    else {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_NOT_IMPLEMENTED,
                                           "Unsupported stream filter"),
                            out_error);
        return NULL;
    }

    if (COS_UNLIKELY(!filter)) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate filter"),
                            out_error);
        return NULL;
    }

    cos_filter_set_options_((CosFilter *)filter, options);

    return filter;
}

// Private function implementations

static bool
cos_filter_name_matches_(const CosString *name,
                         const char *full_name,
                         const char *abbreviation)
{
    COS_IMPL_PARAM_CHECK(name != NULL);

    const char * const data = cos_string_get_data(name);
    const size_t length = cos_string_get_length(name);
    if (!data) {
        return false;
    }

    const size_t full_length = strlen(full_name);
    if (length == full_length && memcmp(data, full_name, length) == 0) {
        return true;
    }

    const size_t abbreviation_length = strlen(abbreviation);
    if (length == abbreviation_length && memcmp(data, abbreviation, length) == 0) {
        return true;
    }

    return false;
}

COS_ASSUME_NONNULL_END
