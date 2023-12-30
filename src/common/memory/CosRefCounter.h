/*
 * Copyright (c) 2023 OpenCOS.
 */

#ifndef LIBCOS_COS_REF_COUNTER_H
#define LIBCOS_COS_REF_COUNTER_H

#include <libcos/common/CosDefines.h>

#include <stddef.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

typedef struct CosRefCounter CosRefCounter;

struct CosRefCounter {
    size_t count;

    void * COS_Nullable context;
    void (*retain)(CosRefCounter *counter);
    void (*release)(CosRefCounter *counter);
    void (*dealloc)(CosRefCounter *counter);
};

void
cos_ref_counter_init(CosRefCounter *counter,
                     void * COS_Nullable context,
                     void (*retain)(CosRefCounter *counter),
                     void (*release)(CosRefCounter *counter),
                     void (*dealloc)(CosRefCounter *counter));

void
cos_ref_counter_retain(CosRefCounter *counter);

void
cos_ref_counter_release(CosRefCounter *counter);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_COS_REF_COUNTER_H */
