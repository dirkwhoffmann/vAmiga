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
    @IBOutlet weak var chipRamView: ActivityView!
    @IBOutlet weak var fastRamView: ActivityView!
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

        if let stats = amigaProxy?.getStats() {

            let wc0 = stats.disk.wordCount.0
            let wc1 = stats.disk.wordCount.1
            let wc2 = stats.disk.wordCount.2
            let wc3 = stats.disk.wordCount.3
            let diskActivity = Double(wc0 + wc1 + wc2 + wc3) / (313.0 * 3)

            var f = stats.agnus.frames
            if f == 0 { f = 1 }

/*
            busUsageView.shiftValues()
            busUsageView.add(value: stats.agnus.count.0 / f, busOwner: 0)
            busUsageView.add(value: stats.agnus.count.1 / f, busOwner: 1)
            busUsageView.add(value: stats.agnus.count.2 / f, busOwner: 2)
            busUsageView.add(value: stats.agnus.count.3 / f, busOwner: 3)
            busUsageView.add(value: stats.agnus.count.4 / f, busOwner: 4)
            busUsageView.add(value: stats.agnus.count.5 / f, busOwner: 5)
            busUsageView.add(value: stats.agnus.count.6 / f, busOwner: 6)
            busUsageView.add(value: stats.agnus.count.7 / f, busOwner: 7)
            busUsageView.add(value: stats.agnus.count.8 / f, busOwner: 8)
            busUsageView.update()
*/
            copperView.logscale = true
            copperView.negative = false
            blitterView.logscale = true
            blitterView.negative = false
            copperView.add(value: Double(stats.agnus.count.7) / Double(f) / (313*113))
            blitterView.add(value: Double(stats.agnus.count.8) / Double(f) / (313*226))
            spriteView.add(value: Double(stats.denise.spriteLines) / 313.0)

            diskView.add(value: diskActivity)
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
