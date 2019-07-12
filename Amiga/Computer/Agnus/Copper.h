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

    // Quick-access references
    class Memory *mem;
    class Agnus *agnus;
    class Colorizer *colorizer;
    
    // Information shown in the GUI inspector panel
    CopperInfo info;

    // The currently executed Copper list (1 or 2)
    uint8_t copList = 1;

    /* Indicates if the next instruction should be skipped.
     * This flag is usually false. It is set to true by the SKIP instruction
     * if the skip condition holds.
     */
    bool skip = false;
     
    // The Copper list location pointers
    uint32_t cop1lc;
    uint32_t cop2lc;

    /* Address of the last executed instruction in each Copper list
     * These values are needed by the debugger to determine the end of the
     * Copper lists. Note that these values cannot be computed directly.
     * They are computed by observing the program counter in advancePC()
     */
    uint32_t cop1end;
    uint32_t cop2end;

    // The Copper Danger Bit (CDANG)
    bool cdang;
    
    // The Copper instruction registers
    uint16_t cop1ins = 0;
    uint16_t cop2ins = 0;

    // The Copper program counter
    uint32_t coppc = 0;

    // The Copper program counter at the time of the COP_FETCH event
    // uint32_t coppcBase = 0;
   
    // Storage for disassembled instruction
    char disassembly[128];

public:

    // Indicates if Copper is currently servicing an event (for debugging only)
    bool servicing = false;
    
    // Temporary debug flag
    bool verbose = false;


    //
    // Constructing and destructing
    //
    
public:
    
    Copper();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:

    void _initialize() override;
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
    CopperInfo getInfo();
    
    // Returns the program counter
    uint32_t getCopPC() { return coppc; }

    
    //
    // Accessing registers
    //
    
public:
    
    void pokeCOPCON(uint16_t value);
    void pokeCOPJMP1();
    void pokeCOPJMP2();
    void pokeCOPINS(uint16_t value);
    void pokeCOP1LCH(uint16_t value);
    void pokeCOP1LCL(uint16_t value);
    void pokeCOP2LCH(uint16_t value);
    void pokeCOP2LCL(uint16_t value);
    void pokeNOOP(uint16_t value);

    
    //
    // Running the device
    //
    
private:
 
    // Advances the program counter.
    void advancePC() { INC_OCS_PTR(coppc, 2); }

    // Switches the Copper list.
    void switchToCopperList(int nr);

    /* Searches for the next matching beam position.
     * This function is called when a WAIT statement is processed. It is uses
     * to compute where the Copper wakes up.
     *
     * Return values:
     *
     * true:  The Copper wakes up in the current frame.
     *        The wake-up position is returned in variable 'result'.
     * false: The Copper does not wake up the current frame.
     *        Variable 'result' remains untouched.
     */
    bool findMatch(Beam &result);

    // Called by findMatch() to determine the vertical trigger position
    bool findVerticalMatch(int16_t vStrt, int16_t vComp, int16_t vMask, int16_t &result);

    // Called by findMatch() to determine the horizontal trigger position
    bool findHorizontalMatch(int16_t hStrt, int16_t hComp, int16_t hMask, int16_t &result);

    // Emulates the Copper writing a value into one of the custom registers
    void move(int addr, uint16_t value);

    // Runs the comparator circuit.
    // DEPRECATED
    bool comparator(uint32_t beam, uint32_t waitpos, uint32_t mask);
    bool comparator(uint32_t waitpos);
    bool comparator(Beam waitpos);
    bool comparator();
    
    /* Computes the beam position where the Copper needs to wake up.
     * This functions is invoked when a WAIT command is processed.
     */
    // DEPRECATED
    uint32_t nextTriggerPosition(); 
    
    
    //
    // Analyzing Copper instructions
    //
    
private:
    
    /*             MOVE              WAIT              SKIP
     * Bit   cop1ins cop2ins   cop1ins cop2ins   cop1ins cop2ins
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

    // Schedules the next Copper event
    void schedule(EventID next);

    // Reschedules the current Copper event
    void reschedule();

private:
    
    // Executed after each frame
    void vsyncAction();
    
 
    //
    // Debugging
    //
    
public:

    // Returns the number of instructions in Copper list 1 or 2
    int instrCount(int nr);

    // Disassembles a single Copper command
    char *disassemble(uint32_t addr);
    char *disassemble(unsigned list, uint32_t offset);
};

#endif 
