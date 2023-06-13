//
// Created by david on 27/05/23.
//

#include "Assert.h"

#include <libcos/io/Format.h>

#include <stdio.h>
#include <stdlib.h>

void
cos_assert_impl_(const char *condition,
                 const char *function_name,
                 const char *file,
                 int line,
                 const char *message,
                 ...)
{
    fprintf(stderr,
            "%s:%d: %s: Assertion '%s' failed",
            file,
            line,
            function_name,
            condition);

    if (message) {
        fputs(": ", stderr);

        va_list args;
        va_start(args, message);
        vfprintf(stderr,
                 message,
                 args);
        va_end(args);
    }
    else {
        fputs(".", stderr);
    }
    fputs("\n", stderr);

    abort();
}
