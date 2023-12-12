//
// Created by david on 24/05/23.
//

#include "CosString.h"

#include "common/Assert.h"

#include <libcos/io/string-support.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

CosString *
cos_string_alloc(void)
{
    CosString * const string = malloc(sizeof(CosString));
    if (!string) {
        return NULL;
    }

    string->data = NULL;
    string->size = 0;

    return string;
}

CosString *
cos_string_alloc_with_str(const char *str)
{
    if (!str) {
        return NULL;
    }

    return cos_string_alloc_with_strn(str,
                                      strlen(str));
}

CosString *
cos_string_alloc_with_strn(const char *str,
                           size_t n)
{
    if (!str) {
        return NULL;
    }

    CosString * const string = cos_string_alloc();
    if (!string) {
        goto failure;
    }

    char * const str_copy = cos_strndup(str, n);
    if (!str_copy) {
        goto failure;
    }

    string->data = str_copy;
    string->size = strlen(str_copy) + 1;

    return string;

failure:
    if (string) {
        cos_string_free(string);
    }
    return NULL;
}

void
cos_string_free(CosString *string)
{
    if (!string) {
        return;
    }

    free(string->data);
    free(string);
}

CosString *
cos_string_copy(const CosString *string)
{
    COS_PARAMETER_ASSERT(string != NULL);

    CosString * const string_copy = cos_string_alloc();
    if (!string_copy) {
        goto failure;
    }

    const size_t count = string->size;
    if (count > 0) {
        char * const str_copy = cos_strndup(string->data,
                                            count);
        if (!str_copy) {
            goto failure;
        }

        string_copy->data = str_copy;
        string_copy->size = count;
    }

    return string_copy;

failure:
    if (string_copy) {
        cos_string_free(string_copy);
    }
    return NULL;
}

CosStringRef
cos_string_make_ref(const CosString *string)
{
    COS_PARAMETER_ASSERT(string != NULL);

    CosStringRef result = {0};
    if (string) {
        result.data = string->data;
        result.count = string->size;
    }
    return result;
}

COS_ASSUME_NONNULL_END
