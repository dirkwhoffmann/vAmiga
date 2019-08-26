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
Blitter::_initialize()
{
    mem = &amiga->mem;
    agnus = &amiga->agnus;
}

void
Blitter::_powerOn()
{
}

void
Blitter::_reset()
{
    RESET_SNAPSHOT_ITEMS

    copycount = 0;
    linecount = 0;
}

void
Blitter::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.active  = agnus->isPending<BLT_SLOT>();
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
    plainmsg("  Accuracy: %d\n", accuracy);
    plainmsg("\n");
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
    plainmsg("             height: %d\n", bltsizeH);
    plainmsg("              width: %d\n", bltsizeW);
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
    debug(BLTREG_DEBUG, "pokeBLTCON0(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltcon0 = value;
}

void
Blitter::pokeBLTCON1(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTCON1(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltcon1 = value;
}

void
Blitter::pokeBLTAPTH(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTAPTH(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltapt = REPLACE_HI_WORD(bltapt, value & 0x0007);
}

void
Blitter::pokeBLTAPTL(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTAPTL(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltapt = REPLACE_LO_WORD(bltapt, value & 0xFFFE);
}

void
Blitter::pokeBLTBPTH(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTBPTH(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltbpt = REPLACE_HI_WORD(bltbpt, value & 0x0007);
}

void
Blitter::pokeBLTBPTL(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTBPTL(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltbpt = REPLACE_LO_WORD(bltbpt, value & 0xFFFE);
}

void
Blitter::pokeBLTCPTH(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTCPTH(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltcpt = REPLACE_HI_WORD(bltcpt, value & 0x0007);
}

void
Blitter::pokeBLTCPTL(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTCPTL(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltcpt = REPLACE_LO_WORD(bltcpt, value & 0xFFFE);
}

void
Blitter::pokeBLTDPTH(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTDPTH(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltdpt = REPLACE_HI_WORD(bltdpt, value & 0x0007);
}

void
Blitter::pokeBLTDPTL(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTDPTL(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltdpt = REPLACE_LO_WORD(bltdpt, value & 0xFFFE);
}

void
Blitter::pokeBLTAFWM(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTAFWM(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltafwm = value;
}

void
Blitter::pokeBLTALWM(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTALWM(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltalwm = value;
}

void
Blitter::pokeBLTSIZE(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTSIZE(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    
    bltsize = value;
    bltsizeW = (bltsize & 0x3F) ? (bltsize & 0x3F) : 64;
    bltsizeH = (bltsize >> 6) ? (bltsize >> 6) : 1024;
    remaining = bltsizeW * bltsizeH;
    cntA = cntB = cntC = cntD = bltsizeW;

    // bzero = true;
    // bbusy = true;

    // Schedule the blit operation
    if (agnus->bltDMA()) {
        agnus->scheduleRel<BLT_SLOT>(DMA_CYCLES(0), BLT_START);
    } else {
        agnus->scheduleAbs<BLT_SLOT>(NEVER, BLT_START);
    }
}

void
Blitter::pokeBLTAMOD(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTAMOD(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltamod = (int16_t)(value & 0xFFFE);
}
void
Blitter::pokeBLTBMOD(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTBMOD(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltbmod = (int16_t)(value & 0xFFFE);
}

void
Blitter::pokeBLTCMOD(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTCMOD(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltcmod = (int16_t)(value & 0xFFFE);
}

void
Blitter::pokeBLTDMOD(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTDMOD(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bltdmod = (int16_t)(value & 0xFFFE);
}

void
Blitter::pokeBLTADAT(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTADAT(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
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
    debug(BLTREG_DEBUG, "pokeBLTBDAT(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    bnew = value;
    /*
     bshift = (bshift << 16) | value;
     */
}

void
Blitter::pokeBLTCDAT(uint16_t value)
{
    debug(BLTREG_DEBUG, "pokeBLTCDAT(%X)\n", value);
    if (bbusy) debug(BLTREG_DEBUG, "Blitter is running");
    chold = value;
}

void
Blitter::pokeDMACON(uint16_t oldValue, uint16_t newValue)
{
    bool oldBltDma = (oldValue & (DMAEN | BLTEN)) == (DMAEN | BLTEN);
    bool newBltDma = (newValue & (DMAEN | BLTEN)) == (DMAEN | BLTEN);

    // Check if Blitter DMA got switched on
    if (!oldBltDma && newBltDma) {

        // Perform pending blit operation (if any)
        if (agnus->hasEvent<BLT_SLOT>(BLT_START)) {
            agnus->scheduleRel<BLT_SLOT>(DMA_CYCLES(0), BLT_START);
        }
    }
}

void
Blitter::serviceEvent(EventID id)
{
    switch (id) {

        case BLT_START:

            startBlit();
            break;

        case BLT_EXEC_SLOW:

            executeSlowBlitter();
            break;

        case BLT_EXEC_FAST:

            executeFastBlitter();
            break;

        default:
            
            assert(false);
            break;
    }
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

    debug(BLT_DEBUG, "data = %X carry = %X\n", data, carry);
    
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

void
Blitter::startBlit()
{
    // assert(!bbusy);

    bzero = true;
    bbusy = true;

    // Based on the accuracy level, we run the slow or the fast Blitter
    bool useSlowBlitter = accuracy >= 2;

    check1 = fnv_1a_init32();
    check2 = fnv_1a_init32();

    if (bltconLINE()) {

        linecount++;
        plaindebug(BLT_CHECKSUM, "BLITTER Line %d (%d,%d) (%d%d%d%d) (%d %d %d %d) %x %x %x %x\n",
                   linecount, bltsizeW, bltsizeH,
                   bltconUSEA(), bltconUSEB(), bltconUSEC(), bltconUSED(),
                   bltamod, bltbmod, bltcmod, bltdmod,
                   bltapt, bltbpt, bltcpt, bltdpt);

        //REMOVE ASAP
        debugLevel = 1;
        
        useSlowBlitter ? beginSlowLineBlit() : beginFastLineBlit();

    } else {

        copycount++;
        if (bltsizeW != 1 || bltsizeH != 4) {
        debug(BLT_CHECKSUM, "BLITTER Blit %d (%d,%d) (%d%d%d%d) (%d %d %d %d) %x %x %x %x %s%s\n",
                   copycount, bltsizeW, bltsizeH,
                   bltconUSEA(), bltconUSEB(), bltconUSEC(), bltconUSED(),
                   bltamod, bltbmod, bltcmod, bltdmod,
                   bltapt, bltbpt, bltcpt, bltdpt,
                   bltconDESC() ? "D" : "", bltconFE() ? "F" : "");
        }
        //REMOVE ASAP
        debugLevel = 1; // (copycount == 15) ? 2 : 1;

        useSlowBlitter ? beginSlowCopyBlit() : beginFastCopyBlit();
    }
}

void
Blitter::terminate()
{
    // Clear the Blitter busy flag
    bbusy = false;

    // Trigger the Blitter interrupt
    agnus->scheduleRel<IRQ_BLIT_SLOT>(0, IRQ_SET);

    // Clear the Blitter slot
    agnus->cancel<BLT_SLOT>();

    // Dump checksums if requested
    if (bltsizeW != 1 || bltsizeH != 4) {
    debug(BLT_CHECKSUM, "BLITTER check1: %x check2: %x ABCD: %x %x %x %x\n", check1, check2, bltapt, bltbpt, bltcpt, bltdpt);
    }
}

void
Blitter::kill()
{
    // assert(!bbusy);
    // assert(!agnus->isPending<BLT_SLOT>());

    // Clear the Blitter busy flag
    bbusy = false;

    // Clear the Blitter slot
    agnus->cancel<BLT_SLOT>();
}
