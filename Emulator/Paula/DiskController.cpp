// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DiskController::DiskController(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("DiskController");

    // Setup initial configuration
    config.connected[0] = true;
    config.connected[1] = false;
    config.connected[2] = false;
    config.connected[3] = false;
    config.asyncFifo = true;
    config.lockDskSync = false;
    config.autoDskSync = false;
}

void
DiskController::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS
    
    prb = 0xFF;
    selected = -1;
    dsksync = 0x4489;
    
    assert(diskToInsert == NULL);
}

void
DiskController::_ping()
{
    for (int df = 0; df < 4; df++) {
        amiga.putMessage(config.connected[df] ? MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, df);
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
    info.dskbytr =  computeDSKBYTR();
    info.dsksync = dsksync;
    info.prb = prb;
 
    for (unsigned i = 0; i < 6; i++) {
        info.fifo[i] = (fifo >> (8 * i)) & 0xFF;
    }
    pthread_mutex_unlock(&lock);
}

void
DiskController::_dumpConfig()
{
    msg("          df0 : %s\n", config.connected[0] ? "connected" : "not connected");
    msg("          df1 : %s\n", config.connected[1] ? "connected" : "not connected");
    msg("          df2 : %s\n", config.connected[2] ? "connected" : "not connected");
    msg("          df3 : %s\n", config.connected[3] ? "connected" : "not connected");
    msg("    asyncFifo : %s\n", config.asyncFifo ? "yes" : "no");
    msg("  lockDskSync : %s\n", config.lockDskSync ? "yes" : "no");
    msg("  autoDskSync : %s\n", config.autoDskSync ? "yes" : "no");
}

void
DiskController::_dump()
{
    msg("     selected : %d\n", selected);
    msg("        state : %s\n", driveStateName(state));
    msg("     syncFlag : %s\n", syncFlag ? "true" : "false");
    msg("     incoming : %X (cylcle = %lld)\n", incoming, incomingCycle);
    msg("         fifo : %llX (count = %d)\n", fifo, fifoCount);
    msg("\n");
    msg("       dsklen : %X\n", dsklen);
    msg("      dsksync : %X\n", dsksync);
    msg("          prb : %X\n", prb);
    msg("\n");
    msg("   spinning() : %d\n", spinning());
}

bool
DiskController::spinning(unsigned driveNr)
{
    assert(driveNr < 4);
    return df[driveNr]->motor();
}

bool
DiskController::spinning()
{
    return df[0]->motor() || df[1]->motor() || df[2]->motor() || df[3]->motor();
}

void
DiskController::setState(DriveState s)
{
    debug(DSK_DEBUG, "%s -> %s\n", driveStateName(state), driveStateName(s));
    if (state == s) return;

    bool wasWriting = (state == DRIVE_DMA_WRITE);
    bool isWriting = (s == DRIVE_DMA_WRITE);

    state = s;
    
    if (wasWriting != isWriting) {
        amiga.putMessage(isWriting ? MSG_DRIVE_WRITE : MSG_DRIVE_READ, selected);
    }
}

void
DiskController::setConnected(int df, bool value)
{
    assert(df < 4);
    
    // We don't allow the internal drive (Df0) to be disconnected
    if (df == 0 && value == false) { return; }
    
    // Plug the drive in our out and inform the GUI
    pthread_mutex_lock(&lock);
    config.connected[df] = value;
    pthread_mutex_unlock(&lock);

    amiga.putMessage(value ? MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, df);
    amiga.putMessage(MSG_CONFIG);
}

void
DiskController::setSpeed(i32 value)
{
    amiga.suspend();

    df[0]->setSpeed(value);
    df[1]->setSpeed(value);
    df[2]->setSpeed(value);
    df[3]->setSpeed(value);

    amiga.resume();
}

void
DiskController::setAsyncFifo(bool value)
{
    pthread_mutex_lock(&lock);
    config.asyncFifo = value;
    pthread_mutex_unlock(&lock);
}

void
DiskController::setLockDskSync(bool value)
{
    pthread_mutex_lock(&lock);
    config.lockDskSync = value;
    pthread_mutex_unlock(&lock);
}

void
DiskController::setAutoDskSync(bool value)
{
    pthread_mutex_lock(&lock);
    config.autoDskSync = value;
    pthread_mutex_unlock(&lock);
}

Drive *
DiskController::getSelectedDrive()
{
    assert(selected < 4);
    return selected < 0 ? NULL : df[selected];
}

void
DiskController::ejectDisk(int nr, Cycle delay)
{
    assert(nr >= 0 && nr <= 3);

    debug("ejectDisk(%d, %d)\n", nr, delay);

    amiga.suspend();
    agnus.scheduleRel<DCH_SLOT>(delay, DCH_EJECT, nr);
    amiga.resume();
}

void
DiskController::insertDisk(class Disk *disk, int nr, Cycle delay)
{
    assert(disk != NULL);
    assert(nr >= 0 && nr <= 3);

    debug(DSK_DEBUG, "insertDisk(%p, %d, %d)\n", disk, nr, delay);

    // The easy case: The emulator is not running
    if (!amiga.isRunning()) {

        df[nr]->ejectDisk();
        df[nr]->insertDisk(disk);
        return;
    }

    // The not so easy case: The emulator is running
    amiga.suspend();

    if (df[nr]->hasDisk()) {

        // Eject the old disk first
        df[nr]->ejectDisk();

        // Make sure there is enough time between ejecting and inserting.
        // Otherwise, the Amiga might not detect the change.
        delay = MAX(SEC(1.5), delay);
    }

    diskToInsert = disk;
    agnus.scheduleRel<DCH_SLOT>(delay, DCH_INSERT, nr);
    
    amiga.resume();
}

void
DiskController::insertDisk(class ADFFile *file, int nr, Cycle delay)
{
    if (Disk *disk = Disk::makeWithFile(file)) {
        insertDisk(disk, nr, delay);
    }
}

void
DiskController::setWriteProtection(int nr, bool value)
{
    assert(nr >= 0 && nr <= 3);
    df[nr]->setWriteProtection(value);
}

u16
DiskController::peekDSKDATR()
{
    // DSKDAT is a strobe register that cannot be accessed by the CPU
    return 0;
}

void
DiskController::pokeDSKLEN(u16 newDskLen)
{
    debug(DSKREG_DEBUG, "pokeDSKLEN(%X)\n", newDskLen);
    
    Drive *drive = getSelectedDrive(); 
    u16 oldDsklen = dsklen;

    // Remember the new value
    dsklen = newDskLen;

    // Initialize checksum (for debugging only)
    if (DSK_CHECKSUM) {
        checkcnt = 0;
        checksum = fnv_1a_init32();
    }

    // Determine if a FIFO buffer should be emulated
    asyncFifo = config.asyncFifo;
    
    // Disable DMA if bit 15 (DMAEN) is zero
    if (!(newDskLen & 0x8000)) {

        setState(DRIVE_DMA_OFF);
        clearFifo();
    }
    
    // Enable DMA if bit 15 (DMAEN) has been written twice
    else if (oldDsklen & newDskLen & 0x8000) {

        // Reset head position in debug mode to generate reproducable results
        if (DRIVE_DEBUG) drive->head.offset = 0;

        // Check if the WRITE bit (bit 14) also has been written twice.
        if (oldDsklen & newDskLen & 0x4000) {
            
            setState(DRIVE_DMA_WRITE);
            clearFifo();
            
        } else {
            
            // Check the WORDSYNC bit in the ADKCON register
            if (GET_BIT(paula.adkcon, 10)) {
                
                // Wait with reading until a sync mark has been found
                setState(DRIVE_DMA_WAIT);
                clearFifo();
                
            } else {
                
                // Start reading immediately
                setState(DRIVE_DMA_READ);
                clearFifo();
            }
        }
    }
    
    // If the selected drive is a turbo drive, perform DMA immediately
    if (drive && drive->isTurbo()) performTurboDMA(drive);
}

void
DiskController::pokeDSKDAT(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKDAT\n");

    // DSKDAT is a strobe register that cannot be accessed by the CPU.
}

u16
DiskController::peekDSKBYTR()
{
    u16 result = computeDSKBYTR();
    
    debug(DSKREG_DEBUG, "peekDSKBYTR() = %X\n", result);
    return result;
}

u16
DiskController::computeDSKBYTR()
{
    /* 15      DSKBYT     Indicates whether this register contains valid data
     * 14      DMAON      Indicates whether disk DMA is actually enabled
     * 13      DISKWRITE  Matches the WRITE bit in DSKLEN
     * 12      WORDEQUAL  Indicates a match with the contents of DISKSYNC
     * 11 - 8             Unused
     *  7 - 0  DATA       Disk byte data
     */
    
    // DATA
    u16 result = incoming;
    
    // DSKBYT
    assert(agnus.clock >= incomingCycle);
    if (agnus.clock - incomingCycle <= 7) SET_BIT(result, 15);
    
    // DMAON
    if (agnus.dskdma() && state != DRIVE_DMA_OFF) SET_BIT(result, 14);

    // DSKWRITE
    if (dsklen & 0x4000) SET_BIT(result, 13);
    
    // WORDEQUAL
    if (syncFlag) SET_BIT(result, 12);

    return result;
}

void
DiskController::pokeDSKSYNC(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKSYNC(%X)\n", value);
    
    if (value != 0x4489 && config.lockDskSync) {
        debug("Write to DSKSYNC blocked (%x)\n", value);
        return;
    }
    
    dsksync = value;
}

u8
DiskController::driveStatusFlags()
{
    u8 result = 0xFF;
    
    if (config.connected[0]) result &= df[0]->driveStatusFlags();
    if (config.connected[1]) result &= df[1]->driveStatusFlags();
    if (config.connected[2]) result &= df[2]->driveStatusFlags();
    if (config.connected[3]) result &= df[3]->driveStatusFlags();
    
    return result;
}

void
DiskController::PRBdidChange(u8 oldValue, u8 newValue)
{
    // debug("PRBdidChange: %X -> %X\n", oldValue, newValue);

    // Store a copy of the new value for reference.
    prb = newValue;
    
    i8 oldSelected = selected;
    selected = -1;
    
    // Iterate over all connected drives
    for (unsigned i = 0; i < 4; i++) {
        if (!config.connected[i]) continue;
        
        // Inform the drive and determine the selected one
        df[i]->PRBdidChange(oldValue, newValue);
        if (df[i]->isSelected()) selected = i;
    }
    
    // Schedule the first rotation event if at least one drive is spinning
    if (!spinning()) {
        agnus.cancel<DSK_SLOT>();
    }
    else if (!agnus.hasEvent<DSK_SLOT>()) {
        agnus.scheduleRel<DSK_SLOT>(DMA_CYCLES(56), DSK_ROTATE);
    }

    // Inform the GUI
    if (oldSelected != selected) amiga.putMessage(MSG_DRIVE_SELECT, selected);
}

void
DiskController::serviceDiskEvent()
{
    if (asyncFifo) {
        
        // Receive next byte from the selected drive
        executeFifo();
    
        // Schedule next event
        agnus.scheduleRel<DSK_SLOT>(DMA_CYCLES(56), DSK_ROTATE);
    }
}

void
DiskController::serviceDiskChangeEvent()
{
    if (agnus.slot[DCH_SLOT].id == EVENT_NONE) return;
    
    int n = (int)agnus.slot[DCH_SLOT].data;
    assert(n >= 0 && n <= 3);

    switch (agnus.slot[DCH_SLOT].id) {

        case DCH_INSERT:

            debug(DSK_DEBUG, "DCH_INSERT (df%d)\n", n);

            assert(diskToInsert != NULL);
            df[n]->insertDisk(diskToInsert);
            diskToInsert = NULL;
            break;

        case DCH_EJECT:

            debug(DSK_DEBUG, "DCH_EJECT (df%d)\n", n);

            df[n]->ejectDisk();
            break;

        default:
            assert(false);
    }

    agnus.cancel<DCH_SLOT>();
}

void
DiskController::clearFifo()
{
    fifo = 0;
    fifoCount = 0;
}

u8
DiskController::readFifo()
{
    // Don't call this function on an empty buffer.
    assert(fifoCount > 0);
    
    // Remove and return the oldest byte.
    fifoCount--;
    return (fifo >> (8 * fifoCount)) & 0xFF;
}

void
DiskController::writeFifo(u8 byte)
{
    assert(fifoCount <= 6);
    
    // Remove oldest word if the FIFO is full
    if (fifoCount == 6) fifoCount -= 2;
    
    // Add the new byte
    fifo = (fifo << 8) | byte;
    fifoCount++;
}

u16
DiskController::readFifo16()
{
    assert(fifoHasWord());
    
    // Remove and return the oldest word.
    fifoCount -= 2;
    return (fifo >> (8 * fifoCount)) & 0xFFFF;
}

bool
DiskController::compareFifo(u16 word)
{
    return fifoHasWord() && (fifo & 0xFFFF) == word;
}

void
DiskController::executeFifo()
{
    // Only proceed if a drive is selected.
    Drive *drive = getSelectedDrive();
    if (drive == NULL) return;

    switch (state) {
            
        case DRIVE_DMA_OFF:
        case DRIVE_DMA_WAIT:
        case DRIVE_DMA_READ:
            
            // Read a byte from the drive and store a time stamp
            incoming = drive->readHead();
            incomingCycle = agnus.clock;
            
            // Write byte into the FIFO buffer
            writeFifo(incoming);
            
            // Check if we've reached a SYNC mark
            if ((syncFlag = compareFifo(dsksync)) ||
                (config.autoDskSync && syncCounter++ > 20000)) {

                // Trigger a word SYNC interrupt
                debug(DSK_DEBUG, "SYNC IRQ (dsklen = %d)\n", dsklen);
                paula.raiseIrq(INT_DSKSYN);

                // Enable DMA if the controller was waiting for it
                if (state == DRIVE_DMA_WAIT) {
                    setState(DRIVE_DMA_READ);
                    clearFifo();
                }
                
                // Reset the watchdog counter
                syncCounter = 0;
            }
            break;
            
        case DRIVE_DMA_WRITE:
        case DRIVE_DMA_FLUSH:
            
            if (fifoIsEmpty()) {
                
                // Switch off DMA if the last byte has been flushed out
                if (state == DRIVE_DMA_FLUSH) setState(DRIVE_DMA_OFF);
                
            } else {
                
                // Read the outgoing byte from the FIFO buffer
                u8 outgoing = readFifo();
                
                // Write byte to disk
                drive->writeHead(outgoing);
            }
            break;
    }
}

void
DiskController::performDMA()
{
    // Emulate the FIFO buffer if asynchronous mode is disabled
    if (!asyncFifo) { executeFifo(); executeFifo(); }
    
    // Only proceed if there are remaining bytes to read
    if ((dsklen & 0x3FFF) == 0) return;

    // Only proceed if DMA is enabled
    if (state != DRIVE_DMA_READ && state != DRIVE_DMA_WRITE) return;

    // Only proceed if a drive is selected
    Drive *drive = getSelectedDrive();
    if (drive == NULL) return;

    // How many word shall we read in?
    u32 count = drive->config.speed;

    // Perform DMA
    switch (state) {
            
        case DRIVE_DMA_READ:
            performDMARead(drive, count);
            break;
            
        case DRIVE_DMA_WRITE:
            performDMAWrite(drive, count);
            break;
            
        default: assert(false);
    }
}

void
DiskController::performDMARead(Drive *drive, u32 remaining)
{
    assert(drive != NULL);

    // Only proceed if the FIFO contains enough data
    if (!fifoHasWord()) return;

    do {
        // Read next word from the FIFO buffer
        u16 word = readFifo16();
        
        // Write word into memory
        agnus.doDiskDMA(word);

        if (DSK_CHECKSUM) {
            checkcnt++;
            checksum = fnv_1a_it32(checksum, word);
        }

        // Finish up if this was the last word to transfer
        if ((--dsklen & 0x3FFF) == 0) {

            paula.raiseIrq(INT_DSKBLK);
            setState(DRIVE_DMA_OFF);

            if (DSK_CHECKSUM)
                plaindebug("read: cnt = %d chk = %X\n", checkcnt, checksum);

            return;
        }
        
        // If the loop repeats, fill the Fifo with new data
        if (--remaining) {
            executeFifo();
            executeFifo();
        }
        
    } while (remaining);
}

void
DiskController::performDMAWrite(Drive *drive, u32 remaining)
{
    assert(drive != NULL);

    // Only proceed if the FIFO has enough free space
    if (!fifoCanStoreWord()) return;

    do {
        // Read next word from memory
        u16 word = agnus.doDiskDMA();

        if (DSK_CHECKSUM) {
            checkcnt++;
            checksum = fnv_1a_it32(checksum, word);
        }

        // Write word into FIFO buffer
        assert(fifoCount <= 4);
        writeFifo(HI_BYTE(word));
        writeFifo(LO_BYTE(word));

        // Finish up if this was the last word to transfer
        if ((--dsklen & 0x3FFF) == 0) {

            paula.raiseIrq(INT_DSKBLK);

            /* The timing-accurate approach: Set state to DRIVE_DMA_FLUSH.
             * The event handler recognises this state and switched to
             * DRIVE_DMA_OFF once the FIFO has been emptied.
             */
            
            // setState(DRIVE_DMA_FLUSH);
            
            /* I'm unsure of the timing-accurate approach works properly,
             * because the disk IRQ would be triggered before the last byte
             * has been written.
             * Hence, we play safe here and flush the FIFO immediately.
             */
            while (!fifoIsEmpty()) {
                drive->writeHead(readFifo());
            }
            setState(DRIVE_DMA_OFF);

            if (DSK_CHECKSUM)
                plaindebug("write: cnt = %d chk = %X\n", checkcnt, checksum);

            return;
        }
        
        // If the loop repeats, do what the event handler would do in between.
        if (--remaining) {
            executeFifo();
            executeFifo();
            assert(fifoCanStoreWord());
        }
        
    } while (remaining);
}

/*
void
DiskController::performSimpleDMA()
{
    Drive *drive = getSelectedDrive();
    
    // Only proceed if a drive is selected
    if (drive == NULL) return;

    // Only proceed if there are remaining bytes to read
    if (!(dsklen & 0x3FFF)) return;

    // How many word shall we read in?
    u32 count = drive->config.speed;
    
    // Perform DMA
    switch (state) {

        case DRIVE_DMA_WAIT:
        {
            performSimpleDMAWait(drive, count);
            break;
        }
        case DRIVE_DMA_READ:
        {
            performSimpleDMARead(drive, count);
            break;
        }
        case DRIVE_DMA_WRITE:
        {
            performSimpleDMAWrite(drive, count);
            break;
        }
        default: return;
    }
}

void
DiskController::performSimpleDMAWait(Drive *drive, u32 remaining)
{
    assert(drive != NULL);

    for (unsigned i = 0; i < remaining; i++) {

        // Read word from disk
        u16 word = drive->readHead16();

        // Check if a SYNC mark has been reached
        if ((syncFlag = (word == dsksync))) {

            debug("SYNC mark found\n"); // REMOVE ASAP
            
            // Trigger a word SYNC interrupt
            debug(DSK_DEBUG, "SYNC IRQ (dsklen = %d)\n", dsklen);
            paula.raiseIrq(INT_DSKSYN);

            // Enable DMA if the controller was waiting for it
            setState(DRIVE_DMA_READ);

            return;
        }
    }
}

void
DiskController::performSimpleDMARead(Drive *drive, u32 remaining)
{
    assert(drive != NULL);

    for (unsigned i = 0; i < remaining; i++) {
        
        // Read word from disk
        u16 word = drive->readHead16();
        
        // Write word into memory
        agnus.doDiskDMA(word);

        if (DSK_CHECKSUM) {
            checkcnt++;
            checksum = fnv_1a_it32(checksum, word);
        }

        if ((--dsklen & 0x3FFF) == 0) {

            paula.raiseIrq(INT_DSKBLK);
            setState(DRIVE_DMA_OFF);

            if (DSK_CHECKSUM)
                plaindebug("simple read: cnt = %d chk = %X\n", checkcnt, checksum);

            return;
        }
    }
}

void
DiskController::performSimpleDMAWrite(Drive *drive, u32 remaining)
{
    assert(drive != NULL);
    // debug("Writing %d words to disk\n", dsklen & 0x3FFF);

    for (unsigned i = 0; i < remaining; i++) {
        
        // Read word from memory
        u16 word = agnus.doDiskDMA();

        if (DSK_CHECKSUM) {
            checkcnt++;
            checksum = fnv_1a_it32(checksum, word);
        }

        // Write word to disk
        drive->writeHead16(word);
        
        if ((--dsklen & 0x3FFF) == 0) {
            
            paula.raiseIrq(INT_DSKBLK);
            setState(DRIVE_DMA_OFF);

            if (DSK_CHECKSUM)
                debug("simple write: cnt = %d chk = %X\n", checkcnt, checksum);

            return;
        }
    }
}
*/

void
DiskController::performTurboDMA(Drive *drive)
{
    assert(drive != NULL);

    // Only proceed if there is anything to read
    if ((dsklen & 0x3FFF) == 0) return;

    // Perform action depending on DMA state
    switch (state) {

        case DRIVE_DMA_WAIT:

            drive->findSyncMark();
            // fallthrough

        case DRIVE_DMA_READ:
            
            performTurboRead(drive);
            paula.raiseIrq(INT_DSKSYN);
            break;
            
        case DRIVE_DMA_WRITE:
            
            performTurboWrite(drive);
            break;
            
        default:
            return;
    }
    
    // Trigger disk interrupt with some delay
    paula.scheduleIrqRel(INT_DSKBLK, DMA_CYCLES(512));
    setState(DRIVE_DMA_OFF);
}

void
DiskController::performTurboRead(Drive *drive)
{
    for (unsigned i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from disk
        u16 word = drive->readHead16();
        
        // Write word into memory
        mem.pokeChip16(agnus.dskpt, word);
        agnus.dskpt += 2;

        if (DSK_CHECKSUM) {
            checkcnt++;
            checksum = fnv_1a_it32(checksum, word);
        }
    }

    if (DSK_CHECKSUM) {
        plaindebug("Turbo read %s: cyl: %d side: %d offset: %d checkcnt = %d checksum = %X\n", drive->getDescription(), drive->head.cylinder, drive->head.side, drive->head.offset, checkcnt, checksum);
    }
}

void
DiskController::performTurboWrite(Drive *drive)
{
    for (unsigned i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from memory
        u16 word = mem.peekChip16(agnus.dskpt);
        agnus.dskpt += 2;
        
        if (DSK_CHECKSUM) {
            checkcnt++;
            checksum = fnv_1a_it32(checksum, word);
        }

        // Write word to disk
        drive->writeHead16(word);
    }

    if (DSK_CHECKSUM) {
        plaindebug("Turbo write %s: checkcnt = %d checksum = %X\n",
                   drive->getDescription(), checkcnt, checksum);
    }
}

