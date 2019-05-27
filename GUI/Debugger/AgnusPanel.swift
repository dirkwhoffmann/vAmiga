// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
    
    func refreshAgnus(everything: Bool) {
        
        guard let dma = amigaProxy?.dma else { return }
        let info = dma.getInfo()
        
        if everything {
            
        }
        
        // DMA control
        dmaDMACON.intValue = Int32(info.dmacon)
        dmaBLTPRI.state = (info.dmacon & 0b0000010000000000 != 0) ? .on : .off
        dmaDMAEN.state  = (info.dmacon & 0b0000001000000000 != 0) ? .on : .off
        dmaBPLEN.state  = (info.dmacon & 0b0000000100000000 != 0) ? .on : .off
        dmaCOPEN.state  = (info.dmacon & 0b0000000010000000 != 0) ? .on : .off
        dmaBLTEN.state  = (info.dmacon & 0b0000000001000000 != 0) ? .on : .off
        dmaSPREN.state  = (info.dmacon & 0b0000000000100000 != 0) ? .on : .off
        dmaDSKEN.state  = (info.dmacon & 0b0000000000010000 != 0) ? .on : .off
        dmaAUD3EN.state  = (info.dmacon & 0b0000000000001000 != 0) ? .on : .off
        dmaAUD2EN.state  = (info.dmacon & 0b0000000000000100 != 0) ? .on : .off
        dmaAUD1EN.state  = (info.dmacon & 0b0000000000000010 != 0) ? .on : .off
        dmaAUD0EN.state  = (info.dmacon & 0b0000000000000001 != 0) ? .on : .off
        
        // Display DMA
        dmaDIWSTRT.integerValue = Int(info.diwstrt)
        dmaDIWSTOP.integerValue = Int(info.diwstop)
        dmaDDFSTRT.integerValue = Int(info.ddfstrt)
        dmaDDFSTOP.integerValue = Int(info.ddfstop)

        dmaBPL1PT.integerValue = Int(info.bplpt.0)
        dmaBPL2PT.integerValue = Int(info.bplpt.1)
        dmaBPL3PT.integerValue = Int(info.bplpt.2)
        dmaBPL4PT.integerValue = Int(info.bplpt.3)
        dmaBPL5PT.integerValue = Int(info.bplpt.4)
        dmaBPL6PT.integerValue = Int(info.bplpt.5)

        dmaBPL1MOD.integerValue = Int(info.bpl1mod)
        dmaBPL2MOD.integerValue = Int(info.bpl2mod)
        
        dmaBpl1Enable.state = info.numBpls >= 1 ? .on : .off
        dmaBpl2Enable.state = info.numBpls >= 2 ? .on : .off
        dmaBpl3Enable.state = info.numBpls >= 3 ? .on : .off
        dmaBpl4Enable.state = info.numBpls >= 4 ? .on : .off
        dmaBpl5Enable.state = info.numBpls >= 5 ? .on : .off
        dmaBpl6Enable.state = info.numBpls >= 6 ? .on : .off

        refreshDmaDebugger()
    }

    @IBAction func dmaDMACONTextAction(_ sender: NSTextField!) {
        
        let value = sender.integerValue & 0xFFFF
        
        track("New value: \(value)")
        amigaProxy?.mem.pokeCustom16(DMACON, value: value)
        refresh(everything: false)
    }

    @IBAction func dmaDMACONButtonAction(_ sender: NSButton!) {
        
        let value = (1 << sender.tag) + (sender.state == .on ? 0x8000 : 0)
        amigaProxy?.mem.pokeCustom16(DMACON, value: Int(value))
        refresh(everything: false)
    }

    @IBAction func dmaVPosAction(_ sender: NSButton!) {
        
        let value = sender.integerValue
        track("New value: \(value)")
        track("Setter not implemented yet")
        refresh(everything: false)
    }

    @IBAction func dmaHPosAction(_ sender: NSButton!) {
        
        let value = sender.integerValue
        track("New value: \(value)")
        track("Setter not implemented yet")
        refresh(everything: false)
    }
    
    @IBAction func dmaBitplaneButtonAction(_ sender: NSButton!) {
        
        let activePlanes = (sender.state == .on) ? sender.tag : sender.tag - 1
        
        assert(activePlanes >= 0 && activePlanes <= 6)
        track("Active planes = \(activePlanes)")
        amigaProxy?.denise.setBPU(activePlanes)
        refresh(everything: false)
    }
}

extension Inspector {

    func refreshDmaDebugger() {

        guard let dma = amigaProxy?.dma else { return }
        let info = dma.getDebuggerInfo()
        let rgb = info.colorRGB

        dmaDebugEnable.state = info.enabled ? .on : .off
        dmaDebugDisk.state = info.visualize.2 ? .on : .off
        dmaDebugAudio.state = info.visualize.3 ? .on : .off
        dmaDebugSprite.state = info.visualize.4 ? .on : .off
        dmaDebugBitplane.state = info.visualize.5 ? .on : .off
        dmaDebugDiskCol.color = NSColor.init(r: rgb.2.0, g: rgb.2.1, b: rgb.2.2)
        dmaDebugAudioCol.color = NSColor.init(r: rgb.3.0, g: rgb.3.1, b: rgb.3.2)
        dmaDebugSpriteCol.color = NSColor.init(r: rgb.4.0, g: rgb.4.1, b: rgb.4.2)
        dmaDebugBitplaneCol.color = NSColor.init(r: rgb.5.0, g: rgb.5.1, b: rgb.5.2)
        dmaDebugOpacity.doubleValue = info.opacity * 100.0

        dmaDebugDisk.isEnabled = info.enabled
        dmaDebugAudio.isEnabled = info.enabled
        dmaDebugSprite.isEnabled = info.enabled
        dmaDebugBitplane.isEnabled = info.enabled
        dmaDebugDiskCol.isEnabled = info.enabled
        dmaDebugAudioCol.isEnabled = info.enabled
        dmaDebugSpriteCol.isEnabled = info.enabled
        dmaDebugBitplaneCol.isEnabled = info.enabled
        dmaDebugOpacity.isEnabled = info.enabled
    }

    @IBAction func dmaDebugOnOffAction(_ sender: NSButton!) {

        if sender.state == .on {
            amigaProxy?.dma.dmaDebugSetEnable(true)
        } else {
            amigaProxy?.dma.dmaDebugSetEnable(false)
        }
        
        refresh(everything: false)
    }

    @IBAction func dmaDebugShowAction(_ sender: NSButton!) {

        track("Tag: \(sender.tag) New value: \(sender.state)")
        let owner = BusOwner(Int8(sender.tag))
        amigaProxy?.dma.dmaDebugSetVisualize(owner, value: sender.state == .on)
        refresh(everything: false)
    }

    @IBAction func dmaDebugColorAction(_ sender: NSColorWell!) {

        let color = sender.color
        track("Tag: \(sender.tag) New value: \(color)")
        let owner = BusOwner(Int8(sender.tag))
        let r = Double(sender.color.redComponent)
        let g = Double(sender.color.greenComponent)
        let b = Double(sender.color.blueComponent)
        amigaProxy?.dma.dmaDebugSetColor(owner, r: r, g: g, b: b)
        refresh(everything: false)
    }

    @IBAction func dmaDebugOpacityAction(_ sender: NSSlider!) {

        track()
        let value = sender.floatValue
        track("New value: \(value)")
        amigaProxy?.dma.dmaDebugSetOpacity(sender.doubleValue / 100.0)
        refresh(everything: false)
    }
}
