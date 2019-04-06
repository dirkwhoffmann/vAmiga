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

class DiskController : public HardwareComponent {
    
    private:
    
    // Information shown in the GUI inspector panel
    DiskControllerInfo info;
    
    
    //
    // Registers
    //
    
    // Disk DMA block length
    uint16_t dsklen;
    
    // Disk write data (from RAM to disk)
    uint16_t dskdat;
    
    // The current drive DMA status (off, read, or write)
    DriveDMA dma;
    
    
    //
    // Constructing and destructing
    //
    
    public:
    
    DiskController();
    
    
    //
    // Methods from HardwareComponent
    //
    
    private:
    
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
    // Accessing registers
    //
    
    public:
    
    void pokeDSKLEN(uint16_t value);
    
    uint16_t peekDSKDATR() { debug("peekDSKDAT: %X\n", dskdat); return dskdat; }
    void pokeDSKDAT(uint16_t value) { dskdat = value; }
    
    uint16_t peekDSKBYTR();

    
    //
    // Performing DMA
    //
    
    public:
    
    // Performs a disk DMA cycle.
    void doDiskDMA();
    
    // Returns true if the specified drive is transferring data via DMA.
    bool doesDMA(int nr);
};

#endif

