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

        { &exact, sizeof(exact), PERSISTANT },

        { &bltcon0,       sizeof(bltcon0),       0 },
        { &bltcon1,       sizeof(bltcon1),       0 },

        { &bltapt,        sizeof(bltapt),        0 },
        { &bltbpt,        sizeof(bltbpt),        0 },
        { &bltcpt,        sizeof(bltcpt),        0 },
        { &bltdpt,        sizeof(bltdpt),        0 },

        { &bltafwm,       sizeof(bltafwm),       0 },
        { &bltalwm,       sizeof(bltalwm),       0 },

        { &bltsize,       sizeof(bltsize),       0 },

        { &bltamod,       sizeof(bltamod),       0 },
        { &bltbmod,       sizeof(bltbmod),       0 },
        { &bltcmod,       sizeof(bltcmod),       0 },
        { &bltdmod,       sizeof(bltdmod),       0 },

        { &anew,          sizeof(anew),          0 },
        { &bnew,          sizeof(bnew),          0 },
        { &aold,          sizeof(aold),          0 },
        { &bold,          sizeof(bold),          0 },
        { &ahold,         sizeof(ahold),         0 },
        { &bhold,         sizeof(bhold),         0 },
        { &chold,         sizeof(chold),         0 },
        { &dhold,         sizeof(dhold),         0 },
        { &ashift,        sizeof(ashift),        0 },
        { &bshift,        sizeof(bshift),        0 },

        { &xCounter,      sizeof(xCounter),      0 },
        { &yCounter,      sizeof(yCounter),      0 },

        { &bbusy,         sizeof(bbusy),         0 },
        { &bzero,         sizeof(bzero),         0 },
    });
    
    // Initialize fill pattern lookup tables
    
    // Inclusive fill
    for (unsigned carryIn = 0; carryIn < 2; carryIn++) {
        
        for (unsigned byte = 0; byte < 256; byte++) {
            
            uint8_t carry = carryIn;
            uint8_t inclPattern = byte;
            uint8_t exclPattern = byte;
            
            for (int bit = 0; bit < 8; bit++) {
                
                inclPattern |= carry << bit; // inclusive fill
                exclPattern ^= carry << bit; // exclusive fill
                
                if (byte & (1 << bit)) carry = !carry;
            }
            fillPattern[0][carryIn][byte] = inclPattern;
            fillPattern[1][carryIn][byte] = exclPattern;
            nextCarryIn[carryIn][byte] = carry;
        }
    }
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
Blitter::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.active  = _handler->isPending(BLT_SLOT);
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
    info.bbusy = bbusy;
    info.bzero = bzero;
    
    pthread_mutex_unlock(&lock);
}

void
Blitter::_dump()
{
    plainmsg("   bltcon0: %X\n", bltcon0);
    plainmsg("\n");
    plainmsg("            Shift A: %d\n", bltconASH());
    plainmsg("              Use A: %s\n", bltconUSEA() ? "yes" : "no");
    plainmsg("              Use B: %s\n", bltconUSEB() ? "yes" : "no");
    plainmsg("              Use C: %s\n", bltconUSEC() ? "yes" : "no");
    plainmsg("              Use D: %s\n", bltconUSED() ? "yes" : "no");
    plainmsg("\n");
    plainmsg("   bltcon1: %X\n", bltcon1);
    plainmsg("\n");
    plainmsg("            Shift B: %d\n", bltconBSH());
    plainmsg("                EFE: %s\n", bltconEFE() ? "yes" : "no");
    plainmsg("                IFE: %s\n", bltconIFE() ? "yes" : "no");
    plainmsg("                FCI: %s\n", bltconFCI() ? "yes" : "no");
    plainmsg("               DESC: %s\n", bltconDESC() ? "yes" : "no");
    plainmsg("               LINE: %s\n", bltconLINE() ? "yes" : "no");
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
    plainmsg("     bbusy: %s bzero: %s\n", bbusy ? "yes" : "no", bzero ? "yes" : "no");
}

