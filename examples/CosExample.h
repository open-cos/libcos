/*
 * Copyright (c) 2025 OpenCOS.
 */

#ifndef LIBCOS_COS_EXAMPLE_H
#define LIBCOS_COS_EXAMPLE_H

#include <libcos/common/CosDefines.h>

#include <stdlib.h>

#define EXAMPLE_MAIN()                                \
    extern int                                        \
    TEST_NAME(int argc,                               \
              char * COS_Nullable argv[COS_Nonnull]); \
                                                      \
    int                                               \
    TEST_NAME(COS_ATTR_UNUSED int argc,               \
              COS_ATTR_UNUSED char * COS_Nullable argv[COS_Nonnull])

#endif /* LIBCOS_COS_EXAMPLE_H */
