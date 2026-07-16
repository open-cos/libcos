/*
 * Copyright (c) 2025 OpenCOS.
 */

#include "CosTest.h"

#include <libcos/CosObj.h>
#include <libcos/CosObjID.h>
#include <libcos/common/CosArray.h>
#include <libcos/common/CosDict.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosString.h>
#include <libcos/objects/CosArrayObjNode.h>
#include <libcos/objects/CosBoolObjNode.h>
#include <libcos/objects/CosDictObjNode.h>
#include <libcos/objects/CosIndirectObjNode.h>
#include <libcos/objects/CosIntObjNode.h>
#include <libcos/objects/CosNameObjNode.h>
#include <libcos/objects/CosNullObjNode.h>
#include <libcos/objects/CosObjNode.h>
#include <libcos/objects/CosRealObjNode.h>

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

// MARK: - Lifecycle tests

static int
create_fromDirectIntNode_succeeds(void)
{
    CosIntObjNode *int_node = cos_int_obj_node_alloc(42);
    TEST_EXPECT(int_node != NULL);

    CosObj *obj = cos_obj_create((CosObjNode *)int_node);
    TEST_EXPECT(obj != NULL);
    TEST_EXPECT(cos_obj_get_type(obj) == CosObjType_Integer);
    TEST_EXPECT(cos_obj_is_direct(obj));
    TEST_EXPECT(!cos_obj_is_indirect(obj));
    TEST_EXPECT(cos_obj_get_int_value(obj) == 42);

    cos_obj_destroy(obj);
    cos_obj_node_release((CosObjNode *)int_node);

    return EXIT_SUCCESS;
}

static int
create_fromIndirectNode_resolvesType(void)
{
    CosIntObjNode *int_node = cos_int_obj_node_alloc(99);
    TEST_EXPECT(int_node != NULL);

    CosObjID id = cos_obj_id_make(1, 0);
    CosIndirectObjNode *indirect =
        cos_indirect_obj_node_alloc(id, (CosObjNode *)int_node);
    TEST_EXPECT(indirect != NULL);

    CosObj *obj = cos_obj_create((CosObjNode *)indirect);
    TEST_EXPECT(obj != NULL);
    TEST_EXPECT(cos_obj_get_type(obj) == CosObjType_Integer);
    TEST_EXPECT(cos_obj_is_indirect(obj));
    TEST_EXPECT(!cos_obj_is_direct(obj));
    TEST_EXPECT(cos_obj_get_int_value(obj) == 99);

    cos_obj_destroy(obj);
    cos_obj_node_release((CosObjNode *)indirect);

    return EXIT_SUCCESS;
}

static int
create_fromNullNode_isNull(void)
{
    CosNullObjNode *null_node = cos_null_obj_node_get();
    TEST_EXPECT(null_node != NULL);

    CosObj *obj = cos_obj_create((CosObjNode *)null_node);
    TEST_EXPECT(obj != NULL);
    TEST_EXPECT(cos_obj_get_type(obj) == CosObjType_Null);
    TEST_EXPECT(cos_obj_is_null(obj));
    TEST_EXPECT(cos_obj_is_direct(obj));

    cos_obj_destroy(obj);

    return EXIT_SUCCESS;
}

// MARK: - Scalar accessor tests

static int
getBoolValue_boolObj_returnsValue(void)
{
    CosBoolObjNode *bool_node = cos_bool_obj_node_alloc(true);
    TEST_EXPECT(bool_node != NULL);

    CosObj *obj = cos_obj_create((CosObjNode *)bool_node);
    TEST_EXPECT(obj != NULL);
    TEST_EXPECT(cos_obj_get_type(obj) == CosObjType_Boolean);
    TEST_EXPECT(cos_obj_get_bool_value(obj) == true);

    cos_obj_destroy(obj);
    cos_obj_node_release((CosObjNode *)bool_node);

    return EXIT_SUCCESS;
}

static int
getRealValue_realObj_returnsValue(void)
{
    CosRealObjNode *real_node = cos_real_obj_node_alloc(3.14);
    TEST_EXPECT(real_node != NULL);

    CosObj *obj = cos_obj_create((CosObjNode *)real_node);
    TEST_EXPECT(obj != NULL);
    TEST_EXPECT(cos_obj_get_type(obj) == CosObjType_Real);
    TEST_EXPECT(cos_obj_get_real_value(obj) == 3.14);

    cos_obj_destroy(obj);
    cos_obj_node_release((CosObjNode *)real_node);

    return EXIT_SUCCESS;
}