BlitterInfo
Blitter::getInfo()
{
    BlitterInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

void
Blitter::pokeBLTCON0(uint16_t value)
{
    debug(2, "pokeBLTCON0(%X)\n", value);
    bltcon0 = value;
}

void
Blitter::pokeBLTCON1(uint16_t value)
{
    debug(2, "pokeBLTCON1(%X)\n", value);
    bltcon1 = value;
}

void
Blitter::pokeBLTAPTH(uint16_t value)
{
    debug(2, "pokeBLTAPTH(%X)\n", value);
    bltapt = REPLACE_HI_WORD(bltapt, value & 0x0007);
}

void
Blitter::pokeBLTAPTL(uint16_t value)
{
    debug(2, "pokeBLTAPTL(%X)\n", value);
    bltapt = REPLACE_LO_WORD(bltapt, value & 0xFFFE);
}

void
Blitter::pokeBLTBPTH(uint16_t value)
{
    debug(2, "pokeBLTBPTH(%X)\n", value);
    bltbpt = REPLACE_HI_WORD(bltbpt, value & 0x0007);
}

void
Blitter::pokeBLTBPTL(uint16_t value)
{
    debug(2, "pokeBLTBPTL(%X)\n", value);
    bltbpt = REPLACE_LO_WORD(bltbpt, value & 0xFFFE);
}

void
Blitter::pokeBLTCPTH(uint16_t value)
{
    debug(2, "pokeBLTCPTH(%X)\n", value);
    bltcpt = REPLACE_HI_WORD(bltcpt, value & 0x0007);
}

void
Blitter::pokeBLTCPTL(uint16_t value)
{
    debug(2, "pokeBLTCPTL(%X)\n", value);
    bltcpt = REPLACE_LO_WORD(bltcpt, value & 0xFFFE);
}

void
Blitter::pokeBLTDPTH(uint16_t value)
{
    debug(2, "pokeBLTDPTH(%X)\n", value);
    bltdpt = REPLACE_HI_WORD(bltdpt, value & 0x0007);
}

void
Blitter::pokeBLTDPTL(uint16_t value)
{
    debug(2, "pokeBLTDPTL(%X)\n", value);
    bltdpt = REPLACE_LO_WORD(bltdpt, value & 0xFFFE);
}

void
Blitter::pokeBLTAFWM(uint16_t value)
{
    debug(2, "pokeBLTAFWM(%X)\n", value);
    bltafwm = value;
}

void
Blitter::pokeBLTALWM(uint16_t value)
{
    debug(2, "pokeBLTALWM(%X)\n", value);
    bltalwm = value;
}

void
Blitter::pokeBLTSIZE(uint16_t value)
{
    debug(2, "pokeBLTSIZE(%X)\n", value);
    
    bltsize = value;
    bzero = true;
    bbusy = true;
    
    // WE ONLY DO FAST BLITS AT THE MOMENT
    _handler->scheduleRel(BLT_SLOT, DMA_CYCLES(1), BLT_FAST_BLIT);
    
    
    /*
    if (bltLINE()) {
        // TODO
    } else {
        
        // Set width and height counters
        wcounter = bltsizeW();
        hcounter = bltsizeH();
        
        // Load micro instruction code
        loadMicrocode();
        
        // Start the blit
        handler->scheduleRel(BLT_SLOT, DMA_CYCLES(1), BLT_EXECUTE);
    }
    */
}

void
Blitter::pokeBLTAMOD(uint16_t value)
{
    debug(2, "pokeBLTAMOD(%X)\n", value);
    bltamod = (int16_t)(value & 0xFFFE);
}
void
Blitter::pokeBLTBMOD(uint16_t value)
{
    debug(2, "pokeBLTBMOD(%X)\n", value);
    bltbmod = (int16_t)(value & 0xFFFE);
}

void
Blitter::pokeBLTCMOD(uint16_t value)
{
    debug(2, "pokeBLTCMOD(%X)\n", value);
    bltcmod = (int16_t)(value & 0xFFFE);
}

void
Blitter::pokeBLTDMOD(uint16_t value)
{
    debug(2, "pokeBLTDMOD(%X)\n", value);
    bltdmod = (int16_t)(value & 0xFFFE);
}

void
Blitter::pokeBLTADAT(uint16_t value)
{
    debug(2, "pokeBLTADAT(%X)\n", value);
    anew = value;
    /*
    // Apply masks
    if (isFirstWord()) value &= bltafwm;
    if (isLastWord()) value &= bltalwm;

    ashift = (ashift << 16) | value;
    */
}
    
void
Blitter::pokeBLTBDAT(uint16_t value)
{
    debug(2, "pokeBLTBDAT(%X)\n", value);
    bnew = value;
    /*
     bshift = (bshift << 16) | value;
     */
}

void
Blitter::pokeBLTCDAT(uint16_t value)
{
    debug(2, "pokeBLTCDAT(%X)\n", value);
    chold = value;
}

void
Blitter::serviceEvent(EventID id)
{
    uint16_t instr;
    
    debug(2, "Servicing Blitter event %d\n", id);
    
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
                amiga->agnus.eventHandler.scheduleNextEvent(BLT_SLOT, DMA_CYCLES(1), BLT_EXECUTE);
            }
            break;
            */
            
        case BLT_EXECUTE:
            
            // Only proceed if Blitter DMA is enabled
            if (!_agnus->bltDMA()) {
                _agnus->eventHandler.disable(BLT_SLOT);
                break;
            }

            // Execute next Blitter micro instruction
            instr = microInstr[bltpc];
            debug(2, "Executing micro instruction %d (%X)\n", bltpc, instr);
            bltpc++;
            
            if (instr & WRITE_D) {
                
                debug(2, "WRITE_D\n");
                _mem->pokeChip16(bltdpt, dhold);
                INC_OCS_PTR(bltdpt, 2 + (isLastWord() ? bltdmod : 0));
            }
            
            if (instr & HOLD_A) {
                
                debug(2, "HOLD_A\n");
                // Emulate the barrel shifter on data path A
                ahold = (ashift >> bltconASH()) & 0xFFFF;
            }

            if (instr & HOLD_B) {

                debug(2, "HOLD_B\n");
                // Emulate the barrel shifter on data path B
                bhold = (bshift >> bltconBSH()) & 0xFFFF;
            }
            
            if (instr & HOLD_D) {

                debug(2, "HOLD_D\n");
                
                // Run the minterm logic circuit
                doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF);
                
                // Run the fill logic circuit
                // TODO
                
                // Update the zero flag
                if (dhold) bzero = false;
                
                // Move to the next coordinate
                if (xCounter > 1) {
                    xCounter--;
                } else {
                    if (yCounter > 1) {
                        xCounter = bltsizeW();
                        yCounter--;
                    }
                }
            }
            
            if (instr & FETCH_A) {
                
                debug(2, "FETCH_A\n");
                // pokeBLTADAT(amiga->mem.peek16(bltapt));
                anew = _mem->peek16(bltapt);
                INC_OCS_PTR(bltapt, 2 + (isLastWord() ? bltamod : 0));
            }
            
            if (instr & FETCH_B) {

                debug(3, "FETCH_B\n");
                // pokeBLTBDAT(amiga->mem.peek16(bltbpt));
                bnew = _mem->peek16(bltbpt);
                INC_OCS_PTR(bltbpt, 2 + (isLastWord() ? bltbmod : 0));
            }

            if (instr & FETCH_C) {
                
                debug(3, "FETCH_C\n");
                // pokeBLTCDAT(amiga->mem.peek16(bltcpt));
                chold = _mem->peek16(bltcpt);
                INC_OCS_PTR(bltcpt, 2 + (isLastWord() ? bltcmod : 0));
            }
            
            if (instr & LOOPBACK) {
                
                debug(2, "LOOPBACK\n");
                uint16_t offset = instr & 0b111;
                
                if ((yCounter > 1 || xCounter > 1) && offset) {

                    // Move PC back to the beginning of the main cycle
                    bltpc = bltpc - offset - 1;
            
                } else {
                    
                    // The remaining code flushes the pipeline.
                    // The Blitter busy flag is already cleared.
                    bbusy = false;
                }
            }
            
            if (instr & BLTDONE) {
                
                debug(2, "BLTDONE\n");
                
                // Clear the Blitter busy flag
                bbusy = false;
                
                // Trigger the Blitter interrupt
                _handler->scheduleSecRel(IRQ_BLIT_SLOT, 0, IRQ_SET);
                
                // Terminate the Blitter
                _handler->cancel(BLT_SLOT);
                
            } else {
            
                // Continue running the Blitter
                _agnus->eventHandler.rescheduleRel(BLT_SLOT, DMA_CYCLES(1));
            }
            
            break;
            
        case BLT_FAST_BLIT:
            
            // Only proceed if Blitter DMA is enabled
            if (!_agnus->bltDMA()) {
                _agnus->eventHandler.disable(BLT_SLOT);
                break;
            }
            
            doFastBlit();
            break;
            
        default:
            
            assert(false);
            break;
    }
}

