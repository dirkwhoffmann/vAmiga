// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "RetroShell.h"

namespace va {

void
RetroShell::pressKey(char c)
{
    printf("pressKey %c\n", c);
    t.pop_back();
    t += c;
    t += " ";
}

const char *
RetroShell::text()
{
    printf("t.c_str = %s\n", t.c_str());
    return t.c_str();
}

}
