// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "IOUtils.h"

namespace vamiga {

void
Blitter::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        dumpConfig(os);
    }

    if (category == Category::Registers) {

        os << tab("BLTCON0") << hex(bltcon0) << std::endl;
        os << tab("ASH") << hex(bltconASH()) << std::endl;
        os << tab("USEA") << bol(bltconUSEA()) << std::endl;
        os << tab("USEB") << bol(bltconUSEB()) << std::endl;
        os << tab("USEC") << bol(bltconUSEC()) << std::endl;
        os << tab("USED") << bol(bltconUSED()) << std::endl;
        os << std::endl;
        os << tab("BLTCON1") << hex(bltcon1) << std::endl;
        os << tab("BSH") << hex(bltconBSH()) << std::endl;
        os << tab("EFE") << bol(bltconEFE()) << std::endl;
        os << tab("IFE") << bol(bltconIFE()) << std::endl;
        os << tab("FCI") << bol(bltconFCI()) << std::endl;
        os << tab("DESC") << bol(bltconDESC()) << std::endl;
        os << tab("LINE") << bol(bltconLINE()) << std::endl;
        os << std::endl;
        os << tab("BLTSIZEH") << hex(bltsizeV) << std::endl;
        os << tab("BLTSIZEW") << hex(bltsizeH) << std::endl;
        os << std::endl;
        os << tab("BLTAPT") << hex(bltapt) << std::endl;
        os << tab("BLTBPT") << hex(bltbpt) << std::endl;
        os << tab("BLTCPT") << hex(bltcpt) << std::endl;
        os << tab("BLTDPT") << hex(bltdpt) << std::endl;
        os << tab("BLTAFWM") << hex(bltafwm) << std::endl;
        os << tab("BLTALWM") << hex(bltalwm) << std::endl;
        os << tab("BLTAMOD") << dec(bltamod) << std::endl;
        os << tab("BLTBMOD") << dec(bltbmod) << std::endl;
        os << tab("BLTCMOD") << dec(bltcmod) << std::endl;
        os << tab("BLTDMOD") << dec(bltdmod) << std::endl;
    }

    if (category == Category::State) {
        
        os << tab("Iteration") << dec(iteration) << std::endl;
        os << tab("Micro instruction PC") << dec(bltpc) << std::endl;
        os << tab("X counter") << dec(xCounter) << std::endl;
        os << tab("Y counter") << dec(yCounter) << std::endl;
        os << tab("A channel counter") << dec(cntA) << std::endl;
        os << tab("B channel counter") << dec(cntB) << std::endl;
        os << tab("C channel counter") << dec(cntC) << std::endl;
        os << tab("D channel counter") << dec(cntD) << std::endl;
        os << tab("D channel lock") << bol(lockD) << std::endl;
        os << tab("Fill carry") << dec(fillCarry) << std::endl;
        os << tab("Mask") << hex(mask) << std::endl;
        os << std::endl;
        os << tab("ANEW") << hex(anew) << std::endl;
        os << tab("BNEW") << hex(bnew) << std::endl;
        os << tab("AHOLD") << hex(ahold) << std::endl;
        os << tab("BHOLD") << hex(bhold) << std::endl;
        os << tab("CHOLD") << hex(chold) << std::endl;
        os << tab("DHOLD") << hex(dhold) << std::endl;
        os << tab("SHIFT") << hex(ashift) << std::endl;
        os << tab("BBUSY") << bol(bbusy) << std::endl;
        os << tab("BZERO") << bol(bzero) << std::endl;
    }
}

void
Blitter::cacheInfo(BlitterInfo &info) const
{
    SYNCHRONIZED
    
    auto minterm = bltconLF();
    auto mintermOut = doMintermLogic(ahold, bhold, chold, (u8)minterm);
    
    info.bltcon0 = bltcon0;
    info.bltcon1 = bltcon1;
    info.ash = bltconASH();
    info.bsh = bltconBSH();
    info.minterm = bltconLF();
    info.bltapt  = bltapt;
    info.bltbpt  = bltbpt;
    info.bltcpt  = bltcpt;
    info.bltdpt  = bltdpt;
    info.bltafwm = bltafwm;
    info.bltalwm = bltalwm;
    info.bltamod = bltamod;
    info.bltbmod = bltbmod;
    info.bltcmod = bltcmod;
    info.bltdmod = bltdmod;
    info.aold = aold;
    info.bold = bold;
    info.anew = anew;
    info.bnew = bnew;
    info.ahold = ahold;
    info.bhold = bhold;
    info.chold = chold;
    info.dhold = dhold;
    info.barrelAin = anew & mask;
    info.barrelAout = barrelShifter(anew & mask, aold, bltconASH(), bltconDESC());
    info.barrelBin = bnew;
    info.barrelBout = barrelShifter(bnew, bold, bltconBSH(), bltconDESC());
    info.mintermOut = mintermOut;
    info.fillIn = mintermOut;
    info.fillOut = dhold;
    info.bbusy = bbusy;
    info.bzero = bzero;
    info.firstWord = isFirstWord();
    info.lastWord = isLastWord();
    info.fci = bltconFCI();
    info.fco = fillCarry;
    info.fillEnable = bltconFE();
    info.storeToDest = bltconUSED() && !lockD;
}

}
