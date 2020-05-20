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

class Blitter : public AmigaComponent
{
    friend class Agnus;

    // Current configuration
    BlitterConfig config;

    // Result of the latest inspection
    BlitterInfo info;

    // The fill pattern lookup tables
    u8 fillPattern[2][2][256];     // [inclusive/exclusive][carry in][data]
    u8 nextCarryIn[2][256];        // [carry in][data]


    //
    // Blitter registers
    //
    
    // The Blitter Control Register
    u16 bltcon0;
    u16 bltcon1;
    
    // The Blitter DMA pointers
    u32 bltapt;
    u32 bltbpt;
    u32 bltcpt;
    u32 bltdpt;
    
    // Blitter A first and last word masks
    u16 bltafwm;
    u16 bltalwm;
    
    // The Blitter size register
    u16 bltsizeH;
    u16 bltsizeV;

    // The Blitter modulo registers
    i16 bltamod;
    i16 bltbmod;
    i16 bltcmod;
    i16 bltdmod;
    
    // The Blitter pipeline registers
    u16 anew;
    u16 bnew;
    u16 aold;
    u16 bold;
    u16 ahold;
    u16 bhold;
    u16 chold;
    u16 dhold;
    u32 ashift;
    u32 bshift;

    //
    // Fast Blitter
    //

    // The Fast Blitter's blit functions
    void (Blitter::*blitfunc[32])(void);


    //
    // Slow Blitter
    //

    // Micro-programs for copy blits
    void (Blitter::*copyBlitInstr[16][2][2][6])(void);

    // Micro-program for line blits
    void (Blitter::*lineBlitInstr[6])(void);

    // The program counter indexing the micro instruction to execute
    u16 bltpc;

    int iteration;
    int incr;
    int ash;
    int bsh;
    i32 amod;
    i32 bmod;
    i32 cmod;
    i32 dmod;

    // Counters tracking the coordinate of the blit window
    u16 xCounter;
    u16 yCounter;

    // Counters tracking the DMA accesses for each channel
    i16 cntA;
    i16 cntB;
    i16 cntC;
    i16 cntD;

    bool fillCarry;
    u16 mask;

    bool lockD;


    //
    // Flags
    //

    /* Indicates if the Blitter is currently running.
     * The flag is set to true when a Blitter operation starts and set to false
     * when the operation ends.
     */
    bool running;

    /* The Blitter busy flag
     * This flag shows up in DMACON and has a similar meaning as variable
     * 'running'. The only difference is that the busy flag is cleared a few
     * cycles before the Blitter actually terminates.
     */
    bool bbusy;

    // The Blitter zero flag
    bool bzero;

    // Indicates if the Blitter interrupts has been triggered
    bool birq;
    
    
    //
    // Counters
    //

private:

    // Counter for tracking the remaining words to process
    int remaining;

    // Debug counters
    int copycount;
    int linecount;

    // Debug checksums
    u32 check1;
    u32 check2;


    //
    // Constructing and serializing
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

        & bltsizeH
        & bltsizeV

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

        & running
        & bbusy
        & bzero
        & birq

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
    void _reset(bool hard) override;
    void _inspect() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

public:

    // Returns the result of the most recent call to inspect()
    BlitterInfo getInfo() { return HardwareComponent::getInfo(info); }


    //
    // Accessing properties
    //
    
public:

    // Returns true if the Blitter is processing a blit
    bool isRunning() { return running; }

    // Returns the value of the Blitter Busy Flag
    bool isBusy() { return bbusy; }

    // Returns the value of the zero flag
    bool isZero() { return bzero; }


    //
    // Accessing registers
    //
    
public:
    
    // OCS register 0x040 (w)
    void pokeBLTCON0(u16 value);
    void setBLTCON0(u16 value);

    u16 bltconASH()   { return bltcon0 >> 12; }
    u16 bltconLF()    { return bltcon0 & 0xF; }
    u16 bltconUSE()   { return (bltcon0 >> 8) & 0xF; }
    bool bltconUSEA() { return bltcon0 & (1 << 11); }
    bool bltconUSEB() { return bltcon0 & (1 << 10); }
    bool bltconUSEC() { return bltcon0 & (1 << 9); }
    bool bltconUSED() { return bltcon0 & (1 << 8); }
    void setBltconASH(u16 ash) { assert(ash <= 0xF); bltcon0 = (bltcon0 & 0x0FFF) | (ash << 12); }

    // OCS register 0x042 (w)
    void pokeBLTCON1(u16 value);
    void setBLTCON1(u16 value);

