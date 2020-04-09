// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Monitor: DialogController {

    // Debugger preferences
    var dmaDebugDialog: DmaDebugDialog?

    // Enables or disables DMA debugging
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

    // Timer for triggering continous update
    var timer: Timer?

    // Lock to prevent reentrance to the timer execution code
     var timerLock = NSLock()
    
    var refreshCounter = 0

    override func awakeFromNib() {

        track()
    }

    override func showWindow(_ sender: Any?) {

        track()
        
        super.showWindow(self)
        refresh(everything: true)

        timer = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true) { _ in

            self.timerLock.lock()
            if self.amiga.isRunning() { self.refresh(everything: false) }
            self.timerLock.unlock()
        }
    }

    deinit {
        track()
    }

    func refresh(everything: Bool) {

        if window?.isVisible == false { return }

        if everything {

        }

        refreshWaveformView()
        refreshActivityViews()

        if refreshCounter % 8 == 0 {
            let info = amiga.agnus.getDebuggerInfo()
            dmaDebugEnable.state = info.enabled ? .on : .off
            dmaDebugDialog?.refresh()
        }
        refreshCounter += 1
    }

    @IBAction func dmaConfigAction(_ sender: Any!) {

        if dmaDebugDialog == nil {
            let name = NSNib.Name("DmaDebugDialog")
            dmaDebugDialog = DmaDebugDialog.make(parent: parent, nibName: name)
        }

        window?.beginSheet(dmaDebugDialog!.window!, completionHandler: nil)
    }
}

extension Monitor: NSWindowDelegate {

    func windowWillClose(_ notification: Notification) {

        track("Closing monitor")

        // Disconnect the inspector from the parent controller
        parent?.monitor = nil

        // Stop the refresh timer
        timerLock.lock()
        timer?.invalidate()
        timer = nil
        timerLock.unlock()
    }
}

//
//  Waveform view
//

extension Monitor {

    func refreshWaveformView() {

        let stats = amiga.paula.getAudioStats()
        let fillLevel = Int32(amiga.paula.fillLevel() * 100)

        audioBufferLevel.intValue = fillLevel
        audioBufferLevelText.stringValue = "\(fillLevel) %"
        audioBufferUnderflows.integerValue = stats.bufferUnderflows
        audioBufferOverflows.integerValue = stats.bufferOverflows
        audioWaveformView.update()
    }
}

//
// Activity views
//

extension Monitor {

    func refreshActivityViews() {

        let agnusStats = amiga.agnus.getStats()
        let memStats = amiga.mem.getStats()
        
        let counts = [
            agnusStats.bus.accumulated.0,
            agnusStats.bus.accumulated.1,
            agnusStats.bus.accumulated.2,
            agnusStats.bus.accumulated.3,
            agnusStats.bus.accumulated.4,
            agnusStats.bus.accumulated.5,
            agnusStats.bus.accumulated.6,
            agnusStats.bus.accumulated.7,
            agnusStats.bus.accumulated.8
        ]
     
        let copDMA = Double(counts[Int(BUS_COPPER.rawValue)]) / (313*120)
        let bltDMA = Double(counts[Int(BUS_BLITTER.rawValue)]) / (313*120)

        // DEPRECATED
        let chipReads = Double(memStats.chipReads.accumulated) / (313*226)
        let chipWrites = Double(memStats.chipWrites.accumulated) / (313*226)
        let fastReads = Double(memStats.fastReads.accumulated) / (313*226)
        let fastWrites = Double(memStats.fastWrites.accumulated) / (313*226)
        let romReads = Double(memStats.romReads.accumulated) / (313*226)
        let romWrites = Double(memStats.romWrites.accumulated) / (313*226)

        copperView.add(val1: copDMA)
        blitterView.add(val1: bltDMA)
        chipView.add(val1: chipReads, val2: chipWrites)
        fastView.add(val1: fastReads, val2: fastWrites)
        romView.add(val1: romReads, val2: romWrites)
    }
}

//
// DMA Debugger
//

extension Monitor {

    @IBAction func dmaDebugEnableAction(_ sender: NSButton!) {

         if sender.state == .on {
             amiga.agnus.dmaDebugSetEnable(true)
         } else {
             amiga.agnus.dmaDebugSetEnable(false)
         }

         refresh(everything: false)
     }
}
