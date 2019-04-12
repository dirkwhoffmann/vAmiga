// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DiskController::DiskController()
{
    setDescription("DiskController");
    
    // Register snapshot items
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &state,         sizeof(state),         0 },
        { &incoming,      sizeof(incoming),      0 },
        { &incomingCycle, sizeof(incomingCycle), 0 },
        { &fifo,          sizeof(fifo),          0 },
        { &fifoCount,     sizeof(fifoCount),     0 },
        { &dsklen,        sizeof(dsklen),        0 },
        { &dskdat,        sizeof(dskdat),        0 },
        { &prb,           sizeof(prb),           0 },
        
    });
}

void
DiskController::_setAmiga()
{
    df[0] = &amiga->df0;
    df[1] = &amiga->df1;
    df[2] = &amiga->df2;
    df[3] = &amiga->df3;
}

void
DiskController::_powerOn()
{
 
}

void
DiskController::_powerOff()
{
    
}

void
DiskController::_reset()
{
    
}

void
DiskController::_ping()
{
    for (int df = 0; df < 4; df++) {
        amiga->putMessage(connected[df] ? MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, df);
    }
}

void
DiskController::_inspect()
{
    pthread_mutex_lock(&lock);
    info.dsklen = dsklen;
    pthread_mutex_unlock(&lock);
}

void
DiskController::_dump()
{
    
}

