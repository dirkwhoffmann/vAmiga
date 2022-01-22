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

    // Check if we need to recompute all events
    if (ddfInitial != ddf) {
        
        if constexpr (SEQ_DEBUG) {
            
            debug(true, "bpv: %d %d\n", ddfInitial.bpv, ddf.bpv);
            debug(true, "bmapen: %d %d\n", ddfInitial.bmapen, ddf.bmapen);
            debug(true, "shw: %d %d\n", ddfInitial.shw, ddf.shw);
            debug(true, "rhw: %d %d\n", ddfInitial.rhw, ddf.rhw);
            debug(true, "bphstart: %d %d\n", ddfInitial.bphstart, ddf.bphstart);
            debug(true, "bphstop: %d %d\n", ddfInitial.bphstop, ddf.bphstop);
            debug(true, "bprun: %d %d\n", ddfInitial.bprun, ddf.bprun);
            debug(true, "lastFu: %d %d\n", ddfInitial.lastFu, ddf.lastFu);
            debug(true, "bmctl: %d %d\n", ddfInitial.bmctl, ddf.bmctl);
            debug(true, "cnt: %d %d\n", ddfInitial.cnt, ddf.cnt);
        }
        
        ddfInitial = ddf;
        trace(SEQ_DEBUG, "hsyncHandler: Forcing an event table update\n");
        hsyncActions |= UPDATE_BPL_TABLE;
    }
    
    // Renew the signal list if it has been modified in the previous line
    /*
    if (sigRecorder.modified ||
        agnus.pos.v == diwVstrt ||
        agnus.pos.v == diwVstop ||
        agnus.inLastRasterline()) {
    */
    if (sigRecorder.modified) {

        trace(SEQ_DEBUG, "hsyncHandler: sigRecorder.modified\n");
        hsyncActions |= UPDATE_SIG_RECORDER;
    }

    // Check the vertical DIW flipflop
    if (agnus.pos.v == diwVstop || agnus.inLastRasterline()) {

        trace(SEQ_DEBUG, "hsyncHandler: Vertical flipflop off\n");
        ddfInitial.bpv = ddf.bpv = false;
        hsyncActions |= UPDATE_SIG_RECORDER;

    } else if (agnus.pos.v == diwVstrt) {

        trace(SEQ_DEBUG, "hsyncHandler: Vertical flipflop on\n");
        ddfInitial.bpv = ddf.bpv = true;
        hsyncActions |= UPDATE_SIG_RECORDER;
    }
    
    lineIsBlank = !ddfInitial.bpv;

    //
    // Determine the disk, audio and sprite DMA status for the line to come
    //

    u16 newDmaDAS = 0;
    
    if (agnus.dmacon & DMAEN) {

        // Copy DMA enable bits from DMACON
        newDmaDAS = agnus.dmacon & 0b111111;

        // Disable sprites outside the sprite DMA area
        if (agnus.pos.v < 25 || agnus.pos.v >= agnus.frame.lastLine()) {
            newDmaDAS &= 0b011111;
        }
    }
    if (newDmaDAS != dmaDAS) {
        
        hsyncActions |= UPDATE_DAS_TABLE;
        dmaDAS = newDmaDAS;
    }

    //
    // Process pending actions
    //

    if (hsyncActions) {

        if (hsyncActions & UPDATE_SIG_RECORDER) {

            hsyncActions &= ~UPDATE_SIG_RECORDER;
            hsyncActions |= UPDATE_BPL_TABLE;
            initSigRecorder();
        }
        if (hsyncActions & UPDATE_BPL_TABLE) {
            
            trace(SEQ_DEBUG, "hsyncActions & UPDATE_BPL_TABLE\n");
            hsyncActions &= ~UPDATE_BPL_TABLE;
            computeBplEvents(sigRecorder);
        }
        if (hsyncActions & UPDATE_DAS_TABLE) {
            
            hsyncActions &= ~UPDATE_DAS_TABLE;
            updateDasEvents(dmaDAS);
        }
    }
}

void
Sequencer::vsyncHandler()
{

}
