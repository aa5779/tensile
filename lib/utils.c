#include "utils.h"

void
tn_sprintf(tn_ptr_location loc, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    tn_vsprintf(loc, fmt, args);
    va_end(args);
}
