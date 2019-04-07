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
    
    // The current drive DMA status (off, read, or write)
    DriveDMA dma;
    
    
    //
    // FIFO buffer
    //
    
    // Number of bytes stored in the FIFO buffer
    uint8_t fifoCount; 
    
    /* Data bytes stored in the FIFO buffer
     * On each DSK_ROTATE event, a byte is read from the selected drive and
     * put into this buffer. Each Disk DMA operation will read two bytes from
     * the buffer and store them at the desired location.
     */
    uint64_t fifo;
 
    
    //
    // Registers
    //
    
    // Disk DMA block length
    uint16_t dsklen;
    
    // Disk write data (from RAM to disk)
    uint16_t dskdat;
    
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
    // Reading the internal state
    //
    
    public:
    
    // Returns the latest internal state recorded by inspect()
    DiskControllerInfo getInfo();
    
    
    //
    // Managing the connection and selection status
    //
    
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
    
    void pokeDSKLEN(uint16_t value);
    
    uint16_t peekDSKDATR() { debug("peekDSKDAT: %X\n", dskdat); return dskdat; }
    void pokeDSKDAT(uint16_t value) { dskdat = value; }
    
    uint16_t peekDSKBYTR();
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(uint8_t oldValue, uint8_t newValue);
    
    
    //
    // Processing events and disk data
    //
    
    public:
    
    // Serves an event in the disk controller slot.
    void serveDiskEvent();
    
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

