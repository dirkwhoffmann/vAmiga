// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Blitter::Blitter()
{
    setDescription("Blitter");
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &bltcon0,    sizeof(bltcon0),    0 },
        { &bltcon1,    sizeof(bltcon1),    0 },

        { &bltapt,     sizeof(bltapt),     0 },
        { &bltbpt,     sizeof(bltbpt),     0 },
        { &bltcpt,     sizeof(bltcpt),     0 },
        { &bltdpt,     sizeof(bltdpt),     0 },

        { &bltafwm,    sizeof(bltafwm),    0 },
        { &bltalwm,    sizeof(bltalwm),    0 },

        { &bltsize,    sizeof(bltsize),    0 },

        { &bltamod,    sizeof(bltamod),    0 },
        { &bltbmod,    sizeof(bltbmod),    0 },
        { &bltcmod,    sizeof(bltcmod),    0 },
        { &bltdmod,    sizeof(bltdmod),    0 },

        { &anew,       sizeof(anew),       0 },
        { &bnew,       sizeof(bnew),       0 },
        { &aold,       sizeof(aold),       0 },
        { &bold,       sizeof(bold),       0 },
        { &ahold,      sizeof(ahold),      0 },
        { &bhold,      sizeof(bhold),      0 },
        { &chold,      sizeof(chold),      0 },
        { &dhold,      sizeof(dhold),      0 },
        { &ashift,     sizeof(ashift),     0 },
        { &bshift,     sizeof(bshift),     0 },

    });
}

BlitterInfo
Blitter::getInfo()
{
    BlitterInfo info;
    
    /* Note: We call the Blitter 'active' if there is a pending message in the
     * Copper event slot.
     */
    info.active  = amiga->dma.eventHandler.isPending(BLT_SLOT);
    info.bltcon0 = bltcon0;
    info.bltcon1 = bltcon1;
    info.bltapt  = bltapt;
    info.bltbpt  = bltbpt;
    info.bltcpt  = bltcpt;
    info.bltdpt  = bltdpt;
    info.bltafwm = bltafwm;
    info.bltalwm = bltalwm;
    info.bltsize = bltsize;
    info.bltamod = bltamod;
    info.bltbmod = bltbmod;
    info.bltcmod = bltcmod;
    info.bltdmod = bltdmod;
    info.anew = anew;
    info.bnew = bnew;
    info.ahold = ahold;
    info.bhold = bhold;
    info.chold = chold;
    info.dhold = dhold;

    return info;
}

void
Blitter::_powerOn()
{
    
}

void
Blitter::_powerOff()
{
    
}

void
Blitter::_reset()
{
    
}

void
Blitter::_ping()
{
    
}

void
Blitter::_dump()
{
    plainmsg("   bltcon0: %X\n", bltcon0);
    plainmsg("\n");
    plainmsg("            Shift A: %d\n", bltASH());
    plainmsg("              Use A: %s\n", bltUSEA() ? "yes" : "no");
    plainmsg("              Use B: %s\n", bltUSEB() ? "yes" : "no");
    plainmsg("              Use C: %s\n", bltUSEC() ? "yes" : "no");
    plainmsg("              Use D: %s\n", bltUSED() ? "yes" : "no");
    plainmsg("\n");
    plainmsg("   bltcon1: %X\n", bltcon1);
    plainmsg("\n");
    plainmsg("            Shift B: %d\n", bltBSH());
    plainmsg("                EFE: %s\n", bltEFE() ? "yes" : "no");
    plainmsg("                IFE: %s\n", bltIFE() ? "yes" : "no");
    plainmsg("                FCI: %s\n", bltFCI() ? "yes" : "no");
    plainmsg("               DESC: %s\n", bltDESC() ? "yes" : "no");
    plainmsg("               LINE: %s\n", bltLINE() ? "yes" : "no");
    plainmsg("\n");
    plainmsg("   bltsize: %X\n", bltsize);
    plainmsg("\n");
    plainmsg("             height: %d\n", bltsizeH());
    plainmsg("              width: %d\n", bltsizeW());
    plainmsg("\n");
    plainmsg("    bltapt: %X\n", bltapt);
    plainmsg("    bltbpt: %X\n", bltbpt);
    plainmsg("    bltcpt: %X\n", bltcpt);
    plainmsg("    bltdpt: %X\n", bltdpt);
    plainmsg("   bltafwm: %X\n", bltafwm);
    plainmsg("   bltalwm: %X\n", bltalwm);
    plainmsg("   bltamod: %X\n", bltamod);
    plainmsg("   bltbmod: %X\n", bltbmod);
    plainmsg("   bltcmod: %X\n", bltcmod);
    plainmsg("   bltdmod: %X\n", bltdmod);
    plainmsg("      anew: %X\n", anew);
    plainmsg("      bnew: %X\n", bnew);
    plainmsg("     ahold: %X\n", ahold);
    plainmsg("     bhold: %X\n", bhold);
    plainmsg("     chold: %X\n", chold);
    plainmsg("     dhold: %X\n", dhold);
    plainmsg("    ashift: %X bshift: %X\n", ashift, bshift);
}

