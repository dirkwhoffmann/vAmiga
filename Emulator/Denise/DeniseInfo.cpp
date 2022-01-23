// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Denise.h"
#include "Agnus.h"
#include "ControlPort.h"
#include "IOUtils.h"

void
Denise::_inspect() const
{
    {   SYNCHRONIZED
        
        info.bplcon0 = bplcon0;
        info.bplcon1 = bplcon1;
        info.bplcon2 = bplcon2;
        info.bpu = bpu();
        
        info.diwstrt = diwstrt;
        info.diwstop = diwstop;
        info.hstrt = hstrt;
        info.hstop = hstop;
        info.vstrt = agnus.sequencer.vstrt;
        info.vstop = agnus.sequencer.vstop;
        
        info.joydat[0] = controlPort1.joydat();
        info.joydat[1] = controlPort2.joydat();
        info.clxdat = 0;
        
        for (isize i = 0; i < 6; i++) {
            info.bpldat[i] = bpldat[i];
        }
        for (isize i = 0; i < 32; i++) {
            info.colorReg[i] = pixelEngine.getColor(i);
            info.color[i] = pixelEngine.getRGBA(i);
        }
    }
}

void
Denise::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
        
        os << tab("Chip revision");
        os << DeniseRevisionEnum::key(config.revision) << std::endl;
        os << tab("Hidden bitplanes");
        os << hex(config.hiddenBitplanes) << std::endl;
        os << tab("Hidden sprites");
        os << hex(config.hiddenSprites) << std::endl;
        os << tab("Hidden layers");
        os << hex(config.hiddenLayers) << std::endl;
        os << tab("Hidden layer alpha");
        os << dec(config.hiddenLayerAlpha) << std::endl;
        os << tab("clxSprSpr");
        os << bol(config.clxSprSpr) << std::endl;
        os << tab("clxSprSpr");
        os << bol(config.clxSprSpr) << std::endl;
        os << tab("clxSprSpr");
        os << bol(config.clxSprSpr) << std::endl;
    }
    
    if (category & dump::Registers) {
        
        os << tab("BPLCON0");
        os << hex(bplcon0) << std::endl;
        os << tab("BPLCON1");
        os << hex(bplcon1) << std::endl;
        os << tab("BPLCON2");
        os << hex(bplcon2) << std::endl;
        os << tab("BPLCON3");
        os << hex(bplcon3) << std::endl;
    
        os << tab("SPRxDATA");
        for (isize i = 0; i < 8; i++) os << hex(sprdata[i]) << ' ';
        os << std::endl;
        os << tab("SPRxDATB");
        for (isize i = 0; i < 8; i++) os << hex(sprdatb[i]) << ' ';
        os << std::endl;
        os << tab("SPRxPOS");
        for (isize i = 0; i < 8; i++) os << hex(sprpos[i]) << ' ';
        os << std::endl;
        os << tab("SPRxCTL");
        for (isize i = 0; i < 8; i++) os << hex(sprctl[i]) << ' ';
        os << std::endl;
    }
}
