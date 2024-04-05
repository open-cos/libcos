/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/common/CosRingBuffer.h"

#include "common/CosError.h"

#include <stdlib.h>
#include <string.h>

#include "Assert.h"

COS_ASSUME_NONNULL_BEGIN

struct CosRingBuffer {
    size_t element_size;
    size_t capacity;
    size_t count;
    size_t head;
    size_t tail;
    unsigned char *data COS_ATTR_NONSTRING;
};

/**
 * @brief Sets an item in a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 * @param index The index of the item.
 * @param item The item.
 */
static void
cos_ring_buffer_set_item_(CosRingBuffer *ring_buffer,
                          size_t index,
                          const void *item);

/**
 * @brief Resizes a ring buffer.
 *
 * @param ring_buffer The ring buffer.
 * @param required_capacity The required capacity.
 * @param out_error On input, a pointer to an error object, or @c NULL.
 * If an error occurs, an error object is stored at the location pointed to by this parameter.
 *
 * @return @c true if the ring buffer was resized, `false` otherwise.
 */
static bool
cos_ring_buffer_resize_(CosRingBuffer *ring_buffer,
                        size_t required_capacity,
                        CosError * COS_Nullable out_error);

CosRingBuffer *
cos_ring_buffer_create(size_t element_size,
                       size_t capacity_hint)
{
    COS_PARAMETER_ASSERT(element_size > 0);
    if (element_size == 0) {
        return NULL;
    }

    CosRingBuffer *ring_buffer;

    ring_buffer = calloc(1, sizeof(CosRingBuffer));
    if (!ring_buffer) {
        goto failure;
    }

    ring_buffer->element_size = element_size;
    ring_buffer->capacity = capacity_hint;
    ring_buffer->data = calloc(capacity_hint, element_size);
    if (!ring_buffer->data) {
        goto failure;
    }

    return ring_buffer;

failure:
    if (ring_buffer) {
        free(ring_buffer);
    }
    return NULL;
}

void
cos_ring_buffer_destroy(CosRingBuffer *ring_buffer)
{
    if (!ring_buffer) {
        return;
    }

    free(ring_buffer->data);
    free(ring_buffer);
}

size_t
cos_ring_buffer_get_count(const CosRingBuffer *ring_buffer)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    if (!ring_buffer) {
        return 0;
    }

    return ring_buffer->count;
}

size_t
cos_ring_buffer_get_capacity(const CosRingBuffer *ring_buffer)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    if (!ring_buffer) {
        return 0;
    }

    return ring_buffer->capacity;
}

void *
cos_ring_buffer_get_item(const CosRingBuffer *ring_buffer,
                         size_t index,
                         CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    if (!ring_buffer) {
        return NULL;
    }

    if (index >= ring_buffer->count) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_OUT_OF_RANGE, "Index out of range"));
        return NULL;
    }

    const size_t ring_index = (ring_buffer->head + index) % ring_buffer->capacity;

    return ring_buffer->data + (ring_index * ring_buffer->element_size);
}

void *
cos_ring_buffer_get_first_item(const CosRingBuffer *ring_buffer)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    if (!ring_buffer) {
        return NULL;
    }

    if (ring_buffer->count == 0) {
        return NULL;
    }

    const size_t item_data_offset = ring_buffer->head * ring_buffer->element_size;

    return ring_buffer->data + item_data_offset;
}

void *
cos_ring_buffer_get_last_item(const CosRingBuffer *ring_buffer)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    if (!ring_buffer) {
        return NULL;
    }

    if (ring_buffer->count == 0) {
        return NULL;
    }

    const size_t item_data_offset = ring_buffer->tail * ring_buffer->element_size;

    return ring_buffer->data + item_data_offset;
}

bool
cos_ring_buffer_push_front(CosRingBuffer *ring_buffer,
                           const void *item,
                           CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    COS_PARAMETER_ASSERT(item != NULL);
    if (!ring_buffer || !item) {
        return false;
    }

    // Resize the ring buffer if necessary.
    if (!cos_ring_buffer_resize_(ring_buffer,
                                 ring_buffer->count + 1,
                                 out_error)) {
        return false;
    }

    if (ring_buffer->head == 0) {
        ring_buffer->head = ring_buffer->capacity - 1;
    }
    else {
        ring_buffer->head--;
    }

    cos_ring_buffer_set_item_(ring_buffer, 0, item);
    ring_buffer->count++;

    return true;
}

