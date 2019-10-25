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

/* The Blitter supports three accuracy levels:
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

class Blitter : public SubComponent {

    // The current configuration
    BlitterConfig config;

    // Information shown in the GUI inspector panel
    BlitterInfo info;

    // Statistics shown in the GUI monitor panel
    long blitCycles; 


    // The fill pattern lookup tables
    uint8_t fillPattern[2][2][256];     // [inclusive/exclusive][carry in][data]
    uint8_t nextCarryIn[2][256];        // [carry in][data]

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
    uint16_t bltsizeW;
    uint16_t bltsizeH;

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
    // Fast Blitter
    //

    // The Fast Blitter's blit functions
    void (Blitter::*blitfunc[32])(void);


    //
    // Slow Blitter
    //

    // The Slow Blitter's micro programs
    void (Blitter::*instruction[16][2][5])(void);

    // The micro program to execute (DEPRECATED)
    uint16_t microInstr[32];

    // The program counter indexing the microInstr array
    uint16_t bltpc;

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

    bool lockD;


    //
    // Flags
    //

    bool bbusy;
    bool bzero;


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
    // Constructing and destructiong
    //
    
public:
    
    Blitter(Amiga& ref);

    void initFastBlitter();
    void initSlowBlitter();

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & config.accuracy;
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
        & bltsizeW
        & bltsizeH

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
        & lockD

        & bbusy
        & bzero

        & remaining;
    }

    
    //
    // Configuring
    //

    // Returns the current configuration
    BlitterConfig getConfig() { return config; }

    // Configures the emulation accuracy level
    int getAccuracy() { return config.accuracy; }
    void setAccuracy(int level) { config.accuracy = level; }


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

public:

    // Returns the result of the most recent call to inspect()
    BlitterInfo getInfo();


    //
    // Accessing properties
    //
    
public:

    // Returns true if the Blitter is busy
    bool isBusy() { return bbusy; }

    // Returns the value of the zero flag
    bool isZero() { return bzero; }


    //
    // Accessing registers
    //
    
public:
    
    // OCS register 0x040 (w)
    void pokeBLTCON0(uint16_t value);
    
    inline uint16_t bltconASH() { return bltcon0 >> 12; }
    inline uint16_t bltconUSE() { return (bltcon0 >> 8) & 0xF; }
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
    // inline uint16_t bltsizeH() { return (bltsize >> 6) ? (bltsize >> 6) : 1024; }
    // inline uint16_t bltsizeW() { return (bltsize & 0x3F) ? (bltsize & 0x3F) : 64; }
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
    
    bool isFirstWord() { return xCounter == bltsizeW; }
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

    // Clears the busy flag and cancels the Blitter slot
    void kill();


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

    // Starts a fast line blit
    void beginFastLineBlit();

    // Starts a fast copy blit
    void beginFastCopyBlit();

    // Performs a copy blit operation via the FastBlitter
    template <bool useA, bool useB, bool useC, bool useD, bool desc>
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

    // Emulates a Blitter micro-instruction
    template <uint16_t instr> void exec();

    // Sets the x or y counter to a new value
    void setXCounter(uint16_t value);
    void setYCounter(uint16_t value);
    void resetXCounter() { setXCounter(bltsizeW); }
    void resetYCounter() { setYCounter(bltsizeH); }
    void decXCounter() { setXCounter(xCounter - 1); }
    void decYCounter() { setYCounter(yCounter - 1); }

    // Emulate the barrel shifter
    void doBarrelShifterA();
    void doBarrelShifterB();
};

#endif