void
Blitter::pokeBLTCON0(uint16_t value)
{
    debug("pokeBLTCON0(%X)\n", value);
    bltcon0 = value;
}

void
Blitter::pokeBLTCON1(uint16_t value)
{
    debug("pokeBLTCON1(%X)\n", value);
    bltcon1 = value;
}

void
Blitter::pokeBLTAPTH(uint16_t value)
{
    debug("pokeBLTAPTH(%X)\n", value);
    bltapt = REPLACE_HI_WORD(bltapt, value & 0x7);
}

void
Blitter::pokeBLTAPTL(uint16_t value)
{
    debug("pokeBLTAPTL(%X)\n", value);
    bltapt = REPLACE_LO_WORD(bltapt, value);
}

void
Blitter::pokeBLTBPTH(uint16_t value)
{
    debug("pokeBLTBPTH(%X)\n", value);
    bltbpt = REPLACE_HI_WORD(bltbpt, value & 0x7);
}

void
Blitter::pokeBLTBPTL(uint16_t value)
{
    debug("pokeBLTBPTL(%X)\n", value);
    bltbpt = REPLACE_LO_WORD(bltbpt, value);
}

void
Blitter::pokeBLTCPTH(uint16_t value)
{
    debug("pokeBLTCPTH(%X)\n", value);
    bltcpt = REPLACE_HI_WORD(bltcpt, value & 0x7);
}

void
Blitter::pokeBLTCPTL(uint16_t value)
{
    debug("pokeBLTCPTL(%X)\n", value);
    bltcpt = REPLACE_LO_WORD(bltcpt, value);
}

void
Blitter::pokeBLTDPTH(uint16_t value)
{
    debug("pokeBLTDPTH(%X)\n", value);
    bltdpt = REPLACE_HI_WORD(bltdpt, value & 0x7);
}

void
Blitter::pokeBLTDPTL(uint16_t value)
{
    debug("pokeBLTDPTL(%X)\n", value);
    bltdpt = REPLACE_LO_WORD(bltdpt, value);
}

void
Blitter::pokeBLTAFWM(uint16_t value)
{
    debug("pokeBLTAFWM(%X)\n", value);
    bltafwm = value;
}

void
Blitter::pokeBLTALWM(uint16_t value)
{
    debug("pokeBLTALWM(%X)\n", value);
    bltalwm = value;
}

void
Blitter::pokeBLTSIZE(uint16_t value)
{
    debug("pokeBLTSIZE(%X)\n", value);
    bltsize = value;
    
    if (bltLINE()) {
        // TODO
    } else {
        
        // Set width and height counters
        wcounter = bltsizeW();
        hcounter = bltsizeH();

        // Load micro instruction code
        loadMicrocode();
        
        // Start the blit
        event->scheduleRel(BLT_SLOT, DMA_CYCLES(1), BLT_EXECUTE);
    }
}

void
Blitter::pokeBLTADAT(uint16_t value)
{
    // Apply masks
    if (isFirstWord()) value &= bltafwm;
    if (isLastWord()) value &= bltalwm;

    ashift = (ashift << 16) | value;
}
    
void
Blitter::pokeBLTBDAT(uint16_t value)
{
   bshift = (bshift << 16) | value;
}

void
Blitter::pokeBLTCDAT(uint16_t value)
{
    chold = value;
}

/*
void
Blitter::reschedule(Cycle delta)
{
    amiga->dma.eventHandler.reschedule(BLT_SLOT, DMA_CYCLES(delta));
}

void
Blitter::cancelEvent()
{
    amiga->dma.eventHandler.cancelEvent(BLT_SLOT);
}
*/