    u16 bltconBSH()   { return bltcon1 >> 12; }
    bool bltconEFE()  { return bltcon1 & (1 << 4); }
    bool bltconIFE()  { return bltcon1 & (1 << 3); }
    bool bltconFE()   { return bltconEFE() || bltconIFE(); }
    bool bltconFCI()  { return bltcon1 & (1 << 2); }
    bool bltconDESC() { return bltcon1 & (1 << 1); }
    bool bltconLINE() { return bltcon1 & (1 << 0); }
    void setBltcon1BSH(u16 bsh) { assert(bsh <= 0xF); bltcon1 = (bltcon1 & 0x0FFF) | (bsh << 12); }

    // OCS registers 0x044 and 0x046 (w)
    void pokeBLTAFWM(u16 value);
    void pokeBLTALWM(u16 value);
    
    // OCS registers 0x048 and 0x056 (w)
    void pokeBLTAPTH(u16 value);
    void pokeBLTAPTL(u16 value);
    void pokeBLTBPTH(u16 value);
    void pokeBLTBPTL(u16 value);
    void pokeBLTCPTH(u16 value);
    void pokeBLTCPTL(u16 value);
    void pokeBLTDPTH(u16 value);
    void pokeBLTDPTL(u16 value);
    
    // OCS register 0x058 (w)
    template <PokeSource s> void pokeBLTSIZE(u16 value);
    void setBLTSIZE(u16 value);

    // ECS register 0x05A (w)
    void pokeBLTCON0L(u16 value);
    void setBLTCON0L(u16 value);
    
    // ECS register 0x05C (w)
    void pokeBLTSIZV(u16 value);
    void setBLTSIZV(u16 value);

    // ECS register 0x05E (w)
    void pokeBLTSIZH(u16 value);

    // OCS registers 0x060 and 0x066 (w)
    void pokeBLTAMOD(u16 value);
    void pokeBLTBMOD(u16 value);
    void pokeBLTCMOD(u16 value);
    void pokeBLTDMOD(u16 value);
    
    // OCS registers 0x070 and 0x074 (w)
    void pokeBLTADAT(u16 value);
    void pokeBLTBDAT(u16 value);
    void pokeBLTCDAT(u16 value);
    
    bool isFirstWord() { return xCounter == bltsizeH; }
    bool isLastWord() { return xCounter == 1; }

    
    //
    // Handling requests of other components
    //

    // Called by Agnus when DMACON is written to
    void pokeDMACON(u16 oldValue, u16 newValue);


    //
    // Serving events
    //
    
public:
    
    // Processes a Blitter event
    void serviceEvent(EventID id);

    // Performs periodic per-frame actions
    void vsyncHandler();


    //
    // Auxiliary functions
    //

    // Emulates the minterm logic circuit
    u16 doMintermLogic(u16 a, u16 b, u16 c, u8 minterm);
    u16 doMintermLogicQuick(u16 a, u16 b, u16 c, u8 minterm);

    // Emulates the fill logic circuit
    void doFill(u16 &data, bool &carry);

    // Clears the busy flag and cancels the Blitter slot
    // void kill();


    //
    // Executing the Blitter (Entry points for all accuracy levels)
    //

private:

    // Prepares for a new Blitter operation
    void prepareBlit();

    // Starts a Blitter operation
    void startBlit();

    // Clears the BBUSY flag and triggers the Blitter interrupt
    void signalEnd();

    // Concludes the current Blitter operation
    void endBlit();


    //
    //  Executing the Blitter
    //

    void beginLineBlit(int level);
    void beginCopyBlit(int level);


    //
    //  Executing the Fast Blitter (Called for lower accuracy levels)
    //

    // Starts a level 0 blit
    void beginFastLineBlit();
    void beginFastCopyBlit();

    // Performs a copy blit operation via the FastBlitter
    template <bool useA, bool useB, bool useC, bool useD, bool desc>
    void doFastCopyBlit();
    
    // Performs a line blit operation via the FastBlitter
    void doFastLineBlit();


    //
    //  Executing the Slow Blitter (Called for higher accuracy levels)
    //

    // Starts a level 1 or level 2 blit
    void beginFakeLineBlit();
    void beginSlowLineBlit();
    void beginFakeCopyBlit();
    void beginSlowCopyBlit();

    // Emulates a Blitter micro-instruction
    template <u16 instr> void exec();
    template <u16 instr> void fakeExec();

    // Check iterations
    bool isFirstIteration() { return xCounter == bltsizeH; }
    bool isLastIteration() { return xCounter == 1; }

    // Sets the x or y counter to a new value
    void setXCounter(u16 value);
    void setYCounter(u16 value);
    void resetXCounter() { setXCounter(bltsizeH); }
    void resetYCounter() { setYCounter(bltsizeV); }
    void decXCounter() { setXCounter(xCounter - 1); }
    void decYCounter() { setYCounter(yCounter - 1); }

    // Emulate the barrel shifter
    void doBarrelShifterA();
    void doBarrelShifterB();
};

#endif
