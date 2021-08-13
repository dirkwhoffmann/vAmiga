// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import AVFoundation

enum WarpMode: Int {

    case auto
    case off
    case on
}

protocol MessageReceiver {
    func processMessage(_ msg: Message)
}

class MyController: NSWindowController, MessageReceiver {

    // var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
    var pref: Preferences { return myAppDelegate.pref }

    // Reference to the connected document
    var mydocument: MyDocument!
    
    // Amiga proxy (bridge between the Swift frontend and the C++ backend)
    var amiga: AmigaProxy!

    // Inspector panel of this emulator instance
    var inspector: Inspector?

    // Monitor panel of this emulator instance
    var monitor: Monitor?
    
    // Configuration panel of this emulator instance
    var configurator: ConfigurationController?
    
    // Snapshot and screenshot browsers
    var snapshotBrowser: SnapshotDialog?
    var screenshotBrowser: ScreenshotDialog?

    // The current emulator configuration
    var config: Configuration!
    
    // Audio Engine
    var macAudio: MacAudio!
     
    // Game pad manager
    var gamePadManager: GamePadManager!
    var gamePad1: GamePad? { return gamePadManager.gamePads[config.gameDevice1] }
    var gamePad2: GamePad? { return gamePadManager.gamePads[config.gameDevice2] }
    
    // Keyboard controller
    var keyboard: KeyboardController!

    // Virtual keyboard
    var virtualKeyboard: VirtualKeyboardController?
    
    // Loop timer for scheduling periodic updates
    var timer: Timer?
    
    // Timer lock
    var timerLock: NSLock!
    
    // Screenshot and snapshot timers
    var snapshotTimer: Timer?
    
    // Speedometer to measure clock frequence and frames per second
    var speedometer: Speedometer!
    
    // Used inside the timer function to fine tune timed events
    var animationCounter = 0
        
    // Remembers if audio is muted (master volume of both channels is 0)
    var muted = false
    
    // Indicates if a status bar is shown
    var statusBar = true

    // Small disk icon to be shown in NSMenuItems
    var smallDisk = NSImage(named: "diskTemplate")!.resize(width: 16.0, height: 16.0)
    
    // Serial input and output
    var serialIn = ""
    var serialOut = ""
    
    // Remembers the running state for the pauseInBackground feature
    var pauseInBackgroundSavedState = false
    
    //
    // Timers
    //
        
    func validateSnapshotTimer() {
        
        snapshotTimer?.invalidate()
        if pref.autoSnapshots && pref.snapshotInterval > 0 {
            
            snapshotTimer =
                Timer.scheduledTimer(timeInterval: TimeInterval(pref.snapshotInterval),
                                     target: self,
                                     selector: #selector(snapshotTimerFunc),
                                     userInfo: nil,
                                     repeats: true)
        }
    }
    
    func stopSnapshotTimer() {
        
        snapshotTimer?.invalidate()
    }

    func updateWarp() {
        
        var warp: Bool
        
        switch pref.warpMode {
        case .auto: warp = amiga.diskController.isSpinning
        case .off: warp = false
        case .on: warp = true
        }
        
        if warp != amiga.warp { amiga.warp = warp }
    }

    //
    // Outlets
    //
    
    // Main screen
    @IBOutlet weak var metal: MetalView!

    var renderer: Renderer!

    // Status bar
    @IBOutlet weak var powerLED: NSButton!
    @IBOutlet weak var df0LED: NSButton!
    @IBOutlet weak var df1LED: NSButton!
    @IBOutlet weak var df2LED: NSButton!
    @IBOutlet weak var df3LED: NSButton!
    @IBOutlet weak var df0Cylinder: NSTextField!
    @IBOutlet weak var df1Cylinder: NSTextField!
    @IBOutlet weak var df2Cylinder: NSTextField!
    @IBOutlet weak var df3Cylinder: NSTextField!
    @IBOutlet weak var df0Disk: NSButton!
    @IBOutlet weak var df1Disk: NSButton!
    @IBOutlet weak var df2Disk: NSButton!
    @IBOutlet weak var df3Disk: NSButton!
    @IBOutlet weak var df0DMA: NSProgressIndicator!
    @IBOutlet weak var df1DMA: NSProgressIndicator!
    @IBOutlet weak var df2DMA: NSProgressIndicator!
    @IBOutlet weak var df3DMA: NSProgressIndicator!

