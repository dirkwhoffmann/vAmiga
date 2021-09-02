// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DiskController.h"
#include "Agnus.h"
#include "ADFFile.h"
#include "Drive.h"
#include "IO.h"
#include "MsgQueue.h"
#include "Paula.h"
#include "SuspendableThread.h"
#include <algorithm>

void
DiskController::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    prb = 0xFF;
    selected = -1;
    dsksync = 0x4489;
    
    if (hard) assert(diskToInsert == nullptr);
}

DiskControllerConfig
DiskController::getDefaultConfig()
{
    DiskControllerConfig defaults;
    
    defaults.connected[0] = true;
    defaults.connected[1] = false;
    defaults.connected[2] = false;
    defaults.connected[3] = false;
    defaults.speed = 1;
    defaults.lockDskSync = false;
    defaults.autoDskSync = false;
    
    return defaults;
}

void
DiskController::resetConfig()
{
    auto defaults = getDefaultConfig();
    
    for (isize i = 0; i < 4; i++) {
        setConfigItem(OPT_DRIVE_CONNECT, i, defaults.connected[i]);
    }
    setConfigItem(OPT_DRIVE_SPEED, defaults.speed);
    setConfigItem(OPT_AUTO_DSKSYNC, defaults.lockDskSync);
    setConfigItem(OPT_LOCK_DSKSYNC, defaults.autoDskSync);
}

i64
DiskController::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_DRIVE_SPEED:   return config.speed;
        case OPT_AUTO_DSKSYNC:  return config.autoDskSync;
        case OPT_LOCK_DSKSYNC:  return config.lockDskSync;
            
        default:
            fatalError;
    }
}

i64
DiskController::getConfigItem(Option option, long id) const
{
    switch (option) {
            
        case OPT_DRIVE_CONNECT:  return config.connected[id];
            
        default:
            fatalError;
    }
}

void
DiskController::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_DRIVE_SPEED:
            
            if (!isValidDriveSpeed((isize)value)) {
                throw VAError(ERROR_OPT_INVARG, "-1, 1, 2, 4, 8");
            }
            config.speed = (i32)value;
            scheduleFirstDiskEvent();
            return;
            
        case OPT_AUTO_DSKSYNC:
            
            config.autoDskSync = value;
            return;
            
        case OPT_LOCK_DSKSYNC:
            
            config.lockDskSync = value;
            return;
            
        default:
            fatalError;
    }
}

void
DiskController::setConfigItem(Option option, long id, i64 value)
{
    switch (option)
    {
        case OPT_DRIVE_CONNECT:
            
            assert(id >= 0 && id <= 3);
            
            // We don't allow the internal drive (Df0) to be disconnected
            if (id == 0 && value == false) return;
            
            // Connect or disconnect the drive
            config.connected[id] = value;
            
            // Inform the GUI
            msgQueue.put(value ? MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, id);
            msgQueue.put(MSG_CONFIG);
            return;
            
        default:
            fatalError;
    }
}

const string &
DiskController::getSearchPath(isize dfn) const
{
    assert(dfn >= 0 && dfn <= 3);
    return searchPath[dfn];
}

void
DiskController::setSearchPath(const string &path, isize dfn)
{
    assert(dfn >= 0 && dfn <= 3);
    searchPath[dfn] = path;
}

void
DiskController::setSearchPath(const string &path)
{
    searchPath[0] = path;
    searchPath[1] = path;
    searchPath[2] = path;
    searchPath[3] = path;
}

void
DiskController::_inspect() const
{
    synchronized
    {
        info.selectedDrive = selected;
        info.state = state;
        info.fifoCount = fifoCount;
        info.dsklen = dsklen;
        info.dskbytr = computeDSKBYTR();
        info.dsksync = dsksync;
        info.prb = prb;
        
        for (isize i = 0; i < 6; i++) {
            info.fifo[i] = (fifo >> (8 * i)) & 0xFF;
        }
    }
}

