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

            print("\(busProbe0.numberOfItems)")
            if busProbe0.numberOfItems == 1 {
            
                initPopup(button: busProbe0)
                initPopup(button: busProbe1)
                initPopup(button: busProbe2)
                initPopup(button: busProbe3)
            }
            
            /*
            ciaPA7.title = "PA7: " + (ciaA ? "GAME1" : "/DTR")
            ciaPA6.title = "PA6: " + (ciaA ? "GAME0" : "/RTS")
            ciaPA5.title = "PA5: " + (ciaA ? "/RDY"  : "/CD")
            ciaPA4.title = "PA4: " + (ciaA ? "/TK0"  : "/CTS")
            ciaPA3.title = "PA3: " + (ciaA ? "/WPRO" : "/DSR")
            ciaPA2.title = "PA2: " + (ciaA ? "/CHNG" : "SEL")
            ciaPA1.title = "PA1: " + (ciaA ? "LED"   : "POUT")
            ciaPA0.title = "PA0: " + (ciaA ? "OVL"   : "BUSY")

            ciaPB7.title = "PB7: " + (ciaA ? "DATA7" : "/MTR")
            ciaPB6.title = "PB6: " + (ciaA ? "DATA6" : "/SEL3")
            ciaPB5.title = "PB5: " + (ciaA ? "DATA5" : "/SEL2")
            ciaPB4.title = "PB4: " + (ciaA ? "DATA4" : "/SEL1")
            ciaPB3.title = "PB3: " + (ciaA ? "DATA3" : "/SEL0")
            ciaPB2.title = "PB2: " + (ciaA ? "DATA2" : "/SIDE")
            ciaPB1.title = "PB1: " + (ciaA ? "DATA1" : "DIR")
            ciaPB0.title = "PB0: " + (ciaA ? "DATA0" : "/STEP")

            let elements = [ ciaPRAbinary: fmt8b,
                             ciaDDRAbinary: fmt8b,
                             ciaPRBbinary: fmt8b,
                             ciaDDRBbinary: fmt8b,
                             ciaICRbinary: fmt8b,
                             ciaIMRbinary: fmt8b,
                             ciaPRA: fmt8,
                             ciaDDRA: fmt8,
                             ciaPRB: fmt8,
                             ciaDDRB: fmt8,
                             ciaCntHi: fmt8,
                             ciaCntMid: fmt8,
                             ciaCntLo: fmt8,
                             ciaAlarmHi: fmt8,
                             ciaAlarmMid: fmt8,
                             ciaAlarmLo: fmt8,
                             ciaIMR: fmt8,
                             ciaICR: fmt8,
                             ciaSDR: fmt8,
                             ciaSSR: fmt8b,
                             ciaTA: fmt16,
                             ciaTAlatch: fmt16,
                             ciaTB: fmt16,
                             ciaTBlatch: fmt16 ]

            for (c, f) in elements { assignFormatter(f, c!) }
            */
        }

        /*
        ciaTA.intValue = Int32(ciaInfo.timerA.count)
        ciaTAlatch.intValue = Int32(ciaInfo.timerA.latch)
        ciaTArunning.state = ciaInfo.timerA.running ? .on : .off
        ciaTAtoggle.state = ciaInfo.timerA.toggle ? .on : .off
        ciaTApbout.state = ciaInfo.timerA.pbout ? .on : .off
        ciaTAoneShot.state = ciaInfo.timerA.oneShot ? .on : .off
        */
        if count % 16 == 0 { busLogicView.update() }
    }
    
    //
    // Action methods
    //
    
    @IBAction func probeAction(_ sender: NSPopUpButton!) {
        
        let tag = sender.selectedTag()
        print("probeAction \(tag)")

        switch sender.tag {
            /*
        case 0:  emu?.set(.LA_PROBE0, value: tag)
        case 1:  emu?.set(.LA_PROBE1, value: tag)
        case 2:  emu?.set(.LA_PROBE2, value: tag)
        case 3:  emu?.set(.LA_PROBE3, value: tag)
             */
        default: break
        }
    }
 
    @IBAction func addrAction(_ sender: NSTextField!) {

        let tag = sender.selectedTag()
        print("addrAction \(tag)")
    }
}