void
Blitter::loadMicrocode()
{
    bltpc = 0;

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
    
    uint8_t A = !!bltconUSEA();
    uint8_t B = !!bltconUSEB();
    uint8_t C = !!bltconUSEC();
    uint8_t D = !!bltconUSED();
    
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
        
            _dump();
            panic("Unimplemented Blitter configuration\n");
            assert(false);
    }
    
    debug(2, "Microcode loaded\n");

}

void
Blitter::doBarrelShifterA()
{
    uint16_t masked = anew;
    
    if (isFirstWord()) masked &= bltafwm;
    if (isLastWord())  masked &= bltalwm;
    
    debug(2, "first = %d last = %d masked = %X\n", isFirstWord(), isLastWord(), masked);
    
    if(bltconDESC()){
        ahold = (aold >> (16 - bltconASH())) | (masked << bltconASH());
    }else{
        ahold = (aold << (16 - bltconASH())) | (masked >> bltconASH());
    }
    
    /*
    if (bltDESC()) {
        uint32_t barrelA = HI_W_LO_W(anew & mask, aold);
        ahold = (barrelA >> (16 - bltASH())) & 0xFFFF;
    } else {
        uint32_t barrelA = HI_W_LO_W(aold, anew & mask);
        ahold = (barrelA >> bltASH()) & 0xFFFF;
    }
    */
}

