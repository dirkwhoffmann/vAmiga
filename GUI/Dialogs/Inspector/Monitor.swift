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
        get { return amiga.getConfig(.HIDDEN_LAYERS) }
        set { amiga.configure(.HIDDEN_LAYERS, value: newValue) }
    }
    var layerAlpha: Int {
        get { return 255 - amiga.getConfig(.HIDDEN_LAYER_ALPHA) }
        set { amiga.configure(.HIDDEN_LAYER_ALPHA, value: 255 - newValue) }
    }
    
    override func awakeFromNib() {

        super.awakeFromNib()
        refresh()
    }
    
    func refresh() {
                
        func enabled(_ i: Int) -> NSControl.StateValue {
            return parent.renderer.monitors.monitors[i].hidden ? .off : .on
        }
        
        let bus = amiga.getConfig(.DMA_DEBUG_ENABLE) != 0
        let mon = monEnable.state == .on
        let syn = synEnable.state == .on
        let col = bus || monEnable.isEnabled
                
        // Bus debugger
        let info = amiga.dmaDebugger.info
        let opacity = amiga.getConfig(.DMA_DEBUG_OPACITY)
        let mode = amiga.getConfig(.DMA_DEBUG_MODE)
        
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
        
        // Activity monitors
        monCopper.state = enabled(Monitors.Monitor.copper)
        monBlitter.state = enabled(Monitors.Monitor.blitter)
        monDisk.state = enabled(Monitors.Monitor.disk)
        monAudio.state = enabled(Monitors.Monitor.audio)
        monSprites.state = enabled(Monitors.Monitor.sprite)
        monBitplanes.state = enabled(Monitors.Monitor.bitplane)
        monChipRam.state = enabled(Monitors.Monitor.chipRam)
        monSlowRam.state = enabled(Monitors.Monitor.slowRam)
        monFastRam.state = enabled(Monitors.Monitor.fastRam)
        monKickRom.state = enabled(Monitors.Monitor.kickRom)
        monLeftWave.state = enabled(Monitors.Monitor.waveformL)
        monRightWave.state = enabled(Monitors.Monitor.waveformR)
        monOpacity.floatValue = parent.renderer.monitors.maxAlpha * 100.0
        monSlider.floatValue = parent.renderer.monitors.monitors[0].angle
        monLayout.selectItem(withTag: parent.renderer.monitors.layout)
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
    
    @IBAction override func okAction(_ sender: Any!) {
                        
        close()
    }
    
    @IBAction func colorAction(_ sender: NSColorWell!) {
        
        let r = Int(sender.color.redComponent * 255)
        let g = Int(sender.color.greenComponent * 255.0)
        let b = Int(sender.color.blueComponent * 255.0)
        let rgb = (r << 24) | (g << 16) | (b << 8)

        amiga.configure(.DMA_DEBUG_COLOR, id: sender.tag, value: rgb)
                
        let monitor = parent.renderer.monitors.monitors[sender.tag]
        monitor.setColor(sender.color)
        
        refresh()
    }

    @IBAction func busEnableAction(_ sender: NSButton!) {
        
        amiga.configure(.DMA_DEBUG_ENABLE, enable: sender.state == .on)
        refresh()
    }
    
    @IBAction func busDisplayAction(_ sender: NSButton!) {
        
        amiga.configure(.DMA_DEBUG_ENABLE, id: sender.tag, enable: sender.state == .on)
        refresh()
    }
    
    @IBAction func busDisplayModeAction(_ sender: NSPopUpButton!) {
        
        amiga.configure(.DMA_DEBUG_MODE, value: sender.selectedTag())
        refresh()
    }
    
    @IBAction func busOpacityAction(_ sender: NSSlider!) {

        amiga.configure(.DMA_DEBUG_OPACITY, value: sender.integerValue)
        refresh()
    }
    
    @IBAction func monEnableAction(_ sender: NSButton!) {
    
        if sender.state == .on {
            parent.renderer.monitors.open(delay: 1.0)
        } else {
            parent.renderer.monitors.close(delay: 1.0)
        }
        refresh()
    }
    
    @IBAction func monDisplayAction(_ sender: NSButton!) {
        
        parent.renderer.monitors.monitors[sender.tag].hidden = sender.state == .off
        refresh()
    }
    
    @IBAction func monLayoutAction(_ sender: NSPopUpButton!) {
        
        parent.renderer.monitors.layout = sender.selectedTag()
        refresh()
    }
    
    @IBAction func monRotationAction(_ sender: NSSlider!) {

        for i in 0 ..< parent.renderer.monitors.monitors.count {
            parent.renderer.monitors.monitors[i].angle = sender.floatValue
        }
        refresh()
    }
    
    @IBAction func monOpacityAction(_ sender: NSSlider!) {

        parent.renderer.monitors.maxAlpha = sender.floatValue / 100.0
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
