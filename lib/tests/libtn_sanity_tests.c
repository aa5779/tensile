#include <signal.h>
#include <stdlib.h>
#include "testing.h"

TESTDEF(test_trivial, "Trivial test")
{
    tnt_assert(true);
}

TESTDEF_SINGLE(test_trivial_trivial, "Trivial trivial test")
{
    tnt_trivial(true);
    tnt_assert(true);
}

TESTDEF_SINGLE(test_trivial_single, "Trivial single test")
{
    tnt_log("%s", "some logging");
}

TESTDEF_EXIT(test_exitstatus, "Test exist status", EXIT_FAILURE)
{
    exit(EXIT_FAILURE);
}

TESTDEF_SIG(test_signal, "Test signal", ABRT)
{
    abort();
}

TESTDEF_XFAIL(test_expect_fail, "Expected to fail")
{
    tnt_assert(false);
}
