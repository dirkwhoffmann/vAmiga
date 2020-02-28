// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    private var selectedCopperList: Int {

        return copSelector.indexOfSelectedItem
    }

    func cacheCopper() {

        copperInfo = amiga!.agnus.getCopperInfo()
    }

    func cacheBlitter() {

        blitterInfo = amiga!.agnus.getBlitterInfo()
    }

    func refreshCopperAndBlitter(count: Int) {

        // Perform a full refresh if needed
        if count == 0 {
            refreshCopperFormatters()
            refreshBlitterFormatters()
        }

        // Update caches
        cacheCopper()
        cacheBlitter()

        let lc = selectedCopperList == 0 ? copperInfo!.cop1lc : copperInfo!.cop2lc
        let len = selectedCopperList == 0 ? copperInfo!.length1 : copperInfo!.length2
        // track("\(selectedCopperList) \(lc) \(len)")

        copList.cache(lc: Int(lc), count: Int(len))

        // Refresh views
        refreshCopperValues()
        refreshBlitterValues()

        // Refresh sub views
        copList.refresh(count: count)
    }

    func refreshCopperFormatters() {

        let elements = [ copPC: fmt24 ]

        for (c, f) in elements { assignFormatter(f, c!) }
    }

    func refreshCopperValues() {

        let nr = Int(copperInfo!.copList)
        let active = copperInfo!.active

        copPC.integerValue = Int(copperInfo!.coppc)
        copCDANG.state = copperInfo!.cdang ? .on : .off
        copActive1.state = active && nr == 1 ? .on : .off
        copActive2.state = active && nr == 2 ? .on : .off
    }

    func refreshBlitterFormatters() {

        let elements = [ bltBLTCON0a: fmt4,
                         bltBLTCON0b: fmt4,
                         bltBLTCON0c: fmt8,
                         bltBLTCON1a: fmt4,
                         bltBLTCON1b: fmt4,
                         bltBLTCON1c: fmt8,
                         bltAhold: fmt16,
                         bltBhold: fmt16,
                         bltChold: fmt16,
                         bltDhold: fmt16,
                         bltAold: fmt16,
                         bltBold: fmt16,
                         bltAnew: fmt16,
                         bltBnew: fmt16,
                         bltY: fmt16,
                         bltX: fmt16
        ]
        for (c, f) in elements { assignFormatter(f, c!) }
    }

    func refreshBlitterValues() {

        let bltcon0 = Int(blitterInfo!.bltcon0)
        bltBLTCON0a.integerValue = (bltcon0 >> 12) & 0xF
        bltBLTCON0b.integerValue = (bltcon0 >> 8) & 0xF
        bltBLTCON0c.integerValue = bltcon0 & 0xFF
        bltUseA.state = bltcon0 & 0x800 != 0 ? .on : .off
        bltUseB.state = bltcon0 & 0x400 != 0 ? .on : .off
        bltUseC.state = bltcon0 & 0x200 != 0 ? .on : .off
        bltUseD.state = bltcon0 & 0x100 != 0 ? .on : .off
        bltLF7.state = bltcon0 & 0x80 != 0 ? .on : .off
        bltLF6.state = bltcon0 & 0x40 != 0 ? .on : .off
        bltLF5.state = bltcon0 & 0x20 != 0 ? .on : .off
        bltLF4.state = bltcon0 & 0x10 != 0 ? .on : .off
        bltLF3.state = bltcon0 & 0x08 != 0 ? .on : .off
        bltLF2.state = bltcon0 & 0x04 != 0 ? .on : .off
        bltLF1.state = bltcon0 & 0x02 != 0 ? .on : .off
        bltLF0.state = bltcon0 & 0x01 != 0 ? .on : .off

        let bltcon1 = Int(blitterInfo!.bltcon1)
        bltBLTCON1a.integerValue = (bltcon1 >> 12) & 0xF
        bltBLTCON1b.integerValue = (bltcon1 >> 8) & 0xF
        bltBLTCON1c.integerValue = bltcon1 & 0xFF
        bltEFE.state = bltcon1 & 0x10 != 0 ? .on : .off
        bltIFE.state = bltcon1 & 0x08 != 0 ? .on : .off
        bltFCI.state = bltcon1 & 0x04 != 0 ? .on : .off
        bltDESC.state = bltcon1 & 0x02 != 0 ? .on : .off
        bltLINE.state = bltcon1 & 0x01 != 0 ? .on : .off

        bltActive.state = blitterInfo!.active ? .on : .off

        bltAhold.integerValue = Int(blitterInfo!.ahold)
        bltBhold.integerValue = Int(blitterInfo!.bhold)
        bltChold.integerValue = Int(blitterInfo!.chold)
        bltDhold.integerValue = Int(blitterInfo!.dhold)
        bltAold.integerValue = Int(blitterInfo!.aold)
        bltBold.integerValue = Int(blitterInfo!.bold)
        bltAnew.integerValue = Int(blitterInfo!.anew)
        bltBnew.integerValue = Int(blitterInfo!.bnew)
        bltFirstWord.state = blitterInfo!.firstIteration ? .on : .off
        bltLastWord.state  = blitterInfo!.lastIteration ? .on : .off
    }

    @IBAction func selectCopperListAction(_ sender: Any!) {

        needsRefresh()
    }
}
