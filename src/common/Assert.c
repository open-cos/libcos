//
// Created by david on 27/05/23.
//

#include "Assert.h"

#include <stdlib.h>

void
cos_assert_impl_(const char *condition,
                 const char *function_name,
                 const char *file,
                 int line,
                 const char *message,
                 ...)
{

    abort();
}
