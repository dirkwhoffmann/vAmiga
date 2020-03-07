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
            let name = NSNib.Name("DMADebugDialog")
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

        let stats = amiga.getStats()

        let copperActivity = Double(stats.agnus.count.7) / (313*100)
        let blitterActivity = Double(stats.agnus.count.8) / (313*120)
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
