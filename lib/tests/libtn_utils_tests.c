#include <signal.h>
#include "testing.h"
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

typedef struct mem_str_ctx {
    char *str;
} mem_str_ctx;

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

static void copy_mem_child(tn_ptr_location loc)
{
    int f = ((mem_child *)*loc.loc)->field;
    TN_ALLOC_TYPED(loc, mem_child);
    ((mem_child *)*loc.loc)->field = f;
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


static int mem_str_ctx_destroy(TN_UNUSED void *arg)
{
    ctx_destroy_count++;
    return 0;
}

/*! [TN_BUG_ON] */
TESTDEF_SINGLE(test_bug_on_ok, "No bug")
{
    TN_BUG_ON(false);
}

TESTDEF_SIG(test_bug_on_abort, "Abort on bug", ABRT)
{
    TN_BUG_ON(true);
}
/*! [TN_BUG_ON] */

TESTDEF(test_random, "Random generator sanity")
{
    int min;
    int max;
    int r;
    min = random() - RAND_MAX / 2;
    max = min + random() / 2;
    r = tn_random_int(min, max);
    tnt_assert_op(int, r, >=, min);
    tnt_assert_op(int, r, <=, max);
}

TESTDEF(test_random_big, "Random generator for big intervals")
{
    int j;
    bool has_neg = false;
    bool has_pos = false;
    for (j = 0; j < 100; j++)
    {
        int r = tn_random_int(INT32_MIN, INT32_MAX);
        if (r < 0)
            has_neg = true;
        else
            has_pos = true;
    }
    tnt_assert(has_neg);
    tnt_assert(has_pos);
}

TESTDEF_SINGLE(test_new_free_global, "Allocate and free global memory")
{
    mem_context *ctx = NULL;
    TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
    tnt_assert_op(ptr, ctx, !=, NULL);
    tnt_assert_op(ptr, ctx->child, ==, NULL);
    tn_free(TN_GLOC(ctx));
    tnt_assert_op(int, ctx_destroy_count, ==, 1);
}

TESTDEF_SINGLE(test_alloc_free_recursive, "Alloc and free nested regions")
{
    mem_context *ctx = NULL;
    TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
    tnt_assert_op(ptr, ctx->child, ==, NULL);
    TN_ALLOC_TYPED(TN_FLOC(ctx, child), mem_child);
    tnt_assert_op(ptr, ctx->child, !=, NULL);
    tnt_assert_op(int, ctx->child->field, ==, 0);
    tn_free(TN_GLOC(ctx));
    tnt_assert_op(int, ctx_destroy_count, ==, 1);
    tnt_assert_op(int, child_destroy_count, ==, 1);
}

TESTDEF_SINGLE(test_alloc_free_shared, "Alloc and free a shared pointer")
{
    mem_context *ctx1 = NULL;
    mem_context *ctx2 = NULL;
    TN_ALLOC_TYPED(TN_GLOC(ctx1), mem_context);
    TN_ALLOC_TYPED(TN_GLOC(ctx2), mem_context);
    TN_ALLOC_TYPED(TN_FLOC(ctx1, child), mem_child);
    tn_copy_ptr(TN_FLOC(ctx2, child), TN_FLOC(ctx1, child));
    tnt_assert_op(ptr, ctx1->child, ==, ctx2->child);
    tnt_assert(tn_is_shared_ptr(ctx1->child));
    tn_free(TN_GLOC(ctx1));
    tnt_assert(!tn_is_shared_ptr(ctx2->child));
    tn_free(TN_FLOC(ctx2, child));
    tnt_assert_op(ptr, ctx2->child, ==, NULL);
    tnt_assert_op(int, child_destroy_count, ==, 1);
    tn_free(TN_GLOC(ctx2));
    tnt_assert_op(int, child_destroy_count, ==, 1);
}

TESTDEF_SINGLE(test_alloc_flex, "Alloc a flexible struct")
{
    mem_context_flex *ctx = NULL;
    TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context_flex);
    TN_ALLOC_TYPED_FLEX(TN_FLOC(ctx, child), mem_child_flex, flex, 2);
    ctx->child->count = 2;
    tnt_assert_op(size_t, talloc_total_size(ctx->child), ==,
                  sizeof(*ctx->child) + 2 * sizeof(ctx->child->flex[0]));
    tn_free(TN_GLOC(ctx));
    tnt_assert_op(int, child_destroy_count, ==, 2);
}