static int
getIntValue_typeMismatch_returnsZero(void)
{
    CosBoolObjNode *bool_node = cos_bool_obj_node_alloc(true);
    TEST_EXPECT(bool_node != NULL);

    CosObj *obj = cos_obj_create((CosObjNode *)bool_node);
    TEST_EXPECT(obj != NULL);
    TEST_EXPECT(cos_obj_get_int_value(obj) == 0);

    cos_obj_destroy(obj);
    cos_obj_node_release((CosObjNode *)bool_node);

    return EXIT_SUCCESS;
}

// MARK: - Array accessor tests

static int
getArrayCount_arrayObj_returnsCount(void)
{
    CosArrayObjNode *array_node = cos_array_obj_node_alloc(NULL);
    TEST_EXPECT(array_node != NULL);

    CosIntObjNode *elem = cos_int_obj_node_alloc(10);
    TEST_EXPECT(elem != NULL);
    TEST_EXPECT(cos_array_obj_node_append(array_node, (CosObjNode *)elem, NULL));

    CosObj *obj = cos_obj_create((CosObjNode *)array_node);
    TEST_EXPECT(obj != NULL);
    TEST_EXPECT(cos_obj_get_type(obj) == CosObjType_Array);
    TEST_EXPECT(cos_obj_get_array_count(obj) == 1);

    CosError error = cos_error_none();
    CosObj *child = cos_obj_get_object_at_index(obj, 0, &error);
    TEST_EXPECT(child != NULL);
    TEST_EXPECT(cos_obj_get_type(child) == CosObjType_Integer);
    TEST_EXPECT(cos_obj_get_int_value(child) == 10);

    cos_obj_destroy(child);
    cos_obj_destroy(obj);
    cos_obj_node_release((CosObjNode *)array_node);

    return EXIT_SUCCESS;
}

// MARK: - Dict accessor tests

static int
getDictValue_existingKey_returnsValue(void)
{
    CosDictObjNode *dict_node = cos_dict_obj_node_create(NULL);
    TEST_EXPECT(dict_node != NULL);

    CosString *key_str = cos_string_alloc_with_str("Type");
    TEST_EXPECT(key_str != NULL);
    CosNameObjNode *key = cos_name_obj_node_alloc(key_str);
    TEST_EXPECT(key != NULL);

    CosIntObjNode *value = cos_int_obj_node_alloc(42);
    TEST_EXPECT(value != NULL);

    TEST_EXPECT(cos_dict_obj_node_set(dict_node, key, (CosObjNode *)value, NULL));

    CosObj *obj = cos_obj_create((CosObjNode *)dict_node);
    TEST_EXPECT(obj != NULL);
    TEST_EXPECT(cos_obj_get_type(obj) == CosObjType_Dict);
    TEST_EXPECT(cos_obj_get_dict_count(obj) == 1);

    CosError error = cos_error_none();
    CosObj *result = cos_obj_get_value_for_key(obj, "Type", &error);
    TEST_EXPECT(result != NULL);
    TEST_EXPECT(cos_obj_get_type(result) == CosObjType_Integer);
    TEST_EXPECT(cos_obj_get_int_value(result) == 42);

    cos_obj_destroy(result);
    cos_obj_destroy(obj);
    cos_obj_node_release((CosObjNode *)dict_node);

    return EXIT_SUCCESS;
}

static int
getDictValue_nonExistingKey_returnsNull(void)
{
    CosDictObjNode *dict_node = cos_dict_obj_node_create(NULL);
    TEST_EXPECT(dict_node != NULL);

    CosObj *obj = cos_obj_create((CosObjNode *)dict_node);
    TEST_EXPECT(obj != NULL);

    CosError error = cos_error_none();
    CosObj *result = cos_obj_get_value_for_key(obj, "NoSuchKey", &error);
    TEST_EXPECT(result == NULL);

    cos_obj_destroy(obj);
    cos_obj_node_release((CosObjNode *)dict_node);

    return EXIT_SUCCESS;
}

// MARK: - Dict iterator tests