    @IBOutlet weak var haltIcon: NSButton!
    @IBOutlet weak var cmdLock: NSButton!
    @IBOutlet weak var debugIcon: NSButton!
    @IBOutlet weak var muteIcon: NSButton!

    @IBOutlet weak var clockSpeed: NSTextField!
    @IBOutlet weak var clockSpeedBar: NSLevelIndicator!
    @IBOutlet weak var warpIcon: NSButton!

    // Toolbar
    @IBOutlet weak var toolbar: MyToolbar!
}

extension MyController {

    // Provides the undo manager
    override open var undoManager: UndoManager? { return metal.undoManager }
     
    // Indicates if the emulator needs saving
    var needsSaving: Bool {
        get {
            return document?.changeCount != 0
        }
        set {
            if newValue && !pref.closeWithoutAsking {
                document?.updateChangeCount(.changeDone)
            } else {
                document?.updateChangeCount(.changeCleared)
            }
        }
    }

    //
    // Initializing
    //
    
    override open func awakeFromNib() {
        
        mydocument = document as? MyDocument
        config = Configuration(with: self)
        macAudio = MacAudio(with: self)
    }

    override open func windowDidLoad() {
                         
        // Create keyboard controller
        keyboard = KeyboardController(parent: self)
        assert(keyboard != nil, "Failed to create keyboard controller")

        // Create game pad manager
        gamePadManager = GamePadManager(parent: self)
        assert(gamePadManager != nil, "Failed to create game pad manager")
        
        // Setup renderer
        renderer = Renderer(view: metal,
                            device: MTLCreateSystemDefaultDevice()!,
                            controller: self)

        // Setup window
        configureWindow()
        
        // Load user defaults
        loadUserDefaults()
        
        // Enable message processing
        setListener()

        // Process attachment (if any)
        try? mydocument.mountAttachment(destination: amiga.df0)

        do {
            // Let the Amiga throw an exception if it is not ready to power on
            try amiga.isReady()
            
            // Switch on and launch the emulator thread
            try amiga.run()

        } catch {

            // Open the Rom dialog
            openConfigurator(tab: "Roms")
        }

        // Create speed monitor and get the timer tunning
        createTimer()
        
        // Update toolbar
        toolbar.validateVisibleItems()
        
        // Update status bar
        refreshStatusBar()
    }
    
    func configureWindow() {
    
        // Add status bar
        window?.autorecalculatesContentBorderThickness(for: .minY)
        window?.setContentBorderThickness(32.0, for: .minY)
        statusBar = true
        
        // Adjust size and enable auto-save for window coordinates
        adjustWindowSize()
        window?.windowController?.shouldCascadeWindows = false // true ?!
        let name = NSWindow.FrameAutosaveName("dirkwhoffmann.de.vAmiga.window")
        window?.setFrameAutosaveName(name)
        
        // Enable fullscreen mode
        window?.collectionBehavior = .fullScreenPrimary
    }
    
    func setListener() {
                
        // Convert 'self' to a void pointer
        let myself = UnsafeRawPointer(Unmanaged.passUnretained(self).toOpaque())
        
        amiga.setListener(myself) { (ptr, type, data) in

            // Convert void pointer back to 'self'
            let myself = Unmanaged<MyController>.fromOpaque(ptr!).takeUnretainedValue()
            
            // Process message in the main thread
            DispatchQueue.main.async {
                let mType = MsgType(rawValue: type)
                myself.processMessage(Message(type: mType!, data: data))
            }
        }
    }
    
    func createTimer() {

        // Create speed monitor
        speedometer = Speedometer()
        
        // Create timer and speedometer
        timerLock = NSLock()
        timer = Timer.scheduledTimer(timeInterval: 1.0/10, // 10 times a second
                                     target: self,
                                     selector: #selector(timerFunc),
                                     userInfo: nil,
                                     repeats: true)
    }

    //
    // Timer and message processing
    //
    
    @objc func timerFunc() {

        assert(timerLock != nil)
        timerLock.lock()
 
        animationCounter += 1

        // Animate the inspector
        if inspector?.window?.isVisible == true { inspector!.continuousRefresh() }
 
        // Do less frequently...
        if (animationCounter % 2) == 0 {
 
        }
        
        // Do even less frequently...
        if (animationCounter % 3) == 0 {
            
            updateSpeedometer()
            updateMonitoringPanels()
            
            // Let the cursor disappear in fullscreen mode
            if renderer.fullscreen &&
                CGEventSource.secondsSinceLastEventType(.combinedSessionState,
                                                        eventType: .mouseMoved) > 1.0 {
                NSCursor.setHiddenUntilMouseMoves(true)
            }
        }
                
        timerLock.unlock()
    }
    
