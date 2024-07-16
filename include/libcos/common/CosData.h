/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_DATA_H
#define LIBCOS_COMMON_COS_DATA_H

#include <libcos/common/CosDataRef.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>
#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief A data object.
 */
struct CosData {
    /**
     * @brief The bytes of the data object.
     */
    unsigned char *bytes;

    /**
     * @brief The size of the data object.
     */
    size_t size;

    /**
     * @brief The capacity of the data object.
     */
    size_t capacity;
};

/**
 * @brief Allocates a new data object.
 *
 * @return A pointer to the allocated data object, or @c NULL if an error occurred.
 */
CosData * COS_Nullable
cos_data_alloc(size_t capacity_hint)
    COS_MALLOC_LIKE
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Frees a data object.
 *
 * @param data The data object to free.
 */
void
cos_data_free(CosData *data);

/**
 * @brief Creates a copy of a data object.
 *
 * @param data The data object to copy.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return A pointer to the copied data object, or @c NULL if an error occurred.
 */
CosData * COS_Nullable
cos_data_copy(const CosData *data,
              CosError * COS_Nullable error)
    COS_MALLOC_LIKE
    COS_WARN_UNUSED_RESULT;

/**
 * @brief Returns a range of bytes from the data object.
 *
 * @param data The data object.
 * @param offset The offset from the start of the data object.
 * @param length The length of the range.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return A reference to the range of bytes, or @c NULL if an error occurred.
 */
CosDataRef
cos_data_get_range(const CosData *data,
                   size_t offset,
                   size_t length,
                   CosError * COS_Nullable error);

/**
 * @brief Reserves capacity for the data object.
 *
 * @param data The data object.
 * @param capacity The capacity to reserve.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the capacity was reserved, otherwise @c false.
 */
bool
cos_data_reserve(CosData *data,
                 size_t capacity,
                 CosError * COS_Nullable error);

/**
 * @brief Resets the data buffer.
 *
 * @param data The data buffer.
 *
 * @return @c true if the data buffer was reset, otherwise @c false.
 */
bool
cos_data_reset(CosData *data);

/**
 * @brief Appends bytes to the data buffer.
 *
 * @param data The data buffer.
 * @param bytes The bytes to append.
 * @param count The number of bytes to append.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the bytes were appended, otherwise @c false.
 */
bool
cos_data_append(CosData *data,
                const unsigned char *bytes,
                size_t count,
                CosError * COS_Nullable error);

/**
 * @brief Appends a single byte to the data buffer.
 *
 * @param data The data buffer.
 * @param byte The byte to append.
 * @param error On input, a pointer to an error object, or @c NULL.
 * On output, if an error occurred, the error object will be set with the error information.
 *
 * @return @c true if the byte was appended, otherwise @c false.
 */
bool
cos_data_push_back(CosData *data,
                   unsigned char byte,
                   CosError * COS_Nullable error);

// MARK: - Data reference

/**
 * @brief Returns a constant reference to the data object.
 *
 * The reference is valid until the data object is modified or freed.
 *
 * @param data The data object.
 *
 * @return A constant reference to the data object.
 */
CosDataRef
cos_data_get_ref(const CosData *data);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_DATA_H */
