// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
         
    var zoom: Double {
        get { busZoomSlider.doubleValue }
        set {
            let w = busScrollView.frame.size.width * CGFloat(newValue)
            let h = busScrollView.frame.size.height
            busLogicView.setFrameSize(NSSize(width: w, height: h))
        }
    }
    
    static let registers: [(String, Int)] = [
        
        ("DMACONR", 0xDFF002),
        ("VPOSR", 0xDFF004),
        ("VHPOSR", 0xDFF006),
        ("DSKDATR", 0xDFF008),
        ("JOY0DAT", 0xDFF00A),
        ("JOY1DAT", 0xDFF00C),
        ("CLXDAT", 0xDFF00E),
        ("ADKCONR", 0xDFF010),
        ("POT0DAT", 0xDFF012),
        ("POT1DAT", 0xDFF014),
        ("POTGOR", 0xDFF016),
        ("SERDATR", 0xDFF018),
        ("DSKBYTR", 0xDFF01A),
        ("INTENAR", 0xDFF01C),
        ("INTREQR", 0xDFF01E),
        ("DENISEID", 0xDFF07C)
    ]
    
    /*
    static let probeLabels: [(String, Probe)] = [
        
    ]
    */
    
    private func cacheBus() {

        busLogicView.cacheData()
    }
 
    func initComboBox(_ box: NSComboBox) {

        box.removeAllItems()

        for (name, _) in Inspector.registers {
            box.addItem(withObjectValue: name)
        }
    }

    func refreshComboBox(_ box: NSComboBox) {

        let probe: Probe? =
        box.tag == 0 ? Probe(rawValue: emu.get(.LA_PROBE0)) :
        box.tag == 1 ? Probe(rawValue: emu.get(.LA_PROBE1)) :
        box.tag == 2 ? Probe(rawValue: emu.get(.LA_PROBE2)) :
        box.tag == 3 ? Probe(rawValue: emu.get(.LA_PROBE3)) : nil

        let addr: Int? =
        box.tag == 0 ? emu.get(.LA_ADDR0) :
        box.tag == 1 ? emu.get(.LA_ADDR1) :
        box.tag == 2 ? emu.get(.LA_ADDR2) :
        box.tag == 3 ? emu.get(.LA_ADDR3) : nil

        if probe == .MEMORY {
            box.stringValue = String(format: "%06X", addr!)
        } else {
            box.stringValue = "<Address>"
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
            }
            
            refreshComboBox(busProbe0)
            refreshComboBox(busProbe1)
            refreshComboBox(busProbe2)
            refreshComboBox(busProbe3)
        }

        if count % 2 == 0 { busLogicView.update() }
        
        //
        // DMA debugger
        //
        
        let bus = emu.get(.DMA_DEBUG_ENABLE) != 0
        let opacity = emu.get(.DMA_DEBUG_OPACITY)
        let mode = emu.get(.DMA_DEBUG_MODE)
        let info = emu.dmaDebugger.info
        
        busEnable.state = bus ? .on : .off
        busCopper.state = info.visualizeCopper ? .on : .off
        busBlitter.state = info.visualizeBlitter ? .on : .off
        busDisk.state = info.visualizeDisk ? .on : .off
        busAudio.state = info.visualizeAudio ? .on : .off
        busSprites.state = info.visualizeSprites ? .on : .off
        busBitplanes.state = info.visualizeBitplanes ? .on : .off
        busCPU.state = info.visualizeCpu ? .on : .off
        busRefresh.state = info.visualizeRefresh ? .on : .off
        busOpacity.integerValue = opacity
        busDisplayMode.selectItem(withTag: mode)
        busBlitter.isEnabled = bus
        busCopper.isEnabled = bus
        busDisk.isEnabled = bus
        busAudio.isEnabled = bus
        busSprites.isEnabled = bus
        busBitplanes.isEnabled = bus
        busCPU.isEnabled = bus
        busRefresh.isEnabled = bus
        busOpacity.isEnabled = bus
        busDisplayMode.isEnabled = bus
        
        colCopper.setColor(info.copperColor)
        colBlitter.setColor(info.blitterColor)
        colDisk.setColor(info.diskColor)
        colAudio.setColor(info.audioColor)
        colSprites.setColor(info.spriteColor)
        colBitplanes.setColor(info.bitplaneColor)
        colCPU.setColor(info.cpuColor)
        colRefresh.setColor(info.refreshColor)
    }
    
    func scrollToHPos() {
        
        let pos = (busLogicView.bounds.width / CGFloat(228)) * CGFloat(emu.amiga.info.hpos + 1)
        let newx = pos - (busScrollView.bounds.width / 2)
        NSAnimationContext.runAnimationGroup({ context in
            context.duration = 0.5
            busScrollView.contentView.animator().setBoundsOrigin(NSPoint(x: newx, y: 0))
        }, completionHandler: nil)
    }
    
    //
    // Action methods (Logic Analyzer)
    //
    
    @IBAction func symAction(_ sender: NSButton!) {

        busLogicView.formatter.symbolic = sender.state == .on
        fullRefresh()
    }

    @IBAction func hexAction(_ sender: NSButton!) {

        busLogicView.formatter.hex = sender.state == .on
        fullRefresh()
    }
 
    @IBAction func zoomInAction(_ sender: NSButton!) {

        if zoom < 21 { zoom += 1 }
    }

    @IBAction func zoomOutAction(_ sender: NSButton!) {

        if zoom > 1 { zoom -= 1 }
    }

    @IBAction func zoomSliderAction(_ sender: NSSlider!) {

        zoom = sender.doubleValue
    }
    
    @IBAction func probeAction(_ sender: NSComboBox!) {
        
        let tag = sender.selectedTag()
        print("probeAction \(tag) \(sender.stringValue)")

        var addr: Int?
        
        // Check if the user input supplied a symbolic name
        for (name, address) in Inspector.registers {
            if sender.stringValue == name { addr = address }
        }

        // Check if the user input supplied a memory address
        if addr == nil { addr = Int(sender.stringValue, radix: 16) }
        
        if let addr = addr {
            
            let probe = Probe.MEMORY.rawValue
            
            switch sender.tag {
            case 0:  emu?.set(.LA_PROBE0, value: probe); emu?.set(.LA_ADDR0, value: addr)
            case 1:  emu?.set(.LA_PROBE1, value: probe); emu?.set(.LA_ADDR1, value: addr)
            case 2:  emu?.set(.LA_PROBE2, value: probe); emu?.set(.LA_ADDR2, value: addr)
            case 3:  emu?.set(.LA_PROBE3, value: probe); emu?.set(.LA_ADDR3, value: addr)
            default: break
            }
            
        } else {
            
            NSSound.beep()
        }
    }
    
    //
    // Action methods (DMA Debugger)
    //
        
    @IBAction func colorAction(_ sender: NSColorWell!) {
        
        let r = Int(sender.color.redComponent * 255.0)
        let g = Int(sender.color.greenComponent * 255.0)
        let b = Int(sender.color.blueComponent * 255.0)
        let rgb = (r << 24) | (g << 16) | (b << 8)

        switch sender.tag {
        case 0:  emu.set(.DMA_DEBUG_COLOR0, value: rgb)
        case 1:  emu.set(.DMA_DEBUG_COLOR1, value: rgb)
        case 2:  emu.set(.DMA_DEBUG_COLOR2, value: rgb)
        case 3:  emu.set(.DMA_DEBUG_COLOR3, value: rgb)
        case 4:  emu.set(.DMA_DEBUG_COLOR4, value: rgb)
        case 5:  emu.set(.DMA_DEBUG_COLOR5, value: rgb)
        case 6:  emu.set(.DMA_DEBUG_COLOR6, value: rgb)
        case 7:  emu.set(.DMA_DEBUG_COLOR7, value: rgb)
        default: break
        }

        fullRefresh()
    }

    @IBAction func busEnableAction(_ sender: NSButton!) {
        
        emu.set(.DMA_DEBUG_ENABLE, enable: sender.state == .on)
        fullRefresh()
    }
    
    @IBAction func busDisplayAction(_ sender: NSButton!) {
        
        switch sender.tag {
        case 0:  emu.set(.DMA_DEBUG_CHANNEL0, enable: sender.state == .on)
        case 1:  emu.set(.DMA_DEBUG_CHANNEL1, enable: sender.state == .on)
        case 2:  emu.set(.DMA_DEBUG_CHANNEL2, enable: sender.state == .on)
        case 3:  emu.set(.DMA_DEBUG_CHANNEL3, enable: sender.state == .on)
        case 4:  emu.set(.DMA_DEBUG_CHANNEL4, enable: sender.state == .on)
        case 5:  emu.set(.DMA_DEBUG_CHANNEL5, enable: sender.state == .on)
        case 6:  emu.set(.DMA_DEBUG_CHANNEL6, enable: sender.state == .on)
        case 7:  emu.set(.DMA_DEBUG_CHANNEL7, enable: sender.state == .on)
        default: break
        }
        fullRefresh()
    }
    
    @IBAction func busDisplayModeAction(_ sender: NSPopUpButton!) {
        
        emu.set(.DMA_DEBUG_MODE, value: sender.selectedTag())
        fullRefresh()
    }
    
    @IBAction func busOpacityAction(_ sender: NSSlider!) {

        emu.set(.DMA_DEBUG_OPACITY, value: sender.integerValue)
        fullRefresh()
    }
}