TESTDEF_SINGLE(test_alloc_move_shared, "Alloc and move a shared pointer")
{
    mem_context *ctx1 = NULL;
    mem_context *ctx2 = NULL;
    void *prev;
    TN_ALLOC_TYPED(TN_GLOC(ctx1), mem_context);
    TN_ALLOC_TYPED(TN_GLOC(ctx2), mem_context);
    TN_ALLOC_TYPED(TN_FLOC(ctx1, child), mem_child);
    prev = ctx1->child;
    tn_move_ptr(TN_FLOC(ctx2, child), TN_FLOC(ctx1, child));
    tnt_assert_op(ptr, ctx2->child, ==, prev);
    tnt_assert_op(ptr, ctx1->child, ==, NULL);
    tnt_assert(!tn_is_shared_ptr(ctx2->child));
    tn_free(TN_GLOC(ctx1));
    tn_free(TN_FLOC(ctx2, child));
    tnt_assert_op(ptr, ctx2->child, ==, NULL);
    tnt_assert_op(int, child_destroy_count, ==, 1);
    tn_free(TN_GLOC(ctx2));
    tnt_assert_op(int, child_destroy_count, ==, 1);
}

TESTDEF_SINGLE(test_copy_move_same,
               "Copy and move from/to the same location")
{
    mem_context *ctx = NULL;
    TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
    tn_copy_ptr(TN_GLOC(ctx), TN_GLOC(ctx));
    tnt_assert(!tn_is_shared_ptr(ctx));
    tnt_assert_op(ptr, ctx, !=, NULL);
    tn_move_ptr(TN_GLOC(ctx), TN_GLOC(ctx));
    tnt_assert_op(ptr, ctx, !=, NULL);
    tn_free(TN_GLOC(ctx));
}

TESTDEF_SIG(test_copy_same_ctx, "Cannot copy to the same context", ABRT)
{
    mem_context *ctx = NULL;
    mem_context *ctx2 = NULL;
    TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
    tn_copy_ptr(TN_GLOC(ctx2), TN_GLOC(ctx));
}

TESTDEF_SINGLE(test_move_same_ctx, "Move to the same context")
{
    mem_context *ctx = NULL;
    mem_context *ctx2 = NULL;
    void *prev;
    TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
    prev = ctx;
    tn_move_ptr(TN_GLOC(ctx2), TN_GLOC(ctx));
    tnt_assert_op(ptr, ctx2, ==, prev);
    tnt_assert_op(ptr, ctx, ==, NULL);
    tnt_assert(!tn_is_shared_ptr(ctx2));
    tn_free(TN_GLOC(ctx2));
}

TESTDEF_SINGLE(test_strdup_null, "Duplicate NULL")
{
    char *s = NULL;
    tn_strdup(TN_GLOC(s), NULL);
    tnt_assert_op(ptr, s, ==, NULL);
}

TESTDEF_SINGLE(test_alloc_trivial_typed, "Alloc typed")
{
    void *tst = NULL;
    tn_alloc_typed(TN_GLOC(tst), NULL, 4, NULL);
    tnt_assert_op(ptr, tst, !=, NULL);
    tn_free(TN_GLOC(tst));
    tnt_assert_op(ptr, tst, ==, NULL);
}


TESTDEF_SINGLE(test_alloc_append_str, "Append a string")
{
    mem_str_ctx *ctx = NULL;
    static const char *str = "str";
    TN_ALLOC_TYPED(TN_GLOC(ctx), mem_str_ctx);
    tn_strdup(TN_FLOC(ctx, str), str);
    tnt_assert_op(ptr, ctx->str, !=, str);
    tn_strcat(TN_FLOC(ctx, str), "ing");
    tnt_assert_fop(strcmp, ctx->str, ==, "string");
    tn_free(TN_GLOC(ctx));
}