void
Blitter::doBarrelShifterB()
{
    if(bltconDESC()) {
        bhold = (bold >> (16 - bltconBSH())) | (bnew << bltconBSH());
    } else {
        bhold = (bold << (16 - bltconBSH())) | (bnew >> bltconBSH());
    }
    
    /*
    if (bltDESC()) {
        uint32_t barrelB = HI_W_LO_W(bnew, bold);
        bhold = (barrelB >> (16 - bltBSH())) & 0xFFFF;
    } else {
        uint32_t barrelB = HI_W_LO_W(bold, bnew);
        bhold = (barrelB >> bltBSH()) & 0xFFFF;
    }
    */
}

uint16_t
Blitter::doMintermLogic(uint16_t a, uint16_t b, uint16_t c, uint8_t minterm)
{
    uint16_t result = 0;
    
    if (minterm & 0b10000000) result |=  a &  b &  c;
    if (minterm & 0b01000000) result |=  a &  b & ~c;
    if (minterm & 0b00100000) result |=  a & ~b &  c;
    if (minterm & 0b00010000) result |=  a & ~b & ~c;
    if (minterm & 0b00001000) result |= ~a &  b &  c;
    if (minterm & 0b00000100) result |= ~a &  b & ~c;
    if (minterm & 0b00000010) result |= ~a & ~b &  c;
    if (minterm & 0b00000001) result |= ~a & ~b & ~c;
    
    return result;
}

