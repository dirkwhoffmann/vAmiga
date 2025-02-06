// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Blitter.h"
#include "Agnus.h"
#include "Checksum.h"
#include "Memory.h"
#include "Paula.h"

namespace vamiga {

void
Blitter::initFastBlitter()
{
    void (Blitter::*blitfunc[32])(void) = {
        &Blitter::doFastCopyBlit<0,0,0,0,0>, &Blitter::doFastCopyBlit<0,0,0,0,1>,
        &Blitter::doFastCopyBlit<0,0,0,1,0>, &Blitter::doFastCopyBlit<0,0,0,1,1>,
        &Blitter::doFastCopyBlit<0,0,1,0,0>, &Blitter::doFastCopyBlit<0,0,1,0,1>,
        &Blitter::doFastCopyBlit<0,0,1,1,0>, &Blitter::doFastCopyBlit<0,0,1,1,1>,
        &Blitter::doFastCopyBlit<0,1,0,0,0>, &Blitter::doFastCopyBlit<0,1,0,0,1>,
        &Blitter::doFastCopyBlit<0,1,0,1,0>, &Blitter::doFastCopyBlit<0,1,0,1,1>,
        &Blitter::doFastCopyBlit<0,1,1,0,0>, &Blitter::doFastCopyBlit<0,1,1,0,1>,
        &Blitter::doFastCopyBlit<0,1,1,1,0>, &Blitter::doFastCopyBlit<0,1,1,1,1>,
        &Blitter::doFastCopyBlit<1,0,0,0,0>, &Blitter::doFastCopyBlit<1,0,0,0,1>,
        &Blitter::doFastCopyBlit<1,0,0,1,0>, &Blitter::doFastCopyBlit<1,0,0,1,1>,
        &Blitter::doFastCopyBlit<1,0,1,0,0>, &Blitter::doFastCopyBlit<1,0,1,0,1>,
        &Blitter::doFastCopyBlit<1,0,1,1,0>, &Blitter::doFastCopyBlit<1,0,1,1,1>,
        &Blitter::doFastCopyBlit<1,1,0,0,0>, &Blitter::doFastCopyBlit<1,1,0,0,1>,
        &Blitter::doFastCopyBlit<1,1,0,1,0>, &Blitter::doFastCopyBlit<1,1,0,1,1>,
        &Blitter::doFastCopyBlit<1,1,1,0,0>, &Blitter::doFastCopyBlit<1,1,1,0,1>,
        &Blitter::doFastCopyBlit<1,1,1,1,0>, &Blitter::doFastCopyBlit<1,1,1,1,1>
    };

    assert(sizeof(this->blitfunc) == sizeof(blitfunc));
    std::memcpy(this->blitfunc, blitfunc, sizeof(blitfunc));
}

void
Blitter::beginFastCopyBlit()
{
    // Only call this function in copy blit mode
    assert(!bltconLINE());

    // Run the fast copy Blitter
    isize nr = ((bltcon0 >> 7) & 0b11110) | (bltconDESC() ? 1 : 0);
    (this->*blitfunc[nr])();

    // Terminate immediately
    clearBusyFlag();
    paula.raiseIrq(IrqSource::BLIT);
    endBlit();
}

void
Blitter::beginFastLineBlit()
{
    // Only call this function in line blit mode
    assert(bltconLINE());

    // Run the fast line Blitter
    doFastLineBlit();

    // Terminate immediately
    clearBusyFlag();
    paula.raiseIrq(IrqSource::BLIT);
    endBlit();
}

template <bool useA, bool useB, bool useC, bool useD, bool desc>
void Blitter::doFastCopyBlit()
{
    u32 apt = bltapt;
    u32 bpt = bltbpt;
    u32 cpt = bltcpt;
    u32 dpt = bltdpt;

    bool fill = bltconFE();
    bool fillCarry;

    int incr = desc ? -2 : 2;
    i32 amod = desc ? -bltamod : bltamod;
    i32 bmod = desc ? -bltbmod : bltbmod;
    i32 cmod = desc ? -bltcmod : bltcmod;
    i32 dmod = desc ? -bltdmod : bltdmod;

    aold = 0;
    bold = 0;

    for (isize y = 0; y < bltsizeV; y++) {

        // Reset the fill carry bit
        fillCarry = !!bltconFCI();

        // Apply the "first word mask" in the first iteration
        u16 mask = bltafwm;

        for (isize x = 0; x < bltsizeH; x++) {

            // Apply the "last word mask" in the last iteration
            if (x == bltsizeH - 1) mask &= bltalwm;

            // Fetch A
            if (useA) {
                anew = mem.peek16 <Accessor::AGNUS> (apt);
                trace(BLT_DEBUG, "    A = %X <- %X\n", anew, apt);
                apt = U32_ADD(apt, incr);
            }

            // Fetch B
            if (useB) {
                bnew = mem.peek16 <Accessor::AGNUS> (bpt);
                trace(BLT_DEBUG, "    B = %X <- %X\n", bnew, bpt);
                bpt = U32_ADD(bpt, incr);
            }

            // Fetch C
            if (useC) {
                chold = mem.peek16 <Accessor::AGNUS> (cpt);
                trace(BLT_DEBUG, "    C = %X <- %X\n", chold, cpt);
                cpt = U32_ADD(cpt, incr);
            }
            
            // Run the barrel shifter on path A (even if channel A is disabled)
            ahold = barrelShifter(anew & mask, aold, bltconASH(), desc);
            aold = anew & mask;

            // Run the barrel shifter on path B (if channel B is enabled)
            if (useB) {
                bhold = barrelShifter(bnew, bold, bltconBSH(), desc);
                bold = bnew;
            }
            
            // Run the minterm circuit
            dhold = doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF);

            // Run the fill logic circuit
            if (fill) doFill(dhold, fillCarry);

            // Update the zero flag
            if (dhold) bzero = false;

            // Write D
            if (useD) {
                mem.poke16 <Accessor::AGNUS> (dpt, dhold);

                if (BLT_CHECKSUM) {
                    check1 = util::fnvIt32(check1, dhold);
                    check2 = util::fnvIt32(check2, dpt & agnus.ptrMask);
                }
                trace(BLT_DEBUG, "    D = %X -> %X\n", dhold, dpt);
                
                dpt = U32_ADD(dpt, incr);
            }

            // Clear the word mask
            mask = 0xFFFF;
        }

        // Add modulo values
        if (useA) apt = U32_ADD(apt, amod);
        if (useB) bpt = U32_ADD(bpt, bmod);
        if (useC) cpt = U32_ADD(cpt, cmod);
        if (useD) dpt = U32_ADD(dpt, dmod);
    }

