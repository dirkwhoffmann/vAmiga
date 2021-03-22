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
    t += c;
}

const char *
RetroShell::text()
{
    return t.c_str();
}

}
