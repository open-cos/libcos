/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "libcos/filters/CosASCII85Filter.h"

#include "common/Assert.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

struct CosASCII85FilterContext {
    unsigned char buffer[4];
};

// Private function prototypes
static size_t
cos_ascii85_filter_read_(CosStream *stream,
                         void *buffer,
                         size_t size,
                         CosError * COS_Nullable error);

static size_t
cos_ascii85_filter_write_(CosStream *stream,
                          const void *buffer,
                          size_t size,
                          CosError * COS_Nullable error);

static void
cos_ascii85_filter_close_(CosStream *stream);

// Public functions

CosASCII85Filter *
cos_ascii85_filter_create(void)
{
    CosASCII85Filter * const ascii_85_filter = calloc(1, sizeof(CosASCII85Filter));
    if (COS_UNLIKELY(!ascii_85_filter)) {
        goto failure;
    }

    if (COS_UNLIKELY(!cos_ascii85_filter_init(ascii_85_filter))) {
        goto failure;
    }

    return ascii_85_filter;

failure:
    if (ascii_85_filter) {
        free(ascii_85_filter);
    }
    return NULL;
}

bool
cos_ascii85_filter_init(CosASCII85Filter *ascii_85_filter)
{
    COS_API_PARAM_CHECK(ascii_85_filter != NULL);
    if (COS_UNLIKELY(!ascii_85_filter)) {
        return false;
    }

    cos_filter_init(&(ascii_85_filter->base),
                    &(CosStreamFunctions){
                        .read_func = cos_ascii85_filter_read_,
                        .write_func = cos_ascii85_filter_write_,
                        .close_func = cos_ascii85_filter_close_,
                    });

    CosASCII85FilterContext *context = calloc(1, sizeof(CosASCII85FilterContext));
    if (!context) {
        return false;
    }

    ascii_85_filter->context = context;

    return true;
}

// Function implementations

static size_t
cos_ascii85_filter_read_(CosStream *stream,
                         void *buffer,
                         size_t size,
                         CosError * COS_Nullable error)
{
    CosASCII85Filter *ascii_85_filter = (CosASCII85Filter *)stream;

    return 0;
}

static size_t
cos_ascii85_filter_write_(CosStream *stream,
                          const void *buffer,
                          size_t size,
                          CosError * COS_Nullable error)
{
    CosASCII85Filter *ascii_85_filter = (CosASCII85Filter *)stream;

    COS_ASSERT(false, "Not implemented yet");

    return 0;
}

static void
cos_ascii85_filter_close_(CosStream *stream)
{
    CosASCII85Filter *ascii_85_filter = (CosASCII85Filter *)stream;

    free(ascii_85_filter->context);

    // TODO: Call the base class's close function?
}

COS_ASSUME_NONNULL_END
