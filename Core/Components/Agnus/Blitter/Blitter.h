// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BlitterTypes.h"
#include "Memory.h"
#include "AgnusTypes.h"
#include "SubComponent.h"

namespace vamiga {

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

class Blitter final : public SubComponent, public Inspectable<BlitterInfo>
{
    friend class Agnus;

    Descriptions descriptions = {{

        .type           = Class::Blitter,
        .name           = "Blitter",
        .description    = "Blitter",
        .shell          = "blitter"
    }};

    Options options = {

        Opt::BLITTER_ACCURACY
    };

    // Current configuration
    BlitterConfig config = {};

    // The fill pattern lookup tables
    u8 fillPattern[2][2][256];     // [inclusive/exclusive][carry in][data]
    u8 nextCarryIn[2][256];        // [carry in][data]
    
    
    //
    // Blitter registers
    //
    
    // Control registers
    u16 bltcon0;
    u16 bltcon1;
    
    // DMA pointers
    u32 bltapt;
    u32 bltbpt;
    u32 bltcpt;
    u32 bltdpt;
    
    // Word masks
    u16 bltafwm;
    u16 bltalwm;
    
    // Size register
    u16 bltsizeH;
    u16 bltsizeV;
    
    // Modulo registers
    i16 bltamod;
    i16 bltbmod;
    i16 bltcmod;
    i16 bltdmod;
    
    // Pipeline registers
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
    void (Blitter::*lineBlitInstr[4][2][8])(void);
    
    // The program counter indexing the micro instruction to execute
    u16 bltpc;
    
    // Blitter state
    isize iteration;
    
    // Counters tracking the coordinate of the blit window
    u16 xCounter;
    u16 yCounter;
    
    // Counters tracking the DMA accesses for each channel
    i16 cntA;
    i16 cntB;
    i16 cntC;
    i16 cntD;
    
    // The fill carry bit
    bool fillCarry;
    
    // Channel A mask
    u16 mask;
    
    // If true, the D register won't be written to memory
    bool lockD;
    
    
    //
    // Flags
    //
    
    /* Indicates if the Blitter is currently running. The flag is set to true
     * when a Blitter operation starts and set to false when the operation ends.
     */
    bool running;
    
    /* The Blitter busy flag. This flag shows up in DMACON and has a similar
     * meaning as variable 'running'. The only difference is that the busy flag
     * is cleared a few cycles before the Blitter actually terminates.
     */
    bool bbusy;
    
    // The Blitter zero flag
    bool bzero;
    
    // Indicates whether the Blitter interrupt has been triggered
    bool birq;
    
    
    //
    // Counters
    //
    
private:
    
    // Counter for tracking the remaining words to process
    isize remaining;
    
    // Debug counters
    isize blitcount;
    isize copycount;
    isize linecount;
    
    // Debug checksums
    u32 check1;
    u32 check2;
    
public:
    
    // Optional storage for recording memory locations if BLT_GUARD is enabled
    Buffer<isize> memguard;
    
    
    //
    // Initializing
    //
    
public:
    
    Blitter(Amiga& ref);
    
private:

    void initFastBlitter();
    void initSlowBlitter();

public:

    Blitter& operator= (const Blitter& other) {

        CLONE(bltcon0)
        CLONE(bltcon1)

        CLONE(bltapt)
        CLONE(bltbpt)
        CLONE(bltcpt)
        CLONE(bltdpt)

        CLONE(bltafwm)
        CLONE(bltalwm)

        CLONE(bltsizeH)
        CLONE(bltsizeV)

        CLONE(bltamod)
        CLONE(bltbmod)
        CLONE(bltcmod)
        CLONE(bltdmod)

        CLONE(anew)
        CLONE(bnew)
        CLONE(aold)
        CLONE(bold)
        CLONE(ahold)
        CLONE(bhold)
        CLONE(chold)
        CLONE(dhold)
        CLONE(ashift)
        CLONE(bshift)

        CLONE(bltpc)
        CLONE(iteration)

        CLONE(xCounter)
        CLONE(yCounter)
        CLONE(cntA)
        CLONE(cntB)
        CLONE(cntC)
        CLONE(cntD)

        CLONE(fillCarry)
        CLONE(mask)
        CLONE(lockD)

        CLONE(running)
        CLONE(bbusy)
        CLONE(bzero)
        CLONE(birq)

        CLONE(remaining)

        CLONE(config)

        return *this;
    }


