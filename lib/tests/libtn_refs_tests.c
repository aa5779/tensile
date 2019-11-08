#include "refs.h"
#include "testing.h"

static void
tn_ref_generate(TN_UNUSED unsigned _i, tn_ref *ref)
{
    ref->lo = (uint64_t)random() << 32;
    ref->lo |= random();
    ref->hi = (uint64_t)random() << 32;
    ref->hi |= random();
    ref->hi &= ~(1ULL << (TNRK_BIT_BASE - 1));
}

#define tn_ref_cleanup(_ref) ((void)0)

TESTDEF_SINGLE(test_ref_incr_zero, "Increment a ref by zero")
{
    FORALL(tn_ref, ref1,
           tn_ref ref2 = tn_ref_add(ref1, 0);
           tnt_assert(tn_ref_eq(ref1, ref2)));
}

TESTDEF(test_ref_incr_nzero, "Increment a ref by non-zero")
{
    FORALL(tn_ref, ref1,
           unsigned r = (unsigned)((uint64_t)random() + 1);
           tn_ref ref2 = tn_ref_add(ref1, r);
           tnt_assert(!tn_ref_eq(ref1, ref2));
           tnt_assert(tn_ref_le(ref1, ref2));
           tnt_assert_op(uint64_t,
                         ref1.hi >> TNRK_BIT_BASE, ==,
                         ref2.hi >> TNRK_BIT_BASE);
        );
}

TESTDEF_SINGLE(test_ref_incr_overflow_lo, "Overflow lower 64 bits of a ref")
{
    FORALL(tn_ref, ref1,
           tn_ref ref2;
           unsigned r;
           ref1.lo |= 0xffffffff00000000ull;
           r = (unsigned)(~0ull - ref1.lo) + 1;
           ref2 = tn_ref_add(ref1, r);
           tnt_assert_op(uint64_t, ref2.hi, ==, ref1.hi + 1);
           tnt_assert_op(uint64_t, ref2.lo, ==, (uint64_t)0);
        );
}


TESTDEF_SINGLE(test_ref_incr_delta_zero, "Reference relative to itself")
{
    FORALL(tn_ref, ref,
           tn_rel_ref rel = tn_ref_mkrel(ref, ref);
           tnt_assert_op(unsigned, rel & TNRK_REL_MASK, ==, 0);
        );
}

TESTDEF(test_ref_delta_nzero, "Relative reference")
{
    FORALL(tn_ref, ref1,
           unsigned r = (unsigned)((random() & TNRK_REL_MASK) + 1);
           tn_ref ref2 = tn_ref_add(ref1, r);
           tn_rel_ref rel = tn_ref_mkrel(ref1, ref2);
           tnt_assert_op(unsigned, rel & TNRK_REL_MASK, ==, r);
           tnt_assert_op(uint64_t, (uint64_t)(rel >> TNRK_REL_BIT_BASE), ==, ref2.hi >> TNRK_BIT_BASE);
        );
}

TESTDEF_SINGLE(test_ref_delta_overflow_lo,
               "Relative reference with ref lower bits wraparound")
{
    tn_ref ref1;
    tn_ref ref2;
    unsigned r;
    tn_rel_ref rel;

    ref1.hi = random();
    r = random() & TNRK_REL_MASK;
    ref1.lo = ~0ull - (uint64_t)r;
    ref2.hi = ref1.hi + 1;
    ref2.lo = 0ull;
    rel = tn_ref_mkrel(ref1, ref2);
    tnt_assert_op(unsigned, rel & TNRK_REL_MASK, ==, r + 1);
}

TESTDEF(test_ref_rel, "Add and subtract relative references")
{
    FORALL(tn_ref, ref1,
           tn_rel_ref rel = random();
           tn_ref ref2 = tn_ref_relative(ref1, rel);
           tn_rel_ref rel2 = tn_ref_mkrel(ref1, ref2);
           tnt_assert_op(unsigned, rel, ==, rel2);
           tnt_assert_op(uint64_t, (uint64_t)(rel >> TNRK_REL_BIT_BASE), ==,
                         ref2.hi >> TNRK_BIT_BASE);
        );
}

TESTDEF(test_ref_rebase, "Rebasing references")
{
    FORALL(tn_ref, ref,
           tn_ref rebase = tn_ref_rebase(ref, ref);
           tnt_assert_op(uint64_t, rebase.lo, ==, ref.lo << 1);
           tnt_assert_op(uint64_t, rebase.hi & TNRK_MASK, ==,
                         (ref.hi & TNRK_MASK) << 1);
           tnt_assert_op(uint64_t, rebase.hi >> TNRK_BIT_BASE, ==,
                         ref.hi >> TNRK_BIT_BASE);
        );
}

TESTDEF_SINGLE(test_ref_rebase_carry, "Rebase with lower bits overflow")
{
    tn_ref ref = {random(), ~0ull};
    tn_ref base = {0, 1};
    tn_ref rebase = tn_ref_rebase(ref, base);
    tnt_assert_op(uint64_t, rebase.lo, ==, (uint64_t)0);
    tnt_assert_op(uint64_t, rebase.hi, ==, ref.hi + 1);
}

TESTDEF_SINGLE(test_ref_eq_self, "Reference is equal to itself")
{
      FORALL(tn_ref, ref,
             tnt_assert(tn_ref_eq(ref, ref));
             tnt_assert(tn_ref_le(ref, ref));
          );
}

TESTDEF_SINGLE(test_ref_neq, "Distinct references are not equal")
{
    FORALL(tn_ref, ref,
           tn_ref ref1 = {~ref.hi, ref.lo};
           tnt_assert(!tn_ref_eq(ref, ref1));
        );
}

TESTDEF_SINGLE(test_ref_le, "References are totally ordered")
{
    FORALL(tn_ref, ref1,
           FORALL(tn_ref, ref2,
                  ref2.hi = (ref2.hi & TNRK_MASK) | (ref1.hi & ~TNRK_MASK);
                  tnt_assert(tn_ref_le(ref1, ref2) || tn_ref_le(ref2,
                  ref1));
               ));
}

TESTDEF_SINGLE(test_ref_kind, "Checking reference type bits")
{               
    for (enum tn_ref_kind kind = TNRK_STATIC;
         kind <= TNRK_OP_INDIRECT;
         kind++)
    {   
        tn_ref ref = {(uint64_t)tn_ref_request_kind(kind) << TNRK_BIT_BASE,
                    (uint64_t)random()};
        tnt_assert(tn_ref_is_kind(ref, kind));
    }
}
