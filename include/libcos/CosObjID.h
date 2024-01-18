/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_OBJ_ID_H
#define LIBCOS_COS_OBJ_ID_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stdbool.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief An indirect object identifier.
 *
 * An indirect object identifier is a pair of numbers that uniquely identify an
 * indirect object within a PDF file.
 */
struct CosObjID {
    /**
     * @brief The object number.
     *
     * @invariant The object number is always greater than zero.
     */
    unsigned int obj_number;

    /**
     * @brief The generation number.
     *
     * @note The generation number is usually zero, but it can be non-zero for objects
     * that have been modified.
     */
    unsigned int gen_number;
};

extern const CosObjID CosObjID_Invalid;

/**
 * @brief Creates an object ID.
 *
 * @param obj_number The object number.
 * @param gen_number The generation number.
 *
 * @return The object ID.
 *
 * @pre @p obj_number must be greater than zero.
 */
CosObjID
cos_obj_id_make(unsigned int obj_number,
                unsigned int gen_number);

bool
cos_obj_id_is_valid(CosObjID obj_id)
    COS_ATTR_PURE;

/**
 * @brief Compares two object IDs.
 *
 * @param lhs The first object ID.
 * @param rhs The second object ID.
 *
 * @return An integer less than, equal to, or greater than zero if @p lhs is less than,
 * equal to, or greater than @p rhs, respectively.
 */
int
cos_obj_id_compare(CosObjID lhs, CosObjID rhs)
    COS_ATTR_PURE
    COS_WARN_UNUSED_RESULT;

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_OBJ_ID_H */
