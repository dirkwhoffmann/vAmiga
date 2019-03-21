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
Blitter::doFastBlit()
{
    // Perform a line blit or a copy blit operation
    bltLINE() ? doLineBlit() : doCopyBlit();
    
    // Clear the Blitter busy flag
    bbusy = false;
    
    // Trigger the Blitter interrupt
    handler->scheduleSecondaryRel(BLIT_IRQ_SLOT, 0, IRQ_SET);
    
    // Terminate the Blitter
    handler->cancel(BLT_SLOT);
}

void
Blitter::doCopyBlit()
{
    debug("Doing a fast copy blit (%d,%d) (%s)\n",
          bltsizeH(), bltsizeW(), bltDESC() ? "DESCENDING" : "ascending");
    
    bool useA = bltUSEA();
    bool useB = bltUSEB();
    bool useC = bltUSEC();
    bool useD = bltUSED();
    debug("A = %d B = %d C = %d D = %d\n", useA, useB, useC, useD);
    
    // Setup increment and modulo counters
    uint16_t incr = 2;
    uint16_t amod = bltamod;
    uint16_t bmod = bltbmod;
    uint16_t cmod = bltcmod;
    uint16_t dmod = bltdmod;
    
    // Reverse direction is descending mode
    if (bltDESC()) {
        incr = -incr;
        amod = -amod;
        bmod = -bmod;
        cmod = -cmod;
        dmod = -dmod;
    }
    
    for (hcounter = bltsizeH(); hcounter >= 1; hcounter--) {
    
        for (wcounter = bltsizeW(); wcounter >= 1;  wcounter--) {
            
            debug("(%d,%d)\n", hcounter, wcounter);
            
            // Fetch A, B, and C
            if (useA) {
                pokeBLTADAT(amiga->mem.peek16(bltapt));
                debug("A = peek(%d) = %d\n", bltapt, amiga->mem.peek16(bltapt));
                INC_OCS_PTR(bltapt, incr + (isLastWord() ? amod : 0));
            }
            if (useB) {
                pokeBLTBDAT(amiga->mem.peek16(bltbpt));
                debug("B = peek(%d) = %d\n", bltbpt, amiga->mem.peek16(bltbpt));
                INC_OCS_PTR(bltbpt, incr + (isLastWord() ? bmod : 0));
            }
            if (useC) {
                pokeBLTCDAT(amiga->mem.peek16(bltcpt));
                debug("C = peek(%d) = %d\n", bltcpt, amiga->mem.peek16(bltcpt));
                INC_OCS_PTR(bltcpt, incr + (isLastWord() ? cmod : 0));
            }
            
            // Run the barrel shifters
            ahold = (ashift >> bltASH()) & 0xFFFF;
            bhold = (bshift >> bltBSH()) & 0xFFFF;
            
            // Run the minterm generator
            dhold = 0;
            if (bltcon0 & 0b10000000) dhold |=  ahold &  bhold &  chold;
            if (bltcon0 & 0b01000000) dhold |=  ahold &  bhold & ~chold;
            if (bltcon0 & 0b00100000) dhold |=  ahold &  bhold &  chold;
            if (bltcon0 & 0b00010000) dhold |=  ahold & ~bhold & ~chold;
            if (bltcon0 & 0b00001000) dhold |= ~ahold &  bhold &  chold;
            if (bltcon0 & 0b00000100) dhold |= ~ahold &  bhold & ~chold;
            if (bltcon0 & 0b00000010) dhold |= ~ahold & ~bhold &  chold;
            if (bltcon0 & 0b00000001) dhold |= ~ahold & ~bhold & ~chold;
            
            // Update the zero flag
            if (dhold) bzero = false;
        
            // Write D
            if (useD) {
                amiga->mem.pokeChip16(bltdpt, dhold);
                debug("D: poke(%d), %d\n", bltdpt, dhold);
                INC_OCS_PTR(bltdpt, 2 + (isLastWord() ? dmod : 0));
            }
        }
    }
}

void
Blitter::doLineBlit()
{
    debug("Doing a fast line blit. NOT IMPLEMENTED YET\n");
}
