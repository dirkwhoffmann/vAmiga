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

class Copper : public HardwareComponent
{

    friend class Agnus;

    public:
    
    // Information shown in the GUI inspector panel
    CopperInfo info;
    
    private:
    
    /* Indicates if the next instruction should be skipped.
     * This flag is usually false. It is set to true by the SKIP instruction
     * if the skip condition holds.
     */
    bool skip = false;
     
    // The Copper DMA pointers
    uint32_t coplc[2];
    
    // The Copper Danger Bit (CDANG)
    bool cdang;
    
    // The Copper instruction registers
    uint16_t copins1 = 0;
    uint16_t copins2 = 0;

    // The Copper program counter
    uint32_t coppc = 0;
   
    // Storage for disassembled instruction
    char disassembly[128];
    
    
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
    void _inspect() override; 
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
 
    // Advances the program counter.
    inline void advancePC() { coppc = (coppc + 2) & 0x7FFFE; }

    // Runs the comparator circuit.
    bool comparator(uint32_t beam, uint32_t waitpos, uint32_t mask);
    bool comparator(uint32_t waitpos);
    bool comparator();
    
    /* Computes the beam position where the Copper needs to wake up.
     * This functions is invoked when a WAIT command is processed.
     */
    uint32_t nextTriggerPosition(); 
    
    
    //
    // Analyzing Copper instructions
    //
    
private:
    
    /*             MOVE              WAIT              SKIP
     * Bit   copins1 copins2   copins1 copins2   copins1 copins2
     *  15      x     DW15       VP7     BFD       VP7     BFD
     *  14      x     DW14       VP6     VM6       VP6     VM6
     *  13      x     DW13       VP5     VM5       VP5     VM5
     *  12      x     DW12       VP4     VM4       VP4     VM4
     *  11      x     DW11       VP3     VM3       VP3     VM3
     *  10      x     DW10       VP2     VM2       VP2     VM2
     *   9      x     DW9        VP1     VM1       VP1     VM1
     *   8     RA8    DW8        VP0     VM0       VP0     VM0
     *   7     RA7    DW7        HP8     HM8       HP8     HM8
     *   6     RA6    DW6        HP7     HM7       HP7     HM7
     *   5     RA5    DW5        HP6     HM6       HP6     HM6
     *   4     RA4    DW4        HP5     HM5       HP5     HM5
     *   3     RA3    DW3        HP4     HM4       HP4     HM4
     *   2     RA2    DW2        HP3     HM3       HP3     HM3
     *   1     RA1    DW1        HP2     HM2       HP2     HM2
     *   0      0     DW0         1       0         1       1
     */
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
    
    uint16_t getRA();
    uint16_t getRA(uint32_t addr);

    uint16_t getDW();
    uint16_t getDW(uint32_t addr);

    bool getBFD();
    bool getBFD(uint32_t addr);

    uint16_t getVPHP();
    uint16_t getVPHP(uint32_t addr);
    uint16_t getVP() { return HI_BYTE(getVPHP()); }
    uint16_t getVP(uint32_t addr) { return HI_BYTE(getVPHP(addr)); }
    uint16_t getHP() { return LO_BYTE(getVPHP()); }
    uint16_t getHP(uint32_t addr) { return LO_BYTE(getVPHP(addr)); }
    
    uint16_t getVMHM();
    uint16_t getVMHM(uint32_t addr);
    uint16_t getVM() { return HI_BYTE(getVMHM()); }
    uint16_t getVM(uint32_t addr) { return HI_BYTE(getVMHM(addr)); }
    uint16_t getHM() { return LO_BYTE(getVMHM()); }
    uint16_t getHM(uint32_t addr) { return LO_BYTE(getVMHM(addr)); }
    
public:
    
    // Returns true if the Copper has no access to this custom register.
    bool isIllegalAddress(uint32_t addr);
    
    // Returns true if the Copper instruction at addr is illegal.
    bool isIllegalInstr(uint32_t addr);
    
 
    //
    // Managing events
    //
    
public:
    
    // Processes a Copper event
    void serviceEvent(EventID id);

private:
    
    // Executed after each frame
    void vsyncAction();
    
 
    //
    // Debugging
    //
    
public:
    
    char *disassemble(uint32_t addr);
    char *disassemble(unsigned list, uint32_t offset);
};

#endif 
