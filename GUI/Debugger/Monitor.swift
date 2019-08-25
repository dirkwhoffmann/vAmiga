// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Monitor: NSWindowController {

    // DMA Debugger
    @IBOutlet weak var dmaDebugEnable: NSButton!

    @IBOutlet weak var dmaDebugCpu: NSButton!
    @IBOutlet weak var dmaDebugRefresh: NSButton!
    @IBOutlet weak var dmaDebugDisk: NSButton!
    @IBOutlet weak var dmaDebugAudio: NSButton!
    @IBOutlet weak var dmaDebugBitplane: NSButton!
    @IBOutlet weak var dmaDebugSprites: NSButton!
    @IBOutlet weak var dmaDebugCopper: NSButton!
    @IBOutlet weak var dmaDebugBlitter: NSButton!

    @IBOutlet weak var dmaDebugCpuCol: NSColorWell!
    @IBOutlet weak var dmaDebugRefreshCol: NSColorWell!
    @IBOutlet weak var dmaDebugDiskCol: NSColorWell!
    @IBOutlet weak var dmaDebugAudioCol: NSColorWell!
    @IBOutlet weak var dmaDebugBitplaneCol: NSColorWell!
    @IBOutlet weak var dmaDebugSpritesCol: NSColorWell!
    @IBOutlet weak var dmaDebugCopperCol: NSColorWell!
    @IBOutlet weak var dmaDebugBlitterCol: NSColorWell!

    @IBOutlet weak var dmaDebugOpacity: NSSlider!
    @IBOutlet weak var dmaDebugOverlay: NSButton!

    // Audio out
    @IBOutlet weak var audioWaveformView: WaveformView!
    @IBOutlet weak var audioBufferLevel: NSLevelIndicator!
    @IBOutlet weak var audioBufferLevelText: NSTextField!
    @IBOutlet weak var audioBufferUnderflows: NSTextField!
    @IBOutlet weak var audioBufferOverflows: NSTextField!

    var timer: Timer?

    // var refreshCounter = 0

    // Factory method
    static func make() -> Monitor? {

        return Monitor.init(windowNibName: "Monitor")
    }

    override func awakeFromNib() {

        track()
    }

    override func showWindow(_ sender: Any?) {

        track()
        super.showWindow(self)
        refresh(everything: true)

        timer = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true) { _ in
            if amigaProxy?.isRunning() == true {
                self.refresh(everything: false)
            }
        }
    }

    func refresh(everything: Bool) {

        if window?.isVisible == false { return }
        // refreshCounter += 1

        if everything {

        }

        refreshWaveformDisplay()
        refreshDmaDebugger()
    }
}

extension Monitor: NSWindowDelegate {

    func windowWillClose(_ notification: Notification) {

        track("Closing monitor")
        timer?.invalidate()
    }
}

//
// Audio waveform
//

extension Monitor {

    func refreshWaveformDisplay() {

        guard let paula = amigaProxy?.paula else { return }
        let fillLevel = Int32(paula.fillLevel() * 100)

        audioBufferLevel.intValue = fillLevel
        audioBufferLevelText.stringValue = "\(fillLevel) %"
        audioBufferUnderflows.intValue = Int32(paula.bufferUnderflows())
        audioBufferOverflows.intValue = Int32(paula.bufferOverflows())
        audioWaveformView.update()
    }
}

//
// DMA debugger
//

extension Monitor {

