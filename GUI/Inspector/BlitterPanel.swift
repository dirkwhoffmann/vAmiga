// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/*
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

        let elements = [ bltAFWM: fmt16,
                         bltALWM: fmt16,
                         bltAold: fmt16,
                         bltBold: fmt16,
                         bltAnew: fmt16,
                         bltBnew: fmt16,
                         bltAhold: fmt16,
                         bltBhold: fmt16,
                         bltChold: fmt16,
                         bltDhold: fmt16,
                         bltAShift: fmt4,
                         bltBShift: fmt4,
                         bltMinterm: fmt8
        ]
        for (c, f) in elements { assignFormatter(f, c!) }
    }

    func refreshBlitterValues() {

        bltAFWM.integerValue = Int(blitterInfo!.bltafwm)
        bltALWM.integerValue = Int(blitterInfo!.bltalwm)
        bltAold.integerValue = Int(blitterInfo!.aold)
        bltBold.integerValue = Int(blitterInfo!.bold)
        bltAnew.integerValue = Int(blitterInfo!.anew)
        bltBnew.integerValue = Int(blitterInfo!.bnew)
        bltAhold.integerValue = Int(blitterInfo!.ahold)
        bltBhold.integerValue = Int(blitterInfo!.bhold)
        bltChold.integerValue = Int(blitterInfo!.chold)
        bltDhold.integerValue = Int(blitterInfo!.dhold)
        bltAShift.integerValue = Int(blitterInfo!.ash)
        bltBShift.integerValue = Int(blitterInfo!.bsh)
        bltMinterm.integerValue = Int(blitterInfo!.minterm)

        bltFirstWordTime1.state = blitterInfo!.firstIteration ? .on : .off
        bltFirstWordTime2.state = blitterInfo!.firstIteration ? .on : .off
        bltSecPlusWordTime.state = blitterInfo!.firstIteration ? .off : .on
        bltLastWordTime.state = blitterInfo!.lastIteration ? .on : .off
        bltFillCarryIn.state = blitterInfo!.fci ? .on : .off
        bltFillCarryOut.state = blitterInfo!.fco ? .on : .off
        bltFillEnable.state = blitterInfo!.fillEnable ? .on : .off
        bltStoreToDest.state = blitterInfo!.storeToDest ? .on : .off
    }
}

 */
