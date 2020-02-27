// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    func cacheBlitter() {

        blitterInfo = amiga!.agnus.getBlitterInfo()
    }

    func refreshBlitter(count: Int) {

        // Perform a full refresh if needed
        if count == 0 { refreshBlitterFormatters() }

        // Update display cache
        cacheBlitter()

        // Refresh display with cached values
        refreshBlitterValues()
    }

    func refreshBlitterFormatters() {

        /*
        let elements = [ dmaVPOS: fmt16,
                         dmaHPOS: fmt16,
        ]
        for (c, f) in elements { assignFormatter(f, c!) }
        */
    }

    func refreshBlitterValues() {

        /*
        dmaVPOS.integerValue = Int(agnusInfo!.vpos)
        dmaHPOS.integerValue = Int(agnusInfo!.hpos)
        dmaDSKEnable.state = dsken ? .on : .off
        */
    }
}