    func refreshDmaDebugger() {

        guard let dma = amigaProxy?.agnus else { return }
        let info = dma.getDebuggerInfo()
        let rgb = info.colorRGB

        dmaDebugEnable.state = info.enabled ? .on : .off

        dmaDebugCpu.state = info.visualize.1 ? .on : .off
        dmaDebugRefresh.state = info.visualize.2 ? .on : .off
        dmaDebugDisk.state = info.visualize.3 ? .on : .off
        dmaDebugAudio.state = info.visualize.4 ? .on : .off
        dmaDebugBitplane.state = info.visualize.5 ? .on : .off
        dmaDebugSprites.state = info.visualize.6 ? .on : .off
        dmaDebugCopper.state = info.visualize.7 ? .on : .off
        dmaDebugBlitter.state = info.visualize.8 ? .on : .off

        dmaDebugRefreshCol.color = NSColor.init(r: rgb.1.0, g: rgb.1.1, b: rgb.1.2)
        dmaDebugDiskCol.color = NSColor.init(r: rgb.3.0, g: rgb.3.1, b: rgb.3.2)
        dmaDebugAudioCol.color = NSColor.init(r: rgb.4.0, g: rgb.4.1, b: rgb.4.2)
        dmaDebugBitplaneCol.color = NSColor.init(r: rgb.5.0, g: rgb.5.1, b: rgb.5.2)
        dmaDebugSpritesCol.color = NSColor.init(r: rgb.6.0, g: rgb.6.1, b: rgb.6.2)
        dmaDebugCopperCol.color = NSColor.init(r: rgb.7.0, g: rgb.7.1, b: rgb.7.2)
        dmaDebugBlitterCol.color = NSColor.init(r: rgb.8.0, g: rgb.8.1, b: rgb.8.2)

        dmaDebugOpacity.doubleValue = info.opacity * 100.0
        dmaDebugOverlay.state = info.overlay ? .on : .off
        
        dmaDebugCpu.isEnabled = info.enabled
        dmaDebugRefresh.isEnabled = info.enabled
        dmaDebugDisk.isEnabled = info.enabled
        dmaDebugAudio.isEnabled = info.enabled
        dmaDebugBitplane.isEnabled = info.enabled
        dmaDebugSprites.isEnabled = info.enabled
        dmaDebugCopper.isEnabled = info.enabled
        dmaDebugBlitter.isEnabled = info.enabled

        dmaDebugRefreshCol.isEnabled = info.enabled
        dmaDebugDiskCol.isEnabled = info.enabled
        dmaDebugAudioCol.isEnabled = info.enabled
        dmaDebugBitplaneCol.isEnabled = info.enabled
        dmaDebugSpritesCol.isEnabled = info.enabled
        dmaDebugCopperCol.isEnabled = info.enabled
        dmaDebugBlitterCol.isEnabled = info.enabled

        dmaDebugOpacity.isEnabled = info.enabled
        dmaDebugOverlay.isEnabled = info.enabled
    }

    @IBAction func dmaDebugOnOffAction(_ sender: NSButton!) {

        if sender.state == .on {
            amigaProxy?.agnus.dmaDebugSetEnable(true)
        } else {
            amigaProxy?.agnus.dmaDebugSetEnable(false)
        }

        refresh(everything: false)
    }

    @IBAction func dmaDebugShowAction(_ sender: NSButton!) {

        let owner = BusOwner(Int8(sender.tag))
        amigaProxy?.agnus.dmaDebugSetVisualize(owner, value: sender.state == .on)
        refresh(everything: false)
    }

    @IBAction func dmaDebugColorAction(_ sender: NSColorWell!) {

        let color = sender.color
        let owner = BusOwner(Int8(sender.tag))
        let r = Double(color.redComponent)
        let g = Double(color.greenComponent)
        let b = Double(color.blueComponent)
        amigaProxy?.agnus.dmaDebugSetColor(owner, r: r, g: g, b: b)
        refresh(everything: false)
    }

    @IBAction func dmaDebugOpacityAction(_ sender: NSSlider!) {

        track("Value = \(sender.doubleValue)")

        amigaProxy?.agnus.dmaDebugSetOpacity(sender.doubleValue / 100.0)
        refresh(everything: false)
    }

    @IBAction func dmaDebugOverlayAction(_ sender: NSButton!) {

        track("Value = \(sender.state)")

        amigaProxy?.agnus.dmaDebugSetOverlay(sender.state == .on)
        refresh(everything: false)
    }

}
