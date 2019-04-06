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
        
        { &dsklen,   sizeof(dsklen),   0 },
        { &dskdat,   sizeof(dskdat),   0 },
        { &dma,      sizeof(dma),      0 },
        
    });
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
