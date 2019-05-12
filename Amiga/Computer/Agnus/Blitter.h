// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _BLITTER_INC
#define _BLITTER_INC

// Micro-instructions
class Blitter : public HardwareComponent {
    
    friend class Agnus;
    
    private:
    
    // Information shown in the GUI inspector panel
    BlitterInfo info;
    
    // The fill pattern lookup tables
    uint8_t fillData[2][2][256];       // [inclusive/exclusive][carry in][data]
    uint8_t nextCarry[2][256];         // [carry in][data]
    
    
    //
    // Configuration items
    //
    
    // Indicates if the Blitter should be emulated cycle-accurately.
    bool exact = false;
    
    
    //
    // Blitter registers
    //
    
    // The Blitter Control Register
    uint16_t bltcon0;
    uint16_t bltcon1;
    
    // The Blitter DMA pointers
    uint32_t bltapt;
    uint32_t bltbpt;
    uint32_t bltcpt;
    uint32_t bltdpt;
    
    // Blitter A first and last word masks
    uint16_t bltafwm;
    uint16_t bltalwm;
    
    // The Blitter size register
    uint16_t bltsize;
    
    // The Blitter modulo registers
    int16_t bltamod;
    int16_t bltbmod;
    int16_t bltcmod;
    int16_t bltdmod;
    
    // The Blitter pipeline registers
    uint16_t anew;
    uint16_t bnew;
    uint16_t aold;
    uint16_t bold;
    uint16_t ahold;
    uint16_t bhold;
    uint16_t chold;
    uint16_t dhold;
    uint32_t ashift;
    uint32_t bshift;
    
    // Counter registers
    uint16_t xCounter;
    uint16_t yCounter;
    
    // Blitter flags;
    bool bbusy;
    bool bzero;
    
    
    //
    // Micro execution unit
    //
    
    /* Although the current implementation is far from being timing-accurate,
     * I am eager to improve it over time. To keep the implementation flexible,
     * the blitter is emulated as a micro-programmable device. When a blit
     * starts, a micro-program is set up that will decide on the action that
     * are performed in each Blitter cycle. The real blitter in the Amiga does
     * not work this way and uses a standard pipeline-based design.
     */
    
    // The Blitter micro-instructions
    static const uint16_t LOOPBACK  = 0b0000000001111; // Loops back or flashes the pipeline
    static const uint16_t FETCH_A   = 0b0000000010000; // Loads register "A new"
    static const uint16_t FETCH_B   = 0b0000000100000; // Loads register "B new"
    static const uint16_t FETCH_C   = 0b0000001000000; // Loads register "C hold"
    static const uint16_t HOLD_A    = 0b0000010000000; // Loads register "A hold"
    static const uint16_t HOLD_B    = 0b0000100000000; // Loads register "B hold"
    static const uint16_t HOLD_D    = 0b0001000000000; // Loads register "D hold"
    static const uint16_t WRITE_D   = 0b0010000000000; // Writes back "D hold"
    static const uint16_t BLTDONE   = 0b0100000000000; // Marks the last instruction
    
    static const uint16_t BLTIDLE   = 0b000000000000; // Does nothing
    static const uint16_t LOOPBACK0 = 0b000000001000; // Signals the end of the main loop
    static const uint16_t LOOPBACK2 = 0b000000001010; // Executes the main loop again
    static const uint16_t LOOPBACK3 = 0b000000001011; // Executes the main loop again
    static const uint16_t LOOPBACK4 = 0b000000001100; // Executes the main loop again
    
    // The micro program to execute
    uint16_t microInstr[32];
    
    // The program counter indexing the microInstr array
    uint16_t bltpc = 0;
    
    // Debugging stuff (REMOVE ASAP)
    int copycount = 0;
    int linecount = 0;

    
    //
    // Constructing and destructing
    //
    
    public:
    
    Blitter();
    
    
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
    BlitterInfo getInfo();
    

    //
    // Accessing properties
    //

    bool getExactEmulation() { return exact; }
    void setExactEmulation(bool value) { exact = value; }
    
    
    //
    // Accessing registers
    //
    
    public:
    
    // OCS register 0x040 (w)
    void pokeBLTCON0(uint16_t value);
    
