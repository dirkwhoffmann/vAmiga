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
    bltLINE() ? doFastLineBlit() : doFastCopyBlit();
    
    // Clear the Blitter busy flag
    bbusy = false;
    
    // Trigger the Blitter interrupt
    handler->scheduleSecRel(IRQ_BLIT_SLOT, 0, IRQ_SET);
    
    // Terminate the Blitter
    handler->cancel(BLT_SLOT);
}

void
Blitter::doFastCopyBlit()
{
    uint64_t check1 = fnv_1a_init();
    uint64_t check2 = fnv_1a_init();
    
    copycount++;

    bltdebug = false; // (copycount == 1189);
    /*
    if (copycount == 958 || copycount == 959) {
        for (unsigned i = 0; i < 256; i+=16) {
            for (unsigned j = 0; j < 8; j+=2) {
                printf("%4X ", amiga->mem.peek16(0xC04A82+i+j));
            }
            printf("\n");
        }
    }
    */
    
    
    /*
    plainmsg("%d COPY BLIT (%d,%d) (%s)\n",
        bltcount++, bltsizeH(), bltsizeW(), bltDESC() ? "descending" : "ascending");
    */
    
    bool useA = bltUSEA();
    bool useB = bltUSEB();
    bool useC = bltUSEC();
    bool useD = bltUSED();
    
    // Setup increment and modulo counters
    int32_t incr = 2;
    int32_t amod = bltamod;
    int32_t bmod = bltbmod;
    int32_t cmod = bltcmod;
    int32_t dmod = bltdmod;
    
    // Reverse direction is descending mode
    if (bltDESC()) {
        incr = -incr;
        amod = -amod;
        bmod = -bmod;
        cmod = -cmod;
        dmod = -dmod;
    }
    
    if (bltdebug) plainmsg("blit %d: A-%06x (%d) B-%06x (%d) C-%06x (%d) D-%06x (%d) W-%d H-%d\n",
                           copycount, bltapt, bltamod, bltbpt, bltbmod, bltcpt, bltcmod, bltdpt, bltdmod,
                           bltsizeW(), bltsizeH());

    for (yCounter = bltsizeH(); yCounter >= 1; yCounter--) {
    
        for (xCounter = bltsizeW(); xCounter >= 1;  xCounter--) {
            
            debug(2, "(%d,%d)\n", yCounter, xCounter);
            
            // Fetch A, B, and C
            if (useA) {
                anew = amiga->mem.peek16(bltapt & ~1);
                if (bltdebug) plainmsg("    A = peek(%X) = %X\n", bltapt, anew);
                INC_OCS_PTR(bltapt, incr + (isLastWord() ? amod : 0));
            }
            if (useB) {
                bnew = amiga->mem.peek16(bltbpt & ~1);
                if (bltdebug) plainmsg("    B = peek(%X) = %X\n", bltbpt, bnew);
                INC_OCS_PTR(bltbpt, incr + (isLastWord() ? bmod : 0));
            }
            if (useC) {
                chold = amiga->mem.peek16(bltcpt & ~1);
                if (bltdebug) plainmsg("    C = peek(%X) = %X\n", bltcpt, chold);
                INC_OCS_PTR(bltcpt, incr + (isLastWord() ? cmod : 0));
            }
            
            // Compute AND mask for data path A
            uint16_t mask = 0xFFFF;
            if (isFirstWord()) mask &= bltafwm;
            if (isLastWord()) mask &= bltalwm;
            if (bltdebug) plainmsg("    first = %d last = %d mask = %X\n", isFirstWord(), isLastWord(), mask);
            
            // Run the two barrel shifters
            if (bltdebug) plainmsg("    ash = %d bsh = %d\n", bltASH(), bltBSH());
            doBarrelShifterA();
            doBarrelShifterB();
            aold = anew & mask;
            bold = bnew;

            // Run the minterm generator
            if (bltdebug) plainmsg("    ahold = %X bhold = %X chold = %X bltcon0 = %X (hex)\n", ahold, bhold, chold, bltcon0);
            doMintermLogic();
            
            // Update the zero flag
            if (dhold) bzero = false;
        
            // Write D
            if (useD) {
                amiga->mem.pokeChip16(bltdpt & ~1, dhold);
                if (bltdebug) plainmsg("    D: poke(%X), %X\n", bltdpt & ~1, dhold);
                check1 = fnv_1a_it(check1, dhold);
                check2 = fnv_1a_it(check2, bltdpt & ~1);
                INC_OCS_PTR(bltdpt, incr + (isLastWord() ? dmod : 0));
            }
        }
    }
    printf("Blitter %d: (%d,%d) (%d%d%d%d) %llX %llX\n", copycount, bltsizeW(), bltsizeH(), useA, useB, useC, useD, check1, check2);
}

