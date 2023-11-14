//
// Created by david on 27/10/23.
//

#ifndef LIBCOS_COMMON_COS_HASH_TABLE_H
#define LIBCOS_COMMON_COS_HASH_TABLE_H

#include <libcos/common/CosDefines.h>

#include <stdbool.h>
#include <stddef.h>

typedef struct CosHashTable CosHashTable;

typedef struct CosHashTableFunctions {
    size_t (*hash)(const void *key);
    bool (*equal)(const void *lhs,
                  const void *rhs);
} CosHashTableFunctions;

CosHashTable *
cos_hash_table_alloc(CosHashTableFunctions functions,
                     size_t capacity)
    COS_ATTR_MALLOC
    COS_WARN_UNUSED_RESULT;

void
cos_hash_table_free(CosHashTable *table);

size_t
cos_hash_table_get_size(const CosHashTable *table);

bool
cos_hash_table_get(const CosHashTable *table,
                   const void *key,
                   void ** COS_Nullable value);

bool
cos_hash_table_set(CosHashTable *table,
                   const void *key,
                   void * COS_Nullable value);

#endif /* LIBCOS_COMMON_COS_HASH_TABLE_H */
