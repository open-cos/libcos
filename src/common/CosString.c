//
// Created by david on 24/05/23.
//

#include "CosString.h"

#include <stdlib.h>
#include <string.h>

struct CosString {

    /**
     * The nul-terminated character array.
     */
    char *characters;

    /**
     * The number of characters in the string, including the nul-terminator.
     */
    size_t count;

    /**
     * The total allocated size of the character array.
     */
    size_t capacity;
};

static bool
cos_string_grow(CosString *string);

CosString *
cos_string_alloc(void)
{
    CosString * const string = malloc(sizeof(CosString));
    if (!string) {
        return NULL;
    }

    string->characters = NULL;
    string->count = 0;
    string->capacity = 0;

    return string;
}

void
cos_string_free(CosString *string)
{
    if (!string) {
        return;
    }

    free(string->characters);
    free(string);
}

size_t
cos_string_get_length(const CosString *string)
{
    return (string->count > 0) ? (string->count - 1) : 0;
}

char *
cos_string_get_characters(const CosString *string)
{
    return string->characters;
}

bool
cos_string_append(CosString *string,
                  const char *characters,
                  size_t length)
{
    if (!string) {
        return false;
    }



    cos_string_grow(string);

    memcpy(string->characters + string->count,
           characters,
           length + 1);

    string->count += length;

    return true;
}

CosString *
cos_string_copy(const CosString *string)
{
    CosString * const string_copy = cos_string_alloc();
    if (!string_copy) {
        return NULL;
    }

    const size_t count = string->count;
    if (count > 0) {
        char * const characters = malloc(count * sizeof(char));
        if (!characters) {
            free(string_copy);
            return NULL;
        }

        string_copy->characters = memcpy(characters,
                                         string->characters,
                                         count);

        string_copy->count = count;
        string_copy->capacity = count;
    }

    return string_copy;
}

static bool
cos_string_grow(CosString *string)
{
    const size_t original_capacity = string->capacity;

    const size_t new_capacity = (original_capacity > 0) ? (original_capacity * 2) : 1;
    //    COS_ASSERT(new_capacity > 0);

    char * const characters = realloc(string->characters,
                                      new_capacity * sizeof(char));
    if (!characters) {
        return false;
    }

    string->characters = characters;
    string->capacity = new_capacity;

    return true;
}
