#include <time.h>
#include <sys/resource.h>
#include <signal.h>
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

SUITE(Utils)

TCASE(Assert)

TEST(test_bug_on_ok, OK, ONCE)
      TN_BUG_ON(false);

TEST(test_bug_on_abort, SIGNAL(ABRT), ONCE)
      setrlimit(RLIMIT_CORE, &(struct rlimit){0, 0});
      TN_BUG_ON(true);

TEST(test_random)
      int min;
      int max;
      int r;
      min = random() - RAND_MAX / 2;
      max = min + random() / 2;
      r = tn_random_int(min, max);
      ck_assert_int_ge(r, min);
      ck_assert_int_le(r, max);

TEST(test_random_big, OK, ONCE)
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
      ck_assert(has_neg);
      ck_assert(has_pos);

TCASE(Memory)

TEST(test_new_free_global, OK, ONCE)
      mem_context *ctx = NULL;
      TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
      ck_assert_ptr_ne(ctx, NULL);
      ck_assert_ptr_eq(ctx->child, NULL);
      tn_free(TN_GLOC(ctx));
      ck_assert_int_eq(ctx_destroy_count, 1);

TEST(test_alloc_free_recursive, OK, ONCE)
      mem_context *ctx = NULL;
      TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
      ck_assert_ptr_eq(ctx->child, NULL);
      TN_ALLOC_TYPED(TN_FLOC(ctx, child), mem_child);
      ck_assert_ptr_ne(ctx->child, NULL);
      ck_assert_int_eq(ctx->child->field, 0);
      tn_free(TN_GLOC(ctx));
      ck_assert_int_eq(ctx_destroy_count, 1);
      ck_assert_int_eq(child_destroy_count, 1);

TEST(test_alloc_free_shared, OK, ONCE)
      mem_context *ctx1 = NULL;
      mem_context *ctx2 = NULL;
      TN_ALLOC_TYPED(TN_GLOC(ctx1), mem_context);
      TN_ALLOC_TYPED(TN_GLOC(ctx2), mem_context);
      TN_ALLOC_TYPED(TN_FLOC(ctx1, child), mem_child);
      tn_copy_ptr(TN_FLOC(ctx2, child), TN_FLOC(ctx1, child));
      ck_assert_ptr_eq(ctx1->child, ctx2->child);
      ck_assert(tn_is_shared_ptr(ctx1->child));
      tn_free(TN_GLOC(ctx1));
      ck_assert(!tn_is_shared_ptr(ctx2->child));
      tn_free(TN_FLOC(ctx2, child));
      ck_assert_ptr_eq(ctx2->child, NULL);
      ck_assert_int_eq(child_destroy_count, 1);
      tn_free(TN_GLOC(ctx2));
      ck_assert_int_eq(child_destroy_count, 1);

TEST(test_alloc_flex, OK, ONCE)
      mem_context_flex *ctx = NULL;
      TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context_flex);
      TN_ALLOC_TYPED_FLEX(TN_FLOC(ctx, child), mem_child_flex, flex, 2);
      ctx->child->count = 2;
      ck_assert_uint_eq(talloc_total_size(ctx->child),
                        sizeof(*ctx->child) + 2 * sizeof(ctx->child->flex[0]));
      tn_free(TN_GLOC(ctx));
      ck_assert_int_eq(child_destroy_count, 2);

TEST(test_alloc_move_shared, OK, ONCE)
      mem_context *ctx1 = NULL;
      mem_context *ctx2 = NULL;
      void *prev;
      TN_ALLOC_TYPED(TN_GLOC(ctx1), mem_context);
      TN_ALLOC_TYPED(TN_GLOC(ctx2), mem_context);
      TN_ALLOC_TYPED(TN_FLOC(ctx1, child), mem_child);
      prev = ctx1->child;
      tn_move_ptr(TN_FLOC(ctx2, child), TN_FLOC(ctx1, child));
      ck_assert_ptr_eq(ctx2->child, prev);
      ck_assert_ptr_eq(ctx1->child, NULL);
      ck_assert(!tn_is_shared_ptr(ctx2->child));
      tn_free(TN_GLOC(ctx1));
      tn_free(TN_FLOC(ctx2, child));
      ck_assert_ptr_eq(ctx2->child, NULL);
      ck_assert_int_eq(child_destroy_count, 1);
      tn_free(TN_GLOC(ctx2));
      ck_assert_int_eq(child_destroy_count, 1);

TEST(test_copy_move_same, OK, ONCE)
      mem_context *ctx = NULL;
      TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
      tn_copy_ptr(TN_GLOC(ctx), TN_GLOC(ctx));
      ck_assert(!tn_is_shared_ptr(ctx));
      ck_assert_ptr_ne(ctx, NULL);
      tn_move_ptr(TN_GLOC(ctx), TN_GLOC(ctx));
      ck_assert_ptr_ne(ctx, NULL);
      tn_free(TN_GLOC(ctx));

TEST(test_copy_same_ctx, SIGNAL(ABRT), ONCE)
      mem_context *ctx = NULL;
      mem_context *ctx2 = NULL;
      TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
      setrlimit(RLIMIT_CORE, &(struct rlimit){0, 0});
      tn_copy_ptr(TN_GLOC(ctx2), TN_GLOC(ctx));

