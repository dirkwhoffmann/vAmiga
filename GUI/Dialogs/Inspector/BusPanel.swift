// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
        
    private func cacheBus() {

        busLogicView.cacheData()
        
        /*
        let cia = ciaA ? emu.ciaA! : emu.ciaB!

        ciaInfo = emu.paused ? cia.info : cia.cachedInfo
        ciaStats = cia.stats
        */
    }

    func initPopup(button: NSPopUpButton) {

        func add(_ title: String, _ probe: Probe) {

            button.addItem(withTitle: title)
            button.lastItem!.tag = probe.rawValue
        }

        button.removeAllItems()
        add("None", .NONE)
        add("Bus Usage", .BUS_OWNER)
        add("Address Bus", .ADDR_BUS)
        add("Data Bus", .DATA_BUS)
        add("Memory", .MEMORY)
    }
    
    func refreshBus(count: Int = 0, full: Bool = false) {

        cacheBus()

        if full {

            if busProbe0.numberOfItems == 1 {
            
                initPopup(button: busProbe0)
                initPopup(button: busProbe1)
                initPopup(button: busProbe2)
                initPopup(button: busProbe3)
            }

            // Probes
            let probe0 = emu.get(.LA_PROBE0)
            let probe1 = emu.get(.LA_PROBE1)
            let probe2 = emu.get(.LA_PROBE2)
            let probe3 = emu.get(.LA_PROBE3)
            busProbe0.selectItem(withTag: probe0)
            busProbe1.selectItem(withTag: probe1)
            busProbe2.selectItem(withTag: probe2)
            busProbe3.selectItem(withTag: probe3)
            busAddr0.integerValue = emu.get(.LA_ADDR0)
            busAddr1.integerValue = emu.get(.LA_ADDR1)
            busAddr2.integerValue = emu.get(.LA_ADDR2)
            busAddr3.integerValue = emu.get(.LA_ADDR3)
        }

        if count % 2 == 0 { busLogicView.update() }
    }
    
    //
    // Action methods
    //
    
    @IBAction func probeAction(_ sender: NSPopUpButton!) {
        
        let tag = sender.selectedTag()
        print("probeAction \(tag)")

        switch sender.tag {
        
        case 0:  emu?.set(.LA_PROBE0, value: tag)
        case 1:  emu?.set(.LA_PROBE1, value: tag)
        case 2:  emu?.set(.LA_PROBE2, value: tag)
        case 3:  emu?.set(.LA_PROBE3, value: tag)
    
        default: break
        }
    }
 
    @IBAction func addrAction(_ sender: NSTextField!) {

        let tag = sender.selectedTag()
        print("addrAction \(tag)")
        
        switch sender.tag {
        
        case 0:  emu?.set(.LA_ADDR0, value: tag)
        case 1:  emu?.set(.LA_ADDR1, value: tag)
        case 2:  emu?.set(.LA_ADDR2, value: tag)
        case 3:  emu?.set(.LA_ADDR3, value: tag)
    
        default: break
        }
    }
}