static int
dictIterator_singleEntry_yieldsEntry(void)
{
    CosDictObjNode *dict_node = cos_dict_obj_node_create(NULL);
    TEST_EXPECT(dict_node != NULL);

    CosString *key_str = cos_string_alloc_with_str("Size");
    TEST_EXPECT(key_str != NULL);
    CosNameObjNode *key = cos_name_obj_node_alloc(key_str);
    TEST_EXPECT(key != NULL);

    CosIntObjNode *value = cos_int_obj_node_alloc(100);
    TEST_EXPECT(value != NULL);

    TEST_EXPECT(cos_dict_obj_node_set(dict_node, key, (CosObjNode *)value, NULL));

    CosObj *obj = cos_obj_create((CosObjNode *)dict_node);
    TEST_EXPECT(obj != NULL);

    CosObjDictIterator iter = cos_obj_dict_iterator_init(obj);
    const char *iter_key = NULL;
    CosObj *iter_value = NULL;

    TEST_EXPECT(cos_obj_dict_iterator_next(&iter, &iter_key, &iter_value));
    TEST_EXPECT(iter_key != NULL);
    TEST_EXPECT(iter_value != NULL);
    TEST_EXPECT(cos_obj_get_type(iter_value) == CosObjType_Integer);
    TEST_EXPECT(cos_obj_get_int_value(iter_value) == 100);

    cos_obj_destroy(iter_value);

    /* No more entries. */
    TEST_EXPECT(!cos_obj_dict_iterator_next(&iter, &iter_key, &iter_value));

    cos_obj_destroy(obj);
    cos_obj_node_release((CosObjNode *)dict_node);

    return EXIT_SUCCESS;
}

// MARK: - Concrete type predicate tests

/*
 * The contract of the is_* predicates is that a true answer makes the cast to
 * the matching node type safe. An indirect object holding a dictionary is not
 * a dictionary node, so it must answer false -- casting it to CosDictObjNode
 * and reading ->value would return the object ID reinterpreted as a pointer.
 */

static int
isDict_dictNode_returnsTrue(void)
{
    CosDictObjNode *dict_node = cos_dict_obj_node_create(NULL);
    TEST_EXPECT(dict_node != NULL);

    TEST_EXPECT(cos_obj_node_is_dict((CosObjNode *)dict_node));
    TEST_EXPECT(!cos_obj_node_is_integer((CosObjNode *)dict_node));

    cos_obj_node_release((CosObjNode *)dict_node);
    return EXIT_SUCCESS;
}

static int
isDict_indirectNodeHoldingDict_returnsFalse(void)
{
    CosDictObjNode *dict_node = cos_dict_obj_node_create(NULL);
    TEST_EXPECT(dict_node != NULL);

    CosIndirectObjNode *indirect =
        cos_indirect_obj_node_alloc(cos_obj_id_make(1, 0), (CosObjNode *)dict_node);
    TEST_EXPECT(indirect != NULL);

    /* The node is an indirect object, whatever it holds. */
    TEST_EXPECT(!cos_obj_node_is_dict((CosObjNode *)indirect));
    TEST_EXPECT(cos_obj_node_is_indirect((CosObjNode *)indirect));
    TEST_EXPECT(!cos_obj_node_is_direct((CosObjNode *)indirect));

    /* ... but it still resolves to one. */
    TEST_EXPECT(cos_obj_node_get_value_type((CosObjNode *)indirect) ==
                CosObjNodeValueType_Dict);

    cos_obj_node_release((CosObjNode *)indirect);
    return EXIT_SUCCESS;
}

static int
isInteger_indirectNodeHoldingInt_returnsFalse(void)
{
    CosIntObjNode *int_node = cos_int_obj_node_alloc(42);
    TEST_EXPECT(int_node != NULL);

    CosIndirectObjNode *indirect =
        cos_indirect_obj_node_alloc(cos_obj_id_make(2, 0), (CosObjNode *)int_node);
    TEST_EXPECT(indirect != NULL);

    TEST_EXPECT(!cos_obj_node_is_integer((CosObjNode *)indirect));
    TEST_EXPECT(cos_obj_node_get_value_type((CosObjNode *)indirect) ==
                CosObjNodeValueType_Integer);

    cos_obj_node_release((CosObjNode *)indirect);
    return EXIT_SUCCESS;
}

