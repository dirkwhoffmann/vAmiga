// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
         
    static let probes: [(String, Probe)] = [
        
        ("Unconnected", .NONE),
        ("Bus Usage", .BUS_OWNER),
        ("Address Bus", .ADDR_BUS),
        ("Data Bus", .DATA_BUS)
    ]
    
    private func cacheBus() {

        busLogicView.cacheData()
        
        /*
        let cia = ciaA ? emu.ciaA! : emu.ciaB!

        ciaInfo = emu.paused ? cia.info : cia.cachedInfo
        ciaStats = cia.stats
        */
    }
 
    func initComboBox(_ box: NSComboBox) {

        box.removeAllItems()

        for (title, probe) in Inspector.probes {
            box.addItem(withObjectValue: title)
        }
    }

    func refreshComboBox(_ box: NSComboBox) {

        let probe: Probe? =
        box.tag == 0 ? Probe(rawValue: emu.get(.LA_PROBE0)) :
        box.tag == 1 ? Probe(rawValue: emu.get(.LA_PROBE1)) :
        box.tag == 2 ? Probe(rawValue: emu.get(.LA_PROBE2)) :
        box.tag == 3 ? Probe(rawValue: emu.get(.LA_PROBE3)) : nil

        let addr: Int? =
        box.tag == 0 ? emu.get(.LA_PROBE0) :
        box.tag == 1 ? emu.get(.LA_PROBE1) :
        box.tag == 2 ? emu.get(.LA_PROBE2) :
        box.tag == 3 ? emu.get(.LA_PROBE3) : nil

        if probe == .MEMORY {
            box.stringValue = String(format: "%06X", addr!)
        } else {
            for (_title, _probe) in Inspector.probes {
                if probe == _probe { box.stringValue = _title }
            }
        }
    }
    
    func refreshBus(count: Int = 0, full: Bool = false) {

        cacheBus()

        if full {

            if busProbe0.numberOfItems == 0 {

                initComboBox(busProbe0)
                initComboBox(busProbe1)
                initComboBox(busProbe2)
                initComboBox(busProbe3)
                
                /*
                busLogicView.signalColor[0] = NSColor.yellow
                busLogicView.signalColor[1] = NSColor.gray
                busLogicView.signalColor[2] = NSColor.blue
                busLogicView.signalColor[3] = NSColor.green
                */
            }
            refreshComboBox(busProbe0)
            refreshComboBox(busProbe1)
            refreshComboBox(busProbe2)
            refreshComboBox(busProbe3)
        }

        if count % 2 == 0 { busLogicView.update() }
    }
    
    //
    // Action methods
    //
    
    @IBAction func probeAction(_ sender: NSComboBox!) {
        
        let tag = sender.selectedTag()
        let tmp = sender.stringValue
        print("probeAction \(tag) \(tmp)")

        var probe: Probe?
        var addr: Int?
        
        // Check if the user input supplied a keyword
        for (_title, _probe) in Inspector.probes {
            if sender.stringValue == _title { probe = _probe }
        }

        // Check if the user input supplied a memory address
        if probe == nil {
            addr = Int(sender.stringValue, radix: 16)
            if addr != nil { probe = .MEMORY }
        }
        
        if let addr = addr {
            
            switch sender.tag {
                
            case 0:  emu?.set(.LA_ADDR0, value: addr)
            case 1:  emu?.set(.LA_ADDR1, value: addr)
            case 2:  emu?.set(.LA_ADDR2, value: addr)
            case 3:  emu?.set(.LA_ADDR3, value: addr)
            default: break
            }
        }
        
        if let probe = probe {
            
            switch sender.tag {
                
            case 0:  emu?.set(.LA_PROBE0, value: probe.rawValue)
            case 1:  emu?.set(.LA_PROBE1, value: probe.rawValue)
            case 2:  emu?.set(.LA_PROBE2, value: probe.rawValue)
            case 3:  emu?.set(.LA_PROBE3, value: probe.rawValue)
            default: break
            }
            
        } else {
            
            NSSound.beep()
        }
    }
}
