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
#include "Agnus.h"

Sequencer::Sequencer(Amiga& ref) : SubComponent(ref)
{
    initDasEventTable();
}

void
Sequencer::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    clearBplEvents();
    clearDasEvents();
}

void
Sequencer::hsyncHandler()
{
    diwVstrtInitial = diwVstrt;
    diwVstopInitial = diwVstop;
    ddfInitial = ddf;
    
    if (agnus.pos.v == diwVstrt) {
        
        trace(DDF_DEBUG, "DDF: FF1 = 1 (DIWSTRT)\n");
        // agnus.hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
        sigRecorder.modified = true;
    }
    if (agnus.pos.v == diwVstop) {
        
        trace(DDF_DEBUG, "DDF: FF1 = 0 (DIWSTOP)\n");
        // agnus.hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
        sigRecorder.modified = true;
    }
    if (agnus.inLastRasterline()) {
        
        trace(DDF_DEBUG, "DDF: FF1 = 0 (EOF)\n");
        // agnus.hsyncActions |= HSYNC_UPDATE_BPL_TABLE;
        sigRecorder.modified = true;
    }

    lineIsBlank = !ddfInitial.bpv;
    
    // Recompute the bitplane event table if necessary
    if (sigRecorder.modified) computeBplEvents();
}

void
Sequencer::vsyncHandler()
{

}
