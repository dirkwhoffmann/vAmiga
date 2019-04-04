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
    // Accessing registers
    //
    
    public:
    
    // BLTCON0
    inline uint16_t bltASH() { return bltcon0 >> 12; }
    inline bool bltUSEA() { return bltcon0 & (1 << 11); }
    inline bool bltUSEB() { return bltcon0 & (1 << 10); }
    inline bool bltUSEC() { return bltcon0 & (1 << 9); }
    inline bool bltUSED() { return bltcon0 & (1 << 8); }
    void pokeBLTCON0(uint16_t value);
    
    // BLTCON1
    inline uint16_t bltBSH() { return bltcon1 >> 12; }
    inline bool bltEFE() { return bltcon1 & (1 << 4); }
    inline bool bltIFE() { return bltcon1 & (1 << 3); }
    inline bool bltFE() { return bltEFE() || bltIFE(); }
    inline bool bltFCI() { return bltcon1 & (1 << 2); }
    inline bool bltDESC() { return bltcon1 & (1 << 1); }
    inline bool bltLINE() { return bltcon1 & (1 << 0); }
    void pokeBLTCON1(uint16_t value);
    
    // BLTAFWM, BLTALWM
    void pokeBLTAFWM(uint16_t value);
    void pokeBLTALWM(uint16_t value);
    
    // BLTxPTH, BLTxPTL
    void pokeBLTAPTH(uint16_t value);
    void pokeBLTAPTL(uint16_t value);
    void pokeBLTBPTH(uint16_t value);
    void pokeBLTBPTL(uint16_t value);
    void pokeBLTCPTH(uint16_t value);
    void pokeBLTCPTL(uint16_t value);
    void pokeBLTDPTH(uint16_t value);
    void pokeBLTDPTL(uint16_t value);
    
    // BLTSIZE
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // H9 H8 H7 H6 H5 H4 H3 H2 H1 H0 W5 W4 W3 W2 W1 W0
    inline uint16_t bltsizeH() { return (bltsize >> 6) ? (bltsize >> 6) : 1024; }
    inline uint16_t bltsizeW() { return (bltsize & 0x3F) ? (bltsize & 0x3F) : 64; }
    void pokeBLTSIZE(uint16_t value);
    
    // BLTxMOD
    void pokeBLTAMOD(uint16_t value) { bltamod = value; }
    void pokeBLTBMOD(uint16_t value) { bltbmod = value; }
    void pokeBLTCMOD(uint16_t value) { bltcmod = value; }
    void pokeBLTDMOD(uint16_t value) { bltdmod = value; }
    
    // BLTxDAT
    void pokeBLTADAT(uint16_t value);
    void pokeBLTBDAT(uint16_t value);
    void pokeBLTCDAT(uint16_t value);
    
    bool isFirstWord() { return xCounter == bltsizeW(); }
    bool isLastWord() { return xCounter == 1; }
    
    
    //
    // Managing events
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
    
    
    //
    // Fast Blitter
    //
    
    public:
    
    /* Performs a blit operation via the fast Blitter
     * Calls either doCopyBlit() or doCopyBlit()
     */
    void doFastBlit();
    
    private:
    
    // Performs a copy blit operation via the fast Blitter
    void doCopyBlit();
    
    // Performs a line blit operation via the fast Blitter
    void doLineBlit();
};

#endif
