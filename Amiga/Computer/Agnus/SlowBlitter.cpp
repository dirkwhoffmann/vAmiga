// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

/* Micro-instructions:
 *
 * To keep the implementation flexible, the SlowBlitter is emulated as a
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
 *
 *   FAKEWRITE : Used in fake-execution mode instead of WRITE_D
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

static const uint16_t FAKEWRITE = 0b0100'0000'0000;

void
Blitter::initSlowBlitter()
{
    /* Micro programs:
     *
     * There are two versions of each microprogram:
     *
     * 1. A full version that operates the bus and all Blitter components.
     *    This version is used by the SlowBlitter (accuracy level 2).
     *
     * 2. A stripped down version that operates the bus only.
     *    This version is used in fake-execution mode (accuracy level 1).
     *
     * In accuracy level 0, no micro-program is executed.
     *
     * The micro programs below are inspired by Table 6.2 of the HRM:
     *
     *           Active
     * BLTCON0  Channels            Cycle sequence
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
     */

    void (Blitter::*instruction[16][2][5])(void) = {

        // 0: -- -- -- --
        {
            {
                &Blitter::exec <BLTIDLE>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <BLTDONE>
            },

            {
                &Blitter::exec <BLTIDLE>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <BLTDONE>
            },
        },

        // 1: D0 -- D1 -- D2
        {
            {
                &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | BUS>,
                &Blitter::exec <HOLD_D | REPEAT>,

                &Blitter::exec <WRITE_D | BUS | BLTDONE>
            },

            {
                &Blitter::exec <FAKEWRITE | BUS>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
            }
        },

        // 2: C0 -- C1 -- C2
        {
            {
                &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                &Blitter::exec <HOLD_D | REPEAT>,

                &Blitter::exec <BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <BLTDONE>
            }
        },

        // 3: C0 -- -- C1 D0 -- C2 D1 -- D2
        {
            {
                &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                &Blitter::exec <WRITE_D | BUS>,
                &Blitter::exec <HOLD_D | REPEAT>,

                &Blitter::exec <WRITE_D | BUS | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <FAKEWRITE | BUS>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
            }
        },

        // 4: B0 -- -- B1 -- -- B2
        {
            {
                &Blitter::exec <FETCH_B | BUS>,
                &Blitter::exec <HOLD_A | HOLD_B>,
                &Blitter::exec <HOLD_D | REPEAT>,

                &Blitter::exec <BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <BLTIDLE>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <BLTDONE>
            }
        },

        // 5: B0 -- -- B1 D0 -- B2 D1 -- D2
        {
            {
                &Blitter::exec <FETCH_B | BUS>,
                &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | BUS>,
                &Blitter::exec <HOLD_D | REPEAT>,

                &Blitter::exec <WRITE_D | BUS | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <FAKEWRITE | BUS>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
            }
        },

        // 6: B0 C0 -- B1 C1 -- B2 C2
        {
            {
                &Blitter::exec <FETCH_B | HOLD_D | BUS>,
                &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS | REPEAT>,

                &Blitter::exec <HOLD_D | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS | REPEAT>,

                &Blitter::exec <BLTDONE>
            }
        },

        // 7: B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- D2
        {
            {
                &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                &Blitter::exec <FETCH_C | HOLD_B | BUS>,
                &Blitter::exec <WRITE_D | HOLD_D | BUS>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <WRITE_D | BUS | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS>,
                &Blitter::exec <FAKEWRITE | BUS>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
            }
        },

        // 8: A0 -- A1 -- A2
        {
            {
                &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                &Blitter::exec <HOLD_A | HOLD_B | REPEAT>,

                &Blitter::exec <HOLD_D | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <BLTDONE>
            }
        },

        // 9: A0 -- A1 D0 A2 D1 -- D2
        {
            {
                &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                &Blitter::exec <WRITE_D | HOLD_A | BUS | REPEAT>,

                &Blitter::exec <HOLD_D>,
                &Blitter::exec <WRITE_D | BUS | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <FAKEWRITE | BUS | REPEAT>,

                &Blitter::exec <BLTIDLE>,
                &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
            }
        },

        // A: A0 C0 A1 C1 A2 C2
        {
            {
                &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS | REPEAT>,

                &Blitter::exec <HOLD_D | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS | REPEAT>,

                &Blitter::exec <BLTDONE>
            }
        },

        // B: A0 C0 -- A1 C1 D0 A2 C2 D1 -- D2
        {
            {
                &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                &Blitter::exec <WRITE_D | REPEAT | BUS>,

                &Blitter::exec <HOLD_D>,
                &Blitter::exec <WRITE_D | BUS | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS>,
                &Blitter::exec <FAKEWRITE | REPEAT | BUS>,

                &Blitter::exec <BLTIDLE>,
                &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
            }
        },

        // C: A0 B0 -- A1 B1 -- A2 B2
        {
            {
                &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                &Blitter::exec <HOLD_B  | REPEAT>,

                &Blitter::exec <HOLD_D | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS>,
                &Blitter::exec <REPEAT>,

                &Blitter::exec <BLTDONE>
            }
        },

        // D: A0 B0 -- A1 B1 D0 A2 B2 D1 -- D2
        {
            {
                &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                &Blitter::exec <WRITE_D | HOLD_B | BUS | REPEAT>,

                &Blitter::exec <HOLD_D>,
                &Blitter::exec <WRITE_D | BUS | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS>,
                &Blitter::exec <FAKEWRITE | BUS | REPEAT>,

                &Blitter::exec <BLTIDLE>,
                &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
            }
        },

        // E: A0 B0 C0 A1 B1 C1 A2 B2 C2
        {
            {
                &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                &Blitter::exec <FETCH_C | HOLD_B | BUS | REPEAT>,

                &Blitter::exec <HOLD_D | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS | REPEAT>,

                &Blitter::exec <BLTDONE>
            }
        },

        // F: A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 D2
        {
            {
                &Blitter::exec <FETCH_A | BUS>,
                &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                &Blitter::exec <FETCH_C | HOLD_B | BUS>,
                &Blitter::exec <WRITE_D | HOLD_D | BUS | REPEAT>,

                &Blitter::exec <WRITE_D | BUS | BLTDONE>
            },

            {
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS>,
                &Blitter::exec <BUS>,
                &Blitter::exec <FAKEWRITE | BUS | REPEAT>,

                &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
            }
        }
    };

    assert(sizeof(this->instruction) == sizeof(instruction));
    memcpy(this->instruction, instruction, sizeof(instruction));

    dump();
}

