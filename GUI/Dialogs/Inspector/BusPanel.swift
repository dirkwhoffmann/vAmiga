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
    
    static let presets: [(String?, (Probe, Int?))] = [
        
        ("None",     (.NONE,   nil)),
        (nil, (.NONE, 0)),
        ("DMACONR",  (.MEMORY, 0xDFF002)),
        ("VPOSR",    (.MEMORY, 0xDFF004)),
        ("VHPOSR",   (.MEMORY, 0xDFF006)),
        ("DSKDATR",  (.MEMORY, 0xDFF008)),
        ("JOY0DAT",  (.MEMORY, 0xDFF00A)),
        ("JOY1DAT",  (.MEMORY, 0xDFF00C)),
        ("CLXDAT",   (.MEMORY, 0xDFF00E)),
        ("ADKCONR",  (.MEMORY, 0xDFF010)),
        ("POT0DAT",  (.MEMORY, 0xDFF012)),
        ("POT1DAT",  (.MEMORY, 0xDFF014)),
        ("POTGOR",   (.MEMORY, 0xDFF016)),
        ("SERDATR",  (.MEMORY, 0xDFF018)),
        ("DSKBYTR",  (.MEMORY, 0xDFF01A)),
        ("INTENAR",  (.MEMORY, 0xDFF01C)),
        ("INTREQR",  (.MEMORY, 0xDFF01E)),
        ("DENISEID", (.MEMORY, 0xDFF07C)),
        (nil, (.NONE, 0)),
        ("IPL",      (.IPL,    nil)),
        (nil, (.NONE, 0))
    ]
         
    private func cacheBus() {

        busLogicView.cacheData()
    }
 
    func initProbeSelector(_ channel: Int, _ popup: NSComboButton) {

        let menu = NSMenu()
        
        // Add presets
        for (index, (name, _)) in Inspector.presets.enumerated() {

            if let name = name {
                let item = NSMenuItem(title: name, action: #selector(busProbeAction(_ :)), keyEquivalent: "")
                item.tag = 128 * channel + index
                menu.addItem(item)
            } else {
                menu.addItem(NSMenuItem.separator())
            }
        }

        // Create a custom view with an embedded text field
        let view = NSView(frame: NSRect(x: 0, y: 0, width: 128+12+12, height: 24+4+4))
        let text = NSTextField(frame: NSRect(x: 12, y: 4, width: 128, height: 24))
        text.placeholderString = "Enter address"
        text.refusesFirstResponder = true
        text.action = #selector(busAddrAction(_ :))
        text.target = self
        text.tag = channel
        view.addSubview(text)

        // Add the address field
        let menuItem = NSMenuItem()
        menuItem.view = view
        menu.addItem(menuItem)
        
        popup.menu = menu
        print("isEnabled = \(popup.isEnabled)")
        popup.isEnabled = true
    }

    func refreshProbeSelector(_ popup: NSComboButton) {
    
        let probe: Probe? =
        popup.tag == 0 ? Probe(rawValue: emu.get(.LA_PROBE0)) :
        popup.tag == 1 ? Probe(rawValue: emu.get(.LA_PROBE1)) :
        popup.tag == 2 ? Probe(rawValue: emu.get(.LA_PROBE2)) :
        popup.tag == 3 ? Probe(rawValue: emu.get(.LA_PROBE3)) : nil

        let addr: Int? =
        popup.tag == 0 ? emu.get(.LA_ADDR0) :
        popup.tag == 1 ? emu.get(.LA_ADDR1) :
        popup.tag == 2 ? emu.get(.LA_ADDR2) :
        popup.tag == 3 ? emu.get(.LA_ADDR3) : nil
        
        switch probe {
        case .NONE:     popup.title = "Connect..."
        case .MEMORY:   popup.title = String(format: "%06X", addr ?? 0)
        case .IPL:      popup.title = "IPL"
        default:        popup.title = "???"
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

        switch probe {
        case .MEMORY:   box.stringValue = String(format: "%06X", addr!)
        case .IPL:      box.stringValue = "IPL"
        default:        box.stringValue = ""
        }
    }
    
    func refreshBus(count: Int = 0, full: Bool = false) {

        cacheBus()
        
        if full {

            if busProbe0.menu.items.isEmpty {

                initProbeSelector(0, busProbe0)
                initProbeSelector(1, busProbe1)
                initProbeSelector(2, busProbe2)
                initProbeSelector(3, busProbe3)
            }

            refreshProbeSelector(busProbe0)
            refreshProbeSelector(busProbe1)
            refreshProbeSelector(busProbe2)
            refreshProbeSelector(busProbe3)
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
    
    @IBAction func comboButtonAction(_ sender: NSComboButton!) {
        
    }
    
    @IBAction func busProbeAction(_ sender: NSMenuItem!) {
        
        let channel = sender.tag / 128
        let index = sender.tag % 128
        
        let probe = Inspector.presets[index].1.0.rawValue
        let addr = Inspector.presets[index].1.1
                
        switch channel {
        case 0:  emu?.set(.LA_PROBE0, value: probe);
        case 1:  emu?.set(.LA_PROBE1, value: probe);
        case 2:  emu?.set(.LA_PROBE2, value: probe);
        case 3:  emu?.set(.LA_PROBE3, value: probe);
        default: break
        }
        
        if let addr = addr {
            
            switch channel {
            case 0:  emu?.set(.LA_ADDR0, value: addr)
            case 1:  emu?.set(.LA_ADDR1, value: addr)
            case 2:  emu?.set(.LA_ADDR2, value: addr)
            case 3:  emu?.set(.LA_ADDR3, value: addr)
            default: break
            }
        }
    }
    
    @IBAction func busAddrAction(_ sender: NSTextField!) {
        
        print("value = \(sender.stringValue) tag = \(sender.tag) ")
        
        if let addr = Int(sender.stringValue, radix: 16) {
                    
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
