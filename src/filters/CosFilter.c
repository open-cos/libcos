/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/filters/CosFilter.h"

#include "common/Assert.h"

COS_ASSUME_NONNULL_BEGIN

void
cos_filter_init(CosFilter *filter,
                const CosStreamFunctions *functions)
{
    COS_API_PARAM_CHECK(filter != NULL);
    COS_API_PARAM_CHECK(functions != NULL);
    if (COS_UNLIKELY(!filter || !functions)) {
        return;
    }

    cos_stream_init((CosStream *)filter,
                    functions);
    
    filter->source = NULL;
}

void
cos_filter_deinit(CosFilter *filter)
{
    COS_API_PARAM_CHECK(filter != NULL);
    if (COS_UNLIKELY(!filter)) {
        return;
    }

    CosStream *source = filter->source;
    if (source) {
        cos_stream_close(source);
    }
}

void
cos_filter_attach_source(CosFilter *filter,
                         CosStream *source)
{
    COS_API_PARAM_CHECK(filter != NULL);
    COS_API_PARAM_CHECK(source != NULL);
    if (COS_UNLIKELY(!filter || !source)) {
        return;
    }

    filter->source = source;
}

void
cos_filter_detach_source(CosFilter *filter)
{
    COS_API_PARAM_CHECK(filter != NULL);
    if (COS_UNLIKELY(!filter)) {
        return;
    }

    filter->source = NULL;
}

COS_ASSUME_NONNULL_END
