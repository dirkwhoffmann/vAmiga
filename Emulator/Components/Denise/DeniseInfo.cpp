// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Denise.h"
#include "Agnus.h"
#include "ControlPort.h"
#include "IOUtils.h"

namespace vamiga {

void
Denise::_inspect() const
{
    {   SYNCHRONIZED

        info.ecs = isECS();
        
        info.bplcon0 = bplcon0;
        info.bplcon1 = bplcon1;
        info.bplcon2 = bplcon2;
        info.bpu = bpu();
        
        info.diwstrt = diwstrt;
        info.diwstop = diwstop;
        info.viewport.hstrt = hstrt;
        info.viewport.hstop = hstop;
        info.viewport.vstrt = agnus.sequencer.vstrt;
        info.viewport.vstop = agnus.sequencer.vstop;
        
        info.joydat[0] = controlPort1.joydat();
        info.joydat[1] = controlPort2.joydat();
        info.clxdat = 0;
        
        for (isize i = 0; i < 6; i++) {
            info.bpldat[i] = bpldat[i];
        }
        for (isize i = 0; i < 32; i++) {
            info.colorReg[i] = pixelEngine.getColor(i);
            info.color[i] = (u32)pixelEngine.palette[i];
        }
    }
}

void
Denise::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        os << tab("Chip revision");
        os << DeniseRevisionEnum::key(config.revision) << std::endl;
        os << tab("Viewport tracking");
        os << bol(config.viewportTracking) << std::endl;
        os << tab("Frame skips in warp mode");
        os << bol(config.frameSkipping) << std::endl;
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
        os << tab("clxSprPlf");
        os << bol(config.clxSprPlf) << std::endl;
        os << tab("clxPlfPlf");
        os << bol(config.clxPlfPlf) << std::endl;
    }

    if (category == Category::State) {

        os << tab("Resolution");
        os << ResolutionEnum::key(res) << std::endl;

    }

    if (category == Category::Registers) {
        
        os << tab("BPLCON0");
        os << hex(bplcon0) << std::endl;
        os << tab("BPLCON1");
        os << hex(bplcon1) << std::endl;
        os << tab("BPLCON2");
        os << hex(bplcon2) << std::endl;
        os << tab("BPLCON3");
        os << hex(bplcon3) << std::endl;
        os << std::endl;
        os << tab("DIWSTART");
        os << hex(diwstrt) << std::endl;
        os << tab("DIWSTOP");
        os << hex(diwstop) << std::endl;
        os << tab("DIWHIGH");
        os << hex(diwhigh) << std::endl;
        os << std::endl;
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

}
