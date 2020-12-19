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
char buf[512]; \
va_list ap; \
va_start(ap, fmt); \
vsnprintf(buf, sizeof(buf), fmt, ap); \
va_end(ap);

/*
void
AmigaObject::msg(const char *fmt, ...)
{
    VAOBJ_PARSE
    fprintf(stderr, "%s", buf);
}
*/

void
AmigaObject::warn(const char *fmt, ...)
{
    VAOBJ_PARSE;
    fprintf(stderr, "WARNING: %s", buf);
}

void
AmigaObject::debug(int verbose, const char *fmt, ...)
{
#ifndef NDEBUG
    
    if (verbose == 0) return;
    
    VAOBJ_PARSE
    fprintf(stderr, "%s: %s", getDescription(), buf);
    
#endif
}

void
AmigaObject::trace(int verbose, const char *fmt, ...)
{
#ifndef NDEBUG
    
    if (verbose == 0) return;
    
    VAOBJ_PARSE
    prefix();
    fprintf(stderr, "%s: %s", getDescription(), buf);
    
#endif
}
