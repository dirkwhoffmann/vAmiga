// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

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
    memcpy(this->blitfunc, blitfunc, sizeof(blitfunc));
}

void
Blitter::beginFastLineBlit()
{
    // Only call this function is line blit mode
    assert(bltconLINE());

    static bool verbose = true;
    if (verbose) { verbose = false; debug("Using the fast line Blitter\n"); }

    doFastLineBlit();
    terminate();
}

void
Blitter::beginFastCopyBlit()
{
    // Only call this function in copy blit mode
    assert(!bltconLINE());

    static bool verbose = true;
    if (verbose) { debug("Using the fast copy Blitter\n"); }

    // Select the proper Blitter function
    int nr = ((bltcon0 >> 7) & 0b11110) | !!bltconDESC();

    // Do the blit
    (this->*blitfunc[nr])();

    // Depending on the accuracy level, either terminate immediately or start
    // fake-executing the micro-program to emulate proper timing.
    switch (config.accuracy) {

        case 0:
            if (verbose) { verbose = false; debug("Immediate termination\n"); }
            terminate();
            return;

        case 1:
            if (verbose) { verbose = false; debug("Fake micro-code execution\n"); }
            agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_EXEC_FAST);
            return;

        default:
            assert(false);
    }
}

template <bool useA, bool useB, bool useC, bool useD, bool desc>
void Blitter::doFastCopyBlit()
{
    uint32_t apt = bltapt;
    uint32_t bpt = bltbpt;
    uint32_t cpt = bltcpt;
    uint32_t dpt = bltdpt;

    bool fill = bltconFE();
    bool fillCarry;

    int incr = desc ? -2 : 2;
    int ash = desc ? 16 - bltconASH() : bltconASH();
    int bsh = desc ? 16 - bltconBSH() : bltconBSH();
    int32_t amod = desc ? -bltamod : bltamod;
    int32_t bmod = desc ? -bltbmod : bltbmod;
    int32_t cmod = desc ? -bltcmod : bltcmod;
    int32_t dmod = desc ? -bltdmod : bltdmod;

    aold = 0;
    bold = 0;

    for (int y = 0; y < bltsizeH; y++) {

        // Reset the fill carry bit
        fillCarry = !!bltconFCI();

        // Apply the "first word mask" in the first iteration
        uint16_t mask = bltafwm;

        for (int x = 0; x < bltsizeW; x++) {

            // Apply the "last word mask" in the last iteration
            if (x == bltsizeW - 1) mask &= bltalwm;

            // Fetch A
            if (useA) {
                anew = mem.peek16<BUS_BLITTER>(apt);
                debug(BLT_DEBUG, "    A = peek(%X) = %X\n", apt, anew);
                INC_OCS_PTR(apt, incr);
            }

            // Fetch B
            if (useB) {
                bnew = mem.peek16<BUS_BLITTER>(bpt);
                debug(BLT_DEBUG, "    B = peek(%X) = %X\n", bpt, bnew);
                INC_OCS_PTR(bpt, incr);
            }

            // Fetch C
            if (useC) {
                chold = mem.peek16<BUS_BLITTER>(cpt);
                debug(BLT_DEBUG, "    C = peek(%X) = %X\n", cpt, chold);
                INC_OCS_PTR(cpt, incr);
            }
            debug(BLT_DEBUG, "    After fetch: A = %x B = %x C = %x\n", anew, bnew, chold);

            debug(BLT_DEBUG, "    After masking with %x (%x,%x) %x\n", mask, bltafwm, bltalwm, anew & mask);

            // Run the barrel shifters on data path A and B
            debug(BLT_DEBUG, "    ash = %d bsh = %d mask = %X\n", bltconASH(), bltconBSH(), mask);
            if (desc) {
                ahold = HI_W_LO_W(anew & mask, aold) >> ash;
                bhold = HI_W_LO_W(bnew, bold) >> bsh;
            } else {
                ahold = HI_W_LO_W(aold, anew & mask) >> ash;
                bhold = HI_W_LO_W(bold, bnew) >> bsh;
            }
            aold = anew & mask;
            bold = bnew;
            debug(BLT_DEBUG, "    After shifting (%d,%d) A = %x B = %x\n", ash, bsh, ahold, bhold);

            // Run the minterm logic circuit
            debug(BLT_DEBUG, "    Minterms: ahold = %X bhold = %X chold = %X bltcon0 = %X (hex)\n", ahold, bhold, chold, bltcon0);
            dhold = doMintermLogicQuick(ahold, bhold, chold, bltcon0 & 0xFF);
            assert(dhold == doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF));

            // Run the fill logic circuit
            if (fill) doFill(dhold, fillCarry);

            // Update the zero flag
            if (dhold) bzero = false;

            // Write D
            if (useD) {
                mem.poke16<BUS_BLITTER>(dpt, dhold);
                check1 = fnv_1a_it32(check1, dhold);
                check2 = fnv_1a_it32(check2, dpt);
                debug(BLT_DEBUG, "D: poke(%X), %X  (check: %X %X)\n", dpt, dhold, check1, check2);
                // plainmsg("    check1 = %X check2 = %X\n", check1, check2);

                INC_OCS_PTR(dpt, incr);
            }

            // Clear the word mask
            mask = 0xFFFF;
        }

        // Add modulo values
        if (useA) INC_OCS_PTR(apt, amod);
        if (useB) INC_OCS_PTR(bpt, bmod);
        if (useC) INC_OCS_PTR(cpt, cmod);
        if (useD) INC_OCS_PTR(dpt, dmod);
    }

    // Do some consistency checks
    assert(apt == useA ? bltapt + (incr * bltsizeW + amod) * bltsizeH : bltapt);
    assert(bpt == useB ? bltbpt + (incr * bltsizeW + bmod) * bltsizeH : bltbpt);
    assert(cpt == useC ? bltcpt + (incr * bltsizeW + cmod) * bltsizeH : bltcpt);
    assert(dpt == useD ? bltdpt + (incr * bltsizeW + dmod) * bltsizeH : bltdpt);

    // Write back pointer registers
    bltapt = apt;
    bltbpt = bpt;
    bltcpt = cpt;
    bltdpt = dpt;
}

