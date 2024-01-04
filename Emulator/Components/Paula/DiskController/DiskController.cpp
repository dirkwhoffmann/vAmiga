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
#include "Amiga.h"
#include "Agnus.h"
#include "ADFFile.h"
#include "FloppyDrive.h"
#include "IOUtils.h"
#include "MsgQueue.h"
#include "Paula.h"
#include "Thread.h"
#include <algorithm>

namespace vamiga {

void
DiskController::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    prb = 0xFF;
    selected = -1;
    dsksync = 0x4489;
}

void
DiskController::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.defaults;

    std::vector <Option> options = {
        
        OPT_DRIVE_SPEED,
        OPT_AUTO_DSKSYNC,
        OPT_LOCK_DSKSYNC
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }
    
    std::vector <Option> moreOptions = {
        
        OPT_DRIVE_CONNECT
    };

    for (auto &option : moreOptions) {
        for (isize i = 0; i < 4; i++) {
            setConfigItem(option, i, defaults.get(option, i));
        }
    }
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
        {
            if (!isValidDriveSpeed((isize)value)) {
                throw VAError(ERROR_OPT_INVARG, "-1, 1, 2, 4, 8");
            }
            
            SUSPENDED
            config.speed = (i32)value;
            scheduleFirstDiskEvent();
            return;
        }

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
            msgQueue.put(MSG_DRIVE_CONNECT, DriveMsg { i16(id), i16(value), 0, 0 } );
            return;
            
        default:
            fatalError;
    }
}

