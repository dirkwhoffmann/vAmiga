// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskControllerTypes.h"
#include "SubComponent.h"
#include "Reflection.h"
#include "FloppyDisk.h"

namespace vamiga {

class DiskController : public SubComponent
{
    // Current configuration
    DiskControllerConfig config = {};

    // Result of the latest inspection
    mutable DiskControllerInfo info = {};
    
    // The currently selected drive (-1 if no drive is selected)
    isize selected = -1;

    // The current drive state (off, read, or write)
    DriveState state;

    // Timestamp of the latest DSKSYNC match
    Cycle syncCycle;

    /* Watchdog counter for SYNC marks. This counter is incremented after each
     * disk rotation and reset when a SYNC mark was found. It is used to
     * implement the auto DSKSYNC feature which forces the DSKSYNC interrupt to
     * trigger even if no SYNC mark is present.
     */
    isize syncCounter = 0;
    
    // Used to synchronize the schedulign of the DSK_ROTATE event
    double dskEventDelay = 0;
    
    
    //
    // Data buffers
    //
    
    // The latest incoming byte (value shows up in DSKBYTER)
    u16 incoming;

    // Data register
    u16 dataReg;

    // Number of bits stored in the data register
    u8 dataRegCount;

    /* The drive controller's FIFO buffer. On each DSK_ROTATE event, a byte is
     * read from the selected drive and put into this buffer. Each Disk DMA
     * operation will read two bytes from the buffer and stores them at the
     * desired location.
     */
    u64 fifo;
    
    // Number of bytes stored in the FIFO buffer
    u8 fifoCount;
    
    
    //
    // Registers
    //
    
    // Disk DMA block length
    u16 dsklen;
    
    // Disk SYNC word
    u16 dsksync;
    
    // A copy of the PRB register of CIA B
    u8 prb;
    
    
    //
    // Debugging
    //
    
    // For debugging, a FNV-32 checksum is computed for each DMA operation
    u32 check1;
    u32 check2;
    u64 checkcnt;


    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;

    
    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override { return "DiskController"; }
    void _dump(Category category, std::ostream& os) const override;
    
private:
    
    void _reset(bool hard) override;
    void _inspect() const override;
    
