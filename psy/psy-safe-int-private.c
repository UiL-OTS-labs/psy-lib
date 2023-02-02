

#include "psy-safe-int-private.h"
#include "psy-config.h"

gboolean
psy_safe_add_gint64(gint64 a, gint64 b, gint64 *result)
{
#if defined(HAVE_BUILTIN_ADD_OVERFLOW)
    return __builtin_add_overflow(a, b, result);
#else
    if (b >= 0) {
        if (G_MAXINT64 - b < a) // over flows
            return TRUE;
    }
    else {
        if (G_MININT64 - b > a) // under flows
            return TRUE;
    }
    *result = a + b;
    return FALSE;
#endif
}

gboolean
psy_safe_sub_gint64(gint64 a, gint64 b, gint64 *result)
{
#if defined(HAVE_BUILTIN_SUB_OVERFLOW)
    return __builtin_sub_overflow(a, b, result);
#else
    if (b >= 0) {
        if (G_MININT64 + b > a) // under flows
            return TRUE;
    }
    else {
        if (G_MAXINT64 - b < a) // over flows
            return TRUE;
    }
    *result = a - b;
    return FALSE;
#endif
}

gboolean
psy_safe_mul_gint64(gint64 a, gint64 b, gint64 *result)
{
#if defined(HAVE_BUILTIN_MUL_OVERFLOW)
    return __builtin_mul_overflow(a, b, result);
#else
    if (b >= 0) {
        if (G_MAXINT64 / b < a) // over flows
            return TRUE;
    }
    else {
        if (G_MININT64 / b > a) // under flows
            return TRUE;
    }
    *result = a * b;
    return FALSE;
#endif
}
