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
    uint32_t check1 = fnv_1a_init32();
    uint32_t check2 = fnv_1a_init32();
    
    copycount++;

    bltdebug = (copycount == 3);

    /*
    plainmsg("%d COPY BLIT (%d,%d) (%s)\n",
        bltcount++, bltsizeH(), bltsizeW(), bltDESC() ? "descending" : "ascending");
    */
    
    uint16_t xmax = bltsizeW();
    uint16_t ymax = bltsizeH();
    
    bool useA = bltUSEA();
    bool useB = bltUSEB();
    bool useC = bltUSEC();
    bool useD = bltUSED();
    
    bool descending = bltDESC();
    
    // Setup shift, increment and modulo offsets
    int     incr = 2;
    int     ash  = bltASH();
    int     bsh  = bltBSH();
    int32_t amod = bltamod;
    int32_t bmod = bltbmod;
    int32_t cmod = bltcmod;
    int32_t dmod = bltdmod;
    
    printf("BLITTER Blit %d (%d,%d) (%d%d%d%d) %X %X %X %X %s\n",
    copycount, bltsizeW(), bltsizeH(), useA, useB, useC, useD,
    bltapt, bltbpt, bltcpt, bltdpt, "");
    
    // Reverse direction is descending mode
    if (bltDESC()) {
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
    
    for (int y = 0; y < ymax; y++) {
        
        // Apply the "first word mask" in the first iteration
        uint16_t mask = bltafwm;
        
        for (int x = 0; x < xmax; x++) {
            
            // Apply the "last word mask" in the last iteration
            if (x == xmax - 1) mask &= bltalwm;

            // Fetch A
            if (useA) {
                anew = amiga->mem.peek16(bltapt);
                if (bltdebug) plainmsg("    A = peek(%X) = %X\n", bltapt, anew);
                INC_OCS_PTR(bltapt, incr);
            }
            
            // Fetch B
            if (useB) {
                bnew = amiga->mem.peek16(bltbpt);
                if (bltdebug) plainmsg("    B = peek(%X) = %X\n", bltbpt, bnew);
                INC_OCS_PTR(bltbpt, incr);
            }
            
            // Fetch C
            if (useC) {
                chold = amiga->mem.peek16(bltcpt);
                if (bltdebug) plainmsg("    C = peek(%X) = %X\n", bltcpt, chold);
                INC_OCS_PTR(bltcpt, incr);
            }
            
            // Run the barrel shifters on data path A and B
            if (bltdebug) plainmsg("    ash = %d bsh = %d\n", bltASH(), bltBSH());
            if (descending) {
                ahold = HI_W_LO_W(anew & mask, aold) >> ash;
                bhold = HI_W_LO_W(bnew, bold) >> bsh;
            } else {
                ahold = HI_W_LO_W(aold, anew & mask) >> ash;
                bhold = HI_W_LO_W(bold, bnew) >> bsh;
            }
            aold = anew & mask;
            bold = bnew;
            
            // Run the minterm logic circuit
            if (bltdebug) plainmsg("    ahold = %X bhold = %X chold = %X bltcon0 = %X (hex)\n", ahold, bhold, chold, bltcon0);
            dhold = doMintermLogicQuick();
            assert(dhold == doMintermLogic());
            
            // Update the zero flag
            if (dhold) bzero = false;
        
            // Write D
            if (useD) {
                amiga->mem.pokeChip16(bltdpt, dhold);
                if (bltdebug) plainmsg("    D: poke(%X), %X\n", bltdpt, dhold);
                check1 = fnv_1a_it32(check1, dhold);
                check2 = fnv_1a_it32(check2, bltdpt);
                // plainmsg("    check1 = %X check2 = %X\n", check1, check2);
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
    
    printf("BLITTER check1: %x check2: %x\n", check1, check2);
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

    uint32_t check = fnv_1a_init32();
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
                check = fnv_1a_it32(check, addr);
                check = fnv_1a_it32(check, pixel);

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
                check = fnv_1a_it32(check, addr);
                check = fnv_1a_it32(check, pixel);
                
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
                check = fnv_1a_it32(check, addr);
                check = fnv_1a_it32(check, pixel);
                
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
                check = fnv_1a_it32(check, addr);
                check = fnv_1a_it32(check, pixel);
                
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
                check = fnv_1a_it32(check, addr);
                check = fnv_1a_it32(check, pixel);
                
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
                check = fnv_1a_it32(check, addr);
                check = fnv_1a_it32(check, pixel);
                
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
                check = fnv_1a_it32(check, addr);
                check = fnv_1a_it32(check, pixel);
                
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
                check = fnv_1a_it32(check, addr);
                check = fnv_1a_it32(check, pixel);
                
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
    
    printf("Lineblitter %d (%d) %X\n", linecount, octCode, check);
}
