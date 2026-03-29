/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/common/CosDict.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

// MARK: - Test callbacks (pointer-identity keys)

static size_t
ptr_hash_(void *key)
{
    return (size_t)key;
}

static bool
ptr_equal_(void *key1,
           void *key2)
{
    return key1 == key2;
}

static const CosDictKeyCallbacks k_key_callbacks = {
    .hash = &ptr_hash_,
    .retain = NULL,
    .release = NULL,
    .equal = &ptr_equal_,
};

static const CosDictValueCallbacks k_value_callbacks = {
    .retain = NULL,
    .release = NULL,
    .equal = &ptr_equal_,
};

// MARK: - Helpers

static CosDict * COS_Nullable
create_dict_(void)
{
    return cos_dict_create(&k_key_callbacks, &k_value_callbacks, 0);
}

// MARK: - Creation tests

static int
create_withValidCallbacks_succeeds(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

// MARK: - Count tests

static int
count_emptyDict_returnsZero(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    TEST_EXPECT(cos_dict_get_count(dict) == 0);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

// MARK: - Set / get tests

static int
set_validKeyAndValue_incrementsCount(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_set(dict, (void *)1, (void *)100, NULL);
    TEST_EXPECT(cos_dict_get_count(dict) == 1);

    cos_dict_set(dict, (void *)2, (void *)200, NULL);
    TEST_EXPECT(cos_dict_get_count(dict) == 2);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
set_duplicateKey_doesNotIncrementCount(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_set(dict, (void *)1, (void *)100, NULL);
    cos_dict_set(dict, (void *)1, (void *)200, NULL);
    TEST_EXPECT(cos_dict_get_count(dict) == 1);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
get_existingKey_returnsValue(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_set(dict, (void *)1, (void *)100, NULL);

    void *out = NULL;
    TEST_EXPECT(cos_dict_get(dict, (void *)1, &out, NULL));
    TEST_EXPECT(out == (void *)100);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
get_nonExistingKey_returnsFalse(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    void *out = NULL;
    TEST_EXPECT(!cos_dict_get(dict, (void *)1, &out, NULL));
    TEST_EXPECT(out == NULL);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

// MARK: - Iterator tests

static int
iterator_emptyDict_returnsNoEntries(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    CosDictIterator iter = cos_dict_iterator_init(dict);
    void *key = NULL;
    void *value = NULL;
    TEST_EXPECT(!cos_dict_iterator_next(&iter, &key, &value));

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
iterator_singleEntry_returnsOneEntry(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_set(dict, (void *)1, (void *)100, NULL);

    CosDictIterator iter = cos_dict_iterator_init(dict);
    void *key = NULL;
    void *value = NULL;

    TEST_EXPECT(cos_dict_iterator_next(&iter, &key, &value));
    TEST_EXPECT(key == (void *)1);
    TEST_EXPECT(value == (void *)100);

    TEST_EXPECT(!cos_dict_iterator_next(&iter, &key, &value));

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

static int
iterator_multipleEntries_visitsAll(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_set(dict, (void *)1, (void *)100, NULL);
    cos_dict_set(dict, (void *)2, (void *)200, NULL);
    cos_dict_set(dict, (void *)3, (void *)300, NULL);

    CosDictIterator iter = cos_dict_iterator_init(dict);
    void *key = NULL;
    void *value = NULL;

    /* Track which keys we visit (order is unspecified). */
    bool seen[4] = {false, false, false, false};
    size_t count = 0;
    while (cos_dict_iterator_next(&iter, &key, &value)) {
        size_t k = (size_t)key;
        TEST_EXPECT(k >= 1 && k <= 3);
        TEST_EXPECT(!seen[k]);
        seen[k] = true;

        /* Verify the value matches the key. */
        TEST_EXPECT((size_t)value == k * 100);
        count++;
    }

    TEST_EXPECT(count == 3);

    cos_dict_destroy(dict);
    return EXIT_SUCCESS;
}

// MARK: - Test driver

TEST_MAIN()
{
    /* Creation */
    TEST_EXPECT(create_withValidCallbacks_succeeds() == EXIT_SUCCESS);

    /* Count */
    TEST_EXPECT(count_emptyDict_returnsZero() == EXIT_SUCCESS);

    /* Set / get */
    TEST_EXPECT(set_validKeyAndValue_incrementsCount() == EXIT_SUCCESS);
    TEST_EXPECT(set_duplicateKey_doesNotIncrementCount() == EXIT_SUCCESS);
    TEST_EXPECT(get_existingKey_returnsValue() == EXIT_SUCCESS);
    TEST_EXPECT(get_nonExistingKey_returnsFalse() == EXIT_SUCCESS);

    /* Iterator */
    TEST_EXPECT(iterator_emptyDict_returnsNoEntries() == EXIT_SUCCESS);
    TEST_EXPECT(iterator_singleEntry_returnsOneEntry() == EXIT_SUCCESS);
    TEST_EXPECT(iterator_multipleEntries_visitsAll() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