uint16_t logicFunction(int minterm,uint16_t wordA, uint16_t wordB, uint16_t wordC) {
    
    //Not section
    uint16_t notA = ~wordA;
    uint16_t notB = ~wordB;
    uint16_t notC = ~wordC;
    uint16_t channelD = 0;
    
    //Logic Section
    // printf("ahold = %d bhold = %d chold = %d bltcon0 = %X (hex)\n", wordA, wordB, wordC, minterm);
    
    if(minterm & 0x80){
        channelD =            (wordA & wordB & wordC);
        // printf("    channelD = %d\n", channelD);
    }
    if(minterm & 0x40){
        channelD = channelD | (wordA & wordB & notC);
        // printf("    channelD = %d\n", channelD);
    }
    if(minterm & 0x20){
        channelD = channelD | (wordA & notB  & wordC);
        // printf("    channelD = %d\n", channelD);
    }
    if(minterm & 0x10){
        channelD = channelD | (wordA & notB  & notC);
        // printf("    channelD = %d\n", channelD);
    }
    if(minterm & 0x08){
        channelD = channelD | (notA  & wordB & wordC);
        // printf("    channelD = %d\n", channelD);
    }
    if(minterm & 0x04){
        channelD = channelD | (notA  & wordB & notC);
        // printf("    channelD = %d\n", channelD);
    }
    if(minterm & 0x02){
        channelD = channelD | (notA  & notB  & wordC);
        // printf("    channelD = %d\n", channelD);
    }
    if(minterm & 0x01){
        channelD = channelD | (notA  & notB  & notC);
        // printf("    channelD = %d\n", channelD);
    }
    
    return channelD;
}