TEST(test_move_same_ctx, OK, ONCE)
      mem_context *ctx = NULL;
      mem_context *ctx2 = NULL;
      void *prev;
      TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
      prev = ctx;
      tn_move_ptr(TN_GLOC(ctx2), TN_GLOC(ctx));
      ck_assert_ptr_eq(ctx2, prev);
      ck_assert_ptr_eq(ctx, NULL);
      ck_assert(!tn_is_shared_ptr(ctx2));
      tn_free(TN_GLOC(ctx2));

TEST(test_strdup_null, OK, ONCE)
      char *s = NULL;
      tn_strdup(TN_GLOC(s), NULL);
      ck_assert_ptr_eq(s, NULL);

TEST(test_alloc_trivial_typed, OK, ONCE)
      void *tst = NULL;
      tn_alloc_typed(TN_GLOC(tst), NULL, 4, NULL);
      ck_assert_ptr_ne(tst, NULL);
      tn_free(TN_GLOC(tst));
      ck_assert_ptr_eq(tst, NULL);

TEST(test_alloc_append_str, OK, ONCE)
      mem_str_ctx *ctx = NULL;
      static const char *str = "str";
      TN_ALLOC_TYPED(TN_GLOC(ctx), mem_str_ctx);
      tn_strdup(TN_FLOC(ctx, str), str);
      ck_assert_ptr_ne(ctx->str, str);
      tn_strcat(TN_FLOC(ctx, str), "ing");
      ck_assert_str_eq(ctx->str, "string");
      tn_free(TN_GLOC(ctx));

TEST(test_alloc_sprintf, OK, ONCE)
      mem_str_ctx *ctx = NULL;
      TN_ALLOC_TYPED(TN_GLOC(ctx), mem_str_ctx);
      tn_sprintf(TN_FLOC(ctx, str), "%s%d.%d", "*", 0, 1);
      ck_assert_str_eq(ctx->str, "*0.1");
      tn_free(TN_GLOC(ctx));

TEST(test_cow, OK, ONCE)
      mem_context *ctx = NULL;
      mem_context *ctx2 = NULL;
      void *prev;
      TN_ALLOC_TYPED(TN_GLOC(ctx), mem_context);
      TN_ALLOC_TYPED(TN_GLOC(ctx2), mem_context);
      TN_ALLOC_TYPED(TN_FLOC(ctx, child), mem_child);
      ctx->child->field = 0xdead;
      prev = ctx->child;
      tn_cow(TN_FLOC(ctx, child), copy_mem_child);
      ck_assert_ptr_eq(ctx->child, prev);
      tn_copy_ptr(TN_FLOC(ctx2, child), TN_FLOC(ctx, child));
      tn_cow(TN_FLOC(ctx2, child), copy_mem_child);
      ck_assert_ptr_ne(ctx2->child, prev);
      ck_assert_ptr_eq(ctx->child, prev);
      ck_assert(!tn_is_shared_ptr(ctx2->child));
      ck_assert(!tn_is_shared_ptr(ctx->child));
      ck_assert_int_eq(ctx->child->field, ctx2->child->field);
      tn_free(TN_GLOC(ctx));
      tn_free(TN_GLOC(ctx2));

TEST(test_append_buffer_null, OK, ONCE)
      char *str = NULL;
      tn_buffer buf = TN_BUFFER_INIT(TN_GLOC(str), 0, 0);
      char *app;

      app = tn_buffer_append(&buf, 10);
      ck_assert_ptr_eq(str, app);
      ck_assert_ptr_ne(app, NULL);
      ck_assert_uint_eq(buf.len, 10);
      tn_free(TN_GLOC(str));

TEST(test_append_buffer_do_append, OK, ONCE)
      char *str = NULL;
      tn_buffer buf;
      char *app;

      tn_strdup(TN_GLOC(str), "buf");
      buf = (tn_buffer)TN_BUFFER_INIT(TN_GLOC(str), sizeof("buf") - 1, 0);
      app = tn_buffer_append(&buf, sizeof("fer"));
      strcpy(app, "fer");
      ck_assert_ptr_eq(app, str + sizeof("buf") - 1);
      ck_assert_str_eq(str, "buffer");
      ck_assert_uint_eq(buf.len, sizeof("buffer"));
      tn_free(TN_GLOC(str));

TEST(test_append_buffer_append_delta, OK, ONCE)
      char *str = NULL;
      tn_buffer buf = TN_BUFFER_INIT(TN_GLOC(str), 0, 0);
      char *app;

      app = tn_buffer_append(&buf, 1);
      tn_buffer_append(&buf, 1);
      ck_assert_ptr_eq(str, app);
      tn_buffer_append(&buf, 1);
      ck_assert_ptr_eq(str, app);
      ck_assert_uint_eq(buf.len, 3);
      ck_assert_uint_eq(buf.bufsize, 65);
      tn_free(TN_GLOC(str));

TEST(test_append_buffer_offset, OK, ONCE)
      mem_child_flex *flex = NULL;
      int *app = NULL;
      tn_buffer buf;
      TN_ALLOC_TYPED_FLEX(TN_GLOC(flex), mem_child_flex, flex, 2);
      buf = (tn_buffer)TN_BUFFER_INIT(TN_GLOC(flex), 2 * sizeof(int),
                                      offsetof(mem_child_flex, flex));
      app = TN_BUFFER_PUSH(&buf, int, 64);
      ck_assert_uint_eq(buf.len, 66 * sizeof(int));
      ck_assert_ptr_eq(app, &flex->flex[2]);
      tn_free(TN_GLOC(flex));