    //
    // Methods from Serializable
    //
    
public:

    template <class T>
    void serialize(T& worker)
    {
        worker

        << bltcon0
        << bltcon1

        << bltapt
        << bltbpt
        << bltcpt
        << bltdpt

        << bltafwm
        << bltalwm

        << bltsizeH
        << bltsizeV

        << bltamod
        << bltbmod
        << bltcmod
        << bltdmod

        << anew
        << bnew
        << aold
        << bold
        << ahold
        << bhold
        << chold
        << dhold
        << ashift
        << bshift

        << bltpc
        << iteration

        << xCounter
        << yCounter
        << cntA
        << cntB
        << cntC
        << cntD

        << fillCarry
        << mask
        << lockD

        << running
        << bbusy
        << bzero
        << birq

        << remaining;

        if (isResetter(worker)) return;

        worker

        << config.accuracy;

    } SERIALIZERS(serialize);

    
    //
    // Methods from CoreComponent
    //
    
public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream &os) const override;
    void _initialize() override;
    void _run() override;
    void _didReset(bool hard) override;


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(BlitterInfo &result) const override;
    

    //
    // Methods from Configurable
    //

public:
    
    const BlitterConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;


    //
    // Accessing
    //
    
public:
    
    // Returns true if the Blitter is processing a blit
    bool isActive() const { return running; }
    
    // Returns the value of the Blitter Busy Flag
    bool isBusy() const { return bbusy; }
    
    // Returns the value of the Blitter Zero Flag
    bool isZero() const { return bzero; }
    
    // BLTCON0
    void pokeBLTCON0(u16 value);
    void setBLTCON0(u16 value);
    void pokeBLTCON0L(u16 value);
    void setBLTCON0L(u16 value);
    void setASH(u16 ash);
    bool incASH();
    bool decASH();
    
    u16 bltconASH()   const { return bltcon0 >> 12; }
    u16 bltconLF()    const { return bltcon0 & 0xF; }
    u16 bltconUSE()   const { return (bltcon0 >> 8) & 0xF; }
    bool bltconUSEA() const { return bltcon0 & (1 << 11); }
    bool bltconUSEB() const { return bltcon0 & (1 << 10); }
    bool bltconUSEC() const { return bltcon0 & (1 << 9); }
    bool bltconUSED() const { return bltcon0 & (1 << 8); }
    u16 bltconUSEBC() const { return (bltcon0 >> 9) & 0x3; }
    
    // BLTCON1
    void pokeBLTCON1(u16 value);
    void setBLTCON1(u16 value);
    void setBSH(u16 bsh);
    bool incBSH();
    bool decBSH();
    
    u16 bltconBSH()   const { return bltcon1 >> 12; }
    bool bltconEFE()  const { return bltcon1 & (1 << 4); }
    bool bltconIFE()  const { return bltcon1 & (1 << 3); }
    bool bltconFE()   const { return bltconEFE() || bltconIFE(); }
    bool bltconFCI()  const { return bltcon1 & (1 << 2); }
    bool bltconDESC() const { return bltcon1 & (1 << 1); }
    bool bltconLINE() const { return bltcon1 & (1 << 0); }
    
    // BLTAxWM
    void pokeBLTAFWM(u16 value);
    void pokeBLTALWM(u16 value);
    
    
    // BLTxPTx
    void pokeBLTAPTH(u16 value);
    void pokeBLTAPTL(u16 value);
    void pokeBLTBPTH(u16 value);
    void pokeBLTBPTL(u16 value);
    void pokeBLTCPTH(u16 value);
    void pokeBLTCPTL(u16 value);
    void pokeBLTDPTH(u16 value);
    void pokeBLTDPTL(u16 value);
    