    inline uint16_t bltconASH() { return bltcon0 >> 12; }
    inline bool bltconUSEA()    { return bltcon0 & (1 << 11); }
    inline bool bltconUSEB()    { return bltcon0 & (1 << 10); }
    inline bool bltconUSEC()    { return bltcon0 & (1 << 9); }
    inline bool bltconUSED()    { return bltcon0 & (1 << 8); }
    void setBltconASH(uint16_t ash) { assert(ash <= 0xF); bltcon0 = (bltcon0 & 0x0FFF) | (ash << 12); }

    // OCS register 0x042 (w)
    void pokeBLTCON1(uint16_t value);

    inline uint16_t bltconBSH() { return bltcon1 >> 12; }
    inline bool bltconEFE()     { return bltcon1 & (1 << 4); }
    inline bool bltconIFE()     { return bltcon1 & (1 << 3); }
    inline bool bltconFE()      { return bltconEFE() || bltconIFE(); }
    inline bool bltconFCI()     { return bltcon1 & (1 << 2); }
    inline bool bltconDESC()    { return bltcon1 & (1 << 1); }
    inline bool bltconLINE()    { return bltcon1 & (1 << 0); }
    void setBltcon1BSH(uint16_t bsh) { assert(bsh <= 0xF); bltcon1 = (bltcon1 & 0x0FFF) | (bsh << 12); }

    // OCS registers 0x044 and 0x046 (w)
    void pokeBLTAFWM(uint16_t value);
    void pokeBLTALWM(uint16_t value);
    
    // OCS registers 0x048 and 0x056 (w)
    void pokeBLTAPTH(uint16_t value);
    void pokeBLTAPTL(uint16_t value);
    void pokeBLTBPTH(uint16_t value);
    void pokeBLTBPTL(uint16_t value);
    void pokeBLTCPTH(uint16_t value);
    void pokeBLTCPTL(uint16_t value);
    void pokeBLTDPTH(uint16_t value);
    void pokeBLTDPTL(uint16_t value);
    
    // OCS register 0x058 (w)
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // H9 H8 H7 H6 H5 H4 H3 H2 H1 H0 W5 W4 W3 W2 W1 W0
    inline uint16_t bltsizeH() { return (bltsize >> 6) ? (bltsize >> 6) : 1024; }
    inline uint16_t bltsizeW() { return (bltsize & 0x3F) ? (bltsize & 0x3F) : 64; }
    void pokeBLTSIZE(uint16_t value);
    
    // OCS registers 0x060 and 0x066 (w)
    void pokeBLTAMOD(uint16_t value);
    void pokeBLTBMOD(uint16_t value);
    void pokeBLTCMOD(uint16_t value);
    void pokeBLTDMOD(uint16_t value);
    
    // OCS registers 0x070 and 0x074 (w)
    void pokeBLTADAT(uint16_t value);
    void pokeBLTBDAT(uint16_t value);
    void pokeBLTCDAT(uint16_t value);
    
    bool isFirstWord() { return xCounter == bltsizeW(); }
    bool isLastWord() { return xCounter == 1; }
    
    
    //
    // Serving events
    //
    
    public:
    
    // Processes a Blitter event
    void serviceEvent(EventID id);
    
    
    //
    // Cycle-accurate Blitter
    //
    
    private:
    
    // Program the device
    void loadMicrocode();

    // Emulate the barrel shifter
    void doBarrelShifterA();
    void doBarrelShifterB();

    // Emulate the minterm logic circuit
    uint16_t doMintermLogic(uint16_t a, uint16_t b, uint16_t c, uint8_t minterm);
    uint16_t doMintermLogicQuick(uint16_t a, uint16_t b, uint16_t c, uint8_t minterm);

    // Emulate the fill logic circuit
    void doFill(uint16_t &data, bool &carry);

    
    //
    // Fast Blitter
    //
    
    public:
    
    /* Performs a blit operation via the fast Blitter
     * Calls either doFastCopyBlit() or doFastLineBlit()
     */
    void doFastBlit();
    
    private:
    
    // Performs a copy blit operation via the fast Blitter
    void doFastCopyBlit();
    
    // Performs a line blit operation via the fast Blitter
    void doFastLineBlit();
};

#endif
