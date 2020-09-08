// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Monitor: DialogController {

    // Display colors
    @IBOutlet weak var colCopper: NSColorWell!
    @IBOutlet weak var colBlitter: NSColorWell!
    @IBOutlet weak var colDisk: NSColorWell!
    @IBOutlet weak var colAudio: NSColorWell!
    @IBOutlet weak var colSprites: NSColorWell!
    @IBOutlet weak var colBitplanes: NSColorWell!
    @IBOutlet weak var colCPU: NSColorWell!
    @IBOutlet weak var colRefresh: NSColorWell!
    
    // Bus debugger
    @IBOutlet weak var busEnable: NSButton!
    
    @IBOutlet weak var busCopper: NSButton!
    @IBOutlet weak var busBlitter: NSButton!
    @IBOutlet weak var busDisk: NSButton!
    @IBOutlet weak var busAudio: NSButton!
    @IBOutlet weak var busSprites: NSButton!
    @IBOutlet weak var busBitplanes: NSButton!
    @IBOutlet weak var busCPU: NSButton!
    @IBOutlet weak var busRefresh: NSButton!
        
    @IBOutlet weak var busOpacity: NSSlider!
    @IBOutlet weak var busDisplayMode: NSPopUpButton!
    
    // Activity monitors
    @IBOutlet weak var monEnable: NSButton!
    
    @IBOutlet weak var monCopper: NSButton!
    @IBOutlet weak var monBlitter: NSButton!
    @IBOutlet weak var monDisk: NSButton!
    @IBOutlet weak var monAudio: NSButton!
    @IBOutlet weak var monSprites: NSButton!
    @IBOutlet weak var monBitplanes: NSButton!

    @IBOutlet weak var monChipRam: NSButton!
    @IBOutlet weak var monSlowRam: NSButton!
    @IBOutlet weak var monFastRam: NSButton!
    @IBOutlet weak var monKickRom: NSButton!

    @IBOutlet weak var monLeftWave: NSButton!
    @IBOutlet weak var monRightWave: NSButton!

    @IBOutlet weak var monOpacity: NSSlider!
    @IBOutlet weak var monLayout: NSPopUpButton!
    @IBOutlet weak var monSlider: NSSlider!

    // Stencils
    @IBOutlet weak var synEnable: NSButton!
    @IBOutlet weak var synSprite0: NSButton!
    @IBOutlet weak var synSprite1: NSButton!
    @IBOutlet weak var synSprite2: NSButton!
    @IBOutlet weak var synSprite3: NSButton!
    @IBOutlet weak var synSprite4: NSButton!
    @IBOutlet weak var synSprite5: NSButton!
    @IBOutlet weak var synSprite6: NSButton!
    @IBOutlet weak var synSprite7: NSButton!
    @IBOutlet weak var synPlayfield1: NSButton!
    @IBOutlet weak var synPlayfield2: NSButton!
    @IBOutlet weak var synOpacity: NSSlider!

    var layers: Int {
        get { return amiga.getConfig(.OPT_HIDDEN_LAYERS) }
        set { amiga.configure(.OPT_HIDDEN_LAYERS, value: newValue) }
    }
    var layerAlpha: Int {
        get { return 255 - amiga.getConfig(.OPT_HIDDEN_LAYER_ALPHA) }
        set { amiga.configure(.OPT_HIDDEN_LAYER_ALPHA, value: 255 - newValue) }
    }
    
    override func awakeFromNib() {

        track()
        super.awakeFromNib()
        refresh()
    }

    func refresh() {
        
        func enabled(_ monitor: Int) -> NSControl.StateValue {
            return parent.renderer.monitorEnabled[monitor] ? .on : .off
        }
        
        let info = amiga.dmaDebugger.getInfo()
        let bus = info.enabled
        let mon = parent.renderer.drawActivityMonitors
        let syn = synEnable.state == .on
        let col = bus || mon
                
        // Bus debugger
        busEnable.state = bus ? .on : .off
        busCopper.state = info.visualizeCopper ? .on : .off
        busBlitter.state = info.visualizeBlitter ? .on : .off
        busDisk.state = info.visualizeDisk ? .on : .off
        busAudio.state = info.visualizeAudio ? .on : .off
        busSprites.state = info.visualizeSprites ? .on : .off
        busBitplanes.state = info.visualizeBitplanes ? .on : .off
        busCPU.state = info.visualizeCpu ? .on : .off
        busRefresh.state = info.visualizeRefresh ? .on : .off
        busOpacity.doubleValue = info.opacity * 100.0
        busDisplayMode.selectItem(withTag: info.displayMode.rawValue)
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
        
        // Activity monitors
        monEnable.state = mon ? .on : .off
        monCopper.state = enabled(Renderer.Monitor.copper)
        monBlitter.state = enabled(Renderer.Monitor.blitter)
        monDisk.state = enabled(Renderer.Monitor.disk)
        monAudio.state = enabled(Renderer.Monitor.audio)
        monSprites.state = enabled(Renderer.Monitor.sprite)
        monBitplanes.state = enabled(Renderer.Monitor.bitplane)
        monChipRam.state = enabled(Renderer.Monitor.chipRam)
        monSlowRam.state = enabled(Renderer.Monitor.slowRam)
        monFastRam.state = enabled(Renderer.Monitor.fastRam)
        monKickRom.state = enabled(Renderer.Monitor.kickRom)
        monLeftWave.state = enabled(Renderer.Monitor.waveformL)
        monRightWave.state = enabled(Renderer.Monitor.waveformR)
        monOpacity.floatValue = parent.renderer.monitorGlobalAlpha * 100.0
        monSlider.floatValue = parent.renderer.monitors[0].angle
        monLayout.selectItem(withTag: parent.renderer.monitorLayout)
        monCopper.isEnabled = mon
        monBlitter.isEnabled = mon
        monDisk.isEnabled = mon
        monAudio.isEnabled = mon
        monSprites.isEnabled = mon
        monBitplanes.isEnabled = mon
        monChipRam.isEnabled = mon
        monSlowRam.isEnabled = mon
        monFastRam.isEnabled = mon
        monKickRom.isEnabled = mon
        monLeftWave.isEnabled = mon
        monRightWave.isEnabled = mon
        monOpacity.isEnabled = mon
        monSlider.isEnabled = mon
        monLayout.isEnabled = mon
        
        // Colors
        colCopper.setColor(info.copperColor)
        colBlitter.setColor(info.blitterColor)
        colDisk.setColor(info.diskColor)
        colAudio.setColor(info.audioColor)
        colSprites.setColor(info.spriteColor)
        colBitplanes.setColor(info.bitplaneColor)
        colCPU.setColor(info.cpuColor)
        colRefresh.setColor(info.refreshColor)
        colBlitter.isHidden = !col
        colCopper.isHidden = !col
        colDisk.isHidden = !col
        colAudio.isHidden = !col
        colSprites.isHidden = !col
        colBitplanes.isHidden = !col
        colCPU.isHidden = !col
        colRefresh.isHidden = !col
        
        // Layers
        synOpacity.integerValue = layerAlpha
        synSprite0.isEnabled = syn
        synSprite1.isEnabled = syn
        synSprite2.isEnabled = syn
        synSprite3.isEnabled = syn
        synSprite4.isEnabled = syn
        synSprite5.isEnabled = syn
        synSprite6.isEnabled = syn
        synSprite7.isEnabled = syn
        synPlayfield1.isEnabled = syn
        synPlayfield2.isEnabled = syn
        synOpacity.isEnabled = syn
    }

    func updateHiddenLayers() {
                        
        var mask = 0
        
        if synEnable.state == .on {
            if synSprite0.state == .on { mask |= 0x01 }
            if synSprite1.state == .on { mask |= 0x02 }
            if synSprite2.state == .on { mask |= 0x04 }
            if synSprite3.state == .on { mask |= 0x08 }
            if synSprite4.state == .on { mask |= 0x10 }
            if synSprite5.state == .on { mask |= 0x20 }
            if synSprite6.state == .on { mask |= 0x40 }
            if synSprite7.state == .on { mask |= 0x80 }
            if synPlayfield1.state == .on { mask |= 0x100 }
            if synPlayfield2.state == .on { mask |= 0x200 }
        }

        layers = mask
        layerAlpha = synOpacity.integerValue
    }
    
    //
    // Action methods
    //
    
    @IBAction func colorAction(_ sender: NSColorWell!) {
        
        let r = Double(sender.color.redComponent)
        let g = Double(sender.color.greenComponent)
        let b = Double(sender.color.blueComponent)
        
        switch sender.tag {
        case 0: amiga.dmaDebugger.setCopperColor(r, g: g, b: b)
        case 1: amiga.dmaDebugger.setBlitterColor(r, g: g, b: b)
        case 2: amiga.dmaDebugger.setDiskColor(r, g: g, b: b)
        case 3: amiga.dmaDebugger.setAudioColor(r, g: g, b: b)
        case 4: amiga.dmaDebugger.setSpriteColor(r, g: g, b: b)
        case 5: amiga.dmaDebugger.setBitplaneColor(r, g: g, b: b)
        case 6: amiga.dmaDebugger.setCpuColor(r, g: g, b: b)
        case 7: amiga.dmaDebugger.setRefreshColor(r, g: g, b: b)
        default: fatalError()
        }
                
        let monitor = parent.renderer.monitors[sender.tag]
        monitor.setColor(sender.color)
        
        refresh()
    }

    @IBAction func busEnableAction(_ sender: NSButton!) {
        
        amiga.dmaDebugger.setEnable(sender.state == .on)
        refresh()
    }
    
    @IBAction func busDisplayAction(_ sender: NSButton!) {
        
        switch sender.tag {
        case 0: amiga.dmaDebugger.visualizeCopper(sender.state == .on)
        case 1: amiga.dmaDebugger.visualizeBlitter(sender.state == .on)
        case 2: amiga.dmaDebugger.visualizeDisk(sender.state == .on)
        case 3: amiga.dmaDebugger.visualizeAudio(sender.state == .on)
        case 4: amiga.dmaDebugger.visualizeSprite(sender.state == .on)
        case 5: amiga.dmaDebugger.visualizeBitplane(sender.state == .on)
        case 6: amiga.dmaDebugger.visualizeCpu(sender.state == .on)
        case 7: amiga.dmaDebugger.visualizeRefresh(sender.state == .on)
        default: fatalError()
        }
        refresh()
    }
    
    @IBAction func busDisplayModeAction(_ sender: NSPopUpButton!) {
        
        amiga.dmaDebugger.setDisplayMode(sender.selectedTag())
        refresh()
    }
    
    @IBAction func busOpacityAction(_ sender: NSSlider!) {
        
        amiga.dmaDebugger.setOpacity(sender.doubleValue / 100.0)
        refresh()
    }
    
    @IBAction func monEnableAction(_ sender: NSButton!) {
    
        parent.renderer.drawActivityMonitors = sender.state == .on
        refresh()
    }
    
    @IBAction func monDisplayAction(_ sender: NSButton!) {
        
        track("\(sender.tag)")
        parent.renderer.monitorEnabled[sender.tag] = sender.state == .on
        refresh()
    }
    
    @IBAction func monLayoutAction(_ sender: NSPopUpButton!) {
        
        track("\(sender.selectedTag())")
        parent.renderer.monitorLayout = sender.selectedTag()
        refresh()
    }
    
    @IBAction func monRotationAction(_ sender: NSSlider!) {
        
        track()
        for i in 0 ..< parent.renderer.monitors.count {
            parent.renderer.monitors[i].angle = sender.floatValue
        }
        refresh()
    }
    
    @IBAction func monOpacityAction(_ sender: NSSlider!) {
        
        parent.renderer.monitorGlobalAlpha = sender.floatValue / 100.0
        refresh()
    }

    @IBAction func synAction(_ sender: NSButton!) {
        
        updateHiddenLayers()
        refresh()
    }

    @IBAction func synOpacityAction(_ sender: NSSlider!) {
        
        updateHiddenLayers()
        refresh()
    }
}

extension Monitor: NSWindowDelegate {

    func windowWillClose(_ notification: Notification) {

        track("Closing monitor")
    }
}
