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
        
        { &connected,     sizeof(connected),     BYTE_ARRAY | PERSISTANT },
        
        { &selectedDrive, sizeof(selectedDrive), 0 },
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
    selectedDrive = -1;
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

    info.selectedDrive = selectedDrive;
    info.state = state;
    info.fifoCount = fifoCount;
    info.dsklen = dsklen;
    info.dskbytr = amiga->mem.spypeekChip16(DSKBYTR);
    info.dsksync = dsksync;
    info.prb = prb;
 
    for (unsigned i = 0; i < 6; i++) {
        info.fifo[i] = (fifo >> (8 * i)) & 0xFF;
    }
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
    
    // Store a copy of the new value for reference.
    prb = newValue;
    
    // Determine the selected drive.
    /* In theory, multiple drives can be selected. In that case, we give
     * priority to the drive with the lowest number and ignore all others.
     */
    selectedDrive =
    ((prb & 0b0001000) == 0) ? 0 :
    ((prb & 0b0010000) == 0) ? 1 :
    ((prb & 0b0100000) == 0) ? 2 :
    ((prb & 0b1000000) == 0) ? 3 : -1;
    
    // Determine the current motor status of all four drives.
    bool motor = df[0]->motor | df[1]->motor | df[2]->motor | df[3]->motor;
    
    // Schedule the first rotation event if at least one drive is spinning.
    if (!motor) {
        handler->cancelSec(DSK_SLOT);
    }
    else if (!handler->hasEventSec(DSK_SLOT)) {
        handler->scheduleSecRel(DSK_SLOT, DMA_CYCLES(56), DSK_ROTATE);
    }
    
    // Pass control over to all four drives.
    for (unsigned i = 0; i < 4; i++) {
        if (connected[i]) df[i]->PRBdidChange(oldValue, newValue);
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
    assert(selectedDrive >= -1 && selectedDrive <= 3);
    
    // Receive next byte from the selected drive.
    readByte();
    
    // Schedule next event.
    handler->scheduleSecRel(DSK_SLOT, DMA_CYCLES(56), DSK_ROTATE);
}

void
DiskController::readByte()
{
    // Only proceed if a drive is selected.
    if (selectedDrive >= 0) {
        
        Drive *dfsel = df[selectedDrive];
        
        // Only proceed if the selected drive provides data.
        if (dfsel->isDataSource()) {
            
            // Read a single byte from the drive head.
            incoming = dfsel->readHead();
            
            // Remember when the incoming byte has been received.
            incomingCycle = amiga->agnus.clock;
            
            // Push the incoming byte into the FIFO buffer.
            writeFifo(incoming);
            
            // Check if we've reached a SYNC mark.
            if (compareFifo(dsksync)) {
                
                // Trigger a word SYNC interrupt.
                amiga->paula.pokeINTREQ(0x9000);
                
                // Enable DMA if the controller was waiting for the SYNC mark.
                if (state == DRIVE_DMA_SYNC_WAIT) {
                    debug(2, "DRIVE_DMA_SYNC_WAIT -> DRIVE_DMA_READ\n");
                    state = DRIVE_DMA_READ;
                    clearFifo();
                }
            }
            
            // Rotate the disk.
            dfsel->rotate();
        }
    }
}

void
DiskController::doDiskDMA()
{
    // Only proceed if DSKLEN has the DMA enable bit set.
    if (!(dsklen & 0x8000)) return;
    
    // Only proceed if there are remaining bytes to read.
    if (!(dsklen & 0x3FFF)) return;
    
    // Only proceed if the FIFO buffer contains at least two bytes (one word).
    if (!fifoHasData()) return;
    
    // Perform DMA (if drive is in read mode).
    if (state == DRIVE_DMA_READ) {
        
        // Read next word from buffer.
        uint16_t word = readFifo();
        
        // Write word into memory.
        amiga->mem.pokeChip16(amiga->agnus.dskpt, word);
        amiga->agnus.dskpt = (amiga->agnus.dskpt + 2) & 0x7FFFF;
        
        dsklen--;
        
        // Trigger interrupt if the last word has been written.
        if (!(dsklen & 0x3FFF)) {
            amiga->paula.pokeINTREQ(0x8002);
            state = DRIVE_DMA_OFF;
            debug(2, "Disk DMA DONE.\n");
        }
    }
}

bool
DiskController::doesDMA(int nr)
{
    return state == DRIVE_DMA_READ;
}