bool
cos_ring_buffer_push_back(CosRingBuffer *ring_buffer,
                          const void *item,
                          CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    COS_PARAMETER_ASSERT(item != NULL);
    if (!ring_buffer || !item) {
        return false;
    }

    // Resize the ring buffer if necessary.
    if (!cos_ring_buffer_resize_(ring_buffer,
                                 ring_buffer->count + 1,
                                 out_error)) {
        return false;
    }

    ring_buffer->tail = (ring_buffer->tail + 1) % ring_buffer->capacity;

    cos_ring_buffer_set_item_(ring_buffer, ring_buffer->count, item);
    ring_buffer->count++;

    return true;
}

void *
cos_ring_buffer_pop_front(CosRingBuffer *ring_buffer,
                          CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    if (!ring_buffer) {
        cos_error_propagate(out_error,
                            cos_error_make_invalid_argument("ring_buffer is NULL"));
        return NULL;
    }

    if (ring_buffer->count == 0) {
        return NULL;
    }

    void * const item = cos_ring_buffer_get_first_item(ring_buffer);

    ring_buffer->head = (ring_buffer->head + 1) % ring_buffer->capacity;
    ring_buffer->count--;

    return item;
}

void *
cos_ring_buffer_pop_back(CosRingBuffer *ring_buffer,
                         CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    if (!ring_buffer) {
        cos_error_propagate(out_error,
                            cos_error_make_invalid_argument("ring_buffer is NULL"));
        return NULL;
    }

    if (ring_buffer->count == 0) {
        return NULL;
    }

    void * const item = cos_ring_buffer_get_last_item(ring_buffer);
    COS_ASSERT(item != NULL, "Expected an item.");

    if (ring_buffer->tail == 0) { // Wrap around.
        COS_ASSERT(ring_buffer->capacity > 0, "");
        ring_buffer->tail = ring_buffer->capacity - 1;
    }
    else {
        ring_buffer->tail--;
    }

    ring_buffer->count--;

    return item;
}

static void
cos_ring_buffer_set_item_(CosRingBuffer *ring_buffer,
                          size_t index,
                          const void *item)
{
    const size_t ring_index = (ring_buffer->head + index) % ring_buffer->capacity;
    const size_t item_data_offset = ring_index * ring_buffer->element_size;

    memcpy(ring_buffer->data + item_data_offset,
           item,
           ring_buffer->element_size);
}

static bool
cos_ring_buffer_resize_(CosRingBuffer *ring_buffer,
                        size_t required_capacity,
                        CosError * COS_Nullable out_error)
{
    COS_PARAMETER_ASSERT(ring_buffer != NULL);
    COS_PARAMETER_ASSERT(required_capacity > 0);
    if (!ring_buffer || required_capacity == 0) {
        return false;
    }

    const size_t old_capacity = ring_buffer->capacity;
    if (old_capacity >= required_capacity) {
        return true;
    }

    const size_t old_count = ring_buffer->count;
    const size_t old_head = ring_buffer->head;
    const size_t old_tail = ring_buffer->tail;

    size_t new_capacity = ring_buffer->capacity;
    while (new_capacity < required_capacity) {
        new_capacity *= 2;
    }

    unsigned char * const new_data = calloc(new_capacity, ring_buffer->element_size);
    if (!new_data) {
        cos_error_propagate(out_error,
                            cos_error_make(COS_ERROR_MEMORY, "Out of memory"));
        return false;
    }

    const size_t new_head = 0;
    const size_t new_tail = old_count;

    if (old_tail > old_head) {
        // The data is contiguous.
        memcpy(new_data,
               ring_buffer->data + (old_head * ring_buffer->element_size),
               old_count * ring_buffer->element_size);
    }
    else { // The data is split.
        // Copy the first part of the data.
        // The first part of the data is from the head to the end of the buffer.
        const void * const first_part_src = ring_buffer->data + (old_head * ring_buffer->element_size);
        void * const first_part_dst = new_data;
        const size_t first_part_size = (old_capacity - old_head) * ring_buffer->element_size;

        memcpy(first_part_dst,
               first_part_src,
               first_part_size);

        // Copy the second part of the data.
        // The second part of the data is from the start of the buffer to the tail.
        const void * const second_part_src = ring_buffer->data;
        void * const second_part_dst = new_data + first_part_size;
        const size_t second_part_size = old_tail * ring_buffer->element_size;

        memcpy(second_part_dst,
               second_part_src,
               second_part_size);
    }

    free(ring_buffer->data);

    ring_buffer->data = new_data;
    ring_buffer->capacity = new_capacity;
    ring_buffer->head = new_head;
    ring_buffer->tail = new_tail;

    return true;
}

COS_ASSUME_NONNULL_END