    func updateSpeedometer() {
        
        speedometer.updateWith(cycle: amiga.cpu.cycles, frame: renderer.frames)
        let mhz = speedometer.mhz
        let fps = speedometer.fps
        clockSpeed.stringValue = String(format: "%.2f MHz %.0f fps", mhz, fps)
        clockSpeedBar.doubleValue = 10 * mhz
    }

    func addValue(_ nr: Int, _ v: Float) {
        if let monitor = renderer.monitors.monitors[nr] as? BarChart {
            monitor.addValue(v)
        }
    }
    
    func addValues(_ nr: Int, _ v1: Float, _ v2: Float) {
        if let monitor = renderer.monitors.monitors[nr] as? BarChart {
            monitor.addValues(v1, v2)
        }
    }

    func updateMonitoringPanels() {
        
        if !renderer.monitors.isVisible { return }
            
        // DMA monitors
        let dma = amiga.agnus.getStats()
        let copDMA = Float(dma.copperActivity) / (313*120)
        let bltDMA = Float(dma.blitterActivity) / (313*120)
        let dskDMA = Float(dma.diskActivity) / (313*3)
        let audDMA = Float(dma.audioActivity) / (313*4)
        let sprDMA = Float(dma.spriteActivity) / (313*16)
        let bplDMA = Float(dma.bitplaneActivity) / 39330
        
        addValue(Monitors.Monitor.copper, copDMA)
        addValue(Monitors.Monitor.blitter, bltDMA)
        addValue(Monitors.Monitor.disk, dskDMA)
        addValue(Monitors.Monitor.audio, audDMA)
        addValue(Monitors.Monitor.sprite, sprDMA)
        addValue(Monitors.Monitor.bitplane, bplDMA)
        
        // Memory monitors
        let mem = amiga.mem.getStats()
        
        let max = Float((HPOS_CNT * VPOS_CNT) / 2)
        let chipR = Float(mem.chipReads.accumulated) / max
        let chipW = Float(mem.chipWrites.accumulated) / max
        let slowR = Float(mem.slowReads.accumulated) / max
        let slowW = Float(mem.slowWrites.accumulated) / max
        let fastR = Float(mem.fastReads.accumulated) / max
        let fastW = Float(mem.fastWrites.accumulated) / max
        let kickR = Float(mem.kickReads.accumulated) / max
        let kickW = Float(mem.kickWrites.accumulated) / max

        addValues(Monitors.Monitor.chipRam, chipR, chipW)
        addValues(Monitors.Monitor.slowRam, slowR, slowW)
        addValues(Monitors.Monitor.fastRam, fastR, fastW)
        addValues(Monitors.Monitor.kickRom, kickR, kickW)
    }
    
    @objc func snapshotTimerFunc() {

        if pref.autoSnapshots { amiga.requestAutoSnapshot() }
    }
            
