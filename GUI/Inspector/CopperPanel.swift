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

        return copSelector.indexOfSelectedItem + 1
    }

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

        let nr = Int(copperInfo.copList)
        let active = copperInfo.active

        copActive1.state = active && nr == 1 ? .on : .off
        copActive2.state = active && nr == 2 ? .on : .off
        cop1LC.integerValue = Int(copperInfo.cop1lc)
        cop2LC.integerValue = Int(copperInfo.cop2lc)
        cop1INS.integerValue = Int(copperInfo.cop1ins)
        cop2INS.integerValue = Int(copperInfo.cop2ins)
        copPC.integerValue = Int(copperInfo.coppc)
        copCDANG.state = copperInfo.cdang ? .on : .off

        copList.refresh(count: count, full: full)
    }

    @IBAction func selectCopperListAction(_ sender: Any!) {

        copList.nr = selectedCopperList
        fullRefresh()
    }

    @IBAction func expandCopperListAction(_ sender: Any!) {

        copList.extraRows += 1
        fullRefresh()
        copList.scrollToBottom()
    }

    @IBAction func shrinkCopperListAction(_ sender: Any!) {

        copList.extraRows = max(copList.extraRows - 1, 0)
        fullRefresh()
        copList.scrollToBottom()
    }
}
