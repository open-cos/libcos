/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/CosList.h"

#include "common/Assert.h"
#include "libcos/common/CosError.h"

#include <stdlib.h>

COS_ASSUME_NONNULL_BEGIN

typedef struct CosListNode CosListNode;

struct CosListNode {
    CosListNode * COS_Nullable next;

    void *value;
};

static CosListNode * COS_Nullable
cos_list_node_alloc_(void *value)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT
    COS_ATTR_ACCESS_READ_ONLY(1);

static void
cos_list_node_free_(CosListNode *node);

struct CosList {
    CosListNode * COS_Nullable head;
    CosListNode * COS_Nullable tail;

    size_t count;
};

#pragma mark - Public

CosList *
cos_list_alloc(void)
{
    CosList * const list = calloc(1, sizeof(CosList));
    if (!list) {
        return NULL;
    }

    list->count = 0;

    return list;
}

void
cos_list_free(CosList *list)
{
    if (!list) {
        return;
    }

    cos_list_clear(list);

    free(list);
}

#pragma mark Accessors

size_t
cos_list_get_count(const CosList *list)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return 0;
    }

    return list->count;
}

void *
cos_list_get_first(const CosList *list)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return NULL;
    }

    if (!list->head) {
        return NULL;
    }

    return list->head->value;
}

void *
cos_list_get_last(const CosList *list)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return NULL;
    }

    if (!list->tail) {
        return NULL;
    }

    return list->tail->value;
}

void *
cos_list_get_at(const CosList *list,
                size_t index,
                CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return NULL;
    }

    if (index >= list->count) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                           "Index out of bounds"),
                            error);
        return NULL;
    }

    CosListNode *node = list->head;
    size_t i = 0;
    while ((node != NULL) && (i < index)) {
        node = node->next;
        i++;
    }
    COS_ASSERT(node != NULL, "Expected a list node");
    if (!node) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to find node at index"),
                            error);
        return NULL;
    }

    return node->value;
}

#pragma mark Operations

bool
cos_list_prepend(CosList *list,
                 void *item,
                 CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return false;
    }

    if (!item) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Item cannot be NULL"),
                            error);
        return false;
    }

    CosListNode * const node = cos_list_node_alloc_(item);
    if (!node) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate node"),
                            error);
        return false;
    }

    // Set the tail if this is the first node.
    if (!list->tail) {
        COS_ASSERT(!list->head, "Head and tail should both be NULL");
        COS_ASSERT(list->count == 0, "Expected an empty list");

        list->tail = node;
    }

    node->next = list->head;
    list->head = node;

    list->count++;

    return true;
}

bool
cos_list_append(CosList *list,
                void *item,
                CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return false;
    }

    if (!item) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Item cannot be NULL"),
                            error);
        return false;
    }

    CosListNode * const node = cos_list_node_alloc_(item);
    if (!node) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate node"),
                            error);
        return false;
    }

    // Set the head if this is the first node.
    if (!list->head) {
        COS_ASSERT(!list->tail, "Head and tail should both be NULL");
        COS_ASSERT(list->count == 0, "Expected an empty list");

        list->head = node;
    }

    if (list->tail) {
        list->tail->next = node;
    }
    list->tail = node;

    list->count++;

    return true;
}

bool
cos_list_insert(CosList *list,
                size_t index,
                void *item,
                CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return false;
    }

    if (index > list->count) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                           "Index out of bounds"),
                            error);
        return false;
    }

    if (!item) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_ARGUMENT,
                                           "Item cannot be NULL"),
                            error);
        return false;
    }

    if (index == 0) {
        return cos_list_prepend(list, item, error);
    }
    else if (index == list->count) {
        return cos_list_append(list, item, error);
    }

    CosListNode * const node = cos_list_node_alloc_(item);
    if (!node) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_MEMORY,
                                           "Failed to allocate node"),
                            error);
        return false;
    }

    CosListNode *prev_node = list->head;
    size_t i = 0;
    while ((prev_node != NULL) && (i + 1 < index)) {
        prev_node = prev_node->next;
        i++;
    }

    COS_ASSERT(prev_node != NULL, "Expected a list node");
    if (!prev_node) {
        cos_list_node_free_(node);

        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to find node at index"),
                            error);
        return false;
    }

    node->next = prev_node->next;
    prev_node->next = node;

    list->count++;

    return true;
}

bool
cos_list_remove_at(CosList *list,
                   size_t index,
                   CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return false;
    }

    if (index >= list->count) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_OUT_OF_RANGE,
                                           "Index out of bounds"),
                            error);
        return false;
    }

    if (index == 0) {
        return cos_list_remove_first(list);
    }
    else if (index == list->count - 1) {
        return cos_list_remove_last(list, error);
    }

    CosListNode *prev_node = list->head;
    size_t i = 0;
    while ((prev_node != NULL) && (i + 1 < index)) {
        prev_node = prev_node->next;
        i++;
    }

    COS_ASSERT(prev_node != NULL, "Expected a list node");
    if (!prev_node) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to find node at index"),
                            error);
        return false;
    }

    CosListNode * const node = prev_node->next;
    if (!node) {
        COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                           "Failed to find node at index"),
                            error);
        return false;
    }

    prev_node->next = node->next;

    cos_list_node_free_(node);
    list->count--;

    return true;
}

bool
cos_list_remove_first(CosList *list)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return false;
    }

    if (!list->head) {
        // Nothing to remove.
        return true;
    }

    CosListNode * const node = list->head;
    list->head = node->next;

    if (!list->head) {
        list->tail = NULL;
    }

    cos_list_node_free_(node);
    list->count--;

    return true;
}

bool
cos_list_remove_last(CosList *list,
                     CosError * COS_Nullable error)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return false;
    }

    if (!list->tail) {
        // Nothing to remove.
        return true;
    }

    CosListNode * const node = list->tail;

    if (list->head == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    }
    else {
        CosListNode *prev_node = list->head;
        while ((prev_node != NULL) && (prev_node->next != node)) {
            prev_node = prev_node->next;
        }

        COS_ASSERT(prev_node != NULL, "Expected a list node");
        if (!prev_node) {
            COS_ERROR_PROPAGATE(cos_error_make(COS_ERROR_INVALID_STATE,
                                               "Failed to find node at index"),
                                error);
            return false;
        }

        prev_node->next = NULL;
        list->tail = prev_node;
    }

    cos_list_node_free_(node);
    list->count--;

    return true;
}

void
cos_list_clear(CosList *list)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return;
    }

    CosListNode *node = list->head;
    while (node != NULL) {
        CosListNode * const next_node = node->next;

        cos_list_node_free_(node);

        node = next_node;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

void *
cos_list_pop_first(CosList *list)
{
    COS_PARAMETER_ASSERT(list != NULL);
    if (!list) {
        return NULL;
    }

    CosListNode * const first = list->head;
    if (!first) {
        // Nothing to pop.
        return NULL;
    }

    void * const value = first->value;

    cos_list_remove_first(list);

    return value;
}

#pragma mark - Private

static CosListNode *
cos_list_node_alloc_(void *value)
{
    COS_PARAMETER_ASSERT(value != NULL);

    CosListNode * const node = calloc(1, sizeof(CosListNode));
    if (!node) {
        return NULL;
    }

    node->next = NULL;
    node->value = value;

    return node;
}

static void
cos_list_node_free_(CosListNode *node)
{
    if (!node) {
        return;
    }

    free(node);
}

COS_ASSUME_NONNULL_END