uint16_t
Blitter::doMintermLogicQuick(uint16_t a, uint16_t b, uint16_t c, uint8_t minterm)
{
    switch (minterm) {
        case 0: return 0;
        case 1: return (~c & ~b & ~a);
        case 2: return (c & ~b & ~a);
        case 3: return (~b & ~a);
        case 4: return (~c & b & ~a);
        case 5: return (~c & ~a);
        case 6: return (c & ~b & ~a) | (~c & b & ~a);
        case 7: return (~b & ~a) | (~c & ~a);
        case 8: return (c & b & ~a);
        case 9: return (~c & ~b & ~a) | (c & b & ~a);
        case 10: return (c & ~a);
        case 11: return (~b & ~a) | (c & ~a);
        case 12: return (b & ~a);
        case 13: return (~c & ~a) | (b & ~a);
        case 14: return (c & ~a) | (b & ~a);
        case 15: return (~a);
        case 16: return (~c & ~b & a);
        case 17: return (~c & ~b);
        case 18: return (c & ~b & ~a) | (~c & ~b & a);
        case 19: return (~b & ~a) | (~c & ~b);
        case 20: return (~c & b & ~a) | (~c & ~b & a);
        case 21: return (~c & ~a) | (~c & ~b);
        case 22: return (c & ~b & ~a) | (~c & b & ~a) | (~c & ~b & a);
        case 23: return (~b & ~a) | (~c & ~a) | (~c & ~b);
        case 24: return (c & b & ~a) | (~c & ~b & a);
        case 25: return (~c & ~b) | (c & b & ~a);
        case 26: return (c & ~a) | (~c & ~b & a);
        case 27: return (~b & ~a) | (c & ~a) | (~c & ~b);
        case 28: return (b & ~a) | (~c & ~b & a);
        case 29: return (~c & ~a) | (b & ~a) | (~c & ~b);
        case 30: return (c & ~a) | (b & ~a) | (~c & ~b & a);
        case 31: return (~a) | (~c & ~b);
        case 32: return (c & ~b & a);
        case 33: return (~c & ~b & ~a) | (c & ~b & a);
        case 34: return (c & ~b);
        case 35: return (~b & ~a) | (c & ~b);
        case 36: return (~c & b & ~a) | (c & ~b & a);
        case 37: return (~c & ~a) | (c & ~b & a);
        case 38: return (c & ~b) | (~c & b & ~a);
        case 39: return (~b & ~a) | (~c & ~a) | (c & ~b);
        case 40: return (c & b & ~a) | (c & ~b & a);
        case 41: return (~c & ~b & ~a) | (c & b & ~a) | (c & ~b & a);
        case 42: return (c & ~a) | (c & ~b);
        case 43: return (~b & ~a) | (c & ~a) | (c & ~b);
        case 44: return (b & ~a) | (c & ~b & a);
        case 45: return (~c & ~a) | (b & ~a) | (c & ~b & a);
        case 46: return (c & ~a) | (b & ~a) | (c & ~b);
        case 47: return (~a) | (c & ~b);
        case 48: return (~b & a);
        case 49: return (~c & ~b) | (~b & a);
        case 50: return (c & ~b) | (~b & a);
        case 51: return (~b);
        case 52: return (~c & b & ~a) | (~b & a);
        case 53: return (~c & ~a) | (~b & a);
        case 54: return (c & ~b) | (~c & b & ~a) | (~b & a);
        case 55: return (~b) | (~c & ~a);
        case 56: return (c & b & ~a) | (~b & a);
        case 57: return (~c & ~b) | (c & b & ~a) | (~b & a);
        case 58: return (c & ~a) | (~b & a);
        case 59: return (~b) | (c & ~a);
        case 60: return (b & ~a) | (~b & a);
        case 61: return (~c & ~a) | (b & ~a) | (~b & a);
        case 62: return (c & ~a) | (b & ~a) | (~b & a);
        case 63: return (~a) | (~b);
        case 64: return (~c & b & a);
        case 65: return (~c & ~b & ~a) | (~c & b & a);
        case 66: return (c & ~b & ~a) | (~c & b & a);
        case 67: return (~b & ~a) | (~c & b & a);
        case 68: return (~c & b);
        case 69: return (~c & ~a) | (~c & b);
        case 70: return (c & ~b & ~a) | (~c & b);
        case 71: return (~b & ~a) | (~c & ~a) | (~c & b);
        case 72: return (c & b & ~a) | (~c & b & a);
        case 73: return (~c & ~b & ~a) | (c & b & ~a) | (~c & b & a);
        case 74: return (c & ~a) | (~c & b & a);
        case 75: return (~b & ~a) | (c & ~a) | (~c & b & a);
        case 76: return (b & ~a) | (~c & b);
        case 77: return (~c & ~a) | (b & ~a) | (~c & b);
        case 78: return (c & ~a) | (b & ~a) | (~c & b);
        case 79: return (~a) | (~c & b);
        case 80: return (~c & a);
        case 81: return (~c & ~b) | (~c & a);
        case 82: return (c & ~b & ~a) | (~c & a);
        case 83: return (~b & ~a) | (~c & a);
        case 84: return (~c & b) | (~c & a);
        case 85: return (~c);
        case 86: return (c & ~b & ~a) | (~c & b) | (~c & a);
        case 87: return (~b & ~a) | (~c);
        case 88: return (c & b & ~a) | (~c & a);
        case 89: return (~c & ~b) | (c & b & ~a) | (~c & a);
        case 90: return (c & ~a) | (~c & a);
        case 91: return (~b & ~a) | (c & ~a) | (~c & a);
        case 92: return (b & ~a) | (~c & a);
        case 93: return (~c) | (b & ~a);
        case 94: return (c & ~a) | (b & ~a) | (~c & a);
        case 95: return (~a) | (~c);
        case 96: return (c & ~b & a) | (~c & b & a);
        case 97: return (~c & ~b & ~a) | (c & ~b & a) | (~c & b & a);
        case 98: return (c & ~b) | (~c & b & a);
        case 99: return (~b & ~a) | (c & ~b) | (~c & b & a);
        case 100: return (~c & b) | (c & ~b & a);
        case 101: return (~c & ~a) | (c & ~b & a) | (~c & b);
        case 102: return (c & ~b) | (~c & b);
        case 103: return (~b & ~a) | (~c & ~a) | (c & ~b) | (~c & b);
        case 104: return (c & b & ~a) | (c & ~b & a) | (~c & b & a);
        case 105: return (~c & ~b & ~a) | (c & b & ~a) | (c & ~b & a) | (~c & b & a);
        case 106: return (c & ~a) | (c & ~b) | (~c & b & a);
        case 107: return (~b & ~a) | (c & ~a) | (c & ~b) | (~c & b & a);
        case 108: return (b & ~a) | (c & ~b & a) | (~c & b);
        case 109: return (~c & ~a) | (b & ~a) | (c & ~b & a) | (~c & b);
        case 110: return (c & ~a) | (b & ~a) | (c & ~b) | (~c & b);
        case 111: return (~a) | (c & ~b) | (~c & b);
        case 112: return (~b & a) | (~c & a);
        case 113: return (~c & ~b) | (~b & a) | (~c & a);
        case 114: return (c & ~b) | (~b & a) | (~c & a);
        case 115: return (~b) | (~c & a);
        case 116: return (~c & b) | (~b & a);
        case 117: return (~c) | (~b & a);
        case 118: return (c & ~b) | (~c & b) | (~b & a);
        case 119: return (~b) | (~c);
        case 120: return (c & b & ~a) | (~b & a) | (~c & a);
        case 121: return (~c & ~b) | (c & b & ~a) | (~b & a) | (~c & a);
        case 122: return (c & ~a) | (~b & a) | (~c & a);
        case 123: return (~b) | (c & ~a) | (~c & a);
        case 124: return (b & ~a) | (~b & a) | (~c & a);
        case 125: return (~c) | (b & ~a) | (~b & a);
        case 126: return (c & ~a) | (b & ~a) | (~b & a) | (~c & a);
        case 127: return (~a) | (~b) | (~c);
        case 128: return (c & b & a);
        case 129: return (~c & ~b & ~a) | (c & b & a);
        case 130: return (c & ~b & ~a) | (c & b & a);
        case 131: return (~b & ~a) | (c & b & a);
        case 132: return (~c & b & ~a) | (c & b & a);
        case 133: return (~c & ~a) | (c & b & a);
        case 134: return (c & ~b & ~a) | (~c & b & ~a) | (c & b & a);
        case 135: return (~b & ~a) | (~c & ~a) | (c & b & a);
        case 136: return (c & b);
        case 137: return (~c & ~b & ~a) | (c & b);
        case 138: return (c & ~a) | (c & b);
        case 139: return (~b & ~a) | (c & ~a) | (c & b);
        case 140: return (b & ~a) | (c & b);
        case 141: return (~c & ~a) | (b & ~a) | (c & b);
        case 142: return (c & ~a) | (b & ~a) | (c & b);
        case 143: return (~a) | (c & b);
        case 144: return (~c & ~b & a) | (c & b & a);
        case 145: return (~c & ~b) | (c & b & a);
        case 146: return (c & ~b & ~a) | (~c & ~b & a) | (c & b & a);
        case 147: return (~b & ~a) | (~c & ~b) | (c & b & a);
        case 148: return (~c & b & ~a) | (~c & ~b & a) | (c & b & a);
        case 149: return (~c & ~a) | (~c & ~b) | (c & b & a);
        case 150: return (c & ~b & ~a) | (~c & b & ~a) | (~c & ~b & a) | (c & b & a);
        case 151: return (~b & ~a) | (~c & ~a) | (~c & ~b) | (c & b & a);
        case 152: return (c & b) | (~c & ~b & a);
        case 153: return (~c & ~b) | (c & b);
        case 154: return (c & ~a) | (~c & ~b & a) | (c & b);
        case 155: return (~b & ~a) | (c & ~a) | (~c & ~b) | (c & b);
        case 156: return (b & ~a) | (~c & ~b & a) | (c & b);
        case 157: return (~c & ~a) | (b & ~a) | (~c & ~b) | (c & b);
        case 158: return (c & ~a) | (b & ~a) | (~c & ~b & a) | (c & b);
        case 159: return (~a) | (~c & ~b) | (c & b);
        case 160: return (c & a);
        case 161: return (~c & ~b & ~a) | (c & a);
        case 162: return (c & ~b) | (c & a);
        case 163: return (~b & ~a) | (c & a);
        case 164: return (~c & b & ~a) | (c & a);
        case 165: return (~c & ~a) | (c & a);
        case 166: return (c & ~b) | (~c & b & ~a) | (c & a);
        case 167: return (~b & ~a) | (~c & ~a) | (c & a);
        case 168: return (c & b) | (c & a);
        case 169: return (~c & ~b & ~a) | (c & b) | (c & a);
        case 170: return (c);
        case 171: return (~b & ~a) | (c);
        case 172: return (b & ~a) | (c & a);
        case 173: return (~c & ~a) | (b & ~a) | (c & a);
        case 174: return (c) | (b & ~a);
        case 175: return (~a) | (c);
        case 176: return (~b & a) | (c & a);
        case 177: return (~c & ~b) | (~b & a) | (c & a);
        case 178: return (c & ~b) | (~b & a) | (c & a);
        case 179: return (~b) | (c & a);
        case 180: return (~c & b & ~a) | (~b & a) | (c & a);
        case 181: return (~c & ~a) | (~b & a) | (c & a);
        case 182: return (c & ~b) | (~c & b & ~a) | (~b & a) | (c & a);
        case 183: return (~b) | (~c & ~a) | (c & a);
        case 184: return (c & b) | (~b & a);
        case 185: return (~c & ~b) | (c & b) | (~b & a);
        case 186: return (c) | (~b & a);
        case 187: return (~b) | (c);
        case 188: return (b & ~a) | (~b & a) | (c & a);
        case 189: return (~c & ~a) | (b & ~a) | (~b & a) | (c & a);
        case 190: return (c) | (b & ~a) | (~b & a);
        case 191: return (~a) | (~b) | (c);
        case 192: return (b & a);
        case 193: return (~c & ~b & ~a) | (b & a);
        case 194: return (c & ~b & ~a) | (b & a);
        case 195: return (~b & ~a) | (b & a);
        case 196: return (~c & b) | (b & a);
        case 197: return (~c & ~a) | (b & a);
        case 198: return (c & ~b & ~a) | (~c & b) | (b & a);
        case 199: return (~b & ~a) | (~c & ~a) | (b & a);
        case 200: return (c & b) | (b & a);
        case 201: return (~c & ~b & ~a) | (c & b) | (b & a);
        case 202: return (c & ~a) | (b & a);
        case 203: return (~b & ~a) | (c & ~a) | (b & a);
        case 204: return (b);
        case 205: return (~c & ~a) | (b);
        case 206: return (c & ~a) | (b);
        case 207: return (~a) | (b);
        case 208: return (~c & a) | (b & a);
        case 209: return (~c & ~b) | (b & a);
        case 210: return (c & ~b & ~a) | (~c & a) | (b & a);
        case 211: return (~b & ~a) | (~c & a) | (b & a);
        case 212: return (~c & b) | (~c & a) | (b & a);
        case 213: return (~c) | (b & a);
        case 214: return (c & ~b & ~a) | (~c & b) | (~c & a) | (b & a);
        case 215: return (~b & ~a) | (~c) | (b & a);
        case 216: return (c & b) | (~c & a);
        case 217: return (~c & ~b) | (c & b) | (b & a);
        case 218: return (c & ~a) | (~c & a) | (b & a);
        case 219: return (~b & ~a) | (c & ~a) | (~c & a) | (b & a);
        case 220: return (b) | (~c & a);
        case 221: return (~c) | (b);
        case 222: return (c & ~a) | (b) | (~c & a);
        case 223: return (~a) | (~c) | (b);
        case 224: return (c & a) | (b & a);
        case 225: return (~c & ~b & ~a) | (c & a) | (b & a);
        case 226: return (c & ~b) | (b & a);
        case 227: return (~b & ~a) | (c & a) | (b & a);
        case 228: return (~c & b) | (c & a);
        case 229: return (~c & ~a) | (c & a) | (b & a);
        case 230: return (c & ~b) | (~c & b) | (b & a);
        case 231: return (~b & ~a) | (~c & ~a) | (c & a) | (b & a);
        case 232: return (c & b) | (c & a) | (b & a);
        case 233: return (~c & ~b & ~a) | (c & b) | (c & a) | (b & a);
        case 234: return (c) | (b & a);
        case 235: return (~b & ~a) | (c) | (b & a);
        case 236: return (b) | (c & a);
        case 237: return (~c & ~a) | (b) | (c & a);
        case 238: return (c) | (b);
        case 239: return (~a) | (c) | (b);
        case 240: return (a);
        case 241: return (~c & ~b) | (a);
        case 242: return (c & ~b) | (a);
        case 243: return (~b) | (a);
        case 244: return (~c & b) | (a);
        case 245: return (~c) | (a);
        case 246: return (c & ~b) | (~c & b) | (a);
        case 247: return (~b) | (~c) | (a);
        case 248: return (c & b) | (a);
        case 249: return (~c & ~b) | (c & b) | (a);
        case 250: return (c) | (a);
        case 251: return (~b) | (c) | (a);
        case 252: return (b) | (a);
        case 253: return (~c) | (b) | (a);
        case 254: return (c) | (b) | (a);
        default:  return 0xFFFF;
    }
}

void
Blitter::doFill(uint16_t &data, bool &carry)
{
    assert(carry == 0 || carry == 1);
    
    uint8_t dataHi = HI_BYTE(data);
    uint8_t dataLo = LO_BYTE(data);
    uint8_t exclusive = !!bltconEFE();
    
    // Remember: A fill operation is carried out from right to left
    uint8_t resultLo = fillPattern[exclusive][carry][dataLo];
    carry = nextCarryIn[carry][dataLo];
    uint8_t resultHi = fillPattern[exclusive][carry][dataHi];
    carry = nextCarryIn[carry][dataHi];
    
    data = HI_LO(resultHi, resultLo);
}