void
Blitter::doFastLineBlit()
{
    int16_t bltamod = (int16_t)this->bltamod;
    int16_t bltbmod = (int16_t)this->bltbmod;
    int16_t bltcmod = (int16_t)this->bltcmod;
    // int16_t bltdmod = (int16_t)this->bltdmod;

    uint64_t check = fnv_1a_init();
    linecount++;

    bltdebug = false;
    
    // Adapted from Omega Amiga Emulator
    int octCode = (bltcon1 >> 2) & 7;
    int length =  bltsizeH();
    int inc1 = bltamod; // 4(dy - dx)
    int D = (int16_t)bltapt;     // start value of 4dy - 2dx
    // uint16_t* chipramW = internal.chipramW;
    
    int planeAddr = bltcpt & 0x1FFFFE; //word address
    
    int planeMod = bltcmod;
    int inc2 = bltbmod;
    int d=0;
    
    int startPixel = bltcon0 >> 12;
    
    int oneDot = (bltcon1 >> 1) & 1;    // I don't support one dot mode yet
    
    if(oneDot==1){
        printf("No Single pixel per H-line mode yet\n");
    }
    
    int minterm = bltcon0 & 255; //0xCA = normal 0x4A = XOR
    
    int patternShift = bltcon1 >>12;
    uint16_t pattern = bnew; //  bltbdat;
    
    pattern = (pattern >> patternShift) | (pattern << (16 - patternShift));
    
    int addr=0;//running address
    
    if (bltdebug) printf("[%d] Octant %d: dx - %d dy - %d @ 0x%06x\n",linecount,octCode,length,inc2/4,bltcpt);
    
    switch(octCode){
            
        case 0:
            
            for(int i=0;i<length;++i){
                
                int offset = d+startPixel;
                addr = (planeAddr +(offset>>3)+(i*planeMod)) & 0x1FFFFE; //  >> 1;
                
                //Pixel plot
                uint16_t pixel = amiga->mem.peek16(addr); // chipramW[addr];
                // debug("0: peek(%d) = %d\n", addr, pixel);
                pixel = logicFunction(minterm,0x8000 >> (offset&15),pattern,pixel);
                // debug("0: pixel = %d\n", pixel);
                amiga->mem.pokeChip16(addr, pixel);
                // debug("0: poke(%d), %d\n", addr, pixel);
                check = fnv_1a_it(check, addr);
                check = fnv_1a_it(check, pixel);

                if(D>0){
                    D = D + inc1;
                    d = d + 1;
                    
                }else{
                    D = D + inc2;
                }
                
            }
            break;
            
        case 1:
            for(int i=0;i<length;++i){
                
                int offset = d+startPixel;
                addr =(planeAddr +(offset>>3)-(i*planeMod)) & 0x1FFFFE; // >> 1;
                
                //Pixel plot
                uint16_t pixel = amiga->mem.peek16(addr); // chipramW[addr];
                pixel = logicFunction(minterm,0x8000 >> (offset&15),pattern,pixel);
                // debug("1: poke(%d), %d\n", addr, pixel);
                amiga->mem.pokeChip16(addr, pixel);
                check = fnv_1a_it(check, addr);
                check = fnv_1a_it(check, pixel);
                
                if(D>0){
                    D = D + inc1;
                    d = d + 1;
                    
                }else{
                    D = D + inc2;
                }
                
            }
            break;
            
        case 2:
            startPixel = 15 - startPixel;
            planeAddr +=1;
            for(int i=0;i<length;++i){
                
                int offset = d+startPixel;
                addr = (planeAddr - (offset>>3)+(i*planeMod)) & 0x1FFFFE; // >>1;
                if (bltdebug) printf("2: planeAddr = %d offset = %d d = %d planeMod = %d D = %d bltapt = %d inc1 = %d inc2 = %d\n", planeAddr, offset, d, planeMod, D, bltapt, inc1, inc2);
                
                //Pixel plot
                uint16_t pixel = amiga->mem.peek16(addr); // chipramW[addr];
                if (bltdebug) printf("2: peek(%d) = %d\n", addr, pixel);
                pixel = logicFunction(minterm,0x0001 << (offset&15),pattern,pixel);
                if (bltdebug) printf("2: poke(%d), %d\n", addr, pixel);
                amiga->mem.pokeChip16(addr, pixel);
                check = fnv_1a_it(check, addr);
                check = fnv_1a_it(check, pixel);
                
                if(D>0){
                    D = D + inc1;
                    d = d + 1;
                    
                }else{
                    D = D + inc2;
                }
                
            }
            break;
            
        case 3:
            
            for(int i=0;i<length;++i){
                
                int offset =d+startPixel;
                addr =(planeAddr +(offset>>3)-(i*planeMod)) & 0x1FFFFE; //>>1;
                
                //Pixel plot
                uint16_t pixel = amiga->mem.peek16(addr); // chipramW[addr];
                pixel = logicFunction(minterm,0x8000 >> (offset&15),pattern,pixel);
                // debug("3: poke(%d), %d\n", addr, pixel);
                amiga->mem.pokeChip16(addr, pixel);
                check = fnv_1a_it(check, addr);
                check = fnv_1a_it(check, pixel);
                
                if(D>0){
                    D = D + inc1;
                    d = d - 1;
                    
                }else{
                    D = D + inc2;
                }
                
            }
            break;
            
        case 4:
            for(int i=0;i<length;++i){
                
                int offset = i+startPixel;
                addr =(planeAddr +(offset>>3)+(d*planeMod)) & 0x1FFFFE; // >>1;
                
                //Pixel plot
                uint16_t pixel = amiga->mem.peek16(addr); // chipramW[addr];
                pixel = logicFunction(minterm,0x8000 >> (offset&15),pattern,pixel);
                // debug("4: poke(%d), %d\n", addr, pixel);
                amiga->mem.pokeChip16(addr, pixel);
                check = fnv_1a_it(check, addr);
                check = fnv_1a_it(check, pixel);
                
                if(D>0){
                    D = D + inc1;
                    d = d + 1;
                    
                }else{
                    D = D + inc2;
                }
                
            }
            break;
            
        case 5:
            startPixel = 15 - startPixel;
            planeAddr +=1;
            for(int i=0;i<length;++i){
                
                int offset = i+startPixel;
                addr = (planeAddr - (offset>>3)+(d*planeMod)) & 0x1FFFFE; //>>1;
                // debug("5: planeAddr = %d offset = %d d = %d planeMod = %d D = %d bltapt = %d inc1 = %d inc2 = %d\n", planeAddr, offset, d, planeMod, D, bltapt, inc1, inc2);

                
                //Pixel plot
                uint16_t pixel = amiga->mem.peek16(addr); // chipramW[addr];
                // debug("5: peek(%X) = %X\n", addr, pixel);
                pixel = logicFunction(minterm,0x0001 << (offset&15),pattern,pixel);
                // debug("5: poke(%X), %X\n", addr, pixel);
                amiga->mem.pokeChip16(addr, pixel);
                check = fnv_1a_it(check, addr);
                check = fnv_1a_it(check, pixel);
                
                if(D>0){
                    D = D + inc1;
                    d = d + 1;
                    
                }else{
                    D = D + inc2;
                }
                
            }
            break;
            
        case 6:
            for(int i=0;i<length;++i){
                
                int offset = i+startPixel;
                addr =(planeAddr +(offset>>3)-(d*planeMod)) & 0x1FFFFE; // >>1;
                
                //Pixel plot
                uint16_t pixel = amiga->mem.peek16(addr); // chipramW[addr];
                if (bltdebug) printf("    planeAddr = %X offset = %X planeMod = %X pixel = %X\n",planeAddr,offset,planeMod, pixel);
                pixel = logicFunction(minterm,0x8000 >> (offset&15),pattern,pixel);
                // debug("6: poke(%d), %d\n", addr, pixel);
                if (bltdebug) printf("    pixel = %X\n",pixel);
                amiga->mem.pokeChip16(addr, pixel);
                check = fnv_1a_it(check, addr);
                check = fnv_1a_it(check, pixel);
                
                if(D>0){
                    D = D + inc1;
                    d = d + 1;
                    
                }else{
                    D = D + inc2;
                }
                
            }
            break;
            
        case 7:
            startPixel = 15 - startPixel;
            planeAddr +=1;
            for(int i=0;i<length;++i){
                
                int offset = i+startPixel;
                addr = (planeAddr - (offset>>3)-(d*planeMod)) & 0x1FFFFE; // >>1;
                
                //Pixel plot
                uint16_t pixel = amiga->mem.peek16(addr); // chipramW[addr];
                pixel = logicFunction(minterm,0x0001 << (offset&15),pattern,pixel);
                // debug("7: poke(%d), %d\n", addr, pixel);
                amiga->mem.pokeChip16(addr, pixel);
                check = fnv_1a_it(check, addr);
                check = fnv_1a_it(check, pixel);
                
                if(D>0){
                    D = D + inc1;
                    d = d + 1;
                    
                }else{
                    D = D + inc2;
                }
                
            }
            break;
            
        default:
            break;
            
    }
    
    
    // chipset->bltcpt =addr;  // update cpt with the last known address... nothing should rely on this...
    bltcpt = addr;
    
    // chipset->bltsizh = 0; // all done;
    // chipset->bltsizv = 0; // all done;
    bltsize = 0;
    
    printf("Lineblitter %d (%d) %llX\n", linecount, octCode, check);
}
