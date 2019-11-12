/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */

/** @file
 *  Storage backend
 */
#ifndef TNH_STORAGE_H
#define TNH_STORAGE_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <tdb.h>
#include "values.h"

/**
 * Storage state.
 */
typedef struct tn_storage {
    uint64_t envid; /**< Runtime ID */
    TDB_CONTEXT *state; /**< State DB */
    TDB_CONTEXT *values; /**< Values */
    TDB_CONTEXT *refcnts; /**< Reference counters */
    TDB_CONTEXT *keys; /**< Association keys */
    TDB_CONTEXT *parents; /**< Value parents */
    TDB_CONTEXT *notifications; /**< Inter-runtime notifications */
} tn_storage;

/**
 * Storage journal.
 */
typedef struct tn_storage_journal {
    TDB_CONTEXT *values; /**< Values shadow */
    TDB_CONTEXT *keys;  /**< Keys shadow */
    TDB_CONTEXT *parents; /**< Parents shadow */
    struct tn_storage_journal *next; /**< @private */
} tn_storage_journal;

/**
 * Initialize the storage.
 */
TN_NOT_NULL_ARGS(1)
extern bool tn_storage_init(tn_storage *storage,
                            const char *dirname,
                            bool shared);

/**
 * Cleanup the storage.
 */
TN_NO_NULL_ARGS
extern void tn_storage_cleanup(tn_storage *storage);

/**
 * Increase a reference counter of value pointed to by @p ref
 */
TN_NO_NULL_ARGS
extern void tn_storage_link_value(tn_storage *storage, tn_ref ref);

/**
 * Unlink a value destroying if nothing else references it
 */
TN_NO_NULL_ARGS
extern void tn_storage_unlink_value(tn_storage *storage, tn_ref ref);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_STORAGE_H */
