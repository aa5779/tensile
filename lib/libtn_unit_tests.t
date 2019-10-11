#include "utils.h"

static int ctx_destroy_count;
static int child_destroy_count;

typedef struct mem_context {
    struct mem_child *child;
} mem_context;

typedef struct mem_child {
    int field;
} mem_child;

typedef struct mem_context_flex {
    struct mem_child_flex *child;
} mem_context_flex;

typedef struct mem_child_flex {
    int count;
    int flex[];
} mem_child_flex;


static int mem_context_destroy(TN_UNUSED void *arg)
{
    ctx_destroy_count++;
    return 0;
}

static int mem_child_destroy(TN_UNUSED void *arg)
{
    child_destroy_count++;
    return 0;
}

static int mem_context_flex_destroy(TN_UNUSED void *arg)
{
    ctx_destroy_count++;
    return 0;
}

static int mem_child_flex_destroy(void *arg)
{
    child_destroy_count += ((mem_child_flex *)arg)->count;
    return 0;
}


#suite Utils

#tcase Sanity

#test test_bug_on_ok
      TN_BUG_ON(false);

#test-exit(1) test_bug_on_abort
      TN_BUG_ON(true);

#test test_new_free_global
      mem_context *ctx = TN_NEW_GLOBAL(mem_context);
      ck_assert_ptr_eq(ctx->child, NULL);
      ctx_destroy_count = 0;
      TN_FREE_GLOBAL(ctx);
      ck_assert_int_eq(ctx_destroy_count, 1);

#test test_alloc_free_recursive
      mem_context *ctx = TN_NEW_GLOBAL(mem_context);
      ck_assert_ptr_eq(ctx->child, NULL);
      TN_SET(ctx, child, mem_child);
      ck_assert_int_eq(ctx->child->field, 0);
      ctx_destroy_count = child_destroy_count = 0;
      TN_FREE_GLOBAL(ctx);
      ck_assert_int_eq(ctx_destroy_count, 1);
      ck_assert_int_eq(child_destroy_count, 1);

#test test_alloc_free_shared
      mem_context *ctx1 = TN_NEW_GLOBAL(mem_context);
      mem_context *ctx2 = TN_NEW_GLOBAL(mem_context);
      TN_SET(ctx1, child, mem_child);
      TN_COPY(ctx2, child, ctx1->child);
      ck_assert_ptr_eq(ctx1->child, ctx2->child);
      ck_assert(tn_is_shared_ptr(ctx1->child));
      child_destroy_count = 0;
      TN_FREE_GLOBAL(ctx1);
      ck_assert(!tn_is_shared_ptr(ctx2->child));
      TN_FREE(ctx2, child);
      ck_assert_ptr_eq(ctx2->child, NULL);
      ck_assert_int_eq(child_destroy_count, 1);

#test test_alloc_flex
      mem_context_flex *ctx = TN_NEW_GLOBAL(mem_context_flex);
      TN_SET_FLEX(ctx, child, mem_child_flex, flex, 2);
      ctx->child->count = 2;
      ctx_destroy_count = child_destroy_count = 0;
      ck_assert_uint_eq(talloc_total_size(ctx->child),
                        sizeof(*ctx->child) + 2 * sizeof(ctx->child->flex[0]));
      TN_FREE_GLOBAL(ctx);
      ck_assert_int_eq(child_destroy_count, 2);
      