    // BLITSIZE
    template <Accessor s> void pokeBLTSIZE(u16 value);
    void setBLTSIZE(u16 value);
    void pokeBLTSIZV(u16 value);
    void setBLTSIZV(u16 value);
    void pokeBLTSIZH(u16 value);
    
    // BLTxMOD
    void pokeBLTAMOD(u16 value);
    void pokeBLTBMOD(u16 value);
    void pokeBLTCMOD(u16 value);
    void pokeBLTDMOD(u16 value);
    
    // BLTxDAT
    void pokeBLTADAT(u16 value);
    void pokeBLTBDAT(u16 value);
    void pokeBLTCDAT(u16 value);
    
    
    //
    // Handling requests of other components
    //
    
public:
    
    // Called by Agnus when DMACON is written to
    void pokeDMACON(u16 oldValue, u16 newValue);
    
    
    //
    // Serving events
    //
    
public:
    
    // Processes a Blitter event
    void serviceEvent();
    void serviceEvent(EventID id);
    
    
    //
    // Running the sub-units
    //
    
private:
    
    // Emulates the barrel shifter
    u16 barrelShifter(u16 anew, u16 aold, u16 shift, bool desc = false) const;
    
    // Emulates the minterm logic circuit
    u16 doMintermLogic     (u16 a, u16 b, u16 c, u8 minterm) const;
    u16 doMintermLogicQuick(u16 a, u16 b, u16 c, u8 minterm) const;
    
    // Emulates the fill logic circuit
    void doFill(u16 &data, bool &carry) const;
    
    // Emulates the line logic circuit
    void doLine();
    
    
    //
    // Executing the Blitter
    //
    
private:
    
    // Prepares for a new Blitter operation (called in state BLT_STRT1)
    void prepareBlit();
    
    // Starts a Blitter operation
    void beginBlit();
    void beginLineBlit(isize level);
    void beginCopyBlit(isize level);
    
    // Clears the BBUSY flag
    void clearBusyFlag();
    
    // Concludes the current Blitter operation
    void endBlit();
    
    
    //
    //  Executing the Fast Blitter
    //
    
private:
    
    // Starts a level 0 blit
    void beginFastCopyBlit();
    void beginFastLineBlit();
    
    // Performs a copy blit operation via the FastBlitter
    template <bool useA, bool useB, bool useC, bool useD, bool desc>
    void doFastCopyBlit();
    
    // Performs a line blit operation via the FastBlitter
    void doFastLineBlit();
    
    
    //
    //  Executing the Slow Blitter
    //
    
private:
    
    // Starts a level 1 blit
    void beginFakeCopyBlit();
    void beginFakeLineBlit();
    
    // Starts a level 2 blit
    void beginSlowLineBlit();
    void beginSlowCopyBlit();
    
    // Emulates a Blitter micro-instruction (area mode)
    template <u16 instr> void exec();
    template <u16 instr> void fakeExec();
    
    // Emulates a Blitter micro-instruction (line mode)
    template <u16 instr> void execLine();
    template <u16 instr> void fakeExecLine();
    
    // Checks iterations
    bool isFirstWord() const { return xCounter == bltsizeH; }
    bool isLastWord() const { return xCounter == 1; }
    
    // Sets the x or y counter to a new value
    void setXCounter(u16 value);
    void setYCounter(u16 value);
    void resetXCounter() { setXCounter(bltsizeH); }
    void resetYCounter() { setYCounter(bltsizeV); }
    void decXCounter() { setXCounter(xCounter - 1); }
    void decYCounter() { setYCounter(yCounter - 1); }
    
    
    //
    // Debugging
    //
    
public:
    
    bool checkMemguard(u32 addr) { return memguard[addr] == blitcount; }
};

}
