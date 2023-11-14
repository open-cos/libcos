//
// Created by david on 27/10/23.
//

#include "CosHashTable.h"

#include <stdlib.h>

typedef struct CosHashTableBucket CosHashTableBucket;

struct CosHashTable {
    CosHashTableFunctions functions;
    size_t capacity;
    size_t size;
    CosHashTableBucket *buckets;
};

struct CosHashTableBucket {
    void *key;
    void *value;
    CosHashTableBucket *next;
};

CosHashTable *
cos_hash_table_alloc(CosHashTableFunctions functions,
                     size_t capacity)
{
    CosHashTable *table = malloc(sizeof(CosHashTable));
    if (!table) {
        return NULL;
    }

    table->functions = functions;
    table->capacity = capacity;
    table->size = 0;
    table->buckets = calloc(capacity,
                            sizeof(CosHashTableBucket));
    if (!table->buckets) {
        free(table);
        return NULL;
    }

    return table;
}
