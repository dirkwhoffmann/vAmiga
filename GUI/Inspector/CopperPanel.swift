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

    func cacheCopper() {

        copperInfo = amiga!.agnus.getCopperInfo()
    }

    func refreshCopper(count: Int) {

        // Refresh sub views
        copList1.refresh(count: count)
        copList2.refresh(count: count)
        
        // Perform a full refresh if needed
        if count == 0 { refreshCopperFormatters() }

        // Update display cache
        cacheCopper()

        // Refresh display with cached values
        refreshCopperValues()
    }

    func refreshCopperFormatters() {

        let elements = [ copCOPPC: fmt24,
                         copCOPINS1: fmt16,
                         copCOPINS2: fmt16,
                         copCOP1LC: fmt24,
                         copCOP2LC: fmt24
        ]
        for (c, f) in elements { assignFormatter(f, c!) }
    }
    
    func refreshCopperValues() {

        copActive.state = copperInfo!.active ? .on : .off
        copCDANG.state = copperInfo!.cdang ? .on : .off
        copCOPPC.integerValue = Int(copperInfo!.coppc)
        copCOPINS1.integerValue = Int(copperInfo!.cop1ins)
        copCOPINS2.integerValue = Int(copperInfo!.cop2ins)
        copCOP1LC.integerValue = Int(copperInfo!.cop1lc)
        copCOP2LC.integerValue = Int(copperInfo!.cop2lc)
    }
}

*/
