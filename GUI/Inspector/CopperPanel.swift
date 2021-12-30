// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    private func cacheCopper() {

        copperInfo = amiga.copper.info
    }

    func refreshCopper(count: Int = 0, full: Bool = false) {

        cacheCopper()

        if full {
            
            let elements = [ cop1LC: fmt24,
                             cop2LC: fmt24,
                             cop1INS: fmt16,
                             cop2INS: fmt16,
                             copPC: fmt24
            ]
            for (c, f) in elements { assignFormatter(f, c!) }
        }

        cop1LC.integerValue = Int(copperInfo.cop1lc)
        cop2LC.integerValue = Int(copperInfo.cop2lc)
        cop1INS.integerValue = Int(copperInfo.cop1ins)
        cop2INS.integerValue = Int(copperInfo.cop2ins)
        copPC.integerValue = Int(copperInfo.coppc)
        copCDANG.state = copperInfo.cdang ? .on : .off

        copList1.nr = 1
        copList1.refresh(count: count, full: full)
        copList2.nr = 2
        copList2.refresh(count: count, full: full)
    }

    @IBAction func copList1FormatAction(_ sender: Any!) {

        copList1.symbolic = copList1Format.indexOfSelectedItem == 1
        fullRefresh()
    }

    @IBAction func copList2FormatAction(_ sender: Any!) {

        copList2.symbolic = copList2Format.indexOfSelectedItem == 1
        fullRefresh()
    }

    @IBAction func expandCopList1Action(_ sender: Any!) {

        extraRowsAction(copList1, delta: 1)
    }

    @IBAction func expandCopList2Action(_ sender: Any!) {

        extraRowsAction(copList2, delta: 1)
    }

    @IBAction func shrinkCopList1Action(_ sender: Any!) {

        extraRowsAction(copList1, delta: -1)
    }

    @IBAction func shrinkCopList2Action(_ sender: Any!) {

        extraRowsAction(copList2, delta: -1)
    }

    func extraRowsAction(_ list: CopperTableView, delta: Int) {
        
        list.extraRows = max(list.extraRows + delta, 0)
        fullRefresh()
        list.scrollToBottom()
    }
}
