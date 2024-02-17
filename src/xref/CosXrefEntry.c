/*
 * Copyright (c) 2024 OpenCOS.
 */

#include "libcos/xref/CosXrefEntry.h"

#include "common/Assert.h"

#include <stddef.h>

COS_ASSUME_NONNULL_BEGIN

void
cos_xref_entry_init_in_use(CosXrefEntry *entry,
                           unsigned int byte_offset,
                           unsigned int gen_number)
{
    COS_PARAMETER_ASSERT(entry != NULL);
    if (!entry) {
        return;
    }

    entry->type = CosXrefEntryType_InUse;
    entry->value.in_use.byte_offset = byte_offset;
    entry->value.in_use.gen_number = gen_number;
}

void
cos_xref_entry_init_free(CosXrefEntry *entry,
                         unsigned int next_free_obj_number,
                         unsigned int gen_number)
{
    COS_PARAMETER_ASSERT(entry != NULL);
    if (!entry) {
        return;
    }

    entry->type = CosXrefEntryType_Free;
    entry->value.free.next_free_obj_number = next_free_obj_number;
    entry->value.free.gen_number = gen_number;
}

COS_ASSUME_NONNULL_END
