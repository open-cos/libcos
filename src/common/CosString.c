//
// Created by david on 24/05/23.
//

#include "CosString.h"

#include "common/Assert.h"

#include <libcos/io/string-support.h>

#include <stdlib.h>
#include <string.h>

COS_ASSUME_NONNULL_BEGIN

static bool
cos_string_append_strn_impl_(CosString *string, const char *str, size_t n)
    COS_ATTR_ACCESS_READONLY_SIZE(2, 3);

static bool
cos_string_ensure_capacity_(CosString *string, size_t required_capacity);

static bool
cos_string_resize_(CosString *string, size_t new_capacity);

CosString *
cos_string_alloc(size_t capacity_hint)
{
    CosString * const string = malloc(sizeof(CosString));
    if (!string) {
        goto failure;
    }

    if (!cos_string_init_capacity(string, capacity_hint)) {
        goto failure;
    }

    return string;

failure:
    if (string) {
        free(string);
    }
    return NULL;
}

CosString *
cos_string_alloc_with_str(const char *str)
{
    if (!str) {
        return NULL;
    }

    return cos_string_alloc_with_strn(str, strlen(str));
}

CosString *
cos_string_alloc_with_strn(const char *str, size_t n)
{
    if (!str) {
        return NULL;
    }

    CosString * const string = malloc(sizeof(CosString));
    if (!string) {
        goto failure;
    }

    char * const str_copy = cos_strndup(str, n);
    if (!str_copy) {
        goto failure;
    }

    string->data = str_copy;
    string->length = strlen(str_copy);
    string->capacity = string->length + 1;

    return string;

failure:
    if (string) {
        free(string);
    }
    return NULL;
}

void
cos_string_free(CosString *string)
{
    if (!string) {
        return;
    }

    cos_string_release(string);

    free(string);
}

bool
cos_string_init(CosString *string)
{
    COS_PARAMETER_ASSERT(string != NULL);

    return cos_string_init_capacity(string, 0);
}

bool
cos_string_init_capacity(CosString *string, size_t capacity_hint)
{
    COS_PARAMETER_ASSERT(string != NULL);

    size_t capacity = capacity_hint;
    if (capacity == 0) {
        capacity = 32;
    }

    char * const data = malloc(capacity * sizeof(char));
    if (!data) {
        return false;
    }

    string->data = data;
    string->length = 0;
    string->capacity = capacity;

    // Nul-terminate the string.
    string->data[string->length] = '\0';

    return true;
}

void
cos_string_release(CosString *string)
{
    if (!string) {
        return;
    }

    free(string->data);
    string->data = NULL;
    string->length = 0;
    string->capacity = 0;
}

CosString *
cos_string_copy(const CosString *string)
{
    COS_PARAMETER_ASSERT(string != NULL);

    return cos_string_alloc_with_strn(string->data, string->length);
}

CosStringRef
cos_string_get_ref(const CosString *string)
{
    COS_PARAMETER_ASSERT(string != NULL);

    CosStringRef result = {0};
    if (string) {
        result.data = string->data;
        result.length = string->length;
    }
    return result;
}

CosStringRef
cos_string_ref_from_str(const char *str)
{
    COS_PARAMETER_ASSERT(str != NULL);
    if (!str) {
        return cos_string_ref_make("", 0);
    }
    else {
        return cos_string_ref_make(str, strlen(str));
    }
}

CosStringRef
cos_string_ref_make(const char *str, size_t length)
{
    COS_PARAMETER_ASSERT(str != NULL);

    CosStringRef result = {0};
    if (str) {
        result.data = str;
        result.length = length;
    }
    return result;
}

int
cos_string_ref_cmp(CosStringRef lhs, CosStringRef rhs)
{
    // Check the lengths first.
    if (lhs.length < rhs.length) {
        return -1;
    }
    else if (lhs.length > rhs.length) {
        return 1;
    }
    else {
        // The lengths are equal, so compare the strings.
        return strncmp(lhs.data, rhs.data, lhs.length);
    }
}

bool
cos_string_append_str(CosString *string, const char *str)
{
    COS_PARAMETER_ASSERT(string != NULL);
    COS_PARAMETER_ASSERT(str != NULL);

    // Make sure we don't try passing a null pointer to strlen().
    if (!str) {
        // Nothing to append.
        return true;
    }

    return cos_string_append_strn_impl_(string, str, strlen(str));
}

bool
cos_string_append_strn(CosString *string, const char *str, size_t n)
{
    COS_PARAMETER_ASSERT(string != NULL);
    COS_PARAMETER_ASSERT(str != NULL);

    return cos_string_append_strn_impl_(string, str, n);
}

bool
cos_string_push_back(CosString *string, char c)
{
    COS_PARAMETER_ASSERT(string != NULL);

    if (c == '\0') {
        // Don't bother appending an extra nul-terminator.
        return true;
    }

    return cos_string_append_strn_impl_(string, &c, 1);
}

static bool
cos_string_append_strn_impl_(CosString *string, const char *str, size_t n)
{
    COS_PARAMETER_ASSERT(string != NULL);
    COS_PARAMETER_ASSERT(str != NULL);
    COS_PARAMETER_ASSERT(n > 0);

    const size_t required_capacity = string->length + n + 1;
    if (!cos_string_ensure_capacity_(string, required_capacity)) {
        return false;
    }

    COS_ASSERT(string->capacity >= required_capacity, "String capacity is too small");

    char * const dest = string->data + string->length;
    if (n > 1) {
        strncpy(dest, str, n);
    }
    else {
        // Avoid calling strncpy() for a single character.
        *dest = *str;
    }
    string->length += n;

    // Nul-terminate the string.
    string->data[string->length] = '\0';

    return true;
}

static bool
cos_string_ensure_capacity_(CosString *string, size_t required_capacity)
{
    COS_PARAMETER_ASSERT(string != NULL);

    if (COS_LIKELY(string->capacity >= required_capacity)) {
        // No need to resize.
        return true;
    }

    size_t new_capacity = (string->capacity > 0) ? string->capacity : 1;
    while (new_capacity < required_capacity) {
        new_capacity *= 2;
    }

    return cos_string_resize_(string, new_capacity);
}

static bool
cos_string_resize_(CosString *string, size_t new_capacity)
{
    COS_PARAMETER_ASSERT(string != NULL);
    COS_PARAMETER_ASSERT(new_capacity > 0);

    size_t new_length = string->length;
    if (new_length > (new_capacity - 1)) {
        // The string will be truncated.
        new_length = new_capacity - 1;
    }

    char * const new_data = realloc(string->data, new_capacity * sizeof(char));
    if (!(new_data)) {
        return false;
    }
    new_data[new_length] = '\0';

    string->data = new_data;
    string->length = new_length;
    string->capacity = new_capacity;

    // Nul-terminate the string.
    string->data[string->length] = '\0';

    return true;
}

COS_ASSUME_NONNULL_END