    // Write back pointer registers
    bltapt = apt;
    bltbpt = bpt;
    bltcpt = cpt;
    bltdpt = dpt;
}

void
Blitter::doFastLineBlit()
{
    bool firstPixel = true;
    bool useB = bltcon0 & BLTCON0_USEB;
    bool useC = bltcon0 & BLTCON0_USEC;
    bool sing = bltcon1 & BLTCON1_SING;
    bool sign = bltcon1 & BLTCON1_SIGN;
    auto ash = bltconASH();
    auto bsh = bltconBSH();

    auto incx = [&]() {
        if (++ash == 16) {
            ash = 0;
            U32_INC(bltcpt, 2);
        }
    };
    
    auto decx = [&]() {
        if (ash-- == 0) {
            ash = 15;
            U32_INC(bltcpt, -2);
        }
    };
    
    auto incy = [&]() {
        U32_INC(bltcpt, bltcmod);
        firstPixel = true;
    };
    
    auto decy = [&]() {
        U32_INC(bltcpt, -bltcmod);
        firstPixel = true;
    };
    
    auto doLineLogic = [&]() {
        
        firstPixel = false;
        
        if (!sign) {
            if (bltcon1 & BLTCON1_SUD) {
                if (bltcon1 & BLTCON1_SUL)
                    decy();
                else
                    incy();
            } else {
                if (bltcon1 & BLTCON1_SUL)
                    decx();
                else
                    incx();
            }
        }
        
        if (bltcon1 & BLTCON1_SUD) {
            if (bltcon1 & BLTCON1_AUL)
                decx();
            else
                incx();
        } else {
            if (bltcon1 & BLTCON1_AUL)
                decy();
            else
                incy();
        }
        
        if (bltcon0 & BLTCON0_USEA) {
            if (sign)
                U32_INC(bltapt, bltbmod);
            else
                U32_INC(bltapt, bltamod);
        }
        
        sign = (i16)bltapt < 0;
    };

    for (isize i = 0; i < bltsizeV; i++) {
        
        // Fetch B
        if (useB) {
            bnew = mem.peek16 <Accessor::AGNUS> (bltbpt);
            U32_INC(bltbpt, bltbmod);
        }
        
        // Fetch C
        if (useC) {
            chold = mem.peek16 <Accessor::AGNUS> (bltcpt);
        }
        
        // Run the barrel shifters
        ahold = barrelShifter(anew & bltafwm, 0, ash);
        bhold = barrelShifter(bnew, bnew, bsh);
        if (bsh-- == 0) bsh = 15;
        
        // Run the minterm circuit
        dhold = doMintermLogic(ahold, (bhold & 1) ? 0xFFFF : 0, chold, bltcon0 & 0xFF);

        bool writeEnable = (!sing || firstPixel) && useC;

        // Run the line logic circuit
        doLineLogic();

        // Update the zero flag
        if (dhold) bzero = false;

        // Write D
        if (writeEnable) {

            mem.poke16 <Accessor::AGNUS> (bltdpt, dhold);
            
            if (BLT_CHECKSUM) {
                check1 = util::fnvIt32(check1, dhold);
                check2 = util::fnvIt32(check2, bltdpt & agnus.ptrMask);
            }
        }
        
        bltdpt = bltcpt;
    }

    // Write back local values
    setASH(ash);
    setBSH(bsh);
    REPLACE_BIT(bltcon1, 6, sign);
}

}
