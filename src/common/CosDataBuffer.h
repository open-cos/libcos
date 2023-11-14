//
// Created by david on 24/08/23.
//

#ifndef LIBCOS_COMMON_COS_DATA_BUFFER_H
#define LIBCOS_COMMON_COS_DATA_BUFFER_H

#include <libcos/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosError.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Allocates a new data buffer.
 *
 * @return A pointer to the allocated data buffer, or NULL if an error occurred.
 */
CosDataBuffer *
cos_data_buffer_alloc(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Frees a data buffer.
 *
 * @param data_buffer The data buffer to free.
 */
void
cos_data_buffer_free(CosDataBuffer *data_buffer);

/**
 * @brief Returns the size of the data buffer.
 *
 * @param data_buffer The data buffer.
 * @return The size of the data buffer.
 */
size_t
cos_data_buffer_get_size(const CosDataBuffer *data_buffer);

/**
 * @brief Returns a pointer to the bytes of the data buffer.
 *
 * @param data_buffer The data buffer.
 *
 * @return A pointer to the bytes of the data buffer.
 */
CosByte *
cos_data_buffer_get_bytes(const CosDataBuffer *data_buffer);

/**
 * @brief Returns the capacity of the data buffer.
 *
 * @param data_buffer The data buffer.
 *
 * @return The capacity of the data buffer.
 */
size_t
cos_data_buffer_get_capacity(const CosDataBuffer *data_buffer);

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
                          CosError **error);

/**
 * @brief Appends bytes to the data buffer.
 *
 * @param data_buffer The data buffer.
 *
 * @param bytes The bytes to append.
 *
 * @param size The number of bytes to append.
 *
 * @return @c true if the bytes were appended, otherwise @c false.
 */
bool
cos_data_buffer_append(CosDataBuffer *data_buffer,
                       const CosByte *bytes,
                       size_t size);

#endif /* LIBCOS_COMMON_COS_DATA_BUFFER_H */