void
DiskController::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
        
        os << tab("Drive df0");
        os << bol(config.connected[0], "connected", "disconnected") << std::endl;
        os << tab("Drive df1");
        os << bol(config.connected[1], "connected", "disconnected") << std::endl;
        os << tab("Drive df2");
        os << bol(config.connected[2], "connected", "disconnected") << std::endl;
        os << tab("Drive df3");
        os << bol(config.connected[3], "connected", "disconnected") << std::endl;
        os << tab("Drive speed");
        os << dec(config.speed) << std::endl;
        os << tab("lockDskSync");
        os << bol(config.lockDskSync) << std::endl;
        os << tab("autoDskSync");
        os << bol(config.autoDskSync) << std::endl;
    }
    
    if (category & dump::State) {
        
        os << tab("selected");
        os << dec(selected) << std::endl;
        os << tab("state");
        os << DriveStateEnum::key(state) << std::endl;
        os << tab("syncCycle");
        os << dec(syncCycle) << std::endl;
        os << tab("incoming");
        os << hex(incoming) << std::endl;
        os << tab("fifo");
        os << hex(fifo) << " (" << dec(fifoCount) << ")" << std::endl;
        os << tab("dsklen");
        os << dec(dsklen) << std::endl;
        os << tab("dsksync");
        os << hex(dsksync) << std::endl;
        os << tab("prb");
        os << hex(prb) << std::endl;
        os << tab("spinning");
        os << bol(spinning()) << std::endl;
        os << tab("Search paths df0");
        os << "\"" << searchPath[0] << "\"" << std::endl;
        os << tab("Search paths df1");
        os << "\"" << searchPath[1] << "\"" << std::endl;
        os << tab("Search paths df2");
        os << "\"" << searchPath[2] << "\"" << std::endl;
        os << tab("Search paths df3");
        os << "\"" << searchPath[3] << "\"" << std::endl;
    }
}

Drive *
DiskController::getSelectedDrive()
{
    assert(selected < 4);
    return selected < 0 ? nullptr : df[(int)selected];
}

bool
DiskController::spinning(isize driveNr) const
{
    assert(driveNr < 4);
    return df[driveNr]->getMotor();
}

bool
DiskController::spinning() const
{
    return df0.getMotor() || df1.getMotor() ||df2.getMotor() || df3.getMotor();
}

void
DiskController::setState(DriveState newState)
{
    if (state != newState) setState(state, newState);
}

void
DiskController::setState(DriveState oldState, DriveState newState)
{
    trace(DSK_DEBUG, "%s -> %s\n",
          DriveStateEnum::key(oldState), DriveStateEnum::key(newState));
    
    state = newState;
    
    switch (state) {
            
        case DRIVE_DMA_OFF:
            
            dsklen = 0;
            break;
            
        case DRIVE_DMA_WRITE:
            
            msgQueue.put(MSG_DRIVE_WRITE, selected);
            break;
            
        default:
            
            if (oldState == DRIVE_DMA_WRITE)
                msgQueue.put(MSG_DRIVE_READ, selected);
    }
}

void
DiskController::ejectDisk(isize nr, Cycle delay)
{
    assert(nr >= 0 && nr <= 3);
    
    suspended {
        agnus.scheduleRel<SLOT_DCH>(delay, DCH_EJECT, nr);
    }
}

void
DiskController::insertDisk(std::unique_ptr<Disk> disk, isize nr, Cycle delay)
{
    assert(disk != nullptr);
    assert(nr >= 0 && nr <= 3);
    
    debug(DSK_DEBUG, "insertDisk(%zd, %lld)\n", nr, delay);
    
    // Only proceed if the disk is compatible with the selected drive
    if (!df[nr]->isInsertable(*disk)) throw VAError(ERROR_DISK_INCOMPATIBLE);
    
    // The easy case: The emulator is not running
    if (!isRunning()) {
        
        df[nr]->ejectDisk();
        df[nr]->insertDisk(std::move(disk));
        return;
    }
    
    // The not so easy case: The emulator is running
    suspended {
        
        if (df[nr]->hasDisk()) {
            
            // Eject the old disk first
            df[nr]->ejectDisk();
            
            // Make sure there is enough time between ejecting and inserting.
            // Otherwise, the Amiga might not detect the change.
            delay = std::max((Cycle)SEC(1.5), delay);
        }
        
        diskToInsert = std::move(disk);
        agnus.scheduleRel<SLOT_DCH>(delay, DCH_INSERT, nr);
    }
}

void
DiskController::insertDisk(class DiskFile &file, isize nr, Cycle delay)
{
    insertDisk(std::make_unique<Disk>(file), nr, delay);
}

