#include "testing.h"
#include "storage.h"

TESTDEF_SINGLE(test_storage_minimal_sanity,
               "In-memory storage creation and finalization works")
{
    tn_storage storage = {};

    tnt_assert(tn_storage_init(&storage, NULL, false));
    tn_storage_cleanup(&storage);
}