void
Blitter::beginSlowLineBlit()
{
    /* Note: There is no such thing as a slow line Blitter yet. Until such a
     * thing has been implemented, we call the fast Blitter instead.
     */

    // Only call this function is line blit mode
    assert(bltconLINE());

    static bool verbose = true;
    if (verbose) { verbose = false; debug("Fall back to the the fast line Blitter\n"); }

    beginFastLineBlit();
}

void
Blitter::beginSlowCopyBlit()
{
    // Only call this function in copy blit mode
    assert(!bltconLINE());

    static bool verbose = true;
    if (verbose) { verbose = false; debug("Using the slow copy Blitter\n"); }

    // Setup parameters
    if (bltconDESC()) {
        incr = -2;
        ash  = 16 - bltconASH();
        bsh  = 16 - bltconBSH();
        amod = -bltamod;
        bmod = -bltbmod;
        cmod = -bltcmod;
        dmod = -bltdmod;
    } else {
        incr = 2;
        ash  = bltconASH();
        bsh  = bltconBSH();
        amod = bltamod;
        bmod = bltbmod;
        cmod = bltcmod;
        dmod = bltdmod;
    }

    // Set width and height counters
    resetXCounter();
    resetYCounter();

    // Reset registers
    aold = 0;
    bold = 0;

    // Reset the fill carry bit
    fillCarry = !!bltconFCI();

    // Lock pipeline stage D
    lockD = true;

    // Schedule the first execution event
    agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_EXEC_SLOW);

#ifdef SLOW_BLT_DEBUG

    // In debug mode, we execute the whole micro program immediately.
    // This let's us compare checksums with the fast Blitter.
    
    BusOwner owner = agnus->busOwner[agnus->pos.h];

    while (agnus->hasEvent<BLT_SLOT>()) {
        agnus->busOwner[agnus->pos.h] = BUS_NONE;
        serviceEvent(agnus->slot[BLT_SLOT].id);
    }

    agnus->busOwner[agnus->pos.h] = owner;

#endif
}