DiskControllerInfo
DiskController::getInfo()
{
    DiskControllerInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

void
DiskController::setState(DriveState state)
{
    /*
    if (this->state == DRIVE_DMA_OFF && state == DRIVE_DMA_READ) {
        clearFifo();
    }
    */
    
    this->state = state;
}

void
DiskController::setConnected(int df, bool value)
{
    assert(df < 4);
    
    // We don't allow the internal drive (Df0) to be disconnected
    if (df == 0 && value == false) { return; }
    
    // Plug the drive in our out and inform the GUI
    connected[df] = value;
    amiga->putMessage(value ? MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, df);
    amiga->putMessage(MSG_CONFIG);
}

uint16_t
DiskController::peekDSKDATR()
{
    debug(2, "peekDSKDATR() = %X\n", dskdat);
    return dskdat;
}

void
DiskController::pokeDSKLEN(uint16_t newDskLen)
{
    debug(2, "pokeDSKLEN(%X)\n", newDskLen);

    uint16_t oldDsklen = dsklen;
        
    // Remember the new value
    dsklen = newDskLen;
    
    // Disable DMA if the DMAEN bit (bit 15) has been cleared.
    if (!(newDskLen & 0x8000)) {
        debug(2, "dma = DRIVE_DMA_OFF\n");
        state = DRIVE_DMA_OFF;
    }
    
    // Enable DMA the DMAEN bit (bit 15) has been written twice.
    else if (oldDsklen & newDskLen & 0x8000) {
        
        // Check if the WRITE bit (bit 14) also has been written twice.
        if (oldDsklen & newDskLen & 0x4000) {
    
            debug(2, "dma = DRIVE_DMA_WRITE\n");
            state = DRIVE_DMA_WRITE;
            
        } else {
        
            // Check the WORDSYNC bit in the ADKCON register
            if (GET_BIT(amiga->paula.adkcon, 10)) {
                
                // Wait with reading until a sync mark has been found
                debug(2, "dma = DRIVE_DMA_READ_SYNC\n");
                state = DRIVE_DMA_SYNC_WAIT;

            } else {

                // Start reading immediately
                debug(2, "dma = DRIVE_DMA_READ\n");
                state = DRIVE_DMA_READ;
                
                // REMOVE ASAP
                for (int i = 0; i < 6; i++) df[0]->rotate();
            }
        }
    }
}

void
DiskController::pokeDSKDAT(uint16_t value)
{
    debug(2, "pokeDSKDAT(%X)\n", value);
    dskdat = value;
}

uint16_t
DiskController::peekDSKBYTR()
{
    /* 15      DSKBYT     Indicates whether this register contains valid data.
     * 14      DMAON      Indicates whether disk DMA is actually enabled.
     * 13      DISKWRITE  Matches the WRITE bit in DSKLEN.
     * 12      WORDEQUAL  Indicates a match with the contents of DISKSYNC.
     * 11 - 8             Unused.
     *  7 - 0  DATA       Disk byte data.
     */
    
    // DATA
    uint16_t result = incoming;
    
    // WORDEQUAL
    if (compareFifo(dsksync)) SET_BIT(result, 12);
    
    // DMAON
    if (amiga->agnus.dskDMA() && state != DRIVE_DMA_OFF) SET_BIT(result, 14);
    
    // DSKBYT
    assert(amiga->agnus.clock >= incomingCycle);
    if (amiga->agnus.clock - incomingCycle <= 7) {
        SET_BIT(result, 15);
    }
    
    debug(2, "peekDSKBYTR() = %X\n", result);
    return result;
}

void
DiskController::pokeDSKSYNC(uint16_t value)
{
    debug(2, "pokeDSKSYNC(%X)\n", value);
    dsksync = value;
}

uint8_t
DiskController::driveStatusFlags()
{
    uint8_t result = 0xFF;
    
    if (connected[0]) result &= df[0]->driveStatusFlags();
    if (connected[1]) result &= df[1]->driveStatusFlags();
    if (connected[2]) result &= df[2]->driveStatusFlags();
    if (connected[3]) result &= df[3]->driveStatusFlags();
    
    return result;
}

void
DiskController::PRBdidChange(uint8_t oldValue, uint8_t newValue)
{
    // debug("PRBdidChange: %X -> %X\n", oldValue, newValue);
    
    // Store a copy of the new PRB value
    prb = newValue;
    
    // Pass control over to all four drives
    for (unsigned i = 0; i < 4; i++) {
        if (connected[i]) df[i]->PRBdidChange(oldValue, newValue);
    }
    
    // Determine the current motor status of all four drives
    bool motor = df[0]->motor | df[1]->motor | df[2]->motor | df[3]->motor;
    
    // Scheduling rotation events if at least one drive is spinning
    if (!motor) {
        handler->cancelSec(DSK_SLOT);
    }
    else if (!handler->hasEventSec(DSK_SLOT)) {
        handler->scheduleSecRel(DSK_SLOT, DMA_CYCLES(56), DSK_ROTATE);
    }
}

void
DiskController::clearFifo()
{
    fifo = 0;
    fifoCount = 0;
}

void
DiskController::writeFifo(uint8_t byte)
{
    assert(fifoCount <= 6);
    
    // Remove oldest word if the FIFO is full
    if (fifoCount == 6) fifoCount -= 2;
    
    // Add the new byte
    fifo = (fifo << 8) | byte;
    fifoCount++;
}

uint16_t
DiskController::readFifo()
{
    assert(fifoHasData());
    
    // Remove and return the oldest word.
    fifoCount -= 2;
    return (fifo >> (8 * fifoCount)) & 0xFFFF;
}

bool
DiskController::compareFifo(uint16_t word)
{
    return fifoHasData() && ((fifo >> (8 * fifoCount)) & 0xFFFF) == word;
}

void
DiskController::serveDiskEvent()
{
    // debug("serveDiskEvent()\n");
    
    for (unsigned i = 0; i < 4; i++) {
        
        if (df[i]->isDataSource()) {
           
            // Read a single byte from the data providing drive.
            incoming = df[i]->readHead();
            
            // Remember when the incoming byte has been received.
            incomingCycle = amiga->agnus.clock;
            
            // Push the incoming byte into the FIFO buffer.
            writeFifo(incoming);
            
            // Check if we've reached a SYNC mark.
            if (compareFifo(dsksync)) {
                
                // Trigger a word SYNC interrupt.
                amiga->paula.pokeINTREQ(0x9000);
                
                // Enable DMA if the controller was waiting for that mark.
                if (state == DRIVE_DMA_SYNC_WAIT) {
                    debug(2, "DRIVE_DMA_SYNC_WAIT -> DRIVE_DMA_READ\n");
                    state = DRIVE_DMA_READ;
                    clearFifo();
                }
            }
            
            // There can only be one data provider.
            break;
        }
    }

    // Rotate the disks
    for (unsigned i = 0; i < 4; i++) {
        df[i]->rotate();
    }
    
    // Schedule next event
    handler->scheduleSecRel(DSK_SLOT, DMA_CYCLES(56), DSK_ROTATE);
}

void
DiskController::doDiskDMA()
{
    // Only proceed if the DMA enable bit is set in DSKLEN
    if (!(dsklen & 0x8000)) return;
    
    // Only proceed if there are still bytes to read
    if (!(dsklen & 0x3FFF)) return;
    
    // Only proceed if the FIFO buffer contains data
    if (!fifoHasData()) return;
    
    // For debugging, we read directly from disk (df0 only)
    /*
    uint8_t data1 = 0xFF;
    uint8_t data2 = 0xFF;
    if (df[0]->isSelected() && df[0]->motor) {
        data1 = df[0]->readHead();
        df[0]->rotate();
        data2 = df[0]->readHead();
        df[0]->rotate();
    }
    uint16_t word = HI_LO(data1, data2);
    */
    
    // Read the next word from the FIFO buffer
    uint16_t word = readFifo();
    
    // Perform DMA if it is enabled
    if (state == DRIVE_DMA_READ) {
        
        // plaindebug("DMA(HI) %d: %X -> %X\n", dsklen & 0x3FFF, HI_BYTE(word), amiga->agnus.dskpt);
        // plaindebug("DMA(LO) %d: %X -> %X\n", dsklen & 0x3FFF, LO_BYTE(word), amiga->agnus.dskpt + 1);
        // debug("DMA(HI) %d: %X\n", dsklen & 0x3FFF, HI_BYTE(word));
        // debug("DMA(LO) %d: %X\n", dsklen & 0x3FFF, LO_BYTE(word));

        amiga->mem.pokeChip16(amiga->agnus.dskpt, word);
        amiga->agnus.dskpt = (amiga->agnus.dskpt + 2) & 0x7FFFF;
        
        dsklen--;
        if ((dsklen & 0x3FFF) == 0) {
            amiga->paula.pokeINTREQ(0x8002);
            state = DRIVE_DMA_OFF;
            debug("Disk DMA DONE.\n");
            
            /*
            for (unsigned i = 0; i < 7358 * 2; i += 8) {
                plaindebug("%02X %02X %02X %02X %02X %02X %02X %02X\n",
                           amiga->mem.chipRam[0x6B14 + i],
                           amiga->mem.chipRam[0x6B14 + i + 1],
                           amiga->mem.chipRam[0x6B14 + i + 2],
                           amiga->mem.chipRam[0x6B14 + i + 3],
                           amiga->mem.chipRam[0x6B14 + i + 4],
                           amiga->mem.chipRam[0x6B14 + i + 5],
                           amiga->mem.chipRam[0x6B14 + i + 6],
                           amiga->mem.chipRam[0x6B14 + i + 7]);
            }
            */
        }
    }
    
    // Did we reach a SYNC mark?
    // debug("head: %d dsksync = %X word = %X\n", df[0]->head.offset, dsksync, word);
    dsksync = 0x4489; // REMOVE ASAP
    if (word == dsksync) {

        debug(2, "SYNC mark found.\n");
        
        // Trigger word SYNC interrupt
        amiga->paula.pokeINTREQ(0x9000);
        
        // Enable DMA if the controller was waiting for the SYNC mark
        if (state == DRIVE_DMA_SYNC_WAIT) {
            debug("Finally enabling DMA.\n");
            state = DRIVE_DMA_READ;
        }
    }
    
    // debug("Disk DMA: %X %X (%d words remain)\n", data1, data2, dsklen);
}

bool
DiskController::doesDMA(int nr)
{
    // TODO: return dma && isSelected(nr);
    return false;
}
