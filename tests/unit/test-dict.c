/*
 * Copyright (c) 2024 OpenCOS.
 */

#include <libcos/common/CosDict.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

static size_t
cos_dict_ptr_hash_(void *key)
{
    return (size_t)key;
}

static bool
cos_dict_ptr_equal_(void *key1,
                    void *key2)
{
    return key1 == key2;
}

extern int
TEST_NAME(int argc,
          char * COS_Nonnull argv[]);

int
TEST_NAME(COS_ATTR_UNUSED int argc,
          COS_ATTR_UNUSED char * COS_Nonnull argv[])
{
    const CosDictKeyCallbacks key_callbacks = {
        .hash = &cos_dict_ptr_hash_,
        .retain = NULL,
        .release = NULL,
        .equal = &cos_dict_ptr_equal_,
    };
    CosDict * const dict = cos_dict_create(&key_callbacks,
                                           NULL,
                                           0);
    if (!dict) {
        return EXIT_FAILURE;
    }

    cos_dict_destroy(dict);

    return 0;
}

COS_ASSUME_NONNULL_END
