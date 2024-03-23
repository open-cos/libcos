/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_COMMON_COS_TYPED_ARRAY_H
#define LIBCOS_COMMON_COS_TYPED_ARRAY_H

#include <libcos/common/CosArray.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosMacros.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

#define COS_TYPED_ARRAY_DECLARE(T, name) COS_TYPED_ARRAY_DECLARE_IMPL_(T, COS_TYPED_ARRAY_NAME_(name))

#pragma mark - Implementation

#define COS_TYPED_ARRAY_NAME_(name) COS_PASTE(COS_PASTE(CosTyped, name), Array)

#define COS_TYPED_ARRAY_DECLARE_IMPL_(T, name)                                                                      \
                                                                                                                    \
    typedef struct CosArray name;                                                                                   \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    name * COS_Nullable COS_PASTE(name, _alloc)(void)                                                               \
        COS_ATTR_MALLOC                                                                                             \
            COS_WARN_UNUSED_RESULT                                                                                  \
    {                                                                                                               \
        return cos_array_create(sizeof(T), (CosArrayCallbacks){0}, 0);                                               \
    }                                                                                                               \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    void COS_PASTE(name, _free)(name * COS_Nullable array);                                                         \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    size_t COS_PASTE(name, _get_count)(const name *array);                                                          \
    COS_STATIC_INLINE                                                                                               \
    size_t COS_PASTE(name, _get_capacity)(const name *array);                                                       \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    void * COS_Nullable COS_PASTE(name, _get_item)(const name *array, size_t index, CosError * COS_Nullable error); \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    bool COS_PASTE(name, _insert_item)(name * array, size_t index, T item, CosError * COS_Nullable error);          \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    bool COS_PASTE(name, _append_item)(name * array,                                                                \
                                       void *item,                                                                  \
                                       CosError * COS_Nullable error);                                              \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    bool COS_PASTE(name, _insert_items)(name * array,                                                               \
                                        size_t index,                                                               \
                                        void *items,                                                                \
                                        size_t count,                                                               \
                                        CosError * COS_Nullable error);                                             \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    bool COS_PASTE(name, _append_items)(name * array,                                                               \
                                        void *items,                                                                \
                                        size_t count,                                                               \
                                        CosError * COS_Nullable error);                                             \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    bool COS_PASTE(name, _remove_item)(name * array,                                                                \
                                       size_t index,                                                                \
                                       CosError * COS_Nullable error);                                              \
                                                                                                                    \
    COS_STATIC_INLINE                                                                                               \
    bool COS_PASTE(name, _remove_items)(name * array,                                                               \
                                        size_t index,                                                               \
                                        size_t count,                                                               \
                                        CosError * COS_Nullable error);

COS_TYPED_ARRAY_DECLARE(int, Int)

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COMMON_COS_TYPED_ARRAY_H */
