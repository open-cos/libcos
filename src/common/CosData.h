//
// Created by david on 20/08/23.
//

#ifndef LIBCOS_COMMON_COS_DATA_H
#define LIBCOS_COMMON_COS_DATA_H

#include <libcos/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stddef.h>

struct CosData {
    unsigned char *bytes;
    size_t size;
};

/**
 * @brief Allocates a new data object.
 *
 * @return A pointer to the allocated data object, or @c NULL if an error occurred.
 */
CosData *
cos_data_alloc(void)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Frees a data object.
 *
 * @param data The data object to free.
 */
void
cos_data_free(CosData *data);

/**
 * @brief Copies a range of bytes from the data object to a buffer.
 *
 * @param data The data object.
 * @param offset The offset of the bytes to return.
 * @param length The length of the bytes to return.
 * @param buffer The buffer to copy the bytes to.
 * @param buffer_size The size of the buffer.
 * @return The number of bytes copied to the buffer.
 */
size_t
cos_data_get_byte_range(const CosData *data,
                        size_t offset,
                        size_t length,
                        CosByte *buffer,
                        size_t buffer_size);

#endif /* LIBCOS_COMMON_COS_DATA_H */
