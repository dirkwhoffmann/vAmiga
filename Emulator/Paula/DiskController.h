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

    /* Indicates whether the FIFO buffer should be filled asynchronously
     * At the beginning of each disk operation, this variable is assigned
     * the value of the corresponding configuration option. Latching the value
     * enables the user to change the configuration in the middle of a disk
      operation without causing any damage.
     */
    bool asyncFifo;

    // Temorary storage for a disk waiting to be inserted
    class Disk *diskToInsert = NULL;

    // The currently selected drive (-1 if no drive is selected)
    i8 selected = -1;

    // The current drive state (off, read, or write)
    DriveState state;

    // Set to true if the currently read disk word matches the sync word
    bool syncFlag = false;
    
    /* Watchdog counter for SYNC marks
     * This counter is incremented after each disk rotation and reset when
     * a SYNC mark was found. It is used to implement the auto DSKSYNC feature
     * which forces the DSKSYNC interrupt to trigger even if no SYNC mark
     * is present.
     */
    i16 syncCounter = 0;
    
    
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
    DiskControllerInfo getInfo() { return HardwareComponent::getInfo(info); }

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & config.connected
        & config.asyncFifo
        & config.lockDskSync
        & config.autoDskSync;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & selected
        & state
        & asyncFifo
        & syncFlag
        & syncCounter
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
    
    void _finalize() override;
    void _reset(bool hard) override;
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
    bool isConnected(int df) { return config.connected[df]; }
    void setConnected(int df, bool value);
    void connect(int df) { setConnected(df, true); }
    void disconnect(int df) { setConnected(df, false); }
    
    // Sets the speed acceleration factor for all connected drives
    void setSpeed(i32 value);
    
    // Enables or disables asynchronous FIFO buffer emulation
    bool getAsyncFifo() { return config.asyncFifo; }
    void setAsyncFifo(bool value);

    // Getters and setters for copy-protection related settings
    bool getLockDskSync() { return config.lockDskSync; }
    void setLockDskSync(bool value);
    bool getAutoDskSync() { return config.autoDskSync; }
    void setAutoDskSync(bool value);

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
    
    // Returns the currently selected drive or NULL if no drive is selected.
    i8 getSelected() { return selected; }
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
    void serviceDiskChangeEvent();

    
    //
    // Working with the FIFO buffer
    //
    
private:
    
    // Informs about the current FIFO fill state
    bool fifoIsEmpty() { return fifoCount == 0; }
    bool fifoIsFull() { return fifoCount == 6; }
    bool fifoHasWord() { return fifoCount >= 2; }
    bool fifoCanStoreWord() { return fifoCount <= 4; }

    // Clears the FIFO buffer
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
  
    // 1. Standard DMA mode
    void performDMA();
    void performDMARead(Drive *drive, u32 count);
    void performDMAWrite(Drive *drive, u32 count);
 
    // 2. Simple DMA mode
    /*
    void performSimpleDMA();
    void performSimpleDMAWait(Drive *drive, u32 count);
    void performSimpleDMARead(Drive *drive, u32 count);
    void performSimpleDMAWrite(Drive *drive, u32 count);
    */
    
    // 3. Turbo DMA mode
    void performTurboDMA(Drive *d);
    void performTurboRead(Drive *drive);
    void performTurboWrite(Drive *drive);
};

#endif
