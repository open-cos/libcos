//
// Created by david on 24/08/23.
//

#ifndef LIBCOS_COMMON_COS_DATA_BUFFER_H
#define LIBCOS_COMMON_COS_DATA_BUFFER_H

#include <libcos/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

struct CosDataBuffer {
    CosByte *bytes;
    size_t size;
    size_t capacity;
};

/**
 * @brief Allocates a new data buffer.
 *
 * @return A pointer to the allocated data buffer, or NULL if an error occurred.
 */
CosDataBuffer *
cos_data_buffer_alloc(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_data_buffer_init(CosDataBuffer *data_buffer);

void
cos_data_buffer_destroy(CosDataBuffer *data_buffer);

bool
cos_data_buffer_reserve(CosDataBuffer *data_buffer,
                        size_t capacity,
                        CosError * COS_Nullable error);

/**
 * @brief Frees a data buffer.
 *
 * @param data_buffer The data buffer to free.
 */
void
cos_data_buffer_free(CosDataBuffer *data_buffer);

/**
 * @brief Resets the data buffer.
 *
 * @param data_buffer The data buffer.
 *
 * @return @c true if the data buffer was reset, otherwise @c false.
 */
bool
cos_data_buffer_reset(CosDataBuffer *data_buffer);

/**
 * @brief Appends a byte to the data buffer.
 *
 * @param data_buffer The data buffer.
 *
 * @param byte The byte to append.
 *
 * @param error On input, a pointer to an error object, or <tt>NULL</tt>.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the byte was appended, otherwise @c false.
 */
bool
cos_data_buffer_push_back(CosDataBuffer *data_buffer,
                          CosByte byte,
                          CosError * COS_Nullable error);

/**
 * @brief Appends bytes to the data buffer.
 *
 * @param data_buffer The data buffer.
 *
 * @param bytes The bytes to append.
 *
 * @param count The number of bytes to append.
 *
* @param error On input, a pointer to an error object, or @c NULL.
* On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the bytes were appended, otherwise @c false.
 */
bool
cos_data_buffer_append(CosDataBuffer *data_buffer,
                       const CosByte *bytes,
                       size_t count,
                       CosError * COS_Nullable error);

CosString * COS_Nullable
cos_data_buffer_to_string(const CosDataBuffer *data_buffer)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

#endif /* LIBCOS_COMMON_COS_DATA_BUFFER_H */
