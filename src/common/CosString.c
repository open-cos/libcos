//
// Created by david on 24/05/23.
//

#include "CosString.h"

#include "CosString-Impl.h"
#include "common/Assert.h"

#include <libcos/io/string-support.h>

#include <stdlib.h>
#include <string.h>

CosString *
cos_string_alloc(void)
{
    CosString * const string = malloc(sizeof(CosString));
    if (!string) {
        return NULL;
    }

    string->str = NULL;
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
        return NULL;
    }

    char * const str_copy = cos_strndup(str, n);
    if (!str_copy) {
        free(string);
        return NULL;
    }

    string->str = str_copy;
    string->size = strlen(str_copy) + 1;

    return string;
}

void
cos_string_free(CosString *string)
{
    if (!string) {
        return;
    }

    free(string->str);
    free(string);
}

size_t
cos_string_get_length(const CosString *string)
{
    return (string->size > 0) ? (string->size - 1) : 0;
}

const char *
cos_string_get_str(const CosString *string)
{
    return string->str;
}

CosString *
cos_string_copy(const CosString *string)
{
    CosString * const string_copy = cos_string_alloc();
    if (!string_copy) {
        return NULL;
    }

    const size_t count = string->size;
    if (count > 0) {
        char * const str_copy = cos_strndup(string->str,
                                            count);
        if (!str_copy) {
            free(string_copy);
            return NULL;
        }

        string_copy->str = str_copy;
        string_copy->size = count;
    }

    return string_copy;
}
