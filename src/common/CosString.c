//
// Created by david on 24/05/23.
//

#include "libcos/common/CosString.h"

#include "common/Assert.h"

#include <libcos/common/CosError.h>
#include <libcos/common/memory/CosMemory.h>
#include <libcos/io/string-support.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define COS_STRING_DEFAULT_CAPACITY 32

COS_ASSUME_NONNULL_BEGIN

struct CosString {
    /**
     * The nul-terminated character array.
     */
    char *data;

    /**
     * The number of characters in the string, not including the nul-terminator.
     */
    size_t length;

    /**
     * The number of characters that can be stored in the string.
     *
     * @invariant The capacity is always large enough to store @c length characters,
     * plus the nul-terminator.
     * @code
     * capacity >= length + 1
     * @endcode
     */
    size_t capacity;
};

static bool
cos_string_append_strn_impl_(CosString *string, const char *str, size_t n,
                             CosError * COS_Nullable error)
    COS_ATTR_ACCESS_READ_ONLY_SIZE(2, 3);

static bool
cos_string_ensure_capacity_(CosString *string, size_t required_capacity,
                            CosError * COS_Nullable error);

static bool
cos_string_resize_(CosString *string, size_t new_capacity,
                   CosError * COS_Nullable error);

CosString *
cos_string_alloc(size_t capacity_hint)
{
    CosString * const string = cos_malloc(sizeof(CosString));
    if (!string) {
        goto failure;
    }

    // The allocation failure is reported through the NULL return, so no error
    // object is threaded here.
    if (!cos_string_init_capacity(string, capacity_hint, NULL)) {
        goto failure;
    }

    return string;

failure:
    if (string) {
        cos_free(string);
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

    CosString * const string = cos_malloc(sizeof(CosString));
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
        cos_free(string);
    }
    return NULL;
}

void
cos_string_free(CosString *string)
{
    if (!string) {
        return;
    }

    cos_free(string->data);

    cos_free(string);
}

bool
cos_string_init(CosString *string, CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(string != NULL);

    return cos_string_init_capacity(string, 0, error);
}

bool
cos_string_init_capacity(CosString *string, size_t capacity_hint, CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(string != NULL);
    if (!string) {
        return false;
    }

    size_t capacity = capacity_hint;
    if (capacity == 0) {
        capacity = COS_STRING_DEFAULT_CAPACITY;
    }

    char * const data = cos_malloc(capacity * sizeof(char));
    if (!data) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate memory for string"),
                            error);
        return false;
    }

    string->data = data;
    string->length = 0;
    string->capacity = capacity;

    // Nul-terminate the string.
    string->data[string->length] = '\0';

    return true;
}

const char *
cos_string_get_data(const CosString *string)
{
    COS_API_PARAM_CHECK(string != NULL);
    if (!string) {
        return NULL;
    }

    return string->data;
}

size_t
cos_string_get_length(const CosString *string)
{
    COS_API_PARAM_CHECK(string != NULL);
    if (!string) {
        return 0;
    }

    return string->length;
}

size_t
cos_string_get_capacity(const CosString *string)
{
    COS_API_PARAM_CHECK(string != NULL);
    if (!string) {
        return 0;
    }

    return string->capacity;
}

CosString *
cos_string_copy(const CosString *string, CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(string != NULL);
    if (!string) {
        return NULL;
    }

    CosString * const copy = cos_string_alloc_with_strn(string->data, string->length);
    if (!copy) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate memory for string copy"),
                            error);
        return NULL;
    }

    return copy;
}

CosStringRef
cos_string_get_ref(const CosString *string)
{
    COS_API_PARAM_CHECK(string != NULL);

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
    COS_API_PARAM_CHECK(str != NULL);
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
    COS_API_PARAM_CHECK(str != NULL);

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
cos_string_append_str(CosString *string, const char *str, CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(string != NULL);
    COS_API_PARAM_CHECK(str != NULL);

    // Make sure we don't try passing a null pointer to strlen().
    if (!str) {
        // Nothing to append.
        return true;
    }

    return cos_string_append_strn_impl_(string, str, strlen(str), error);
}

bool
cos_string_append_strn(CosString *string, const char *str, size_t n, CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(string != NULL);
    COS_API_PARAM_CHECK(str != NULL);

    return cos_string_append_strn_impl_(string, str, n, error);
}

bool
cos_string_push_back(CosString *string, char c, CosError * COS_Nullable error)
{
    COS_API_PARAM_CHECK(string != NULL);

    if (c == '\0') {
        // Don't bother appending an extra nul-terminator.
        return true;
    }

    return cos_string_append_strn_impl_(string, &c, 1, error);
}

#if SIZE_MAX == UINT32_MAX
    #define COS_SIZE_IS_32_BIT 1
#else
    #define COS_SIZE_IS_32_BIT 0
#endif

#if COS_SIZE_IS_32_BIT

    #define COS_STRING_HASH_SEED_32 2166136261U
    #define COS_STRING_HASH_FACTOR_32 16777619

static uint32_t
hash_string_32_(const char *key,
                size_t length)
{
    uint32_t hash = COS_STRING_HASH_SEED_32;
    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= COS_STRING_HASH_FACTOR_32;
    }
    return hash;
}

#else

    #define COS_STRING_HASH_SEED_64 14695981039346656037ULL
    #define COS_STRING_HASH_FACTOR_64 1099511628211ULL

static uint64_t
hash_string_64_(const char *key,
                size_t length)
{
    uint64_t hash = COS_STRING_HASH_SEED_64;
    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= COS_STRING_HASH_FACTOR_64;
    }
    return hash;
}

#endif

size_t
cos_string_get_hash(const CosString *string)
{
    COS_API_PARAM_CHECK(string != NULL);

    if (!string) {
        return 0;
    }

    // Use the appropriate hash function based on the size of size_t.
#if COS_SIZE_IS_32_BIT
    return hash_string_32_(string->data, string->length);
#else
    return hash_string_64_(string->data, string->length);
#endif
}

static bool
cos_string_append_strn_impl_(CosString *string, const char *str, size_t n,
                             CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(string != NULL);
    COS_IMPL_PARAM_CHECK(str != NULL);
    COS_IMPL_PARAM_CHECK(n > 0);
    if (!string || !str || n == 0) {
        return false;
    }

    const size_t required_capacity = string->length + n + 1;
    if (!cos_string_ensure_capacity_(string, required_capacity, error)) {
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
cos_string_ensure_capacity_(CosString *string, size_t required_capacity,
                            CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(string != NULL);

    if (COS_LIKELY(string->capacity >= required_capacity)) {
        // No need to resize.
        return true;
    }

    size_t new_capacity = (string->capacity > 0) ? string->capacity : 1;
    while (new_capacity < required_capacity) {
        new_capacity *= 2;
    }

    return cos_string_resize_(string, new_capacity, error);
}

static bool
cos_string_resize_(CosString *string, size_t new_capacity,
                   CosError * COS_Nullable error)
{
    COS_IMPL_PARAM_CHECK(string != NULL);
    COS_IMPL_PARAM_CHECK(new_capacity > 0);
    if (!string || new_capacity == 0) {
        return false;
    }

    size_t new_length = string->length;
    if (new_length > (new_capacity - 1)) {
        // The string will be truncated.
        new_length = new_capacity - 1;
    }

    char * const new_data = cos_realloc(string->data,
                                    new_capacity * sizeof(char));
    if (!new_data) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate memory for string"),
                            error);
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