#define blitterLineIncreaseX(a_shift, cpt) \
if (a_shift < 15) a_shift++; \
else \
{ \
a_shift = 0; \
INC_OCS_PTR(cpt, 2); \
}

#define blitterLineDecreaseX(a_shift, cpt) \
{ \
if (a_shift == 0) \
{ \
a_shift = 16; \
INC_OCS_PTR(cpt, -2); \
} \
a_shift--; \
}

#define blitterLineIncreaseY(cpt, cmod) \
INC_OCS_PTR(cpt, cmod);

#define blitterLineDecreaseY(cpt, cmod) \
INC_OCS_PTR(cpt, -cmod);

void
Blitter::doFastLineBlit()
{
    //
    // Adapted from WinFellow
    //

    uint32_t bltcon = HI_W_LO_W(bltcon0, bltcon1);
    
    int height = bltsizeH;
    
    uint16_t bltadat_local = 0;
    uint16_t bltbdat_local = 0;
    uint16_t bltcdat_local = chold;
    uint16_t bltddat_local = 0;
    
    uint16_t mask = (bnew >> bltconBSH()) | (bnew << (16 - bltconBSH()));
    bool a_enabled = bltcon & 0x08000000;
    bool c_enabled = bltcon & 0x02000000;
    
    bool decision_is_signed = (((bltcon >> 6) & 1) == 1);
    uint32_t decision_variable = bltapt;
    
    // Quirk: Set decision increases to 0 if a is disabled, ensures bltapt remains unchanged
    int16_t decision_inc_signed = a_enabled ? bltbmod : 0;
    int16_t decision_inc_unsigned = a_enabled ? bltamod : 0;
    
    uint32_t bltcpt_local = bltcpt;
    uint32_t bltdpt_local = bltdpt;
    uint32_t blit_a_shift_local = bltconASH();
    uint32_t bltzero_local = 0;
    uint32_t i;
    
    uint32_t sulsudaul = (bltcon >> 2) & 0x7;
    bool x_independent = (sulsudaul & 4);
    bool x_inc = ((!x_independent) && !(sulsudaul & 2)) || (x_independent && !(sulsudaul & 1));
    bool y_inc = ((!x_independent) && !(sulsudaul & 1)) || (x_independent && !(sulsudaul & 2));
    bool single_dot = false;
    uint8_t minterm = (uint8_t)(bltcon >> 16);
    
    for (i = 0; i < height; ++i)
    {
        // Read C-data from memory if the C-channel is enabled
        if (c_enabled) {
            bltcdat_local = mem.peek16<BUS_BLITTER>(bltcpt_local);
        }
        
        // Calculate data for the A-channel
        bltadat_local = (anew & bltafwm) >> blit_a_shift_local;
        
        // Check for single dot
        if (x_independent) {
            if (bltcon & 0x00000002) {
                if (single_dot) {
                    bltadat_local = 0;
                } else {
                    single_dot = TRUE;
                }
            }
        }
        
        // Calculate data for the B-channel
        bltbdat_local = (mask & 1) ? 0xFFFF : 0;
        
        // Calculate result
        bltddat_local = doMintermLogicQuick(bltadat_local, bltbdat_local, bltcdat_local, minterm);
        
        // Save result to D-channel, same as the C ptr after first pixel.
        if (c_enabled) { // C-channel must be enabled
            mem.poke16<BUS_BLITTER>(bltdpt_local, bltddat_local);
            check1 = fnv_1a_it32(check1, bltddat_local);
            check2 = fnv_1a_it32(check2, bltdpt_local);
        }
        
        // Remember zero result status
        bltzero_local = bltzero_local | bltddat_local;
        
        // Rotate mask
        mask = (mask << 1) | (mask >> 15);
        
        // Test movement in the X direction
        // When the decision variable gets positive,
        // the line moves one pixel to the right
        
        // decrease/increase x
        if (decision_is_signed) {
            // Do not yet increase, D has sign
            // D = D + (2*sdelta = bltbmod)
            decision_variable += decision_inc_signed;
        } else {
            // increase, D reached a positive value
            // D = D + (2*sdelta - 2*ldelta = bltamod)
            decision_variable += decision_inc_unsigned;
            
            if (!x_independent) {
                if (x_inc) {
                    blitterLineIncreaseX(blit_a_shift_local, bltcpt_local);
                } else {
                    blitterLineDecreaseX(blit_a_shift_local, bltcpt_local);
                }
            } else {
                if (y_inc) {
                    blitterLineIncreaseY(bltcpt_local, bltcmod);
                } else {
                    blitterLineDecreaseY(bltcpt_local, bltcmod);
                }
                single_dot = false;
            }
        }
        decision_is_signed = ((int16_t)decision_variable < 0);
        
        if (!x_independent)
        {
            // decrease/increase y
            if (y_inc) {
                blitterLineIncreaseY(bltcpt_local, bltcmod);
            } else {
                blitterLineDecreaseY(bltcpt_local, bltcmod);
            }
        }
        else
        {
            if (x_inc) {
                blitterLineIncreaseX(blit_a_shift_local, bltcpt_local);
            } else {
                blitterLineDecreaseX(blit_a_shift_local, bltcpt_local);
            }
        }
        bltdpt_local = bltcpt_local;
    }
    bltcon = bltcon & 0x0FFFFFFBF;
    if (decision_is_signed) bltcon |= 0x00000040;
    
    setBltconASH(blit_a_shift_local);
    bnew   = bltbdat_local;
    bltapt = OCS_PTR(decision_variable);
    bltcpt = OCS_PTR(bltcpt_local);
    bltdpt = OCS_PTR(bltdpt_local);
    bzero  = bltzero_local;
}
    /*
     void blitterLineMode(void)
     {
     ULO bltadat_local;
     ULO bltbdat_local;
     ULO bltcdat_local = blitter.bltcdat;
     ULO bltddat_local;
     UWO mask = (UWO) ((blitter.bltbdat_original >> blitter.b_shift_asc) | (blitter.bltbdat_original << (16 - blitter.b_shift_asc)));
     BOOLE a_enabled = blitter.bltcon & 0x08000000;
     BOOLE c_enabled = blitter.bltcon & 0x02000000;
     
     BOOLE decision_is_signed = (((blitter.bltcon >> 6) & 1) == 1);
     LON decision_variable = (LON) blitter.bltapt;
     
     // Quirk: Set decision increases to 0 if a is disabled, ensures bltapt remains unchanged
     WOR decision_inc_signed = (a_enabled) ? ((WOR) blitter.bltbmod) : 0;
     WOR decision_inc_unsigned = (a_enabled) ? ((WOR) blitter.bltamod) : 0;
     
     ULO bltcpt_local = blitter.bltcpt;
     ULO bltdpt_local = blitter.bltdpt;
     ULO blit_a_shift_local = blitter.a_shift_asc;
     ULO bltzero_local = 0;
     ULO i;
     
     ULO sulsudaul = (blitter.bltcon >> 2) & 0x7;
     BOOLE x_independent = (sulsudaul & 4);
     BOOLE x_inc = ((!x_independent) && !(sulsudaul & 2)) || (x_independent && !(sulsudaul & 1));
     BOOLE y_inc = ((!x_independent) && !(sulsudaul & 1)) || (x_independent && !(sulsudaul & 2));
     BOOLE single_dot = FALSE;
     UBY minterm = (UBY) (blitter.bltcon >> 16);
     
     for (i = 0; i < blitter.height; ++i)
     {
     // Read C-data from memory if the C-channel is enabled
     if (c_enabled)
     {
     bltcdat_local = chipmemReadWord(bltcpt_local);
     }
     
     // Calculate data for the A-channel
     bltadat_local = (blitter.bltadat & blitter.bltafwm) >> blit_a_shift_local;
     
     // Check for single dot
     if (x_independent)
     {
     if (blitter.bltcon & 0x00000002)
     {
     if (single_dot)
     {
     bltadat_local = 0;
     }
     else
     {
     single_dot = TRUE;
     }
     }
     }
     
     // Calculate data for the B-channel
     bltbdat_local = (mask & 1) ? 0xffff : 0;
     
     // Calculate result
     blitterMinterms(bltadat_local, bltbdat_local, bltcdat_local, bltddat_local, minterm);
     
     // Save result to D-channel, same as the C ptr after first pixel.
     if (c_enabled) // C-channel must be enabled
     {
     chipmemWriteWord(bltddat_local, bltdpt_local);
     }
     
     // Remember zero result status
     bltzero_local = bltzero_local | bltddat_local;
     
     // Rotate mask
     mask = (mask << 1) | (mask >> 15);
     
     // Test movement in the X direction
     // When the decision variable gets positive,
     // the line moves one pixel to the right
     
     // decrease/increase x
     if (decision_is_signed)
     {
     // Do not yet increase, D has sign
     // D = D + (2*sdelta = bltbmod)
     decision_variable += decision_inc_signed;
     }
     else
     {
     // increase, D reached a positive value
     // D = D + (2*sdelta - 2*ldelta = bltamod)
     decision_variable += decision_inc_unsigned;
     
     if (!x_independent)
     {
     if (x_inc)
     {
     blitterLineIncreaseX(blit_a_shift_local, bltcpt_local);
     }
     else
     {
     blitterLineDecreaseX(blit_a_shift_local, bltcpt_local);
     }
     }
     else
     {
     if (y_inc)
     {
     blitterLineIncreaseY(bltcpt_local, blitter.bltcmod);
     }
     else
     {
     blitterLineDecreaseY(bltcpt_local, blitter.bltcmod);
     }
     single_dot = FALSE;
     }
     }
     decision_is_signed = ((WOR)decision_variable < 0);
     
     if (!x_independent)
     {
     // decrease/increase y
     if (y_inc)
     {
     blitterLineIncreaseY(bltcpt_local, blitter.bltcmod);
     }
     else
     {
     blitterLineDecreaseY(bltcpt_local, blitter.bltcmod);
     }
     }
     else
     {
     if (x_inc)
     {
     blitterLineIncreaseX(blit_a_shift_local, bltcpt_local);
     }
     else
     {
     blitterLineDecreaseX(blit_a_shift_local, bltcpt_local);
     }
     }
     bltdpt_local = bltcpt_local;
     }
     blitter.bltcon = blitter.bltcon & 0x0FFFFFFBF;
     if (decision_is_signed) blitter.bltcon |= 0x00000040;
     
     blitter.a_shift_asc = blit_a_shift_local;
     blitter.a_shift_desc = 16 - blitter.a_shift_asc;
     blitter.bltbdat = bltbdat_local;
     blitter.bltapt = decision_variable;
     blitter.bltcpt = bltcpt_local;
     blitter.bltdpt = bltdpt_local;
     blitter.bltzero = bltzero_local;
     memoryWriteWord(0x8040, 0x00DFF09C);
     }
     */
