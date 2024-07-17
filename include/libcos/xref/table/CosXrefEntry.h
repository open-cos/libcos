/*
 * Copyright (c) 2024 OpenCOS.
 */

#ifndef LIBCOS_XREF_COS_XREF_ENTRY_H
#define LIBCOS_XREF_COS_XREF_ENTRY_H

#include <libcos/common/CosDefines.h>
#include <libcos/common/CosTypes.h>

COS_DECLS_BEGIN
COS_ASSUME_NONNULL_BEGIN

/**
 * @brief An in-use entry in the cross-reference table.
 */
struct CosXrefInUseEntry {
    /**
     * @brief The byte offset of the object in the decoded stream.
     *
     * This is the number of bytes from the beginning of the file to the beginning of the
     * object.
     */
    unsigned int byte_offset;

    /**
     * @brief The generation number of the object.
     */
    unsigned int gen_number;
};

/**
 * @brief A free entry in the cross-reference table.
 */
struct CosXrefFreeEntry {
    /**
     * @brief The object number of the next free object.
     */
    unsigned int next_free_obj_number;

    /**
     * @brief The generation number of the object.
     */
    unsigned int gen_number;
};

typedef enum CosXrefEntryType {
    /**
     * @brief An in-use entry.
     */
    CosXrefEntryType_InUse,

    /**
     * @brief A free entry.
     */
    CosXrefEntryType_Free,
} CosXrefEntryType;

struct CosXrefEntry {
    /**
     * @brief The type of the entry.
     */
    CosXrefEntryType type;

    union {
        struct CosXrefInUseEntry in_use;
        struct CosXrefFreeEntry free;
    } value;
};

void
cos_xref_entry_init_in_use(CosXrefEntry *entry,
                           unsigned int byte_offset,
                           unsigned int gen_number);

void
cos_xref_entry_init_free(CosXrefEntry *entry,
                         unsigned int next_free_obj_number,
                         unsigned int gen_number);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_ENTRY_H */
