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

class Blitter : public HardwareComponent {
    
    friend class Agnus;

    //
    // Constants
    //

    /* Micro-instructions
     *
     * To keep the implementation flexible, the blitter is emulated as a
     * micro-programmable device. When a blit starts, a micro-program is set up
     * that will decide on the action that are performed in each Blitter cycle.
     *
     * A micro-program consists of the following micro-instructions:
     *
     *     BLTIDLE : Does nothing.
     *         BUS : Acquires the bus.
     *     WRITE_D : Writes back D hold.
     *     FETCH_A : Loads register A new.
     *     FETCH_B : Loads register B new.
     *     FETCH_C : Loads register C hold.
     *      HOLD_A : Loads register A hold.
     *      HOLD_B : Loads register B hold.
     *      HOLD_D : Loads register D hold.
     *     BLTDONE : Marks the last instruction.
     *      REPEAT : Continues with the next word.
     */

    static const uint16_t BLTIDLE   = 0b0000'0000'0000;
    static const uint16_t BUS       = 0b0000'0000'0001;
    static const uint16_t WRITE_D   = 0b0000'0000'0010;
    static const uint16_t FETCH_A   = 0b0000'0000'0100;
    static const uint16_t FETCH_B   = 0b0000'0000'1000;
    static const uint16_t FETCH_C   = 0b0000'0001'0000;
    static const uint16_t HOLD_A    = 0b0000'0010'0000;
    static const uint16_t HOLD_B    = 0b0000'0100'0000;
    static const uint16_t HOLD_D    = 0b0000'1000'0000;
    static const uint16_t BLTDONE   = 0b0001'0000'0000;
    static const uint16_t REPEAT    = 0b0010'0000'0000;


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
     *
     * The following accuracy levels are implemented:
     *
     * Level 0: Moves data in a single chunk.
     *          Terminates immediately without using up any bus cycles.
     *
     * Level 1: Moves data in a single chunk.
     *          Uses up bus cycles like the real Blitter does.
     *
     * Level 2: Moves data word by word like the real Blitter does.
     *          Uses up bus cycles like the real Blitter does.
     *
     * Level 0 and 1 invoke the FastBlitter. Level 2 invokes the SlowBlitter.
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

    //
    // Slow Blitter
    //

    // The micro program to execute
    uint16_t microInstr[32];

    // The program counter indexing the microInstr array
    uint16_t bltpc = 0;

    int iteration;
    int incr;
    int ash;
    int bsh;
    int32_t amod;
    int32_t bmod;
    int32_t cmod;
    int32_t dmod;

    // Counters tracking the coordinate of the blit window
    uint16_t xCounter;
    uint16_t yCounter;

    // Counters tracking the DMA accesses for each channel
    int16_t cntA;
    int16_t cntB;
    int16_t cntC;
    int16_t cntD;

    bool fillCarry;
    uint16_t mask;


    //
    // Flags
    //

    bool bbusy;
    bool bzero;


    /*
     * Priority logic (CPU versus Blitter)
     *
     * To block the Blitter, three conditions must hold:
     *
     *     - The BLTPRI flag is false
     *     - The CPU must request the bus
     *     - The CPU request must have been denied for three consecutive cycles
     */

public:

    bool cpuRequestsBus;
    int cpuDenials;


    //
    // Counters
    //

private:

    // Counter for tracking the remaining words to process
    int remaining;

    // Debug counters
    int copycount = 0;
    int linecount = 0;

    // Debug checksums
    uint32_t check1;
    uint32_t check2;


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

        & microInstr
        & bltpc

        & iteration
        & incr
        & ash
        & bsh
        & amod
        & bmod
        & cmod
        & dmod

        & xCounter
        & yCounter
        & cntA
        & cntB
        & cntC
        & cntD

        & fillCarry
        & mask

        & bbusy
        & bzero

        & cpuRequestsBus
        & cpuDenials
        
        & remaining;
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
    void setAccuracy(int level) { accuracy = level; debug("accuracy = %d\n", accuracy); }
    
    
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

    // Clears the busy flag, triggeres an IRQ, and cancels the Blitter slot
    void terminate();


    //
    // Executing the Blitter (Entry points for all accuracy levels)
    //

private:

    // Initiates a blit
    void startBlit();

    //  Concluded a blit
    void endBlit();


    //
    //  Executing the Fast Blitter (Called for lower accuracy levels)
    //

    // Invokes the FastBlitter to perform a blit
    // void startFastBlitter();

    // Starts a fast line blit
    void beginFastLineBlit();

    // Starts a fast copy blit
    void beginFastCopyBlit();

    // Fake-emulates the next micro-instruction of the copy Blitter
    void executeFastBlitter();

    // Performs a copy blit operation via the FastBlitter
    void doFastCopyBlit();
    
    // Performs a line blit operation via the FastBlitter
    void doFastLineBlit();


    //
    //  Executing the Slow Blitter (Called for higher accuracy levels)
    //

    // Starts a slow line blit
    void beginSlowLineBlit();

    // Starts a slow copy blit
    void beginSlowCopyBlit();

    // Emulates the next Blitter micro-instruction
    void executeSlowBlitter();

    // Tries to acquire the bus for the Blitter
    bool blitterCanHaveBus();

    // Sets the x or y counter to a new value
    // DEPRECATED
    void setXCounter(uint16_t value);
    void setYCounter(uint16_t value);
    void resetXCounter() { setXCounter(bltsizeW()); }
    void resetYCounter() { setYCounter(bltsizeH()); }
    void decXCounter() { setXCounter(xCounter - 1); }
    void decYCounter() { setYCounter(yCounter - 1); }

    // Program the device
    void loadMicrocode();

    // Emulate the barrel shifter
    void doBarrelShifterA();
    void doBarrelShifterB();
};

#endif