static int
isName_nameNode_returnsTrue(void)
{
    CosNameObjNode *name_node = cos_name_obj_node_alloc(cos_string_alloc_with_str("Type"));
    TEST_EXPECT(name_node != NULL);

    TEST_EXPECT(cos_obj_node_is_name((CosObjNode *)name_node));
    TEST_EXPECT(!cos_obj_node_is_dict((CosObjNode *)name_node));

    cos_obj_node_release((CosObjNode *)name_node);
    return EXIT_SUCCESS;
}

static int
isNull_nullNode_returnsTrue(void)
{
    CosNullObjNode * const null_node = cos_null_obj_node_get();
    TEST_EXPECT(null_node != NULL);

    TEST_EXPECT(cos_obj_node_is_null((CosObjNode *)null_node));
    TEST_EXPECT(!cos_obj_node_is_boolean((CosObjNode *)null_node));

    return EXIT_SUCCESS;
}

static int
predicates_eachNodeType_onlyItsOwnIsTrue(void)
{
    CosBoolObjNode * const bool_node = cos_bool_obj_node_alloc(true);
    CosIntObjNode * const int_node = cos_int_obj_node_alloc(1);
    CosRealObjNode * const real_node = cos_real_obj_node_alloc(1.5);
    CosArrayObjNode * const array_node = cos_array_obj_node_alloc(NULL);
    TEST_EXPECT(bool_node && int_node && real_node && array_node);

    TEST_EXPECT(cos_obj_node_is_boolean((CosObjNode *)bool_node));
    TEST_EXPECT(!cos_obj_node_is_integer((CosObjNode *)bool_node));

    TEST_EXPECT(cos_obj_node_is_integer((CosObjNode *)int_node));
    TEST_EXPECT(!cos_obj_node_is_real((CosObjNode *)int_node));

    TEST_EXPECT(cos_obj_node_is_real((CosObjNode *)real_node));
    TEST_EXPECT(!cos_obj_node_is_integer((CosObjNode *)real_node));

    TEST_EXPECT(cos_obj_node_is_array((CosObjNode *)array_node));
    TEST_EXPECT(!cos_obj_node_is_dict((CosObjNode *)array_node));

    cos_obj_node_release((CosObjNode *)bool_node);
    cos_obj_node_release((CosObjNode *)int_node);
    cos_obj_node_release((CosObjNode *)real_node);
    cos_obj_node_release((CosObjNode *)array_node);
    return EXIT_SUCCESS;
}

// MARK: - Test driver

TEST_MAIN()
{
    /* Lifecycle */
    TEST_EXPECT(create_fromDirectIntNode_succeeds() == EXIT_SUCCESS);
    TEST_EXPECT(create_fromIndirectNode_resolvesType() == EXIT_SUCCESS);
    TEST_EXPECT(create_fromNullNode_isNull() == EXIT_SUCCESS);

    /* Scalar accessors */
    TEST_EXPECT(getBoolValue_boolObj_returnsValue() == EXIT_SUCCESS);
    TEST_EXPECT(getRealValue_realObj_returnsValue() == EXIT_SUCCESS);
    TEST_EXPECT(getIntValue_typeMismatch_returnsZero() == EXIT_SUCCESS);

    /* Array */
    TEST_EXPECT(getArrayCount_arrayObj_returnsCount() == EXIT_SUCCESS);

    /* Dict */
    TEST_EXPECT(getDictValue_existingKey_returnsValue() == EXIT_SUCCESS);
    TEST_EXPECT(getDictValue_nonExistingKey_returnsNull() == EXIT_SUCCESS);

    /* Dict iterator */
    TEST_EXPECT(dictIterator_singleEntry_yieldsEntry() == EXIT_SUCCESS);

    /* Concrete type predicates */
    TEST_EXPECT(isDict_dictNode_returnsTrue() == EXIT_SUCCESS);
    TEST_EXPECT(isDict_indirectNodeHoldingDict_returnsFalse() == EXIT_SUCCESS);
    TEST_EXPECT(isInteger_indirectNodeHoldingInt_returnsFalse() == EXIT_SUCCESS);
    TEST_EXPECT(isName_nameNode_returnsTrue() == EXIT_SUCCESS);
    TEST_EXPECT(isNull_nullNode_returnsTrue() == EXIT_SUCCESS);
    TEST_EXPECT(predicates_eachNodeType_onlyItsOwnIsTrue() == EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

COS_ASSUME_NONNULL_END
