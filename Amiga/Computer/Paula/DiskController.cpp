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
        
        { &dma,        sizeof(dma),        0 },
        { &fifoCount,  sizeof(fifoCount),  0 },
        { &fifo,       sizeof(fifo),       0 },
        { &dsklen,     sizeof(dsklen),     0 },
        { &dskdat,     sizeof(dskdat),     0 },
        { &prb,        sizeof(prb),        0 },
        
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

void
DiskController::pokeDSKLEN(uint16_t newDskLen)
{
    uint16_t oldDsklen = dsklen;
    
    // Remember the new value
    dsklen = newDskLen;
    
    // Disable DMA if the DMAEN bit (bit 15) has been cleared.
    if (!(newDskLen & 0x8000)) {
        dma = DRIVE_DMA_OFF;
    }
    
    // Enable DMA the DMAEN bit (bit 15) has been written twice.
    else if (oldDsklen & newDskLen & 0x8000) {
        
        // Check if the WRITE bit (bit 14) also has been written twice.
        if (oldDsklen & newDskLen & 0x4000) {
            
            debug("dma = DRIVE_DMA_WRITE\n");
            dma = DRIVE_DMA_WRITE;
        } else {
            
            debug("dma = DRIVE_DMA_READ\n");
            dma = DRIVE_DMA_READ;
        }
    }
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
    uint16_t result = 42; // TODO
    
    // WORDEQUAL
    // TODO
    
    // DMAON
    if (amiga->agnus.dskDMA() && dma != DRIVE_DMA_OFF) SET_BIT(result, 14);
    
    // DSKBYT
    // TODO: Make this bit flip in a timing accurate way
    SET_BIT(result, 15);
    
    return result;
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
DiskController::serveDiskEvent()
{
    // debug("serveDiskEvent()\n");
    
    // Rotate the disks
    for (unsigned i = 0; i < 4; i++) {
        df[i]->rotate();
    }
    
    // Read a byte from the data providing drive
    for (unsigned i = 0; i < 4; i++) {
        if (df[i]->isDataSource()) writeFifo(df[i]->readHead());
    }
    
    // Schedule next event
    handler->scheduleSecRel(DSK_SLOT, DMA_CYCLES(56), DSK_ROTATE);
}

void
DiskController::writeFifo(uint8_t byte)
{
    assert(fifoCount <= 6);
    
    // Remove oldest word if the FIFO is full
    if (fifoCount == 6) (void)readFifo();
    
    // Add the new byte
    fifo = (fifo << 8) | byte;
    fifoCount++;
}

uint16_t
DiskController::readFifo()
{
    // This function assumes that the FIFO contains at least two bytes.
    assert(fifoCount > 1);
 
    // Remove and return the oldest word.
    fifoCount -= 2;
    return (fifo >> (8 * fifoCount)) & 0xFFFF;
}

void
DiskController::doDiskDMA()
{
    // Only proceed if the DMA enable bit is set in DSKLEN
    if (dsklen & 0x8000) {
        
        // Only proceed if there are still bytes to read
        if (dsklen & 0x3FFF) {
            
            uint8_t data1 = 0; // = floppyRead()
            uint8_t data2 = 0; // = floppyRead()
            
            amiga->mem.pokeChip8(amiga->agnus.dskpt, data1);
            amiga->agnus.dskpt = (amiga->agnus.dskpt + 1) & 0x7FFFF;
            amiga->mem.pokeChip8(amiga->agnus.dskpt, data2);
            amiga->agnus.dskpt = (amiga->agnus.dskpt + 1) & 0x7FFFF;
            
            // debug("Disk DMA: %X %X (%d words remain)\n", data1, data2, dsklen);
            
            dsklen--;
            if ((dsklen & 0x3FFF) == 0) {
                amiga->paula.pokeINTREQ(0x8002);
                // debug("Disk DMA finished. Setting INTREQ bit\n");
            }
        }
    }
}

bool
DiskController::doesDMA(int nr)
{
    // TODO: return dma && isSelected(nr);
    return false;
}
