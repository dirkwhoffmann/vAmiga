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

#define VAPRINTPLAIN(prefix) \
fprintf(stderr, "%s%s", prefix, buf);

#define VAPRINT(prefix) \
if (description) \
fprintf(stderr, "%s: %s%s", description, prefix, buf); \
else \
fprintf(stderr, "%s%s", prefix, buf);

void
AmigaObject::trace()
{
#ifndef NDEBUG // Generate code in debug build, only.

    if (traceCounter > 0) {
        traceCounter--;
        _trace();
    }

#endif
}

void msg(const char *fmt, ...);
void plainmsg(const char *fmt, ...);

void debug(const char *fmt, ...);
void debug(int level, const char *fmt, ...);
void plaindebug(const char *fmt, ...);
void plaindebug(int level, const char *fmt, ...);

void warn(const char *fmt, ...);
void panic(const char *fmt, ...);


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