    func processMessage(_ msg: Message) {
        
        var driveNr: Int { return msg.data & 0xFF }
        var driveCyl: Int { return (msg.data >> 8) & 0xFF; }
                
        // Only proceed if the proxy object is still alive
        if amiga == nil { return }

        switch msg.type {
    
        case .REGISTER:
            track("Successfully connected to message queue")
                        
        case .CONFIG:
            inspector?.fullRefresh()

        case .POWER_ON:
            renderer.canvas.open(delay: 1.5)
            serialIn = ""
            serialOut = ""
            virtualKeyboard = nil
            toolbar.updateToolbar()
            inspector?.fullRefresh()
            
        case .POWER_OFF:
            toolbar.updateToolbar()
            inspector?.fullRefresh()

        case .RUN:
            needsSaving = true
            toolbar.updateToolbar()
            inspector?.fullRefresh()
            refreshStatusBar()

        case .PAUSE:
            toolbar.updateToolbar()
            inspector?.fullRefresh()
            refreshStatusBar()

        case .RESET:
            mydocument.deleteBootDiskID()
            mydocument.setBootDiskID(amiga.df0.fnv)
            inspector?.fullRefresh()
            updateWarp()
            
        case .SCRIPT_DONE,
             .SCRIPT_PAUSE,
             .SCRIPT_ABORT:
            renderer.console.isDirty = true
            
        case .SCRIPT_WAKEUP:
            track()
            amiga.continueScript()
            renderer.console.isDirty = true
            
        case .HALT:
            shutDown()

        case .MUTE_ON:
            muted = true
            refreshStatusBar()
            
        case .MUTE_OFF:
            muted = false
            refreshStatusBar()

        case .WARP_ON,
             .WARP_OFF:
            refreshStatusBar()

        case .POWER_LED_ON:
            powerLED.image = NSImage(named: "powerLedOn")

        case .POWER_LED_DIM:
            powerLED.image = NSImage(named: "powerLedDim")

        case .POWER_LED_OFF:
            powerLED.image = NSImage(named: "powerLedOff")

        case .CLOSE_CONSOLE:
            renderer.console.close(delay: 0.25)
            
        case .DMA_DEBUG_ON:
            renderer.zoomTextureOut()

        case .DMA_DEBUG_OFF:
            renderer.zoomTextureIn()

        case .BREAKPOINT_CONFIG,
             .BREAKPOINT_REACHED,
             .WATCHPOINT_REACHED:
            inspector?.fullRefresh()
            inspector?.scrollToPC()

        case .CPU_HALT:
            refreshStatusBar()
            
        case .MEM_LAYOUT:
            inspector?.fullRefresh()

        case .DRIVE_CONNECT:
            hideOrShowDriveMenus()
            refreshStatusBar()
            
        case .DRIVE_DISCONNECT:
            hideOrShowDriveMenus()
            refreshStatusBar()

        case .DRIVE_SELECT:
            refreshStatusBar(writing: nil)

        case .DRIVE_READ:
            refreshStatusBar(writing: false)
            
        case .DRIVE_WRITE:
            refreshStatusBar(writing: true)

        case .DRIVE_LED_ON,
             .DRIVE_LED_OFF:
            refreshStatusBar()
            
        case .DRIVE_MOTOR_ON,
             .DRIVE_MOTOR_OFF:
            refreshStatusBar()
            updateWarp()

        case .DRIVE_STEP:
            macAudio.playStepSound(drive: driveNr)
            refreshStatusBar(drive: driveNr, cylinder: driveCyl)

        case .DRIVE_POLL:
            macAudio.playPollSound(drive: driveNr)
            refreshStatusBar(drive: driveNr, cylinder: driveCyl)
  
        case .DISK_INSERT:
            if driveNr == 0 { mydocument.setBootDiskID(amiga.df0.fnv) }
            macAudio.playInsertSound(drive: driveNr)
            refreshStatusBar()
            
        case .DISK_EJECT:
            macAudio.playEjectSound(drive: driveNr)
            refreshStatusBar()
            
        case .DISK_UNSAVED,
             .DISK_SAVED,
             .DISK_PROTECT,
             .DISK_UNPROTECT:
            refreshStatusBar()

        case .CTRL_AMIGA_AMIGA:
            resetAction(self)
            
        case .SER_IN:
            serialIn += String(UnicodeScalar(msg.data & 0xFF)!)

        case .SER_OUT:
            serialOut += String(UnicodeScalar(msg.data & 0xFF)!)

        case .AUTO_SNAPSHOT_TAKEN:
            mydocument.snapshots.append(amiga.latestAutoSnapshot)

        case .USER_SNAPSHOT_TAKEN:
            mydocument.snapshots.append(amiga.latestUserSnapshot)
            renderer.flash()
            
        case .SNAPSHOT_RESTORED:
            renderer.rotateRight()
            refreshStatusBar()
            hideOrShowDriveMenus()
            
        case .RECORDING_STARTED:
            window?.backgroundColor = .warningColor
            refreshStatusBar()
                
        case .RECORDING_STOPPED:
            window?.backgroundColor = .windowBackgroundColor
            refreshStatusBar()
            
        case .SHAKING:
            metal.lastShake = DispatchTime(uptimeNanoseconds: 0)
            if pref.releaseMouseByShaking {
                metal.releaseMouse()
            }
            
        default:
            track("Unknown message: \(msg)")
            assert(false)
        }
    }

    //
    // Action methods (status bar)
    //
    
    @IBAction func warpAction(_ sender: Any!) {

        track()

        switch pref.warpMode {
        case .auto: pref.warpMode = .off
        case .off: pref.warpMode = .on
        case .on: pref.warpMode = .auto
        }

        refreshStatusBar()
    }
}
