// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Monitor: NSWindowController {

    // Preferences controller
    var dmaDebugController: DMADebugController?

    // DMA Debugger
    @IBOutlet weak var dmaDebugEnable: NSButton!

    // Waveform view
    @IBOutlet weak var audioWaveformView: WaveformView!
    @IBOutlet weak var audioBufferLevel: NSLevelIndicator!
    @IBOutlet weak var audioBufferLevelText: NSTextField!
    @IBOutlet weak var audioBufferUnderflows: NSTextField!
    @IBOutlet weak var audioBufferOverflows: NSTextField!

    // Activity views
    @IBOutlet weak var blitterView: ActivityView!
    @IBOutlet weak var copperView: ActivityView!
    @IBOutlet weak var spriteView: ActivityView!
    @IBOutlet weak var chipView: ActivityView!
    @IBOutlet weak var fastView: ActivityView!
    @IBOutlet weak var romView: ActivityView!
    @IBOutlet weak var diskView: ActivityView!
    @IBOutlet weak var serialView: ActivityView!

    var timer: Timer?
    var refreshCounter = 0

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

        if everything {

        }

        refreshWaveformView()
        refreshActivityViews()

        if refreshCounter % 8 == 0 {
            if let info = amigaProxy?.agnus.getDebuggerInfo() {
                dmaDebugEnable.state = info.enabled ? .on : .off
            }
            dmaDebugController?.refresh()
        }
        refreshCounter += 1
    }

    @IBAction func dmaConfigAction(_ sender: Any!) {

        if dmaDebugController == nil {
            let nibName = NSNib.Name("DMADebugDialog")
            dmaDebugController = DMADebugController.init(windowNibName: nibName)
        }
        // dmaDebugController!.showSheet()
        window?.beginSheet(dmaDebugController!.window!, completionHandler: { result in
             if result == NSApplication.ModalResponse.OK {
             }
         })

    }
}

extension Monitor: NSWindowDelegate {

    func windowWillClose(_ notification: Notification) {

        track("Closing monitor")
        timer?.invalidate()
    }
}

//
//  Waveform view
//

extension Monitor {

    func refreshWaveformView() {

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
// Activity views
//

extension Monitor {

    func refreshActivityViews() {

        // let col1 = NSColor.init(r: 0, g: 204, b: 102, a: 255)
        // let col2 = NSColor.init(r: 0, g: 128, b: 255, a: 255)

        copperView.logscale = true
        copperView.splitview = false
        blitterView.logscale = true
        blitterView.splitview = false
        spriteView.logscale = false
        spriteView.splitview = false
        chipView.logscale = true
        chipView.splitview = true
        // chipView.color1 = col1
        // chipView.color2 = col2
        fastView.logscale = true
        fastView.splitview = true
        romView.logscale = true
        romView.splitview = true
        // romView.color1 = col1
        // romView.color2 = col2
        diskView.logscale = false
        diskView.splitview = true
        serialView.logscale = false
        serialView.splitview = true

        if let stats = amigaProxy?.getStats() {

            let copperActivity = Double(stats.agnus.count.7) / (313*113)
            let blitterActivity = Double(stats.agnus.count.8) / (313*226)
            let spriteActivity = Double(stats.denise.spriteLines) / 313
            let chipReads = Double(stats.mem.chipReads) / (313*226)
            let chipWrites = Double(stats.mem.chipWrites) / (313*226)
            let fastReads = Double(stats.mem.fastReads) / (313*226)
            let fastWrites = Double(stats.mem.fastWrites) / (313*226)
            let romReads = Double(stats.mem.romReads) / (313*226)
            let romWrites = Double(stats.mem.romWrites) / (313*226)

            let wc0 = stats.disk.wordCount.0
            let wc1 = stats.disk.wordCount.1
            let wc2 = stats.disk.wordCount.2
            let wc3 = stats.disk.wordCount.3
            let diskActivity = Double(wc0 + wc1 + wc2 + wc3) / (313.0 * 3)
            let serialReads = Double(stats.uart.reads) / 500
            let serialWrites = Double(stats.uart.writes) / 500

            let frames = Double(max(stats.frames, 1))

            copperView.add(val1: copperActivity / frames)
            blitterView.add(val1: blitterActivity / frames)
            spriteView.add(val1: spriteActivity / frames)
            chipView.add(val1: chipReads / frames, val2: chipWrites / frames)
            fastView.add(val1: fastReads / frames, val2: fastWrites / frames)
            romView.add(val1: romReads / frames, val2: romWrites / frames)
            diskView.add(val1: diskActivity / frames)
            serialView.add(val1: serialReads / frames, val2: serialWrites / frames)
        }
    }
}

//
// DMA Debugger
//

extension Monitor {

    @IBAction func dmaDebugEnableAction(_ sender: NSButton!) {

         if sender.state == .on {
             amigaProxy?.agnus.dmaDebugSetEnable(true)
         } else {
             amigaProxy?.agnus.dmaDebugSetEnable(false)
         }

         refresh(everything: false)
     }
}
