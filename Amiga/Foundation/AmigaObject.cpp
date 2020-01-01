// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "AmigaObject.h"
#include <cstdlib>

#define VAOBJ_PARSE \
char buf[256]; \
va_list ap; \
va_start(ap, fmt); \
vsnprintf(buf, sizeof(buf), fmt, ap); \
va_end(ap);

#define VAPRINTPLAIN(trailer) \
fprintf(stderr, "%s%s", trailer, buf);

#define VAPRINT(trailer) \
prefix(); \
fprintf(stderr, "%s: %s%s", getDescription(), trailer, buf);

void
AmigaObject::prefix()
{
    if (description)
        fprintf(stderr, "%s: ", description);
}

void
AmigaObject::msg(const char *fmt, ...)
{
    VAOBJ_PARSE
    VAPRINT("")
}

void
AmigaObject::plainmsg(const char *fmt, ...)
{
    VAOBJ_PARSE
    VAPRINTPLAIN("")
}

void
AmigaObject::debug(const char *fmt, ...)
{
#ifndef NDEBUG
    VAOBJ_PARSE
    VAPRINT("")
#endif
}

void
AmigaObject::debug(int level, const char *fmt, ...)
{
#ifndef NDEBUG
    if (level <= debugLevel) {
        VAOBJ_PARSE
        VAPRINT("")
    }
#endif
}

void
AmigaObject::plaindebug(const char *fmt, ...)
{
#ifndef NDEBUG
    VAOBJ_PARSE
    VAPRINTPLAIN("")
#endif
}

void
AmigaObject::plaindebug(int level, const char *fmt, ...)
{
#ifndef NDEBUG
    if (level <= debugLevel) {
        VAOBJ_PARSE
        VAPRINTPLAIN("")
    }
#endif
}

void
AmigaObject::warn(const char *fmt, ...)
{
    VAOBJ_PARSE;
    VAPRINT("WARNING: ")
}

void
AmigaObject::panic(const char *fmt, ...)
{
    VAOBJ_PARSE;
    VAPRINT("PANIC: ")
    std::abort();
}

void
AmigaObject::reportSuspiciousBehavior()
{
    plainmsg("\n");
    plainmsg("Execution has been aborted, because a suspicous program operation has been\n");
    plainmsg("detected. Manual investigation is needed to determine whether this is an\n");
    plainmsg("error or not.\n\n");
    plainmsg("Please file a bug report.\n\n");

    std::abort();
}
