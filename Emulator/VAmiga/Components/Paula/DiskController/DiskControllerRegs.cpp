// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "DiskController.h"
#include "Agnus.h"
#include "Checksum.h"
#include "FloppyDrive.h"
#include "MsgQueue.h"
#include "Paula.h"

namespace vamiga {

u16
DiskController::peekDSKDATR() const
{
    // DSKDAT is a strobe register that cannot be accessed by the CPU
    return 0;
}

void
DiskController::pokeDSKLEN(u16 value)
{
    trace(DSKREG_DEBUG, "pokeDSKLEN(%X)\n", value);

    setDSKLEN(dsklen, value);
}

void
DiskController::setDSKLEN(u16 oldValue, u16 newValue)
{
    trace(DSKREG_DEBUG, "setDSKLEN(%x) [%ld,%ld,%ld]\n",
          newValue, df0.head.cylinder, df0.head.head, df0.head.offset);

    FloppyDrive *drive = getSelectedDrive();

    dsklen = newValue;

    // Initialize checksum (for debugging only)
    if (DSK_CHECKSUM) {
        
        checkcnt = 0;
        check1 = util::fnvInit32();
        check2 = util::fnvInit32();
    }
    
    // Disable DMA if bit 15 (DMAEN) is zero
    if (!(newValue & 0x8000)) {

        setState(DriveDmaState::OFF);
        clearFifo();
    }
    
    // Enable DMA if bit 15 (DMAEN) has been written twice
    if (oldValue & newValue & 0x8000) {

        if (state != DriveDmaState::OFF) {
            xfiles("DSKLEN: Written in DMA state %ld\n", isize(state));
        }

        // Only proceed if there are bytes to process
        if ((dsklen & 0x3FFF) == 0) { paula.raiseIrq(IrqSource::DSKBLK); return; }

        // In debug mode, reset head position to generate reproducable results
        if (ALIGN_HEAD) if (drive) drive->head.offset = 0;

        // Check if the WRITE bit (bit 14) also has been written twice
        if (oldValue & newValue & 0x4000) {
            
            setState(DriveDmaState::WRITE);
            clearFifo();
            
        } else {
            
            // Check the WORDSYNC bit in the ADKCON register
            if (GET_BIT(paula.adkcon, 10)) {
                
                // Wait with reading until a sync mark has been found
                setState(DriveDmaState::WAIT);
                clearFifo();
                
            } else {
                
                // Start reading immediately
                setState(DriveDmaState::READ);
                clearFifo();
            }
        }
    }

    // If turbo drives are emulated, perform DMA immediately
    if (turboMode()) performTurboDMA(drive);
}

void
DiskController::pokeDSKDAT(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKDAT\n");
}

u16
DiskController::peekDSKBYTR()
{
    u16 result = computeDSKBYTR();
    
    // Clear the DSKBYT bit, so it won't show up in the next read
    incoming &= 0x7FFF;

    debug(DSKREG_DEBUG, "peekDSKBYTR() = %x\n", result);
    return result;
}

u16
DiskController::computeDSKBYTR() const
{
    /* 15      DSKBYT     Indicates whether this register contains valid data
     * 14      DMAON      Indicates whether disk DMA is actually enabled
     * 13      DISKWRITE  Matches the WRITE bit in DSKLEN
     * 12      WORDEQUAL  Indicates a match with the contents of DISKSYNC
     * 11 - 8             Unused
     *  7 - 0  DATA       Disk byte data
     */
    
    // DSKBYT and DATA
    u16 result = incoming;

    // DMAON
    if (agnus.dskdma() && state != DriveDmaState::OFF) SET_BIT(result, 14);

    // DSKWRITE
    if (dsklen & 0x4000) SET_BIT(result, 13);
    
    // WORDEQUAL
    assert(agnus.clock >= syncCycle);
    if (agnus.clock - syncCycle <= USEC(2)) SET_BIT(result, 12);
    
    return result;
}

void
DiskController::pokeDSKSYNC(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKSYNC(%x)\n", value);
    
    if (value != 0x4489) {
        
        xfiles("DSKSYNC: Unusual sync mark $%04X\n", value);
        
        if (config.lockDskSync) {
            debug(DSKREG_DEBUG, "Write to DSKSYNC blocked (%x)\n", value);
            return;
        }
    }
    
    dsksync = value;
}

u8
DiskController::driveStatusFlags() const
{
    u8 result = 0xFF;
    
    result &= df[0]->driveStatusFlags();
    result &= df[1]->driveStatusFlags();
    result &= df[2]->driveStatusFlags();
    result &= df[3]->driveStatusFlags();
    
    return result;
}

void
DiskController::PRBdidChange(u8 oldValue, u8 newValue)
{
    // Store a copy of the new value for reference
    prb = newValue;
    
    auto oldSelected = selected;
    selected = -1;
    
    // Iterate over all connected drives
    for (isize i = 0; i < 4; i++) {

        if (df[i]->isConnected()) {
            
            // Inform the drive and determine the selected one
            df[i]->PRBdidChange(oldValue, newValue);
            if (df[i]->isSelected()) selected = i;
        }
    }

    if (oldSelected != selected) {
        
        if (selected == -1) {
            debug(DSKREG_DEBUG, "Deselecting df%ld\n", oldSelected);
        } else {
            debug(DSKREG_DEBUG, "Selecting df%ld\n", selected);
        }

        // Inform the GUI
        msgQueue.put(Msg::DRIVE_SELECT, selected);
    }
}

}
