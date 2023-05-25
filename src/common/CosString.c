//
// Created by david on 24/05/23.
//

#include "CosString.h"

#include "common/CosVector.h"

#include <stdlib.h>

struct CosString {
    CosVector *vector;
};

CosString *
cos_string_alloc(void)
{
    CosString * const string = malloc(sizeof(CosString));
    if (!string) {
        return NULL;
    }

    CosVector * const vector = cos_vector_alloc(sizeof(char));
    if (!vector) {
        free(string);

        return NULL;
    }

    string->vector = vector;

    return string;
}

void
cos_string_free(CosString *string)
{
    free(string);
}