template <uint16_t instr> void
Blitter::exec()
{
    wordCount++;

    // Check if this instruction needs the bus
    if (instr & BUS) {
        if (!agnus.allocateBus<BUS_BLITTER>()) return;
    }

    bltpc++;

    // Execute the current instruction
    if (instr & WRITE_D) {

        /* D is not written in the first iteration, because the pipepline needs
         * to ramp up.
         */
        // if (iteration == 0) {
        if (lockD) {

            debug(BLT_DEBUG, "WRITE_D (skipped)\n");
            lockD = false;

        } else {

            agnus.blitterWrite(bltdpt, dhold);
            check1 = fnv_1a_it32(check1, dhold);
            check2 = fnv_1a_it32(check2, bltdpt);
            debug(BLT_DEBUG, "D: poke(%X), %X (check: %X %X)\n", bltdpt, dhold, check1, check2);

            INC_OCS_PTR(bltdpt, incr);
            if (--cntD == 0) {
                INC_OCS_PTR(bltdpt, dmod);
                cntD = bltsizeW;
                fillCarry = !!bltconFCI();
            }
            // if (xCounter == bltsizeW()) INC_OCS_PTR(dpt, dmod);
        }
    }

    if (instr & FAKEWRITE) {

        // This instruction is only used in fake-execution mode. We simply
        // record some fake data to make the DMA debugger happy.
        assert(agnus.pos.h < HPOS_CNT);
        agnus.busValue[agnus.pos.h] = 0x8888;
    }

    if (instr & FETCH_A) {

        debug(BLT_DEBUG, "FETCH_A\n");

        anew = agnus.blitterRead(bltapt);
        debug(BLT_DEBUG, "    A = peek(%X) = %X\n", bltapt, anew);
        debug(BLT_DEBUG, "    After fetch: A = %X\n", anew);
        INC_OCS_PTR(bltapt, incr);
        if (--cntA == 0) {
            INC_OCS_PTR(bltapt, amod);
            cntA = bltsizeW;
        }
    }

    if (instr & FETCH_B) {

        debug(BLT_DEBUG, "FETCH_B\n");

        bnew = agnus.blitterRead(bltbpt);
        debug(BLT_DEBUG, "    B = peek(%X) = %X\n", bltbpt, bnew);
        debug(BLT_DEBUG, "    After fetch: B = %X\n", bnew);
        INC_OCS_PTR(bltbpt, incr);
        if (--cntB == 0) {
            INC_OCS_PTR(bltbpt, bmod);
            cntB = bltsizeW;
        }
    }

    if (instr & FETCH_C) {

        debug(BLT_DEBUG, "FETCH_C\n");

        chold = agnus.blitterRead(bltcpt);
        debug(BLT_DEBUG, "    C = peek(%X) = %X\n", bltcpt, chold);
        debug(BLT_DEBUG, "    After fetch: C = %X\n", chold);
        INC_OCS_PTR(bltcpt, incr);
        if (--cntC == 0) {
            INC_OCS_PTR(bltcpt, cmod);
            cntC = bltsizeW;
        }
    }

    if (instr & HOLD_A) {

        debug(BLT_DEBUG, "HOLD_A\n");

        debug(BLT_DEBUG, "    After masking with %x (%x,%x) %x\n", mask, bltafwm, bltalwm, anew & mask);

        // Run the barrel shifters on data path A
        debug(BLT_DEBUG, "    ash = %d mask = %X\n", bltconASH(), mask);
        if (bltconDESC()) {
            ahold = HI_W_LO_W(anew & mask, aold) >> ash;
        } else {
            ahold = HI_W_LO_W(aold, anew & mask) >> ash;
        }
        aold = anew & mask;
        debug(BLT_DEBUG, "    After shifting A (%d) A = %x\n", ash, ahold);
    }

    if (instr & HOLD_B) {

        debug(BLT_DEBUG, "HOLD_B\n");

        // Run the barrel shifters on data path B
        debug(BLT_DEBUG, "    bsh = %d\n", bltconBSH());
        if (bltconDESC()) {
            bhold = HI_W_LO_W(bnew, bold) >> bsh;
        } else {
            bhold = HI_W_LO_W(bold, bnew) >> bsh;
        }
        bold = bnew;
        debug(BLT_DEBUG, "    After shifting B (%d) B = %x\n", bsh, bhold);
    }

    if (instr & HOLD_D) {

        if (lockD) {

            debug(BLT_DEBUG, "HOLD_D (skipped)\n");

        } else {

            debug(BLT_DEBUG, "HOLD_D\n");

            // Run the minterm logic circuit
            debug(BLT_DEBUG, "    Minterms: ahold = %X bhold = %X chold = %X bltcon0 = %X (hex)\n", ahold, bhold, chold, bltcon0);
            dhold = doMintermLogicQuick(ahold, bhold, chold, bltcon0 & 0xFF);
            assert(dhold == doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF));

            // Run the fill logic circuit
            if (bltconFE()) doFill(dhold, fillCarry);

            // Update the zero flag
            if (dhold) bzero = false;
        }
    }

    if (instr & REPEAT) {

        debug(BLT_DEBUG, "REPEAT\n");
        iteration++;

        if (xCounter > 1) {

            bltpc = 0;
            decXCounter();

        } else if (yCounter > 1) {

            bltpc = 0;
            resetXCounter();
            decYCounter();

        } else {

            // The remaining micro-instructions flush the pipeline.
            // The Blitter busy flag gets cleared at this point.
            // bbusy = false;
        }
    }

    if (instr & BLTDONE) {

        debug(BLT_DEBUG, "BLTDONE\n");
        terminate();
    }
}

void
Blitter::setXCounter(uint16_t value)
{
    xCounter = value;

    // Set the mask for this iteration
    mask = 0xFFFF;

    // Apply the "first word mask" in the first iteration
    if (xCounter == bltsizeW) mask &= bltafwm;

    // Apply the "last word mask" in the last iteration
    if (xCounter == 1) mask &= bltalwm;
}

void
Blitter::setYCounter(uint16_t value)
{
    yCounter = value;
}

void
Blitter::doBarrelShifterA()
{
    uint16_t masked = anew;

    if (isFirstWord()) masked &= bltafwm;
    if (isLastWord())  masked &= bltalwm;

    if(bltconDESC()){
        ahold = (aold >> (16 - bltconASH())) | (masked << bltconASH());
    }else{
        ahold = (aold << (16 - bltconASH())) | (masked >> bltconASH());
    }
}

void
Blitter::doBarrelShifterB()
{
    if(bltconDESC()) {
        bhold = (bold >> (16 - bltconBSH())) | (bnew << bltconBSH());
    } else {
        bhold = (bold << (16 - bltconBSH())) | (bnew >> bltconBSH());
    }
}
