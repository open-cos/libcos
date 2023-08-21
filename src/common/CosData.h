//
// Created by david on 20/08/23.
//

#ifndef LIBCOS_COMMON_COS_DATA_H
#define LIBCOS_COMMON_COS_DATA_H

#include <libcos/CosBasicTypes.h>
#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

#include <stddef.h>

CosData *
cos_data_alloc(void);

void
cos_data_free(CosData *data);

size_t
cos_data_get_size(const CosData *data);

CosByte *
cos_data_get_bytes(const CosData *data);

size_t
cos_data_get_byte_range(const CosData *data,
                        size_t offset,
                        size_t length,
                        CosByte *buffer,
                        size_t buffer_size);

#endif /* LIBCOS_COMMON_COS_DATA_H */
