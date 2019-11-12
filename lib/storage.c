/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */

#include "storage.h"

static TDB_CONTEXT *
open_storage_table(const char *dirname, const char *tname,
                   bool shared, bool create)
{
    char path[_POSIX_PATH_MAX];

    if (dirname != NULL)
    {
        int rc = snprintf(path, sizeof(path), "%s/%s", dirname, tname);
        if (rc < 0 || (size_t)rc > sizeof(path))
            return NULL;
    }

    return tdb_open(dirname == NULL ? dirname : path,
                    0,
                    (dirname == NULL ? TDB_INTERNAL | TDB_NOLOCK : 0) |
                    (shared ? 0 : TDB_NOLOCK),
                    O_RDWR | (create ? O_CREAT : 0),
                    S_IRUSR | S_IWUSR |
                    (shared ? S_IRGRP | S_IWGRP : 0));
}

bool
tn_storage_init(tn_storage *storage, const char *dirname, bool shared)
{
    TN_BUG_ON(storage->state != NULL);

    storage->state = open_storage_table(dirname, "state", shared, false);
    storage->values = open_storage_table(dirname, "values", shared, false);
    if (storage->values == NULL)
    {
        tn_storage_cleanup(storage);
        return false;
    }
    storage->refcnts = open_storage_table(dirname, "refcnts", shared, false);
    if (storage->refcnts == NULL)
    {
        tn_storage_cleanup(storage);
        return false;
    }
    storage->keys = open_storage_table(dirname, "keys", shared, false);
    if (storage->keys == NULL)
    {
        tn_storage_cleanup(storage);
        return false;
    }
    storage->parents = open_storage_table(dirname, "keys", shared, false);
    if (storage->parents == NULL)
    {
        tn_storage_cleanup(storage);
        return false;
    }
    if (dirname == NULL || shared)
        storage->notifications = NULL;
    else
    {
        storage->notifications = open_storage_table(dirname, "notify",
                                                    true, true);
        if (storage->notifications == NULL)
        {
            tn_storage_cleanup(storage);
            return false;
        }
    }
    return true;
}

void
tn_storage_cleanup(tn_storage *storage)
{
    if (storage->state != NULL)
        tdb_close(storage->state);
    if (storage->values != NULL)
        tdb_close(storage->values);
    if (storage->refcnts != NULL)
        tdb_close(storage->refcnts);
    if (storage->keys != NULL)
        tdb_close(storage->keys);
    if (storage->parents != NULL)
        tdb_close(storage->parents);
    if (storage->notifications != NULL)
        tdb_close(storage->notifications);
}
