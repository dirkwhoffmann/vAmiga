// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RetroShell.h"

namespace va {

//
// Top-level commands
//

template <> void
RetroShell::handler <Token::clear> (Arguments &argv, long param)
{
    printf("Token::clear Handler\n");
}

}
