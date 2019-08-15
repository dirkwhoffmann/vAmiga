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

    // Quick-access references
    class Memory *mem; 
    class Agnus *agnus;

    private:
    
    // Information shown in the GUI inspector panel
    BlitterInfo info;
    
    // The fill pattern lookup tables
    uint8_t fillPattern[2][2][256];     // [inclusive/exclusive][carry in][data]
    uint8_t nextCarryIn[2][256];        // [carry in][data]
    
    
    //
    // Configuration items
    //
    
    /* Blitter emulation accuracy.
     * The following accuracy levels are implemented at the moment:
     *
     * Level 0: Blits are performed immediately by the FastBlitter.
     *          The busy flag is cleared immediately.
     *          The Blitter IRQ is triggered immediately.
     *          The CPU is never blocked.
     *
     * Level 1: Blits are performed immediately by the FastBlitter.
     *          The busy flag is cleared with a delay.
     *          The Blitter IRQ is triggered with a delay.
     *          The CPU is never blocked.
     *
     * Level 2: Blits are performed immediately by the FastBlitter.
     *          The busy flag is cleared with a delay.
     *          The Blitter IRQ is triggered with a delay.
     *          The CPU is blocked while the Blitter is operating.
     */
     int accuracy = 0;
    
    
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
    
    // Debug counters
    int copycount = 0;
    int linecount = 0;

    
    //
    // Constructing and destructing
    //
    
    public:
    
    Blitter();
    

    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & accuracy;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & bltcon0
        & bltcon1

        & bltapt
        & bltbpt
        & bltcpt
        & bltdpt

        & bltafwm
        & bltalwm

        & bltsize

        & bltamod
        & bltbmod
        & bltcmod
        & bltdmod

        & anew
        & bnew
        & aold
        & bold
        & ahold
        & bhold
        & chold
        & dhold
        & ashift
        & bshift

        & xCounter
        & yCounter

        & bbusy
        & bzero;
    }

    
    //
    // Methods from HardwareComponent
    //
    
    private:

    void _initialize() override;
    void _powerOn() override;
    void _reset() override;
    void _inspect() override; 
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Reading the internal state
    //
    
    public:
    
    // Returns the latest internal state recorded by inspect()
    BlitterInfo getInfo();
    

    //
    // COnfiguring the device
    //

    int getAccuracy() { return accuracy; }
    void setAccuracy(int level) { accuracy = level; }
    
    
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
    // Handling requests of other components
    //

    // Called by Agnus when DMACON is written to
    void pokeDMACON(uint16_t oldValue, uint16_t newValue);



    //
    // Serving events
    //
    
    public:
    
    // Processes a Blitter event
    void serviceEvent(EventID id);

    //
    // Auxiliary functions
    //

    // Emulates the minterm logic circuit
    uint16_t doMintermLogic(uint16_t a, uint16_t b, uint16_t c, uint8_t minterm);
    uint16_t doMintermLogicQuick(uint16_t a, uint16_t b, uint16_t c, uint8_t minterm);

    // Emulates the fill logic circuit
    void doFill(uint16_t &data, bool &carry);

    // Returns the estimated number of DMA cycles of the current Blitter operation
    int estimatesCycles(uint16_t bltcon0, int width, int height);


    //
    // Slow Blitter (more accurate)  (NOT WORKING YET)
    //
    
    private:
    
    // Program the device
    void loadMicrocode();

    // Emulate the barrel shifter
    void doBarrelShifterA();
    void doBarrelShifterB();

    
    //
    // Fast Blitter (less accurate)
    //
    
    private:
    
    // Invokes the FastBlitter to perform a blit
    void startFastBlit();

    // Called at the end of a FastBlitter operation
    void endFastBlit();

    // Performs a copy blit operation via the FastBlitter
    void doFastCopyBlit();
    
    // Performs a line blit operation via the FastBlitter
    void doFastLineBlit();
};

#endif
