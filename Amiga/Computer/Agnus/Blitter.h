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

    /* Blitter execution diagram (HRM, Table 6.2):
     *
     *           Active
     * BLTCON0  Channels            Cycle sequence
     *
     *    F     A B C D    A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 D2
     *    E     A B C      A0 B0 C0 A1 B1 C1 A2 B2 C2
     *    D     A B   D    A0 B0 -- A1 B1 D0 A2 B2 D1 -- D2
     *    C     A B        A0 B0 -- A1 B1 -- A2 B2
     *    B     A   C D    A0 C0 -- A1 C1 D0 A2 C2 D1 -- D2
     *    A     A   C      A0 C0 A1 C1 A2 C2
     *    9     A     D    A0 -- A1 D0 A2 D1 -- D2
     *    8     A          A0 -- A1 -- A2
     *    7       B C D    B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- D2
     *    6       B C      B0 C0 -- B1 C1 -- B2 C2
     *    5       B   D    B0 -- -- B1 D0 -- B2 D1 -- D2
     *    4       B        B0 -- -- B1 -- -- B2
     *    3         C D    C0 -- -- C1 D0 -- C2 D1 -- D2
     *    2         C      C0 -- C1 -- C2
     *    1           D    D0 -- D1 -- D2
     *    0                -- -- -- --
     *
     * From this table we derive:
     *
     *           Cyles: DMA cycles per word
     *                  [ Total / Bus blocking / Non blocking ]
     *
     *     Wait states: Average number of CPU wait states
     *                  [ DMA cycles / Master cycles ]
     *
     * BLTCON0    Cyles          Wait states
     *
     *    F     4 (4 / 0)   infinity / infinity
     *    E     3 (3 / 0)   infinity / infinity
     *    D     3 (3 / 0)   infinity / infinity
     *    C     3 (2 / 1)          1 / 8
     *    B     3 (3 / 0)   infinity / infinity
     *    A     2 (2 / 0)   infinity / infinity
     *    9     2 (2 / 0)   infinity / infinity
     *    8     2 (1 / 1)        0.5 / 4
     *    7     4 (3 / 1)        1.5 / 12
     *    6     3 (2 / 1)          1 / 8
     *    5     3 (2 / 1)          1 / 8
     *    4     3 (1 / 2)       2.66 / 21.33
     *    3     3 (2 / 1)          1 / 21.33
     *    2     2 (1 / 1)        0.5 / 4
     *    1     2 (1 / 1)        0.5 / 4
     *    0     2 (0 / 2)          0 / 0
     */
    const int blitCycles[16] = {
        2, 2, 2, 3,
        3, 3, 3, 4,
        2, 2, 2, 3,
        3, 3, 3, 4
    };
    const int waitStates[16] = {
        0, 4, 4, 21,
        21, 8, 8, 12,
        4, -1, -1, -1,
        8, -1, -1, -1
    };


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
     * Level 0: Moves data in a single chunk by using the FastBlitter.
     *          Consumes no bus cycles while operating.
     *          Keeps the CPU keeps running full speed.
     *
     * Level 1: Moves data in a single chunk by using the FastBlitter.
     *          Consumes bus cycles while operating.
     *          Keeps the CPU keeps running full speed.
     *
     * Level 2: Moves data in a single chunk by using the FastBlitter.
     *          Consumes bus cycles while operating.
     *          Slows down the CPU by inserting wait states.
     *
     * Level 3: Moves data word by word by using the SlowBlitter.
     *          Consumes bus cycles while operating.
     *          Slows down the CPU by inserting wait states.
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

    int     iteration;
    int     incr;
    int     ash;
    int     bsh;
    int32_t amod;
    int32_t bmod;
    int32_t cmod;
    int32_t dmod;

    uint16_t xCounter;
    uint16_t yCounter;

    bool fillCarry;
    uint16_t mask;


    //
    // Blitter flags
    //

    bool bbusy;
    bool bzero;

    //
    // Counters
    //

    // Total number of words to be processed in copy blit
    int bltwords;

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

        & fillCarry
        & mask

        & bbusy
        & bzero
        
        & bltwords;
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

    /* Estimates the duration of a blit operation
     * The returned value is the estimates number of bus cycles needed by the
     * Blitter to perform the specified blit. It is used by the FastBlitter to
     * terminate Blitter activity after the proper amount of time.
     */
    int estimatesCycles(uint16_t bltcon0, int width, int height);


    //
    // Fast Blitter (Accuracy levels 0 ... 2)
    //
    
private:
    
    // Invokes the FastBlitter to perform a blit
    void startFastBlitter();

    // Called at the end of a FastBlitter operation
    void endFastBlit();

    // Performs a copy blit operation via the FastBlitter
    void doFastCopyBlit();
    
    // Performs a line blit operation via the FastBlitter
    void doFastLineBlit();


    //
    // Slow Blitter (Accuracy level 3)
    //

private:

    // Performs a Blitter operation with the slow Blitter
    void startSlowBlitter();

    // Emulates the next Blitter micro-instruction
    void executeSlowBlitter();

    // Sets the x or y counter to a new value
    void setXCounter(uint16_t value);
    void setYCounter(uint16_t value);
    void resetXCounter() { setXCounter(bltsizeW()); }
    void resetYCounter() { setYCounter(bltsizeH()); }
    void decXCounter() { setXCounter(xCounter - 1); }
    void decYCounter() { setYCounter(yCounter - 1); }
    bool lastIteration() { return xCounter == 1 && yCounter == 1; }


    // Program the device
    void loadMicrocode();

    // Emulate the barrel shifter
    void doBarrelShifterA();
    void doBarrelShifterB();


};

#endif
