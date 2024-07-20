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
 * @brief A free entry in the cross-reference table.
 */
typedef struct CosXrefFreeEntry {
    /**
     * @brief The object number of the next free object.
     */
    unsigned int next_free_obj_number;

    /**
     * @brief The generation number of the object.
     */
    unsigned int gen_number;
} CosXrefFreeEntry;

/**
 * @brief An in-use entry in the cross-reference table.
 */
typedef struct CosXrefInUseEntry {
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
} CosXrefInUseEntry;

/**
 * @brief A compressed cross-reference entry.
 */
typedef struct CosXrefCompressedEntry {
    /**
     * @brief The object number of the object stream in which this object is stored.
     *
     * This is the object number of the object stream that contains the object. The generation
     * number of the object stream is always 0.
     */
    unsigned int obj_stream_number;

    /**
     * @brief The index of this entry's object within the object stream.
     */
    unsigned int obj_stream_index;
} CosXrefCompressedEntry;

typedef enum CosXrefEntryType {
    /**
     * @brief A free entry.
     */
    CosXrefEntryType_Free,

    /**
     * @brief An in-use entry.
     */
    CosXrefEntryType_InUse,

    /**
     * @brief A compressed entry.
     */
    CosXrefEntryType_Compressed,
} CosXrefEntryType;

/**
 * @brief A cross-reference table entry.
 */
struct CosXrefEntry {
    /**
     * @brief The type of the entry.
     */
    CosXrefEntryType type;

    /**
     * @brief The entry's value.
     */
    union {
        /**
         * @brief The entry's value if it is a free entry.
         */
        CosXrefFreeEntry free;

        /**
         * @brief The entry's value if it is an in-use entry.
         */
        CosXrefInUseEntry in_use;

        /**
         * @brief The entry's value if it is a compressed entry.
         */
        CosXrefCompressedEntry compressed;
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

void
cos_xref_entry_init_compressed(CosXrefEntry *entry,
                               unsigned int obj_stream_number,
                               unsigned int obj_stream_index);

COS_ASSUME_NONNULL_END
COS_DECLS_END

#endif /* LIBCOS_XREF_COS_XREF_ENTRY_H */
