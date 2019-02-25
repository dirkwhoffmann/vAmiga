// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _COPPER_INC
#define _COPPER_INC

typedef enum {
    
    COPPER_RESET = 0,
    COPPER_READ_FIRST = 1,
    COPPER_READ_SECOND = 2,
    COPPER_TRANSFER = 3
    
} CopperState;

class Copper : public HardwareComponent {
   
    friend class DMAController;
    
    // Current state of the Copper
    CopperState state; 
    
    // Copper DMA pointers
    uint32_t coplc[2];
    
    // The Copper Danger Bit (CDANG)
    bool cdang;
    
    // The Copper instruction register
    uint16_t copins = 0;
    
    // The Copper program counter
    uint32_t coppc = 0;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Copper();
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    
    
    //
    // Collecting information
    //
    
public:
    
    // Collects the data shown in the GUI's debug panel.
    CopperInfo getInfo();
    
    
    //
    // Accessing registers
    //
    
public:
    
    void pokeCOPCON(uint16_t value);
    void pokeCOPJMP(int x);
    void pokeCOPINS(uint16_t value);
    
    void pokeCOPxLCH(int x, uint16_t value);
    void pokeCOPxLCL(int x, uint16_t value);
    
    //
    // Running the device
    //
    
private:
    
    // Convenience wrappers for scheduling Copper events
    void scheduleEventRel(Cycle delta, int32_t type, int64_t data = 0);
    void cancelEvent();
    
    // Executed after each frame
    void vsyncAction();
    
    // Returns true if the Copper has no access to this custom register
    bool illegalAddress(uint32_t address);
};

#endif 
