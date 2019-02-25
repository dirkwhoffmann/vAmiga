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

class Copper : public HardwareComponent {
   
    friend class DMAController;
    
    // Current state of the Copper
    int32_t state;
    
    // Copper DMA pointers
    uint32_t coplc[2];
    
    // The Copper Danger Bit (CDANG)
    bool cdang;
    
    // The Copper instruction registers
    uint16_t copins1 = 0;
    uint16_t copins2 = 0;

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
 
    // Advances the program counter
    inline void advancePC() { coppc = (coppc + 2) & 0x7FFFE; }

   
    //
    // Analyzing Copper instructions
    //
    
    /* Each functions comes in two variante. The first variant analyzes the
     * instruction in the instructions register. The second variant analyzes
     * the instruction at a certain location in memory.
     */
 
    bool isMoveCmd();
    bool isMoveCmd(uint32_t addr);
    
    bool isWaitCmd();
    bool isWaitCmd(uint32_t addr);

    bool isSkipCmd();
    bool isSkipCmd(uint32_t addr);
    

 


    
    //
    // Managing events
    //
    
    // Schedules a new Copper event
    void scheduleEventRel(Cycle delta, int32_t type, int64_t data = 0);

    // Cancels a scheduled Copper event
    void cancelEvent();
    
public:
    
    // Processes a Copper event
    void processEvent(int32_t type, int64_t data);

private:
    
    // Executed after each frame
    void vsyncAction();
    
    // Returns true if the Copper has no access to this custom register
    bool illegalAddress(uint32_t address);
};

#endif 
