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
    
    static let probeLabels: [(String, Probe)] = [
        
        ("Select...", .NONE),
        ("Bus Usage", .BUS_OWNER),
        ("Address Bus", .ADDR_BUS),
        ("Data Bus", .DATA_BUS)
    ]
    
    private func cacheBus() {

        busLogicView.cacheData()
    }
 
    func initComboBox(_ box: NSComboBox) {

        box.removeAllItems()

        for (title, _) in Inspector.probeLabels {
            box.addItem(withObjectValue: title)
        }
    }

    func refreshComboBox(_ box: NSComboBox) {

        let probe: Probe? =
        box.tag == 0 ? Probe(rawValue: emu.get(.LA_PROBE0)) :
        box.tag == 1 ? Probe(rawValue: emu.get(.LA_PROBE1)) :
        box.tag == 2 ? Probe(rawValue: emu.get(.LA_PROBE2)) :
        box.tag == 3 ? Probe(rawValue: emu.get(.LA_PROBE3)) :
        box.tag == 4 ? Probe(rawValue: emu.get(.LA_PROBE4)) :
        box.tag == 5 ? Probe(rawValue: emu.get(.LA_PROBE5)) : nil

        let addr: Int? =
        box.tag == 0 ? emu.get(.LA_ADDR0) :
        box.tag == 1 ? emu.get(.LA_ADDR1) :
        box.tag == 2 ? emu.get(.LA_ADDR2) :
        box.tag == 3 ? emu.get(.LA_ADDR3) :
        box.tag == 4 ? emu.get(.LA_ADDR4) :
        box.tag == 5 ? emu.get(.LA_ADDR5) : nil

        if probe == .MEMORY {
            box.stringValue = String(format: "%06X", addr!)
        } else {
            for (_title, _probe) in Inspector.probeLabels {
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
                initComboBox(busProbe4)
                initComboBox(busProbe5)
            }
            
            refreshComboBox(busProbe0)
            refreshComboBox(busProbe1)
            refreshComboBox(busProbe2)
            refreshComboBox(busProbe3)
            refreshComboBox(busProbe4)
            refreshComboBox(busProbe5)
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
        let tmp = sender.stringValue
        print("probeAction \(tag) \(tmp)")

        var probe: Probe?
        var addr: Int?
        
        // Check if the user input supplied a keyword
        for (_title, _probe) in Inspector.probeLabels {
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
            case 4:  emu?.set(.LA_ADDR4, value: addr)
            case 5:  emu?.set(.LA_ADDR5, value: addr)
            default: break
            }
        }
        
        if let probe = probe {
            
            switch sender.tag {
                
            case 0:  emu?.set(.LA_PROBE0, value: probe.rawValue)
            case 1:  emu?.set(.LA_PROBE1, value: probe.rawValue)
            case 2:  emu?.set(.LA_PROBE2, value: probe.rawValue)
            case 3:  emu?.set(.LA_PROBE3, value: probe.rawValue)
            case 4:  emu?.set(.LA_PROBE4, value: probe.rawValue)
            case 5:  emu?.set(.LA_PROBE5, value: probe.rawValue)
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