void
DiskController::_inspect() const
{
    {   SYNCHRONIZED

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
DiskController::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
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

    if (category == Category::State) {
        
        os << tab("selected");
        os << dec(selected) << std::endl;
        os << tab("state");
        os << DriveStateEnum::key(state) << std::endl;
        os << tab("syncCycle");
        os << dec(syncCycle) << std::endl;
        os << tab("incoming");
        os << hex(incoming) << std::endl;
        os << tab("dataReg");
        os << hex(dataReg) << " (" << dec(dataRegCount) << ")" << std::endl;
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
    }
}

FloppyDrive *
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
DiskController::setWriteProtection(isize nr, bool value)
{
    assert(nr >= 0 && nr <= 3);
    df[nr]->setProtectionFlag(value);
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
    fifo = (fifo & 0x00FF'FFFF'FFFF'FFFF) << 8 | byte;
    fifoCount++;
}

void
DiskController::transferByte()
{
    switch (state) {
            
        case DRIVE_DMA_OFF:
        case DRIVE_DMA_WAIT:
        case DRIVE_DMA_READ:

            readByte();
            break;

        case DRIVE_DMA_WRITE:
        case DRIVE_DMA_FLUSH:

            writeByte();
            break;

        default:
            fatalError;

    }
}

void
DiskController::readByte()
{
    FloppyDrive *drive = getSelectedDrive();

    // Read a byte from the drive
    incoming = drive ? drive->readByteAndRotate() : 0;

    // Set the byte ready flag (shows up in DSKBYT)
    incoming |= 0x8000;

    // Process all bits
    for (isize i = 7; i >= 0; i--) readBit(GET_BIT(incoming, i));
}

void
DiskController::readBit(bool bit)
{
    dataReg = (u16)((u32)dataReg << 1 | bit);

    // Fill the FIFO if we've received an entire byte
    if (++dataRegCount == 8) {

        writeFifo((u8)dataReg);
        dataRegCount = 0;
    }

    // Check if we've reached a SYNC mark
    if (dataReg == dsksync || (config.autoDskSync && syncCounter++ > 8*20000)) {

        // Save time stamp
        syncCycle = agnus.clock;

        // Trigger a word SYNC interrupt
        trace(DSK_DEBUG, "SYNC IRQ (dsklen = %d)\n", dsklen);
        paula.raiseIrq(INT_DSKSYN);

        // Enable DMA if the controller was waiting for it
        if (state == DRIVE_DMA_WAIT) {

            dataRegCount = 0;
            clearFifo();
            setState(DRIVE_DMA_READ);
        }

        // Reset the watchdog counter
        syncCounter = 0;
    }
}

void
DiskController::writeByte()
{
    FloppyDrive *drive = getSelectedDrive();

    if (fifoIsEmpty()) {

        // Switch off DMA if the last byte has been flushed out
        if (state == DRIVE_DMA_FLUSH) setState(DRIVE_DMA_OFF);

    } else {

        // Read the outgoing byte from the FIFO buffer
        u8 outgoing = readFifo();

        // Write byte to disk
        if (drive) drive->writeByteAndRotate(outgoing);
    }
}

void
DiskController::performDMA()
{
    FloppyDrive *drive = getSelectedDrive();
    
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
DiskController::performDMARead(FloppyDrive *drive, u32 remaining)
{
    // Only proceed if the FIFO contains enough data
    if (!fifoHasWord()) return;
    
    do {
        
        // Read next word from the FIFO buffer
        u16 word = readFifo16();
        
        // Write word into memory
        if (DSK_CHECKSUM) {
            
            checkcnt++;
            check1 = util::fnvIt32(check1, word);
            check2 = util::fnvIt32(check2, agnus.dskpt & agnus.ptrMask);
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
            
            transferByte();
            transferByte();
        }
        
    }
    while (remaining);
}

void
DiskController::performDMAWrite(FloppyDrive *drive, u32 remaining)
{
    // Only proceed if the FIFO has enough free space
    if (!fifoCanStoreWord()) return;
    
    do {

        // Read next word from memory
        if (DSK_CHECKSUM) {
            checkcnt++;
            check2 = util::fnvIt32(check2, agnus.dskpt & agnus.ptrMask);
        }
        u16 word = agnus.doDiskDmaRead();
        
        if (DSK_CHECKSUM) {
            check1 = util::fnvIt32(check1, word);
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
            
            transferByte();
            transferByte();
            assert(fifoCanStoreWord());
        }
        
    } while (remaining);
}

void
DiskController::performTurboDMA(FloppyDrive *drive)
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
    Cycle delay = MIMIC_UAE ? 2 * HPOS_CNT_PAL - agnus.pos.h + 30 : 512;
    paula.scheduleIrqRel(INT_DSKBLK, DMA_CYCLES(delay));
    
    setState(DRIVE_DMA_OFF);
}

void
DiskController::performTurboRead(FloppyDrive *drive)
{
    for (isize i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from disk
        u16 word = drive->readWordAndRotate();
        
        // Write word into memory
        if (DSK_CHECKSUM) {
            
            checkcnt++;
            check1 = util::fnvIt32(check1, word);
            check2 = util::fnvIt32(check2, agnus.dskpt & agnus.ptrMask);
        }
        mem.poke16 <ACCESSOR_AGNUS> (agnus.dskpt, word);
        agnus.dskpt += 2;
    }
    
    debug(DSK_CHECKSUM, "Turbo read %s: cyl: %ld side: %ld offset: %ld ",
          drive->getDescription(),
          drive->head.cylinder,
          drive->head.head,
          drive->head.offset);
    
    debug(DSK_CHECKSUM, "checkcnt = %llu check1 = %x check2 = %x\n",
          checkcnt, check1, check2);
}

void
DiskController::performTurboWrite(FloppyDrive *drive)
{
    for (isize i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from memory
        u16 word = mem.peek16 <ACCESSOR_AGNUS> (agnus.dskpt);
        
        if (DSK_CHECKSUM) {
            
            checkcnt++;
            check1 = util::fnvIt32(check1, word);
            check2 = util::fnvIt32(check2, agnus.dskpt & agnus.ptrMask);
        }
        
        agnus.dskpt += 2;
        
        // Write word to disk
        drive->writeWordAndRotate(word);
    }
    
    debug(DSK_CHECKSUM,
          "Turbo write %s: checkcnt = %llu check1 = %x check2 = %x\n",
          drive->getDescription(), checkcnt, check1, check2);
}

}
