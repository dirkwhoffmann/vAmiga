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
        hsyncActions |= UPDATE_SIG_RECORDER;
    }
    if (agnus.pos.v == diwVstop) {
        
        trace(DDF_DEBUG, "DDF: FF1 = 0 (DIWSTOP)\n");
        hsyncActions |= UPDATE_SIG_RECORDER;
    }
    if (agnus.inLastRasterline()) {
        
        trace(DDF_DEBUG, "DDF: FF1 = 0 (EOF)\n");
        hsyncActions |= UPDATE_SIG_RECORDER;
    }
    if (sigRecorder.modified) {
        
        hsyncActions |= UPDATE_SIG_RECORDER;
    }

    lineIsBlank = !ddfInitial.bpv;

    //
    // Process pending actions
    //

    if (hsyncActions) {

        if (hsyncActions & UPDATE_SIG_RECORDER) {

            hsyncActions &= ~UPDATE_SIG_RECORDER;
            // TODO: Only initialize the sigRecorder and set the UPDATE_BPL_TABLE flag
            computeBplEvents();
        }
        if (hsyncActions & UPDATE_BPL_TABLE) {
            
            hsyncActions &= ~UPDATE_BPL_TABLE;
            computeBplEvents();
        }
    }
}

void
Sequencer::vsyncHandler()
{

}
