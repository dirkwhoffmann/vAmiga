// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

int bltdebug = 0; // REMOVE ASAP

void
Blitter::doFastBlit()
{
    // Perform a line blit or a copy blit operation
    bltconLINE() ? doFastLineBlit() : doFastCopyBlit();

    // Clear the Blitter busy flag
    bbusy = false;
    
    // Trigger the Blitter interrupt
    events->scheduleSecRel(IRQ_BLIT_SLOT, 0, IRQ_SET);
    
    // Terminate the Blitter
    events->cancel(BLT_SLOT);
}

void
Blitter::doFastCopyBlit()
{
    copycount++;

    uint32_t check1 = fnv_1a_init32();
    uint32_t check2 = fnv_1a_init32();
    
    uint16_t xmax = bltsizeW();
    uint16_t ymax = bltsizeH();
    
    bool useA = bltconUSEA();
    bool useB = bltconUSEB();
    bool useC = bltconUSEC();
    bool useD = bltconUSED();
    
    bool descending = bltconDESC();
    
    bool fillCarry;
    
    // Setup shift, increment and modulo offsets
    int     incr = 2;
    int     ash  = bltconASH();
    int     bsh  = bltconBSH();
    int32_t amod = bltamod;
    int32_t bmod = bltbmod;
    int32_t cmod = bltcmod;
    int32_t dmod = bltdmod;

    plaindebug(BLT_DEBUG, "BLITTER Blit %d (%d,%d) (%d%d%d%d) %x %x %x %x %s\n",
    copycount, bltsizeW(), bltsizeH(), useA, useB, useC, useD,
           bltapt, bltbpt, bltcpt, bltdpt, bltconDESC() ? "D" : "");

    // if (copycount == 1182) bltdebug = 1;

    // Reverse direction is descending mode
    if (bltconDESC()) {
        incr = -incr;
        ash  = 16 - ash;
        bsh  = 16 - bsh;
        amod = -amod;
        bmod = -bmod;
        cmod = -cmod;
        dmod = -dmod;
    }
    
    if (bltdebug) plainmsg("blit %d: A-%06x (%d) B-%06x (%d) C-%06x (%d) D-%06x (%d) W-%d H-%d\n",
                           copycount, bltapt, bltamod, bltbpt, bltbmod, bltcpt, bltcmod, bltdpt, bltdmod,
                           bltsizeW(), bltsizeH());
    
    aold = 0;
    bold = 0;

    for (int y = 0; y < ymax; y++) {
        
        // Reset the fill carry bit
        fillCarry = !!bltconFCI();
        
        // Apply the "first word mask" in the first iteration
        uint16_t mask = bltafwm;
        
        for (int x = 0; x < xmax; x++) {
            
            // Apply the "last word mask" in the last iteration
            if (x == xmax - 1) mask &= bltalwm;

            // Fetch A
            if (useA) {
                anew = mem->peek16(bltapt); // TODO: Call peekChip
                if (bltdebug) plainmsg("    A = peek(%X) = %X\n", bltapt, anew);
                INC_OCS_PTR(bltapt, incr);
            }
            
            // Fetch B
            if (useB) {
                bnew = mem->peek16(bltbpt); // TODO: Call peekChip
                if (bltdebug) plainmsg("    B = peek(%X) = %X\n", bltbpt, bnew);
                INC_OCS_PTR(bltbpt, incr);
            }
            
            // Fetch C
            if (useC) {
                chold = mem->peek16(bltcpt); // TODO: Call peekChip
                if (bltdebug) plainmsg("    C = peek(%X) = %X\n", bltcpt, chold);
                INC_OCS_PTR(bltcpt, incr);
            }
            if (bltdebug) plainmsg("    After fetch: A = %x B = %x C = %x\n", anew, bnew, chold);
            
            if (bltdebug) plainmsg("    After masking (%x,%x) %x\n", bltafwm, bltalwm, anew & mask);
            
            // Run the barrel shifters on data path A and B
            if (bltdebug) plainmsg("    ash = %d bsh = %d\n", bltconASH(), bltconBSH());
            if (descending) {
                ahold = HI_W_LO_W(anew & mask, aold) >> ash;
                bhold = HI_W_LO_W(bnew, bold) >> bsh;
            } else {
                ahold = HI_W_LO_W(aold, anew & mask) >> ash;
                bhold = HI_W_LO_W(bold, bnew) >> bsh;
            }
            aold = anew & mask;
            bold = bnew;
            if (bltdebug) printf("    After shifting (%d,%d) A = %x B = %x\n", ash, bsh, ahold, bhold);
            
            // Run the minterm logic circuit
            if (bltdebug) plainmsg("    ahold = %X bhold = %X chold = %X bltcon0 = %X (hex)\n", ahold, bhold, chold, bltcon0);
            // dhold = doMintermLogicQuick(ahold, bhold, chold, bltcon0 & 0xFF);
            // assert(dhold == doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF));
             dhold = doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF);

            // Run the fill logic circuit
            if (bltconFE()) doFill(dhold, fillCarry);
            
            // Update the zero flag
            if (dhold) bzero = false;
        
            // Write D
            if (useD) {
                mem->pokeChip16(bltdpt, dhold);
                if (bltdebug) plainmsg("D: poke(%X), %X\n", bltdpt, dhold);
                check1 = fnv_1a_it32(check1, dhold);
                check2 = fnv_1a_it32(check2, bltdpt);
                // plainmsg("    check1 = %X check2 = %X\n", check1, check2);
                assert(bltdpt);
                INC_OCS_PTR(bltdpt, incr);
            }
            
            // Clear the word mask
            mask = 0xFFFF;
        }
        
        // Add modulo values
        if (useA) INC_OCS_PTR(bltapt, amod);
        if (useB) INC_OCS_PTR(bltbpt, bmod);
        if (useC) INC_OCS_PTR(bltcpt, cmod);
        if (useD) INC_OCS_PTR(bltdpt, dmod);
    }
    
    // printf("BLITTER check1: %x check2: %x\n", check1, check2);
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
    uint32_t check1 = fnv_1a_init32();
    uint32_t check2 = fnv_1a_init32();
    
    linecount++;
    
    /*
    bool useA = bltUSEA();
    bool useB = bltUSEB();
    bool useC = bltUSEC();
    bool useD = bltUSED();
    */

    plaindebug(BLT_DEBUG, "BLITTER Line %d (%d,%d) (%d%d%d%d) %x %x %x %x\n",
               linecount, bltsizeW(), bltsizeH(),
               bltconUSEA(), bltconUSEB(), bltconUSEC(), bltconUSED(),
               bltapt, bltbpt, bltcpt, bltdpt);
    
    // Adapted from WinFellow
    
    uint32_t bltcon = HI_W_LO_W(bltcon0, bltcon1);
    
    int height = bltsizeH();
    // int width  = bltsizeW();
    
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
            bltcdat_local = mem->peekChip16(bltcpt_local);
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
            mem->pokeChip16(bltdpt_local, bltddat_local);
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
    
    // printf("BLITTER check1: %x check2: %x\n", check1, check2);
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
