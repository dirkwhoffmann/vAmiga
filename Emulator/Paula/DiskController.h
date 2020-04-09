// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DISK_CONTROLLER_INC
#define _DISK_CONTROLLER_INC

#include "AmigaComponent.h"

class DiskController : public AmigaComponent {

    friend class Amiga;

    // Bookkeeping
    DiskControllerConfig config;
    DiskControllerInfo info;

    // Temorary storage for a disk waiting to be inserted
    class Disk *diskToInsert = NULL;

    // The currently selected drive (-1 if no drive is selected)
    i8 selected = -1;

    // The current drive state (off, read, or write)
    DriveState state;

    // Indicates if the current disk operation used FIFO buffering
    bool useFifo;

    // Set to true if the currently read disk word matches the sync word.
    bool syncFlag = false;
    
    
    //
    // Data buffers
    //
    
    // The latest incoming byte (value shows up in DSKBYTER)
    u8 incoming;
    
    // Timestamp of the latest write to variable 'incoming'
    Cycle incomingCycle;
    
    /* The drive controller's FIFO buffer
     * On each DSK_ROTATE event, a byte is read from the selected drive and
     * put into this buffer. Each Disk DMA operation will read two bytes from
     * the buffer and store them at the desired location.
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
    u32 checksum;
    u64 checkcnt;


    //
    // Constructing and serializing
    //
    
public:
    
    DiskController(Amiga& ref);

    DiskControllerConfig getConfig() { return config; }
    DiskControllerInfo getInfo();

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & config.connected
        & config.useFifo;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & selected
        & state
        & useFifo
        & syncFlag
        & incoming
        & incomingCycle
        & fifo
        & fifoCount
        & dsklen
        & dsksync
        & prb;
    }


    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _reset() override;
    void _ping() override;
    void _inspect() override;
    void _dumpConfig() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Getter and setter
    //

public:

    // Connects or disconnect an external drive
     void setConnected(int df, bool value);
     void connect(int df) { setConnected(df, true); }
     void disconnect(int df) { setConnected(df, false); }

     // Sets the speed acceleration factor for all connected drives
     void setSpeed(i32 value);

     // Enables or disables the emulation of a FIFO buffer
     void setUseFifo(bool value);

    // Indicates if the motor of the specified drive is switched on
    bool spinning(unsigned driveNr);

    // Indicates if the motor of at least one drive is switched on
    bool spinning();
    
    // Drive state
    DriveState getState() { return state; }
    
private:
    
    void setState(DriveState s);
    
    
    //
    // Getter and setter
    //
    
public:
    
    // Connection status
    bool getUseFifoLatched() { return useFifo; }
    
    // Returns the currently selected drive or NULL if no drive is selected.
    class Drive *getSelectedDrive();


    //
    // Handling disks
    //

    // Ejects a disk from the specified drive
    void ejectDisk(int nr, Cycle delay = 0);

    // Inserts a disk into the specified drive
    void insertDisk(class Disk *disk, int nr, Cycle delay = 0);
    void insertDisk(class ADFFile *file, int nr, Cycle delay = 0);

    // Write protects or unprotects a disk
    void setWriteProtection(int nr, bool value);


    //
    // Accessing registers
    //
    
public:
    
    // OCR register 0x008 (r)
    u16 peekDSKDATR();
    
    // OCR register 0x024 (w)
    void pokeDSKLEN(u16 value);
    
    // OCR register 0x026 (w)
    void pokeDSKDAT(u16 value);
    
    // OCR register 0x01A (r)
    u16 peekDSKBYTR();
    u16 computeDSKBYTR();
    
    // OCR register 0x07E (w)
    void pokeDSKSYNC(u16 value);
    
    // Read handler for the PRA register of CIA A
    u8 driveStatusFlags();
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(u8 oldValue, u8 newValue);
    
    
    //
    // Event handlers
    //
    
public:
    
    // Services an event in the disk controller slot
    void serviceDiskEvent();

    // Services an event in the disk change slot
    void serviceDiskChangeEvent(EventID id, int driveNr);

    // Performs periodic actions for each frame
    void vsyncHandler();


    //
    // Working with the FIFO buffer
    //
    
private:
    
    // Informs about the current FIFO fill state
    bool fifoIsEmpty() { return fifoCount == 0; }
    bool fifoIsFull() { return fifoCount == 6; }
    bool fifoHasWord() { return fifoCount >= 2; }
    bool fifoCanStoreWord() { return fifoCount <= 4; }

    // Clears the FIFO buffer.
    void clearFifo();
    
    // Reads or writes a byte from or to the FIFO
    u8 readFifo();
    void writeFifo(u8 byte);

    // Reads a word from the FIFO buffer. DEPRECATED
    u16 readFifo16();
    
    // Returns true if the next word to read matches the specified value
    bool compareFifo(u16 word);

    /* Emulates a data transfert between the selected drive and the FIFO buffer.
     * This function is executed periodically in serviceDiskEvent().
     * The exact operation is dependent of the current DMA state. If DMA is
     * off, no action is taken. If a read mode is active, the FIFO is filled
     * with data from the drive. If a write mode is active, data from the FIFO
     * is written to the drive head.
     */
    void executeFifo();
         
    
    //
    // Performing DMA
    //

public:

    /* The emulator supports three disk DMA modes at the moment:
     *
     *     1. Standard DMA mode    (most accurate)
     *     2. Simple DMA mode
     *     3. Turbo DMA mode       (least accurate)
     *
     * In standard DMA mode, performDMA() is invoked three times per raster
     * line, in each of the three DMA slots. Communication with the drive is
     * decoupled by a FIFO buffer. Data is never read directly from or written
     * to the drive. It is always exchanged via the FIFO. Data transfer
     * between the FIFO and the drive takes place in serviceDiskEvent(), which
     * is called periodically by the event handler.
     *
     * In simple DMA mode, performDMA() is called three times per raster
     * line, just like in standard mode. The FIFO phase is skipped. I.e., data
     * is read from or written to the drive immediately when a DMA transfer
     * takes place.
     *
     * Turbo DMA mode is applied when the drive is configured as a turbo drive.
     * With these drives, data is transferred immediately when the DSKLEN
     * register is written. This mode is the least compatible. Neither does it
     * uses the rasterline DMA slots, nor does it use a FIFO buffer.
     */
  
    // 1. Standard DMA mode
    void performDMA();
    void performDMARead(Drive *drive, u32 count);
    void performDMAWrite(Drive *drive, u32 count);
 
    // 2. Simple DMA mode
    void performSimpleDMA();
    void performSimpleDMAWait(Drive *drive, u32 count);
    void performSimpleDMARead(Drive *drive, u32 count);
    void performSimpleDMAWrite(Drive *drive, u32 count);

    // 3. Turbo DMA mode
    void performTurboDMA(Drive *d);
    void performTurboRead(Drive *drive);
    void performTurboWrite(Drive *drive);
};

#endif
