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
    
    // Pictograms for being used in NSMenuItems
    var smallDisk = NSImage(named: "diskTemplate")!.resize(width: 16.0, height: 16.0)
    var smallHdr = NSImage(named: "hdrTemplate")!.resize(width: 16.0, height: 16.0)

    // Serial input and output
    var serialIn = ""
    var serialOut = ""
    
    // Remembers the running state for the pauseInBackground feature
    var pauseInBackgroundSavedState = false
    
    //
    // Outlets
    //
    
    // Main screen
    @IBOutlet weak var metal: MetalView!
    
    var renderer: Renderer!
    
    // Status bar
    @IBOutlet weak var powerLED: NSButton!
    @IBOutlet weak var ledSlot0: NSButton!
    @IBOutlet weak var ledSlot1: NSButton!
    @IBOutlet weak var letSlot2: NSButton!
    @IBOutlet weak var ledSlot3: NSButton!
    @IBOutlet weak var cylSlot0: NSTextField!
    @IBOutlet weak var cylSlot1: NSTextField!
    @IBOutlet weak var cylSlot2: NSTextField!
    @IBOutlet weak var cylSlot3: NSTextField!
    @IBOutlet weak var iconSlot0: NSButton!
    @IBOutlet weak var iconSlot1: NSButton!
    @IBOutlet weak var iconSlot2: NSButton!
    @IBOutlet weak var iconSlot3: NSButton!
    @IBOutlet weak var busySlot0: NSProgressIndicator!
    @IBOutlet weak var busySlot1: NSProgressIndicator!
    @IBOutlet weak var busySlot2: NSProgressIndicator!
    @IBOutlet weak var busySlot3: NSProgressIndicator!
    
    @IBOutlet weak var haltIcon: NSButton!
    @IBOutlet weak var cmdLock: NSButton!
    @IBOutlet weak var debugIcon: NSButton!
    @IBOutlet weak var muteIcon: NSButton!
    
    @IBOutlet weak var warpIcon: NSButton!
    @IBOutlet weak var activityType: NSPopUpButton!
    @IBOutlet weak var activityInfo: NSTextField!
    @IBOutlet weak var activityBar: NSLevelIndicator!
    
    // Toolbar
    @IBOutlet weak var toolbar: MyToolbar!
    
    // Quick-access references
    var ledSlot: [NSButton]!
    var cylSlot: [NSTextField]!
    var iconSlot: [NSButton]!
    var busySlot: [NSProgressIndicator]!

    // Slot assignments
    var drvLED: [NSButton?] = Array(repeating: nil, count: 8)
    var drvCyl: [NSTextField?] = Array(repeating: nil, count: 8)
    var drvIcon: [NSButton?] = Array(repeating: nil, count: 8)
    var drvBusy: [NSProgressIndicator?] = Array(repeating: nil, count: 8)
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
        
        ledSlot = [ ledSlot0, ledSlot1, letSlot2, ledSlot3 ]
        cylSlot = [ cylSlot0, cylSlot1, cylSlot2, cylSlot3 ]
        iconSlot = [ iconSlot0, iconSlot1, iconSlot2, iconSlot3 ]
        busySlot = [ busySlot0, busySlot1, busySlot2, busySlot3 ]
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
        registerAsListener()
        
        // Process attachment (if any)
        try? mydocument.mountAttachment(destination: amiga.df0)
        
        do {
            // Switch the Amiga on
            amiga.powerOn()
            
            // Start the emulation
            try amiga.run()
            
        } catch {
            
            // Switch the Amiga off
            amiga.powerOff()
            
            // Open the Rom dialog after a small delay
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.75) {
                self.openConfigurator(tab: "Roms")
            }
        }
        
        // Create speed monitor
        speedometer = Speedometer()
        
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
    
    func registerAsListener() {
        
        // Convert 'self' to a void pointer
        let myself = UnsafeRawPointer(Unmanaged.passUnretained(self).toOpaque())
        
        amiga.setListener(myself) { (ptr, type, data) in
            
            // Convert void pointer back to 'self'
            let myself = Unmanaged<MyController>.fromOpaque(ptr!).takeUnretainedValue()
            
            // Process message in the main thread
            DispatchQueue.main.async {
                let mType = MsgType(rawValue: type)
                myself.processMessage(Message(type: mType!, data: i64(data)))
            }
        }
    }
    
    //
    // Timer and message processing
    //
    
    func timerFunc() {
        
        animationCounter += 1
        
        // Animate the inspector
        if inspector?.window?.isVisible == true { inspector!.continuousRefresh() }
        
        // Do less times...
        if (animationCounter % 3) == 0 {
            
            updateSpeedometer()
            updateMonitoringPanels()
        }
        
        // Do lesser times...
        if (animationCounter % 32) == 0 {
            
            // Let the cursor disappear in fullscreen mode
            if renderer.fullscreen &&
                CGEventSource.secondsSinceLastEventType(.combinedSessionState,
                                                        eventType: .mouseMoved) > 1.0 {
                NSCursor.setHiddenUntilMouseMoves(true)
            }
        }
    }
    
    func updateWarp() {
        
        var warp: Bool
        
        switch pref.warpMode {
        case .auto: warp = amiga.diskController.isSpinning
        case .off: warp = false
        case .on: warp = true
        }
        
        if warp != amiga.warpMode { amiga.warpMode = warp }
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
    
    func processMessage(_ msg: Message) {
        
        var driveNr: Int { return Int(msg.data) & 0xFF }
        var driveCyl: Int { return (Int(msg.data) >> 8) & 0xFF }
        var word1: Int { return (Int(msg.data) >> 48) & 0xFFFF }
        var word2: Int { return (Int(msg.data) >> 32) & 0xFFFF }
        var word3: Int { return (Int(msg.data) >> 16) & 0xFFFF }
        var word4: Int { return Int(msg.data) & 0xFFFF }
                                                         
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
            inspector?.powerOn()
            
        case .POWER_OFF:
            toolbar.updateToolbar()
            inspector?.powerOff()
            
        case .RUN:
            needsSaving = true
            toolbar.updateToolbar()
            inspector?.run()
            refreshStatusBar()
            
        case .PAUSE:
            toolbar.updateToolbar()
            inspector?.pause()
            refreshStatusBar()
            
        case .STEP:
            needsSaving = true
            inspector?.step()
            
        case .RESET:
            mydocument.deleteBootDiskID()
            mydocument.setBootDiskID(amiga.df0.fnv)
            inspector?.reset()
            updateWarp()
            
        case .CLOSE_CONSOLE:
            renderer.console.close(delay: 0.25)
            
        case .UPDATE_CONSOLE:
            renderer.console.isDirty = true

        case .SCRIPT_DONE, .SCRIPT_PAUSE, .SCRIPT_ABORT:
            break
            
        case .SCRIPT_WAKEUP:
            amiga.continueScript()
            
        case .HALT:
            shutDown()
            
        case .MUTE_ON:
            muted = true
            refreshStatusBar()
            
        case .MUTE_OFF:
            muted = false
            refreshStatusBar()
            
        case .WARP_ON, .WARP_OFF, .DEBUG_ON, .DEBUG_OFF:
            refreshStatusBar()
            
        case .POWER_LED_ON:
            powerLED.image = NSImage(named: "powerLedOn")
            
        case .POWER_LED_DIM:
            powerLED.image = NSImage(named: "powerLedDim")
            
        case .POWER_LED_OFF:
            powerLED.image = NSImage(named: "powerLedOff")
                        
        case .DMA_DEBUG_ON:
            renderer.zoomTextureOut()
            
        case .DMA_DEBUG_OFF:
            renderer.zoomTextureIn()
            
        case .BREAKPOINT_CONFIG:
            inspector?.fullRefresh()
            inspector?.scrollToPC()
            
        case .BREAKPOINT_REACHED:
            inspector?.signalBreakPoint(pc: Int(msg.data))
            
        case .WATCHPOINT_REACHED:
            inspector?.signalWatchPoint(pc: Int(msg.data))
            
        case .CPU_HALT:
            refreshStatusBar()
            
        case .MEM_LAYOUT:
            inspector?.fullRefresh()
            
        case .DRIVE_CONNECT:
            hideOrShowDriveMenus()
            assignSlots()
            refreshStatusBar()
            
        case .DRIVE_DISCONNECT:
            hideOrShowDriveMenus()
            assignSlots()
            refreshStatusBar()
            
        case .DRIVE_SELECT:
            refreshStatusBar(writing: nil)
            
        case .DRIVE_READ:
            refreshStatusBar(writing: false)
            
        case .DRIVE_WRITE:
            refreshStatusBar(writing: true)
            
        case .DRIVE_LED_ON, .DRIVE_LED_OFF:
            refreshStatusBar()
            
        case .DRIVE_MOTOR_ON, .DRIVE_MOTOR_OFF:
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
            
        case .DISK_UNSAVED, .DISK_SAVED, .DISK_PROTECT, .DISK_UNPROTECT:
            refreshStatusBar()

        case .HDR_CONNECT:
            hideOrShowDriveMenus()
            assignSlots()
            refreshStatusBar()
            
        case .HDR_DISCONNECT:
            hideOrShowDriveMenus()
            assignSlots()
            refreshStatusBar()

        case .HDR_STEP:
            
            macAudio.playSound(name: "hdr_click", volume: word3, pan: word2)
            
            track("Cylinder: \(word1)")
            track("Pan: \(word2)")
            track("Volume: \(word3)")
            
            refreshStatusBar()
            
        case .CTRL_AMIGA_AMIGA:
            resetAction(self)
            
        case .SER_IN:
            serialIn += String(UnicodeScalar(Int(msg.data) & 0xFF)!)
            
        case .SER_OUT:
            serialOut += String(UnicodeScalar(Int(msg.data) & 0xFF)!)
            
        case .AUTO_SNAPSHOT_TAKEN:
            mydocument.snapshots.append(amiga.latestAutoSnapshot)
            
        case .USER_SNAPSHOT_TAKEN:
            mydocument.snapshots.append(amiga.latestUserSnapshot)
            renderer.flash()
            
        case .SNAPSHOT_RESTORED:
            renderer.rotateRight()
            hideOrShowDriveMenus()
            assignSlots()
            refreshStatusBar()
            
        case .RECORDING_STARTED:
            window?.backgroundColor = .warningColor
            refreshStatusBar()
            
        case .RECORDING_STOPPED:
            window?.backgroundColor = .windowBackgroundColor
            refreshStatusBar()
            
        case .RECORDING_ABORTED:
            refreshStatusBar()
            VAError.recordingAborted()
            
        case .SHAKING:
            metal.lastShake = DispatchTime(uptimeNanoseconds: 0)
            if pref.releaseMouseByShaking {
                metal.releaseMouse()
            }
            
        case .SRV_STATE:
            refreshStatusBar()
            
        case .SRV_RECEIVE, .SRV_SEND:
            break
            
        default:
            track("Unknown message: \(msg)")
            fatalError()
        }
    }
}
