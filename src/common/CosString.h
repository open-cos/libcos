//
// Created by david on 24/05/23.
//

#ifndef LIBCOS_COS_STRING_H
#define LIBCOS_COS_STRING_H

#include <stdbool.h>
#include <stddef.h>

struct CosString;
typedef struct CosString CosString;

CosString *
cos_string_alloc(void);

void
cos_string_free(CosString *string);

size_t
cos_string_get_length(const CosString *string);

char *
cos_string_get_characters(const CosString *string);

bool
cos_string_append(CosString *string,
                  const char *characters,
                  size_t length);

/**
 * Append a single character to the string.
 *
 * @param string The string.
 * @param character The character to append.
 *
 * @return true if the character was appended successfully, false otherwise.
 */
bool
cos_string_append_char(CosString *string,
                       char character);

CosString *
cos_string_copy(const CosString *string);

#endif /* LIBCOS_COS_STRING_H */
