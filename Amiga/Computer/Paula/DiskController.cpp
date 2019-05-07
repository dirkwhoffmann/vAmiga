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
        
        { &selected, sizeof(selected), 0 },
        { &acceleration,  sizeof(acceleration),  0 },
        { &state,         sizeof(state),         0 },
        { &syncFlag,      sizeof(syncFlag),      0 },
        { &floppySync,    sizeof(floppySync),    0 },
        { &incoming,      sizeof(incoming),      0 },
        { &incomingCycle, sizeof(incomingCycle), 0 },
        { &fifo,          sizeof(fifo),          0 },
        { &fifoCount,     sizeof(fifoCount),     0 },
        { &dsklen,        sizeof(dsklen),        0 },
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
    selected = -1;
    dsksync = 0x4489;
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

    info.selectedDrive = selected;
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

bool
DiskController::spinning(unsigned driveNr)
{
    assert(driveNr < 4);
    return df[driveNr]->motor;
}

bool
DiskController::spinning()
{
    return df[0]->motor | df[1]->motor | df[2]->motor | df[3]->motor;
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

Drive *
DiskController::getSelectedDrive()
{
    assert(selected < 4);
    return selected < 0 ? NULL : df[selected];
}

uint16_t
DiskController::peekDSKDATR()
{
    // DSKDAT is a strobe register that cannot be accessed by the CPU
    return 0;
}

void
DiskController::pokeDSKLEN(uint16_t newDskLen)
{
    // plaindebug(1, "pokeDSKLEN(%X)\n", newDskLen);
    
    Drive *drive = getSelectedDrive(); 
    uint16_t oldDsklen = dsklen;

    // Initialize checksum (for debugging only)
    checksum = fnv_1a_init32();
    
    if (drive) drive->head.offset = 0;
    
    // Remember the new value
    dsklen = newDskLen;
    
    // Disable DMA if the DMAEN bit (15) is zero
    if (!(newDskLen & 0x8000)) {
        // plaindebug(1, "dma = DRIVE_DMA_OFF\n");
        state = DRIVE_DMA_OFF;
    }
    
    // Enable DMA the DMAEN bit (bit 15) has been written twice.
    else if (oldDsklen & newDskLen & 0x8000) {
        
        // Check if the WRITE bit (bit 14) also has been written twice.
        if (oldDsklen & newDskLen & 0x4000) {
            
            // plaindebug(1, "dma = DRIVE_DMA_WRITE\n");
            state = DRIVE_DMA_WRITE;
            
        } else {
            
            // Check the WORDSYNC bit in the ADKCON register
            if (GET_BIT(amiga->paula.adkcon, 10)) {
                
                // Wait with reading until a sync mark has been found
                // plaindebug(1, "dma = DRIVE_DMA_READ_SYNC\n");
                state = DRIVE_DMA_SYNC_WAIT;
                
            } else {
                
                // Start reading immediately
                // plaindebug(1, "dma = DRIVE_DMA_READ\n");
                state = DRIVE_DMA_READ;
                clearFifo();
            }
        }
    }
    
    // If the selected drive is a turbo drive, perform DMA immediately
    if (drive && drive->isTurboDrive()) performTurboDMA(drive);
}

void
DiskController::pokeDSKDAT(uint16_t value)
{
    // DSKDAT is a strobe register that cannot be accessed by the CPU.
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
    
    // DSKBYT
    assert(amiga->agnus.clock >= incomingCycle);
    if (amiga->agnus.clock - incomingCycle <= 7) SET_BIT(result, 15);
    
    // DMAON
    if (amiga->agnus.dskDMA() && state != DRIVE_DMA_OFF) SET_BIT(result, 14);

    // DSKWRITE
    if (dsklen & 0x4000) SET_BIT(result, 13);
    
    // WORDEQUAL
#ifdef EASY_DISK
    if (compareFifo(dsksync)) SET_BIT(result, 12);
#else
    if (syncFlag) SET_BIT(result, 12);
#endif

    debug(1, "peekDSKBYTR() = %X\n", result);
    return result;
}

void
DiskController::pokeDSKSYNC(uint16_t value)
{
    assert(false);
    debug(1, "pokeDSKSYNC(%X)\n", value);
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
    
    selected = -1;
    
    // Iterate over all connected drives
    for (unsigned i = 0; i < 4; i++) { if (!connected[i]) continue;
        
        // Inform the drive and determine the selected one
        df[i]->PRBdidChange(oldValue, newValue);
        if (df[i]->isSelected()) {
            selected = i;
            acceleration = df[i]->getSpeed();
        }
    }
    
    // Schedule the first rotation event if at least one drive is spinning.
    if (!spinning()) {
        handler->cancelSec(DSK_SLOT);
    }
    else if (!handler->hasEventSec(DSK_SLOT)) {
        handler->scheduleSecRel(DSK_SLOT, DMA_CYCLES(56), DSK_ROTATE);
    }
}

void
DiskController::serveDiskEvent()
{
    assert(selected >= -1 && selected <= 3);
    
    // Receive next byte from the selected drive.
    readByte();
    
    // Schedule next event.
    handler->scheduleSecRel(DSK_SLOT, DMA_CYCLES(56), DSK_ROTATE);
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

#ifdef EASY_DISK

void
DiskController::readByte()
{
}

#else

void
DiskController::readByte()
{
    Drive *d = getSelectedDrive();

    // Only proceed if a drive is selected.
    if (d == NULL) return;
        
    // Only proceed if there are remaining bytes to read.
    if ((dsklen & 0x3FFF) == 0) return;
    
    // Only proceed if the selected drive provides data.
    // if (d->isDataSource()) {
    if (state == DRIVE_DMA_READ || state == DRIVE_DMA_SYNC_WAIT) {
        
        // Read a single byte from the drive head.
        incoming = d->readHead();
        
        // Remember when the incoming byte has been received.
        incomingCycle = amiga->agnus.clock;
        
        // Push the incoming byte into the FIFO buffer.
        writeFifo(incoming);
        
        // Check if we've reached a SYNC mark.
        /*
         if (compareFifo(dsksync)) {
         
         // Trigger a word SYNC interrupt.
         debug(2, "SYNC IRQ\n");
         amiga->paula.pokeINTREQ(0x9000);
         
         // Enable DMA if the controller was waiting for the SYNC mark.
         if (state == DRIVE_DMA_SYNC_WAIT) {
         debug(1, "DRIVE_DMA_SYNC_WAIT -> DRIVE_DMA_READ\n");
         state = DRIVE_DMA_READ;
         clearFifo();
         }
         }
         */
    }
}

#endif

void
DiskController::performDMA()
{
    Drive *drive = getSelectedDrive();
    
    // Only proceed if a drive is selected.
    if (drive == NULL) return;
    
    // Only proceed if there are remaining bytes to read.
    if (!(dsklen & 0x3FFF)) return;
    
    // Only proceed if DMA is enabled.
    if (state != DRIVE_DMA_READ && state != DRIVE_DMA_WRITE) return;
    
    // debug("performDMA()\n");
    
    // Only proceed if the FIFO buffer contains at least one data word.
    // if (!fifoHasData()) return;
    
    // Perform DMA
    switch (state) {
        
        case DRIVE_DMA_READ:
        if (!fifoHasData()) return;
        performDMARead(drive);
        break;
        
        case DRIVE_DMA_WRITE:
        performDMAWrite(drive);
        break;
        
        default: assert(false);
    }
}

void
DiskController::performDMARead(Drive *d)
{
    // Determine how many words we are supposed to transfer.
    uint32_t remaining = acceleration;
    
    do {
        // Read next word from the FIFO queue.
        uint16_t word = readFifo();
        
        // Write word into memory.
        amiga->mem.pokeChip16(amiga->agnus.dskpt, word);
        
        amiga->agnus.dskpt = (amiga->agnus.dskpt + 2) & 0x7FFFF;
        checksum = fnv_1a_it32(checksum, word);
        
        // Finish up if this was the last word to transfer.
        if ((--dsklen & 0x3FFF) == 0) {
            
            amiga->paula.pokeINTREQ(0x8002);
            state = DRIVE_DMA_OFF;
            plainmsg("performRead: Checksum = %X\n", checksum);
            floppySync = 0;
            return;
        }
        
        // Fill the FIFO with the next word if the loop repeats.
        if (--remaining) {
            readByte();
            readByte();
            assert(fifoHasData());
        }
        
    } while (remaining);
}

void
DiskController::performDMAWrite(Drive *d)
{
    // Determine how many words we are supposed to transfer.
    uint32_t remaining = acceleration;
    
    do {
        // Read next word from memory.
        uint16_t word = amiga->mem.peekChip16(amiga->agnus.dskpt);
        amiga->agnus.dskpt = (amiga->agnus.dskpt + 2) & 0x7FFFF;
        checksum = fnv_1a_it32(checksum, word);
        // plaindebug("%d: %X (%X)\n", dsklen & 0x3FFF, word, dcheck);
        
        // Write word to disk (TODO: WRITE INTO FIFO)
        d->writeHead(HI_BYTE(word));
        d->writeHead(LO_BYTE(word));
        
        // Finish up if this was the last word to transfer.
        if ((--dsklen & 0x3FFF) == 0) {
            
            amiga->paula.pokeINTREQ(0x8002);
            state = DRIVE_DMA_OFF;
            plainmsg("* performWrite: Checksum = %X\n", checksum);
            floppySync = 0;
            return;
        }
        
        // TODO: Write another word if the loop repeats.
        if (--remaining) {
            // writeByte();
            // writeByte();
        }
        
    } while (remaining);
}

void
DiskController::performSimpleDMA()
{
    Drive *drive = getSelectedDrive();
    
    // Only proceed if a drive is selected.
    if (drive == NULL) return;
    
    // Only proceed if there are remaining bytes to read.
    if (!(dsklen & 0x3FFF)) return;

    // Only proceed if DMA is enabled.
    if (state != DRIVE_DMA_READ && state != DRIVE_DMA_WRITE) return;
    
    // Perform DMA
    switch (state) {

        case DRIVE_DMA_READ:
        performSimpleDMARead(drive);
        break;

        case DRIVE_DMA_WRITE:
        performSimpleDMAWrite(drive);
        break;
        
        default: assert(false);
    }
}

void
DiskController::performSimpleDMARead(Drive *drive)
{
    for (unsigned i = 0; i < acceleration; i++) {
        
        // Read word from disk.
        uint16_t word = drive->readHead16();
        
        // Write word into memory.
        amiga->mem.pokeChip16(amiga->agnus.dskpt, word);
  
        amiga->agnus.dskpt = (amiga->agnus.dskpt + 2) & 0x7FFFF;
        checksum = fnv_1a_it32(checksum, word);
        
        dsklen--;
        
        if ((dsklen & 0x3FFF) == 0) {
            
            amiga->paula.pokeINTREQ(0x8002);
            state = DRIVE_DMA_OFF;
            plainmsg("doSimpleDMARead: Checksum = %X\n", checksum);
            floppySync = 0;
            return;
        }
    }
}

void
DiskController::performSimpleDMAWrite(Drive *drive)
{
    // debug("Writing %d words to disk\n", dsklen & 0x3FFF);
    
    for (unsigned i = 0; i < acceleration; i++) {
        
        // Read word from memory
        uint16_t word = amiga->mem.peekChip16(amiga->agnus.dskpt);
        plainmsg("%X -> %X (%d)\n", word, amiga->agnus.dskpt, drive->head.offset);
        amiga->agnus.dskpt = (amiga->agnus.dskpt + 2) & 0x7FFFF;
        checksum = fnv_1a_it32(checksum, word);
        // plaindebug("%d: %X (%X)\n", dsklen & 0x3FFF, word, dcheck);
        
        // Write word to disk
        drive->writeHead16(word);
        
        if ((--dsklen & 0x3FFF) == 0) {
            
            amiga->paula.pokeINTREQ(0x8002);
            state = DRIVE_DMA_OFF;
            plainmsg("* doSimpleDMAWrite: Checksum = %X\n", checksum);
            return;
        }
    }
}

void
DiskController::performTurboDMA(Drive *drive)
{
    // Only proceed if there are remaining bytes to read.
    if ((dsklen & 0x3FFF) == 0) return;
    
    // Perform action depending on DMA state
    switch (state) {
            
        case DRIVE_DMA_READ:
            
            performTurboRead(drive);
            break;
            
        case DRIVE_DMA_WRITE:
            
            performTurboWrite(drive);
            break;
            
        default:
            return;
    }
    
    // Trigger disk interrupt
    amiga->paula.pokeINTREQ(0x8002);
    state = DRIVE_DMA_OFF;
}

void
DiskController::performTurboRead(Drive *drive)
{
    plaindebug(1, "Turbo-reading %d words from disk.\n", dsklen & 0x3FFF);
    
    for (unsigned i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from disk.
        uint16_t word = drive->readHead16();
        
        // Write word into memory.
        amiga->mem.pokeChip16(amiga->agnus.dskpt, word);
        amiga->agnus.dskpt = (amiga->agnus.dskpt + 2) & 0x7FFFF;
        
        // Compute checksum (for debugging only)
        checksum = fnv_1a_it32(checksum, word);
    }
        
    plaindebug(2, "Turbo read %s: checksum = %X\n", drive->getDescription(), checksum);
}

void
DiskController::performTurboWrite(Drive *drive)
{
    plaindebug(1, "Turbo-writing %d words to disk.\n", dsklen & 0x3FFF);
    
    for (unsigned i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from memory
        uint16_t word = amiga->mem.peekChip16(amiga->agnus.dskpt);
        // plainmsg("%X -> %X (%d)\n", word, amiga->agnus.dskpt, d->head.offset);
        amiga->agnus.dskpt = (amiga->agnus.dskpt + 2) & 0x7FFFF;
        
        // Compute checksum (for debugging only)
        checksum = fnv_1a_it32(checksum, word);
        
        // Write word to disk
        drive->writeHead16(word);
    }
    
    plaindebug(2, "Turbo write %s: checksum = %X\n", drive->getDescription(), checksum);
}

