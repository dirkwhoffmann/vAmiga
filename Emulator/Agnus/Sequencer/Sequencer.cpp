// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Sequencer.h"

Sequencer::Sequencer(Amiga& ref) : SubComponent(ref)
{

}

void
Sequencer::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
}
