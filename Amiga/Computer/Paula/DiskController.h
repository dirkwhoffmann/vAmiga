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
    
    // References to the disk drives (for easy access)
    Drive *df[4] = { NULL, NULL, NULL, NULL };
    
    /* Connection status of all four drives.
     * Note: The internal drive (Df0) must not be disconnected. Hence,
     * connected[0] must never be set to false.
     */
    bool connected[4] = { true, false, false, false };
    
    // The current drive state (off, read, or write)
    DriveState state;
    
    
    //
    // Data buffers
    //
    
    // The latest incoming byte (value shows up in DSKBYTER)
    uint8_t incoming;
    
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
    
    // Disk write data (from RAM to disk)
    uint16_t dskdat;
    
    // Disk SYNC word
    uint16_t dsksync;
    
    // A copy of the PRB register of CIA B
    uint8_t prb;
    
    
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
    
    // Returns the latest internal state recorded by inspect()
    DiskControllerInfo getInfo();
    
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
    // Processing events and disk data
    //
    
public:
    
    // Serves an event in the disk controller slot.
    void serveDiskEvent();
    
    // Clears the FIFO buffer.
    void clearFifo();
    
    // Returns true if the FIFO buffer contains at least 2 bytes of data.
    bool fifoHasData() { return fifoCount >= 2; }
    
    // Writes a byte into the FIFO buffer.
    void writeFifo(uint8_t byte);
    
    // Reads a word from the FIFO buffer.
    uint16_t readFifo();
    
    // Performs a disk DMA cycle.
    void doDiskDMA();
    
    // Returns true if the specified drive is transferring data via DMA.
    bool doesDMA(int nr);
};

#endif