TESTDEF_SINGLE(test_alloc_sprintf, "Append a formatted string")
{
    mem_str_ctx *ctx = NULL;
    TN_ALLOC_TYPED(TN_GLOC(ctx), mem_str_ctx);
    tn_sprintf(TN_FLOC(ctx, str), "%s%d.%d", "*", 0, 1);
    tnt_assert_fop(strcmp, ctx->str, ==, "*0.1");
    tn_free(TN_GLOC(ctx));
}

TESTDEF_SINGLE(test_cow, "Copy on write")
{
    mem_context *ctx = NULL;
    mem_context *ctx2 = NULL;
    void *prev;
    TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
    TN_ALLOC_TYPED(TN_GLOC(ctx2), mem_context);
    TN_ALLOC_TYPED(TN_FLOC(ctx, child), mem_child);
    ctx->child->field = 0xdead;
    prev = ctx->child;
    tn_cow(TN_FLOC(ctx, child), copy_mem_child);
    tnt_assert_op(ptr, ctx->child, ==, prev);
    tn_copy_ptr(TN_FLOC(ctx2, child), TN_FLOC(ctx, child));
    tn_cow(TN_FLOC(ctx2, child), copy_mem_child);
    tnt_assert_op(ptr, ctx2->child, !=, prev);
    tnt_assert_op(ptr, ctx->child, ==, prev);
    tnt_assert(!tn_is_shared_ptr(ctx2->child));
    tnt_assert(!tn_is_shared_ptr(ctx->child));
    tnt_assert_op(int, ctx->child->field, ==, ctx2->child->field);
    tn_free(TN_GLOC(ctx));
    tn_free(TN_GLOC(ctx2));
}

TESTDEF_SINGLE(test_append_buffer_null,
               "Append to a an empty buffer works")
{
    char *str = NULL;
    tn_buffer buf = TN_BUFFER_INIT(TN_GLOC(str), 0, 0);
    char *app;

    app = tn_buffer_append(&buf, 10);
    tnt_assert_op(ptr, str, ==, app);
    tnt_assert_op(ptr, app, !=, NULL);
    tnt_assert_op(size_t, buf.len, ==, (size_t)10);
    tn_free(TN_GLOC(str));
}

TESTDEF_SINGLE(test_append_buffer_do_append,
               "Append to a non-empty buffer works")
{
    char *str = NULL;
    tn_buffer buf;
    char *app;

    tn_strdup(TN_GLOC(str), "buf");
    buf = (tn_buffer)TN_BUFFER_INIT(TN_GLOC(str), sizeof("buf") - 1, 0);
    app = tn_buffer_append(&buf, sizeof("fer"));
    strcpy(app, "fer");
    tnt_assert_op(ptr, app, ==, str + sizeof("buf") - 1);
    tnt_assert_fop(strcmp, str, ==, "buffer");
    tnt_assert_op(size_t, buf.len, ==, sizeof("buffer"));
    tn_free(TN_GLOC(str));
}

TESTDEF_SINGLE(test_append_buffer_append_delta,
               "Buffer has some space pre-allocated")
{
    char *str = NULL;
    tn_buffer buf = TN_BUFFER_INIT(TN_GLOC(str), 0, 0);
    char *app;

    app = tn_buffer_append(&buf, 1);
    tn_buffer_append(&buf, 1);
    tnt_assert_op(ptr, str, ==, app);
    tn_buffer_append(&buf, 1);
    tnt_assert_op(ptr, str, ==, app);
    tnt_assert_op(size_t, buf.len, ==, (size_t)3);
    tnt_assert_op(size_t, buf.bufsize, ==, (size_t)65);
    tn_free(TN_GLOC(str));
}

TESTDEF_SINGLE(test_append_buffer_offset,
               "Appending to a buffer with non-zero offset works")
{
    mem_child_flex *flex = NULL;
    int *app = NULL;
    tn_buffer buf;
    TN_ALLOC_TYPED_FLEX(TN_GLOC(flex), mem_child_flex, flex, 2);
    buf = (tn_buffer)TN_BUFFER_INIT(TN_GLOC(flex), 2 * sizeof(int),
                                    offsetof(mem_child_flex, flex));
    app = TN_BUFFER_PUSH(&buf, int, 64);
    tnt_assert_op(size_t, buf.len, ==, 66 * sizeof(int));
    tnt_assert_op(ptr, app, ==, &flex->flex[2]);
    tn_free(TN_GLOC(flex));
}