void
DiskController::insertDisk(const string &name, isize nr, Cycle delay)
{
    assert(nr >= 0 && nr <= 3);
    
    bool append = !util::isAbsolutePath(name) && searchPath[nr] != "";
    string path = append ? searchPath[nr] + "/" + name : name;
    
    std::unique_ptr<DiskFile> file(DiskFile::make(path));
    insertDisk(*file, nr, delay);
}

void
DiskController::insertNew(isize nr, Cycle delay)
{
    assert(nr >= 0 && nr <= 3);
    
    ADFFile adf;
    
    // Create a suitable ADF for the specified drive
    switch (df[nr]->config.type) {
            
        case DRIVE_DD_35: adf.init(INCH_35, DISK_DD); break;
        case DRIVE_HD_35: adf.init(INCH_35, DISK_HD); break;
        case DRIVE_DD_525: adf.init(INCH_525, DISK_SD); break;
    }
    
    // Add a file system
    adf.formatDisk(df[nr]->config.defaultFileSystem,
                   df[nr]->config.defaultBootBlock);
    
    // Insert the disk
    insertDisk(adf, nr, delay);
}

void
DiskController::setWriteProtection(isize nr, bool value)
{
    assert(nr >= 0 && nr <= 3);
    df[nr]->setWriteProtection(value);
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
    assert(fifoCount >= 1);
    
    // Remove and return the oldest byte
    fifoCount -= 1;
    return (fifo >> (8 * fifoCount)) & 0xFF;
}

