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

// MARK: - Test callbacks (boxed keys, counting releases)

/*
 * The dictionary owns whatever it holds: it releases every key and value in
 * cos_dict_destroy(), and must do the same for a pair it displaces.
 *
 * These keys are boxed ints compared by value, so that two distinct pointers
 * can be equal keys -- which is what the object parser produces, where each
 * "/A" in "<< /A 1 /A 2 >>" is a separate name node. Pointer-identity keys
 * cannot express that case.
 */

static size_t g_key_release_count = 0;
static size_t g_value_release_count = 0;

static size_t
boxed_hash_(void *key)
{
    return (size_t)*(const int *)key;
}

static bool
boxed_equal_(void *key1,
             void *key2)
{
    return *(const int *)key1 == *(const int *)key2;
}

static void
counting_key_release_(void *key)
{
    (void)key;
    g_key_release_count++;
}

static void
counting_value_release_(void *value)
{
    (void)value;
    g_value_release_count++;
}

static const CosDictKeyCallbacks k_counting_key_callbacks = {
    .hash = &boxed_hash_,
    .retain = NULL,
    .release = &counting_key_release_,
    .equal = &boxed_equal_,
};

static const CosDictValueCallbacks k_counting_value_callbacks = {
    .retain = NULL,
    .release = &counting_value_release_,
    .equal = &boxed_equal_,
};

// MARK: - Helpers

static CosDict * COS_Nullable
create_dict_(void)
{
    return cos_dict_create(&k_key_callbacks, &k_value_callbacks, 0);
}

static CosDict * COS_Nullable
create_counting_dict_(void)
{
    g_key_release_count = 0;
    g_value_release_count = 0;
    return cos_dict_create(&k_counting_key_callbacks,
                           &k_counting_value_callbacks,
                           0);
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
set_equalButDistinctKey_releasesDisplacedKeyAndValue(void)
{
    /* Models "<< /A 1 /A 2 >>": key_a and key_b are equal, not identical. */
    int key_a = 1;
    int key_b = 1;
    int value_1 = 100;
    int value_2 = 200;

    CosDict * const dict = create_counting_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_set(dict, &key_a, &value_1, NULL);
    TEST_EXPECT(g_key_release_count == 0);
    TEST_EXPECT(g_value_release_count == 0);

    /* Replacing the entry must release the key and value it displaces. */
    cos_dict_set(dict, &key_b, &value_2, NULL);
    TEST_EXPECT(g_key_release_count == 1);
    TEST_EXPECT(g_value_release_count == 1);
    TEST_EXPECT(cos_dict_get_count(dict) == 1);

    void *found = NULL;
    TEST_EXPECT(cos_dict_get(dict, &key_a, &found));
    TEST_EXPECT(found == &value_2);

    /* Destroying releases the pair still held, and no more. */
    cos_dict_destroy(dict);
    TEST_EXPECT(g_key_release_count == 2);
    TEST_EXPECT(g_value_release_count == 2);
    return EXIT_SUCCESS;
}

static int
set_sameKeyAndValuePointer_doesNotRelease(void)
{
    /*
     * Re-setting an entry to the pointers it already holds must not release
     * them: with ownership-transferring callbacks that would free the very
     * key and value being stored.
     */
    int key = 1;
    int value = 100;

    CosDict * const dict = create_counting_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_set(dict, &key, &value, NULL);
    cos_dict_set(dict, &key, &value, NULL);
    TEST_EXPECT(g_key_release_count == 0);
    TEST_EXPECT(g_value_release_count == 0);
    TEST_EXPECT(cos_dict_get_count(dict) == 1);

    cos_dict_destroy(dict);
    TEST_EXPECT(g_key_release_count == 1);
    TEST_EXPECT(g_value_release_count == 1);
    return EXIT_SUCCESS;
}

static int
set_distinctKeys_releaseNothingUntilDestroy(void)
{
    int key_a = 1;
    int key_b = 2;
    int value_1 = 100;
    int value_2 = 200;

    CosDict * const dict = create_counting_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_set(dict, &key_a, &value_1, NULL);
    cos_dict_set(dict, &key_b, &value_2, NULL);
    TEST_EXPECT(g_key_release_count == 0);
    TEST_EXPECT(g_value_release_count == 0);

    cos_dict_destroy(dict);
    TEST_EXPECT(g_key_release_count == 2);
    TEST_EXPECT(g_value_release_count == 2);
    return EXIT_SUCCESS;
}

static int
get_existingKey_returnsValue(void)
{
    CosDict * const dict = create_dict_();
    TEST_EXPECT(dict != NULL);

    cos_dict_set(dict, (void *)1, (void *)100, NULL);

    void *out = NULL;
    TEST_EXPECT(cos_dict_get(dict, (void *)1, &out));
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
    TEST_EXPECT(!cos_dict_get(dict, (void *)1, &out));
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
    TEST_EXPECT(set_equalButDistinctKey_releasesDisplacedKeyAndValue() == EXIT_SUCCESS);
    TEST_EXPECT(set_sameKeyAndValuePointer_doesNotRelease() == EXIT_SUCCESS);
    TEST_EXPECT(set_distinctKeys_releaseNothingUntilDestroy() == EXIT_SUCCESS);
    TEST_EXPECT(get_existingKey_returnsValue() == EXIT_SUCCESS);
    TEST_EXPECT(get_nonExistingKey_returnsFalse() == EXIT_SUCCESS);

    /* Iterator */
    TEST_EXPECT(iterator_emptyDict_returnsNoEntries() == EXIT_SUCCESS);
    TEST_EXPECT(iterator_singleEntry_returnsOneEntry() == EXIT_SUCCESS);
    TEST_EXPECT(iterator_multipleEntries_visitsAll() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
