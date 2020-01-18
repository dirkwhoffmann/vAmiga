// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    func cacheAgnus() {

        agnusInfo = amiga!.agnus.getInfo()
    }

    func refreshAgnus(count: Int) {

        // Perform a full refresh if needed
        if count == 0 { refreshAgnusFormatters() }

        // Update display cache
        cacheAgnus()

        // Refresh display with cached values
        refreshAgnusValues()
    }

    func refreshAgnusFormatters() {

        let elements = [ dmaDIWSTRT: fmt16,
                         dmaDIWSTOP: fmt16,
                         dmaDDFSTRT: fmt16,
                         dmaDDFSTOP: fmt16,

                         dmaBPL1PT: fmt16,
                         dmaBPL2PT: fmt16,
                         dmaBPL3PT: fmt16,
                         dmaBPL4PT: fmt16,
                         dmaBPL5PT: fmt16,
                         dmaBPL6PT: fmt16
        ]
        for (c, f) in elements { assignFormatter(f, c!) }
    }

    func refreshAgnusValues() {

        let dmacon = Int(agnusInfo!.dmacon)

        dmaDMACON.integerValue = dmacon
        dmaBLTPRI.state = (dmacon & 0b0000010000000000 != 0) ? .on : .off
        dmaDMAEN.state  = (dmacon & 0b0000001000000000 != 0) ? .on : .off
        dmaBPLEN.state  = (dmacon & 0b0000000100000000 != 0) ? .on : .off
        dmaCOPEN.state  = (dmacon & 0b0000000010000000 != 0) ? .on : .off
        dmaBLTEN.state  = (dmacon & 0b0000000001000000 != 0) ? .on : .off
        dmaSPREN.state  = (dmacon & 0b0000000000100000 != 0) ? .on : .off
        dmaDSKEN.state  = (dmacon & 0b0000000000010000 != 0) ? .on : .off
        dmaAUD3EN.state  = (dmacon & 0b0000000000001000 != 0) ? .on : .off
        dmaAUD2EN.state  = (dmacon & 0b0000000000000100 != 0) ? .on : .off
        dmaAUD1EN.state  = (dmacon & 0b0000000000000010 != 0) ? .on : .off
        dmaAUD0EN.state  = (dmacon & 0b0000000000000001 != 0) ? .on : .off

        dmaDIWSTRT.integerValue = Int(agnusInfo!.diwstrt)
        dmaDIWSTOP.integerValue = Int(agnusInfo!.diwstop)
        dmaDDFSTRT.integerValue = Int(agnusInfo!.ddfstrt)
        dmaDDFSTOP.integerValue = Int(agnusInfo!.ddfstop)

        dmaBPL1PT.integerValue = Int(agnusInfo!.bplpt.0)
        dmaBPL2PT.integerValue = Int(agnusInfo!.bplpt.1)
        dmaBPL3PT.integerValue = Int(agnusInfo!.bplpt.2)
        dmaBPL4PT.integerValue = Int(agnusInfo!.bplpt.3)
        dmaBPL5PT.integerValue = Int(agnusInfo!.bplpt.4)
        dmaBPL6PT.integerValue = Int(agnusInfo!.bplpt.5)

        dmaBPL1MOD.integerValue = Int(agnusInfo!.bpl1mod)
        dmaBPL2MOD.integerValue = Int(agnusInfo!.bpl2mod)

        let bpu = agnusInfo!.bpu
        dmaBpl1Enable.state = bpu >= 1 ? .on : .off
        dmaBpl2Enable.state = bpu >= 2 ? .on : .off
        dmaBpl3Enable.state = bpu >= 3 ? .on : .off
        dmaBpl4Enable.state = bpu >= 4 ? .on : .off
        dmaBpl5Enable.state = bpu >= 5 ? .on : .off
        dmaBpl6Enable.state = bpu >= 6 ? .on : .off
    }
}