u16
DiskController::readFifo16()
{
    assert(fifoCount >= 2);
    
    // Remove and return the oldest word
    fifoCount -= 2;
    return (fifo >> (8 * fifoCount)) & 0xFFFF;
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

bool
DiskController::compareFifo(u16 word) const
{
    if (fifoHasWord()) {
        
        for (isize i = 0; i < 8; i++) {
            if ((fifo >> i & 0xFFFF) == word) return true;
        }
    }
    return false;
}

void
DiskController::executeFifo()
{
    // Only proceed if a drive is selected
    Drive *drive = getSelectedDrive();
    
    switch (state) {
            
        case DRIVE_DMA_OFF:
        case DRIVE_DMA_WAIT:
        case DRIVE_DMA_READ:
            
            // Read a byte from the drive
            incoming = drive ? drive->readByteAndRotate() : 0;
            
            // Write byte into the FIFO buffer
            writeFifo((u8)incoming);
            incoming |= 0x8000;
            
            // Check if we've reached a SYNC mark
            if (compareFifo(dsksync) ||
                (config.autoDskSync && syncCounter++ > 20000)) {
                
                // Save time stamp
                syncCycle = agnus.clock;
                
                // Trigger a word SYNC interrupt
                trace(DSK_DEBUG, "SYNC IRQ (dsklen = %d)\n", dsklen);
                paula.raiseIrq(INT_DSKSYN);
                
                // Enable DMA if the controller was waiting for it
                if (state == DRIVE_DMA_WAIT)
                {
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
                if (drive) drive->writeByteAndRotate(outgoing);
            }
            break;
    }
}

void
DiskController::performDMA()
{
    Drive *drive = getSelectedDrive();
    
    // Only proceed if there are remaining bytes to process
    if ((dsklen & 0x3FFF) == 0) return;
    
    // Only proceed if DMA is enabled
    if (state != DRIVE_DMA_READ && state != DRIVE_DMA_WRITE) return;
    
    // How many words shall we read in?
    u32 count = drive ? config.speed : 1;
    
    // Perform DMA
    switch (state) {
            
        case DRIVE_DMA_READ:
            
            performDMARead(drive, count);
            break;
            
        case DRIVE_DMA_WRITE:
            
            performDMAWrite(drive, count);
            break;
            
        default:
            fatalError;
    }
}

void
DiskController::performDMARead(Drive *drive, u32 remaining)
{
    // Only proceed if the FIFO contains enough data
    if (!fifoHasWord()) return;
    
    do {
        
        // Read next word from the FIFO buffer
        u16 word = readFifo16();
        
        // Write word into memory
        if constexpr (DSK_CHECKSUM) {
            
            checkcnt++;
            check1 = util::fnv_1a_it32(check1, word);
            check2 = util::fnv_1a_it32(check2, agnus.dskpt & agnus.ptrMask);
        }
        agnus.doDiskDmaWrite(word);
        
        // Finish up if this was the last word to transfer
        if ((--dsklen & 0x3FFF) == 0) {
            
            paula.raiseIrq(INT_DSKBLK);
            setState(DRIVE_DMA_OFF);
            
            debug(DSK_CHECKSUM,
                  "read: cnt = %llu check1 = %x check2 = %x\n", checkcnt, check1, check2);
            
            return;
        }
        
        // If the loop repeats, fill the Fifo with new data
        if (--remaining) {
            
            executeFifo();
            executeFifo();
        }
        
    }
    while (remaining);
}

void
DiskController::performDMAWrite(Drive *drive, u32 remaining)
{
    // Only proceed if the FIFO has enough free space
    if (!fifoCanStoreWord()) return;
    
    do {

        // Read next word from memory
        if constexpr (DSK_CHECKSUM) {
            checkcnt++;
            check2 = util::fnv_1a_it32(check2, agnus.dskpt & agnus.ptrMask);
        }
        u16 word = agnus.doDiskDmaRead();
        
        if constexpr (DSK_CHECKSUM) {
            check1 = util::fnv_1a_it32(check1, word);
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
                
                u8 value = readFifo();
                if (drive) drive->writeByteAndRotate(value);
            }
            setState(DRIVE_DMA_OFF);
            
            debug(DSK_CHECKSUM, "write: cnt = %llu ", checkcnt);
            debug(DSK_CHECKSUM, "check1 = %x check2 = %x\n", check1, check2);

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

void
DiskController::performTurboDMA(Drive *drive)
{
    // Only proceed if there is anything to read or write
    if ((dsklen & 0x3FFF) == 0) return;
    
    // Perform action depending on DMA state
    switch (state) {
            
        case DRIVE_DMA_WAIT:
            
            drive->findSyncMark();
            [[fallthrough]];
            
        case DRIVE_DMA_READ:
            
            if (drive) performTurboRead(drive);
            if (drive) paula.raiseIrq(INT_DSKSYN);
            break;
            
        case DRIVE_DMA_WRITE:
            
            if (drive) performTurboWrite(drive);
            break;
            
        default:
            return;
    }
    
    // Trigger disk interrupt with some delay
    Cycle delay = MIMIC_UAE ? 2 * HPOS_CNT - agnus.pos.h + 30 : 512;
    paula.scheduleIrqRel(INT_DSKBLK, DMA_CYCLES(delay));
    
    setState(DRIVE_DMA_OFF);
}

void
DiskController::performTurboRead(Drive *drive)
{
    for (isize i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from disk
        u16 word = drive->readWordAndRotate();
        
        // Write word into memory
        if constexpr (DSK_CHECKSUM) {
            
            checkcnt++;
            check1 = util::fnv_1a_it32(check1, word);
            check2 = util::fnv_1a_it32(check2, agnus.dskpt & agnus.ptrMask);
        }
        mem.poke16 <ACCESSOR_AGNUS> (agnus.dskpt, word);
        agnus.dskpt += 2;
    }
    
    debug(DSK_CHECKSUM, "Turbo read %s: cyl: %zd side: %zd offset: %zd ",
          drive->getDescription(),
          drive->head.cylinder,
          drive->head.side,
          drive->head.offset);
    
    debug(DSK_CHECKSUM, "checkcnt = %llu check1 = %x check2 = %x\n",
          checkcnt, check1, check2);
}

void
DiskController::performTurboWrite(Drive *drive)
{
    for (isize i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from memory
        u16 word = mem.peek16 <ACCESSOR_AGNUS> (agnus.dskpt);
        
        if constexpr (DSK_CHECKSUM) {
            
            checkcnt++;
            check1 = util::fnv_1a_it32(check1, word);
            check2 = util::fnv_1a_it32(check2, agnus.dskpt & agnus.ptrMask);
        }
        
        agnus.dskpt += 2;
        
        // Write word to disk
        drive->writeWordAndRotate(word);
    }
    
    debug(DSK_CHECKSUM,
          "Turbo write %s: checkcnt = %llu check1 = %x check2 = %x\n",
          drive->getDescription(), checkcnt, check1, check2);
}