void
Blitter::serviceEvent(EventID id, int64_t data)
{
    uint16_t instr;
    
    debug("Servicing Blitter event %d\n", id);
    
    switch (id) {
            
            /*
        case BLT_INIT:

            // MOVE THIS FUNCTIONALITY TO pokeBLTSIZE
            
            if (bltLINE()) {
                // TODO
            } else {
                
                // Load micro instruction code
                
                // Set counters
                wcounter = bltsizeW();
                hcounter = bltsizeH();
                
                // Schedule code execution
                amiga->dma.eventHandler.scheduleNextEvent(BLT_SLOT, DMA_CYCLES(1), BLT_EXECUTE);
            }
            break;
            */
            
        case BLT_EXECUTE:
            
            // Only proceed if Blitter DMA is disabled
            if (!amiga->dma.bltDMA()) {
                amiga->dma.eventHandler.reschedule(BLT_SLOT, INT32_MAX);
                break;
            }

            // Execute next Blitter micro instruction
            instr = microInstr[bltpc];
            debug("Executing micro instruction %d (%X)\n", bltpc, instr);

            
            if (instr & WRITE_D) {
                
                debug("WRITE_D\n");
                amiga->mem.pokeChip16(bltdpt, dhold);
                INC_OCS_PTR(bltdpt, 2 + (isLastWord() ? bltdmod : 0));
            }
            
            if (instr & HOLD_A) {
                
                debug("HOLD_A\n");
                // Emulate the barrel shifter on data path A
                ahold = (ashift >> bltASH()) & 0xFFFF;
            }

            if (instr & HOLD_B) {

                debug("HOLD_B\n");
                // Emulate the barrel shifter on data path B
                bhold = (bshift >> bltBSH()) & 0xFFFF;
            }
            
            if (instr & HOLD_D) {

                debug("HOLD_D\n");
                
                dhold = 0;

                // Run the minterm generator
                if (bltcon0 & 0b10000000) dhold |=  ahold &  bhold &  chold;
                if (bltcon0 & 0b01000000) dhold |=  ahold &  bhold & ~chold;
                if (bltcon0 & 0b00100000) dhold |=  ahold &  bhold &  chold;
                if (bltcon0 & 0b00010000) dhold |=  ahold & ~bhold & ~chold;
                if (bltcon0 & 0b00001000) dhold |= ~ahold &  bhold &  chold;
                if (bltcon0 & 0b00000100) dhold |= ~ahold &  bhold & ~chold;
                if (bltcon0 & 0b00000010) dhold |= ~ahold & ~bhold &  chold;
                if (bltcon0 & 0b00000001) dhold |= ~ahold & ~bhold & ~chold;
                
                // Run the fill logic circuit
                // TODO
                
                // Move to the next coordinate
                if (wcounter > 1) {
                    wcounter--;
                } else {
                    if (hcounter > 1) {
                        wcounter = bltsizeW();
                        hcounter--;
                    }
                }
            }
            
            if (instr & FETCH_A) {
                
                debug("FETCH_A\n");
                pokeBLTADAT(amiga->mem.peek16(bltapt));
                INC_OCS_PTR(bltapt, 2 + (isLastWord() ? bltamod : 0));
            }
            
            if (instr & FETCH_B) {

                debug("FETCH_B\n");
                pokeBLTBDAT(amiga->mem.peek16(bltbpt));
                INC_OCS_PTR(bltbpt, 2 + (isLastWord() ? bltbmod : 0));
            }

            if (instr & FETCH_C) {
                
                debug("FETCH_C\n");
                pokeBLTCDAT(amiga->mem.peek16(bltcpt));
                INC_OCS_PTR(bltcpt, 2 + (isLastWord() ? bltcmod : 0));
            }
            
            if ((instr & LOOPBACK) && (wcounter > 1 || hcounter > 1)) {

                debug("LOOPBACK\n");
                bltpc -= instr & 7;
            
            } else {
                
                bltpc++;
            }
            
            if (instr & BLTDONE) {
                
                debug("BLTDONE\n");
                // Terminate the Blitter
                amiga->dma.eventHandler.cancel(BLT_SLOT);
                
            } else {
            
                // Continue running the Blitter
                amiga->dma.eventHandler.reschedule(BLT_SLOT, DMA_CYCLES(1));
                break;
            }
            
        default:
            
            assert(false);
            break;
    }
}

void
Blitter::loadMicrocode()
{
    /* The following code is inspired by Table 6.2 of the HRM:
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
    
    uint8_t A = !!bltUSEA();
    uint8_t B = !!bltUSEB();
    uint8_t C = !!bltUSEC();
    uint8_t D = !!bltUSED();
    
    switch ((A << 3) | (B << 2) | (C << 1) | D) {
            
        case 0b1111: { // A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 D2
            
            uint16_t prog[] = {
                
                FETCH_A,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B,
                HOLD_D,
                
                FETCH_A,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B,
                WRITE_D | HOLD_D | LOOPBACK3,
                
                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }
            
        case 0b1110: { // A0 B0 C0 A1 B1 C1 A2 B2 C2
            
            uint16_t prog[] = {
                
                FETCH_A,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B,
                
                FETCH_A | HOLD_D,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B | LOOPBACK2,
                
                HOLD_D  | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }
   
        case 0b0000: { // -- -- -- --
            
            uint16_t prog[] = {
                
                BLTIDLE,
                BLTIDLE,
                BLTIDLE,
                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }
            
        default:
            
            fatalError("Unimplemented Blitter configuration\n");
            assert(false);
    }
    
    debug("Microcode loaded\n");

}
