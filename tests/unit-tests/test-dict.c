/*
 * Copyright (c) 2024 OpenCOS.
 */

#include <libcos/common/CosDict.h>

#include <stdlib.h>

#define TEST_EXPECT(expr)            \
    do {                             \
        if (COS_UNLIKELY(!(expr))) { \
            return EXIT_FAILURE;     \
        }                            \
    } while (0)

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

static const CosDictKeyCallbacks key_callbacks = {
    .hash = &cos_dict_ptr_hash_,
    .retain = NULL,
    .release = NULL,
    .equal = &cos_dict_ptr_equal_,
};

static const CosDictValueCallbacks value_callbacks = {
    .retain = NULL,
    .release = NULL,
    .equal = &cos_dict_ptr_equal_,
};

static int
createDictWithNullCallbacks_ReturnsInvalidDict(void)
{
    int result = EXIT_SUCCESS;

    CosDict * const dict = cos_dict_create(COS_nonnull_cast(NULL),
                                           COS_nonnull_cast(NULL),
                                           0);
    if (dict != NULL) {
        result = EXIT_FAILURE;
        goto finish;
    }

finish:
    if (dict) {
        cos_dict_destroy(dict);
    }
    return result;
}

static int
createDictWithValidCallbacks_ReturnsValidDict(void)
{
    CosDict * const dict = cos_dict_create(&key_callbacks,
                                           &value_callbacks,
                                           0);
    TEST_EXPECT(dict != NULL);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
addToDict_ValidKeyAndValue_IncreasesDictSize(void)
{
    // Assuming cos_dict_add and cos_dict_count functions exist
    CosDict * const dict = cos_dict_create(&key_callbacks,
                                           &value_callbacks,
                                           0);
    TEST_EXPECT(dict != NULL);

    void *key = (void *)123;
    void *value = (void *)456;
    cos_dict_set(dict, key, value, NULL);
    TEST_EXPECT(cos_dict_get_count(dict) == 1);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

//static int
//removeFromDict_ValidKey_DecreasesDictSize(void)
//{
//    // Assuming cos_dict_remove and cos_dict_count functions exist
//    CosDict * const dict = cos_dict_create(&key_callbacks, &value_callbacks, 0);
//    void *key = (void *)123;
//    void *value = (void *)456;
//    cos_dict_set(dict, key, value, NULL);
//    cos_dict_remove(dict, key);
//    TEST_EXPECT(cos_dict_count(dict) == 0);
//    cos_dict_destroy(dict);
//    return EXIT_SUCCESS;
//}

static int
lookupInDict_ExistingKey_ReturnsCorrectValue(void)
{
    // Assuming cos_dict_get function exists
    CosDict * const dict = cos_dict_create(&key_callbacks,
                                           &value_callbacks,
                                           0);
    TEST_EXPECT(dict != NULL);

    void *key = (void *)123;
    void *value = (void *)456;
    cos_dict_set(dict, key, value, NULL);

    void *out_value = NULL;
    TEST_EXPECT(cos_dict_get(dict, key, &out_value, NULL) && out_value == value);
    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
lookupInDict_NonExistingKey_ReturnsNull(void)
{
    // Assuming cos_dict_get function exists
    CosDict * const dict = cos_dict_create(&key_callbacks, &value_callbacks, 0);
    TEST_EXPECT(dict != NULL);

    void *key = (void *)123;
    void *out_value = NULL;

    TEST_EXPECT(!cos_dict_get(dict, key, &out_value, NULL) &&
                out_value == NULL);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
dictWithZeroCapacity_InitialSizeIsZero(void)
{
    // Assuming cos_dict_count function exists
    CosDict * const dict = cos_dict_create(&key_callbacks, &value_callbacks, 0);
    TEST_EXPECT(dict != NULL);

    TEST_EXPECT(cos_dict_get_count(dict) == 0);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
addToDict_NullKey_ReturnsError(void)
{
    // Assuming cos_dict_add returns an error code for null keys
    CosDict * const dict = cos_dict_create(&key_callbacks, &value_callbacks, 0);
    TEST_EXPECT(dict != NULL);

    void *value = (void *)456;
    CosError *error = NULL;
    TEST_EXPECT(!cos_dict_set(dict, COS_nonnull_cast(NULL), value, error));

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
addToDict_NullValue_HandledGracefully(void)
{
    // Assuming cos_dict_add can handle null values gracefully
    CosDict * const dict = cos_dict_create(&key_callbacks, &value_callbacks, 0);
    TEST_EXPECT(dict != NULL);

    void *key = (void *)123;
    TEST_EXPECT(cos_dict_set(dict, key, NULL, NULL) == EXIT_SUCCESS);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

extern int
TEST_NAME(int argc,
          char * COS_Nonnull argv[]);

int
TEST_NAME(COS_ATTR_UNUSED int argc,
          COS_ATTR_UNUSED char * COS_Nonnull argv[])
{
//    TEST_EXPECT(createDictWithNullCallbacks_ReturnsInvalidDict() == EXIT_SUCCESS);
    TEST_EXPECT(createDictWithValidCallbacks_ReturnsValidDict() == EXIT_SUCCESS);
    TEST_EXPECT(addToDict_ValidKeyAndValue_IncreasesDictSize() == EXIT_SUCCESS);
    //TEST_EXPECT(removeFromDict_ValidKey_DecreasesDictSize() == EXIT_SUCCESS);
    TEST_EXPECT(lookupInDict_ExistingKey_ReturnsCorrectValue() == EXIT_SUCCESS);
    TEST_EXPECT(lookupInDict_NonExistingKey_ReturnsNull() == EXIT_SUCCESS);
    TEST_EXPECT(dictWithZeroCapacity_InitialSizeIsZero() == EXIT_SUCCESS);
//    TEST_EXPECT(addToDict_NullKey_ReturnsError() == EXIT_SUCCESS);
    TEST_EXPECT(addToDict_NullValue_HandledGracefully() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
