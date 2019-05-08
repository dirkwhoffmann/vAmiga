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

#include "HardwareComponent.h"

class Drive;

class DiskController : public HardwareComponent {
    
private:
    
    // Information shown in the GUI inspector panel
    DiskControllerInfo info;
    
    // Quick-references to the disk drives
    Drive *df[4] = { NULL, NULL, NULL, NULL };
    
    /* Connection status of all four drives.
     * Note: connected[0] is always true, because the internal drive cannot be
     * disconnected.
     */
    bool connected[4] = { true, false, false, false };
    
    // The currently selected drive (-1 if no drive is selected)
    int8_t selected = -1;
    
    // The number of words transferred during a single DMA cycle.
    int32_t acceleration = 1;
    
    // The current drive state (off, read, or write)
    DriveState state;
    
    // Set to true if the currently read disk word matches the sync word.
    // NOT USED AT THE MOMENT
    bool syncFlag = false;
    
    
    //
    // Data buffers
    //
    
private:
    
    // The latest incoming byte (value shows up in DSKBYTER)
    uint8_t incoming;
    
    // Timestamp of the latest write to variable 'incoming'
    Cycle incomingCycle;
    
    /* The drive controller's FIFO buffer
     * On each DSK_ROTATE event, a byte is read from the selected drive and
     * put into this buffer. Each Disk DMA operation will read two bytes from
     * the buffer and store them at the desired location.
     */
    uint64_t fifo;
    
    // Number of bytes stored in the FIFO buffer
    uint8_t fifoCount;
    
    
    //
    // Registers
    //
    
    // Disk DMA block length
    uint16_t dsklen;
    
    // Disk SYNC word
    uint16_t dsksync;
    
    // A copy of the PRB register of CIA B
    uint8_t prb;
    
    //
    // Debugging
    //
    
    // For debugging, an FNV-32 checksum is computed for each DMA operation.
    uint32_t checksum = fnv_1a_init32();
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    DiskController();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _setAmiga() override;
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _inspect() override;
    void _dump() override;
    
    
    //
    // Accesing the internal state
    //
    
public:
    
    // Returns the latest internal state recorded by inspect().
    DiskControllerInfo getInfo();

    // Indicates if the motor of the specified drive is switched on.
    bool spinning(unsigned driveNr);

    // Indicates if the motor of at least one drive is switched on.
    bool spinning();
    
    // Returns the current drive state
    DriveState getState() { return state; }
    
private:
    
    void setState(DriveState state);
    
    
    //
    // Managing the connection and selection status
    //
    
public:
    
    // Returns true if the specified drive is connected to the Amiga
    bool isConnected(int df) { assert(df < 4); return connected[df]; }
    
    // Connects or disconnects a drive
    void setConnected(int df, bool value);
    
    // Convenience wrappers
    void connect(int df) { setConnected(df, true); }
    void disconnect(int df) { setConnected(df, false); }
    void toggleConnected(int df) { setConnected(df, !isConnected(df)); }
    
    // Returns the currently selected drive or NULL if no drive is selected.
    Drive *getSelectedDrive();

    
    //
    // Accessing registers
    //
    
public:
    
    // OCR register 0x008 (r)
    uint16_t peekDSKDATR();
    
    // OCR register 0x024 (w)
    void pokeDSKLEN(uint16_t value);
    
    // OCR register 0x026 (w)
    void pokeDSKDAT(uint16_t value);
    
    // OCR register 0x01A (r)
    uint16_t peekDSKBYTR();
    
    // OCR register 0x07E (w)
    void pokeDSKSYNC(uint16_t value);
    
    // Read handler for the PRA register of CIA A
    uint8_t driveStatusFlags();
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(uint8_t oldValue, uint8_t newValue);
    
    
    //
    // Processing events
    //
    
public:
    
    // Serves an event in the disk controller slot.
    void serveDiskEvent();
    
    
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
    uint8_t readFifo();
    void writeFifo(uint8_t byte);

    // Reads a word from the FIFO buffer. DEPRECATED
    uint16_t readFifo16();
    
    // Returns true if the next word to read matches the specified value
    bool compareFifo(uint16_t word);

    /* Emulates a data transfert between the selected drive and the FIFO buffer.
     * This function is executed periodically in serveDiskEvent().
     * The exact operation is dependent of the current DMA state. If DMA is
     * off, no action is taken. If a read mode is active, the FIFO is filled
     * with data from the drive. If a write mode is active, data from the FIFO
     * is written to the drive head.
     */
    void executeFifo();
    
    // Finished up writing by emptying the FIFO buffer.
    void flushFifo(Drive *drive);
     
    
    //
    // Performing DMA
    //

public:
    
    /* Emulates a disk DMA operaton.
     * This DMA emulation is the most accurate one. The bytes from or to the
     * disk are stored in a FIFO buffer and memory transfer is performed
     * in the dedicated drive DMA slots in each rasterline.
     */
    void performDMA();
    void performDMARead(Drive *drive);
    void performDMAWrite(Drive *drive);
 
    void readByte(); // DEPRECATED, replaced by executeFifo()

    /* Emulates a (simplified) disk DMA operaton.
     * The simplified DMA transfer does not emulate a FIFO buffer.
     */
    void performSimpleDMA();
    void performSimpleDMARead(Drive *drive);
    void performSimpleDMAWrite(Drive *drive);

    /* Emulates a turbo-drive DMA access.
     * If a turbo drive is attached, these function are used. They are called
     * immediately when DSKLEN is written to.
     */
    void performTurboDMA(Drive *d);
    void performTurboRead(Drive *drive);
    void performTurboWrite(Drive *drive);
};

#endif
