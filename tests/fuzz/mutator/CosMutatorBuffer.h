/*
 * Copyright (c) 2026 OpenCOS.
 */

#ifndef LIBCOS_TESTS_FUZZ_COS_MUTATOR_BUFFER_H
#define LIBCOS_TESTS_FUZZ_COS_MUTATOR_BUFFER_H

#include <libcos/common/CosDefines.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * A growable byte buffer with a hard length cap.
 *
 * Every mutation writes through this type, and every operation refuses to push
 * the length past @c max_length. That is what keeps the engines' @c max_size
 * contract from having to be re-checked by each operator.
 */
typedef struct CosMutBuffer {
    unsigned char * COS_Nullable data;
    size_t length;
    size_t capacity;
    size_t max_length;
} CosMutBuffer;

void
cos_mut_buffer_init_(CosMutBuffer *buffer);

void
cos_mut_buffer_free_(CosMutBuffer *buffer);

/**
 * Sets the hard length cap. Truncates the contents if they already exceed it.
 */
void
cos_mut_buffer_set_max_(CosMutBuffer *buffer,
                        size_t max_length);

/**
 * Replaces the contents, truncating to the cap if @p size exceeds it.
 */
bool
cos_mut_buffer_assign_(CosMutBuffer *buffer,
                       const unsigned char * COS_Nullable data,
                       size_t size);

/**
 * Splices @p insert_size bytes in at @p offset, removing @p remove_size bytes
 * first.
 *
 * Returns false, leaving the buffer unchanged, when the result would exceed the
 * cap or when @p offset and @p remove_size do not name a range within the
 * buffer. Callers may treat a false return as "this mutation does not apply".
 *
 * @p insert must not alias the buffer's own storage. Growing the buffer may
 * reallocate it, and moving the tail may shift it, either of which would leave
 * an aliased source dangling or stale. An operator copying one region of the
 * buffer over another must stage the bytes elsewhere first.
 */
bool
cos_mut_buffer_splice_(CosMutBuffer *buffer,
                       size_t offset,
                       size_t remove_size,
                       const unsigned char * COS_Nullable insert,
                       size_t insert_size);

/**
 * Shortens the buffer. Growing is not permitted; a @p length at or above the
 * current length leaves the buffer unchanged.
 */
void
cos_mut_buffer_truncate_(CosMutBuffer *buffer,
                         size_t length);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_TESTS_FUZZ_COS_MUTATOR_BUFFER_H */