    template <class T>
    void serialize(T& worker)
    {
        worker

        << selected
        << state
        << syncCycle
        << syncCounter
        << dskEventDelay
        << incoming
        << dataReg
        << dataRegCount
        << fifo
        << fifoCount
        << dsklen
        << dsksync
        << prb;

        if (util::isResetter(worker)) return;

        worker

        << config.connected
        << config.speed
        << config.lockDskSync
        << config.autoDskSync;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Configuring
    //
    
public:
    
    const DiskControllerConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    bool turboMode() const { return config.speed == -1; }

    i64 getConfigItem(Option option) const;
    i64 getConfigItem(Option option, long id) const;
    
    void setConfigItem(Option option, i64 value);
    void setConfigItem(Option option, long id, i64 value);

    
    //
    // Analyzing
    //
    
public:
    
    DiskControllerInfo getInfo() const { return CoreComponent::getInfo(info); }


    //
    // Accessing
    //

public:
    
    // Returns the number of the currently selected drive
    isize getSelected() const { return selected; }

    // Returns the currently selected (nullptr if none is selected)
    class FloppyDrive *getSelectedDrive();

    // Indicates if the motor of the specified drive is switched on
    bool spinning(isize driveNr) const;

    // Indicates if the motor of at least one drive is switched on
    bool spinning() const;
    
    // Returns the current drive state
    DriveState getState() const { return state; }
    
private:
    
    // Changes the current drive state
    void setState(DriveState s);
    void setState(DriveState oldState, DriveState newState);

    
    //
    // Accessing registers
    //
    
public:
    
    // OCR register 0x008 (r)
    u16 peekDSKDATR() const;
    
    // OCR register 0x024 (w)
    void pokeDSKLEN(u16 value);
    void setDSKLEN(u16 oldValue, u16 newValue);
    
    // OCR register 0x026 (w)
    void pokeDSKDAT(u16 value);
    
    // OCR register 0x01A (r)
    u16 peekDSKBYTR();
    u16 computeDSKBYTR() const;
    
    // OCR register 0x07E (w)
    void pokeDSKSYNC(u16 value);
    
    // Read handler for the PRA register of CIA A
    u8 driveStatusFlags() const;
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(u8 oldValue, u8 newValue);
    
    
    //
    // Handling disks
    //

    // Write protects or unprotects a disk
    void setWriteProtection(isize nr, bool value);


    //
    // Serving events
    //
    
public:

    // Services an event in the disk controller slot
    void serviceDiskEvent();

    // Schedules the first or next event in the disk controller slot
    void scheduleFirstDiskEvent();
    void scheduleNextDiskEvent();

    
    //
    // Working with the FIFO buffer
    //
    
private:
    
    // Informs about the current FIFO fill state
    bool fifoIsEmpty() const { return fifoCount == 0; }
    bool fifoIsFull() const { return fifoCount == 6; }
    bool fifoHasWord() const { return fifoCount >= 2; }
    bool fifoCanStoreWord() const { return fifoCount <= 4; }

    // Clears the FIFO buffer
    void clearFifo();
    
    // Reads a byte or word from the FIFO buffer
    u8 readFifo();
    u16 readFifo16();

    // Writes a word into the FIFO buffer
    void writeFifo(u8 byte);

    /* Emulates a data transfert between the selected drive and the FIFO
     * buffer. This function is executed periodically in serviceDiskEvent().
     * The exact operation is dependent of the current DMA state.
     */
    void transferByte();

    // Called inside transferByte, depending on the current DMA state
    void readByte();
    void writeByte();
    void readBit(bool bit);


    //
    // Performing DMA
    //

public:

    /* The emulator supports two basic disk DMA modes:
     *
     *     1. Standard DMA mode    (more compatible, but slow)
     *     2. Turbo DMA mode       (fast, but less compatible)
     *
     * In standard DMA mode, performDMA() is invoked three times per raster
     * line, in each of the three DMA slots. Communication with the drive is
     * managed by a FIFO buffer. Data is never read directly from or written
     * to the drive. It is always exchanged via the FIFO.
     *
     * The FIFO buffer supports two emulation modes:
     *
     *     1. Asynchronous        (more compatible)
     *     2. Synchronous         (less compatibile)
     *
     * If the FIFO buffer is emulated asynchronously, the event scheduler
     * is utilized to execute a DSK_ROTATE event from time to time. Whenever
     * this event triggers, a byte is read from the disk drive and fed into
     * the buffer. If the FIFO buffer is emulated synchronously, the DSK_ROTATE
     * events have no effect. Instead, the FIFO buffer is filled at the same
     * time when the drive DMA slots are processed. Synchronous mode is
     * slightly faster, because the FIFO can never run out of data. It is filled
     * exactly at the time when data is needed.
     *
     * To speed up emulation, standard drives can be run with an acceleration
     * factor greater than 1. In this case, multiple words are transferred
     * in each disk drive DMA slot. The first word is taken from the Fifo as
     * usual. All other words are emulated on-the-fly, with the same mechanism
     * as used in synchronous Fifo mode.
     *
     * Turbo DMA is applied iff the drive is configured as a turbo drive.
     * In this mode, data is transferred immediately when the DSKLEN
     * register is written to. This mode is fast, but far from being accurate.
     * Neither does it uses the disk DMA slots, nor does it interact with
     * the FIFO buffer.
     */

    // Performs DMA in standard mode
    void performDMA();
    void performDMARead(FloppyDrive *drive, u32 count);
    void performDMAWrite(FloppyDrive *drive, u32 count);

    // Performs DMA in turbo mode
    void performTurboDMA(FloppyDrive *d);
    void performTurboRead(FloppyDrive *drive);
    void performTurboWrite(FloppyDrive *drive);
};

}
