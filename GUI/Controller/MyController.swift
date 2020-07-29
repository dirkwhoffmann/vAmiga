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

    var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
    var pref: Preferences { return myAppDelegate.pref }

    // Reference to the connected document
    var mydocument: MyDocument!
    
    // Amiga proxy
    // Implements a bridge between the emulator written in C++ and the
    // GUI written in Swift. Because Swift cannot interact with C++ directly,
    // the proxy is written in Objective-C.
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
    
    // Loop timer
    // The timer fires 60 times a second and executes all tasks that need to be
    //  done perdiodically (e.g., updating the speedometer and the debug panels)
    var timer: Timer?
    
    // Timer lock
    var timerLock: NSLock!

    // Screenshot and snapshot timers
    var snapshotTimer: Timer?
    var screenshotTimer: Timer?
    
    // Speedometer to measure clock frequence and frames per second
    var speedometer: Speedometer!
    
    // Used inside the timer function to fine tune timed events
    var animationCounter = 0
        
    // Remembers if audio is muted (master volume of both channels is 0)
    var muted = false

    // Remembers if we run in warp mode
    // var warp = false
    
    // Indicates if a status bar is shown
    var statusBar = true

    // Small disk icon to be shown in NSMenuItems
    var smallDisk = NSImage.init(named: "diskTemplate")!.resize(width: 16.0, height: 16.0)
    
    // Drive that receives drag and drop inputs
    var dragAndDropDrive: DriveProxy?

    // Serial input and output
    var serialIn = ""
    var serialOut = ""
    
    // Remembers the running state for the pauseInBackground feature
    var pauseInBackgroundSavedState = false
         
    //
    // Timers
    //
        
    func startSnapshotTimer() {
        
        if pref.snapshotInterval > 0 {
            
            snapshotTimer?.invalidate()
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
            
    func startScreenshotTimer() {
        
        if pref.snapshotInterval > 0 {
            
            screenshotTimer?.invalidate()
            screenshotTimer =
                Timer.scheduledTimer(timeInterval: TimeInterval(pref.screenshotInterval),
                                     target: self,
                                     selector: #selector(screenshotTimerFunc),
                                     userInfo: nil,
                                     repeats: true)
        }
    }
    
    func stopScreenshotTimer() {
        
        screenshotTimer?.invalidate()
    }
    
    // Loads a snapshot into the emulator
    func load(snapshot: SnapshotProxy?) {
        
        if snapshot == nil { return }
        
        amiga.suspend()
        amiga.load(fromSnapshot: snapshot)
        amiga.resume()
    }
        
    // Updates the warp status
    func updateWarp() {
        
        var warp: Bool
        
        switch pref.warpMode {
        case .auto: warp = amiga.diskController.isSpinning
        case .off: warp = false
        case .on: warp = true
        }
        
        if warp != amiga.warp {
            warp ? amiga.warpOn() : amiga.warpOff()
        }
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
    @IBOutlet weak var toolbar: NSToolbar!
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
    // Initialization
    //
    
    override open func awakeFromNib() {

        track()
        
        mydocument = document as? MyDocument
        config = Configuration.init(with: self)
        macAudio = MacAudio.init(with: self)
    }

    override open func windowDidLoad() {
 
        track()
                
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
        
        // Register listener
        addListener()

        // Process attachment (if any)
        mydocument.mountAttachment()

        // Check if the Amiga is ready to power on
        if amiga.isReady() {

            // Power on the Amiga
            amiga.powerOn()

            // Launch the emulator thread
            amiga.run()

        } else {

            // Open the Rom dialog
            openConfigurator(tab: "Roms")
            renderer.zoomOut()
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
    
    func addListener() {
        
        track()
        
        // Convert 'self' to a void pointer
        let myself = UnsafeRawPointer(Unmanaged.passUnretained(self).toOpaque())
        
        amiga.addListener(myself) { (ptr, type, data) in

            // Convert void pointer back to 'self'
            let myself = Unmanaged<MyController>.fromOpaque(ptr!).takeUnretainedValue()
            
            // Process message in the main thread
            DispatchQueue.main.async {
                let mType = MessageType(rawValue: type)
                myself.processMessage(Message(type: mType, data: data))
            }
        }
    }
    
    func createTimer() {

        track()

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
 
        // Do less times...
        if (animationCounter % 2) == 0 {
 
        }
        
        // Do lesser times...
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
        if let monitor = renderer.monitors[nr] as? BarChart {
            monitor.addValue(v)
        }
    }
    
    func addValues(_ nr: Int, _ v1: Float, _ v2: Float) {
        if let monitor = renderer.monitors[nr] as? BarChart {
            monitor.addValues(v1, v2)
        }
    }

    func updateMonitoringPanels() {
        
        if !renderer.drawActivityMonitors { return }
            
        // DMA monitors
        let dma = amiga.agnus.getStats()
        let copDMA = Float(dma.copperActivity) / (313*120)
        let bltDMA = Float(dma.blitterActivity) / (313*120)
        let dskDMA = Float(dma.diskActivity) / (313*3)
        let audDMA = Float(dma.audioActivity) / (313*4)
        let sprDMA = Float(dma.spriteActivity) / (313*16)
        let bplDMA = Float(dma.bitplaneActivity) / 39330
        
        addValue(Renderer.Monitor.copper, copDMA)
        addValue(Renderer.Monitor.blitter, bltDMA)
        addValue(Renderer.Monitor.disk, dskDMA)
        addValue(Renderer.Monitor.audio, audDMA)
        addValue(Renderer.Monitor.sprite, sprDMA)
        addValue(Renderer.Monitor.bitplane, bplDMA)
        
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

        addValues(Renderer.Monitor.chipRam, chipR, chipW)
        addValues(Renderer.Monitor.slowRam, slowR, slowW)
        addValues(Renderer.Monitor.fastRam, fastR, fastW)
        addValues(Renderer.Monitor.kickRom, kickR, kickW)
    }
    
    @objc func snapshotTimerFunc() {

        if pref.autoSnapshots { takeAutoSnapshot() }
    }
    
    @objc func screenshotTimerFunc() {
        
        if pref.autoScreenshots { takeAutoScreenshot() }
    }
        
    func processMessage(_ msg: Message) {
        
        switch msg.type {
    
        case MSG_REGISTER:
            track("Registered to message queue")
            
        case MSG_UNREGISTER:
            track("Unregistered from message queue")
            // From now on, it's save to delete the document.
            // To trigger deletion, we remove any reference to it.
            mydocument = nil
            
        case MSG_CONFIG:
            inspector?.fullRefresh()

        case MSG_POWER_ON:
            serialIn = ""
            serialOut = ""
            virtualKeyboard = nil
            renderer.zoomIn()
            toolbar.validateVisibleItems()
            inspector?.fullRefresh()

        case MSG_POWER_OFF:
            renderer.zoomOut(steps: 20) // blendOut()
            toolbar.validateVisibleItems()
            inspector?.fullRefresh()

        case MSG_RUN:
            needsSaving = true
            toolbar.validateVisibleItems()
            inspector?.fullRefresh()
            refreshStatusBar()

        case MSG_PAUSE:
            toolbar.validateVisibleItems()
            inspector?.fullRefresh()
            refreshStatusBar()

        case MSG_RESET:
            mydocument.deleteBootDiskID()
            mydocument.setBootDiskID(amiga.df0.fnv)
            inspector?.fullRefresh()

        case MSG_MUTE_ON:
            muted = true
            refreshStatusBar()
            
        case MSG_MUTE_OFF:
            muted = false
            refreshStatusBar()

        case MSG_WARP_ON:
            // warp = true
            refreshStatusBar()

        case MSG_WARP_OFF:
            // warp = false
            refreshStatusBar()

        case MSG_POWER_LED_ON:
            powerLED.image = NSImage.init(named: "powerLedOn")

        case MSG_POWER_LED_DIM:
            powerLED.image = NSImage.init(named: "powerLedDim")

        case MSG_POWER_LED_OFF:
            powerLED.image = NSImage.init(named: "powerLedOff")

        case MSG_DMA_DEBUG_ON:
            renderer.zoomTextureOut()

        case MSG_DMA_DEBUG_OFF:
            renderer.zoomTextureIn()

        case MSG_BREAKPOINT_CONFIG,
             MSG_BREAKPOINT_REACHED,
             MSG_WATCHPOINT_REACHED:
            inspector?.fullRefresh()
            inspector?.scrollToPC()

        case MSG_CPU_HALT:
            refreshStatusBar()
            
        case MSG_MEM_LAYOUT:
            inspector?.fullRefresh()

        case MSG_DRIVE_CONNECT:
            myAppDelegate.hideOrShowDriveMenus(proxy: amiga)
            refreshStatusBar()
            
        case MSG_DRIVE_DISCONNECT:
            myAppDelegate.hideOrShowDriveMenus(proxy: amiga)
            refreshStatusBar()

            // Remove drop target status from the disconnect drive
            if dragAndDropDrive === amiga.df(msg.data) {
                dragAndDropDrive = nil
            }

        case MSG_DRIVE_SELECT:
            refreshStatusBar(writing: nil)

        case MSG_DRIVE_READ:
            refreshStatusBar(writing: false)
            
        case MSG_DRIVE_WRITE:
            refreshStatusBar(writing: true)

        case MSG_DRIVE_LED_ON,
             MSG_DRIVE_LED_OFF:
            refreshStatusBar()
            
        case MSG_DRIVE_MOTOR_ON,
             MSG_DRIVE_MOTOR_OFF:
            refreshStatusBar()
            updateWarp()

        case MSG_DRIVE_HEAD:
            if pref.driveSounds && pref.driveHeadSound {
                macAudio.playSound(name: "drive_head", volume: 0.3)
            }
            refreshStatusBar(drive: msg.data >> 8, cyclinder: msg.data % 0xFF)
  
        case MSG_DRIVE_HEAD_POLL:
            if pref.driveSounds && pref.drivePollSound {
                macAudio.playSound(name: "drive_head", volume: 0.3)
            }
            refreshStatusBar(drive: msg.data >> 8, cyclinder: msg.data % 0xFF)

        case MSG_DISK_INSERT:
            if pref.driveSounds && pref.driveInsertSound {
                macAudio.playSound(name: "insert", volume: 0.3)
            }
            if msg.data == 0 { mydocument.setBootDiskID(amiga.df0.fnv) }
            refreshStatusBar()
            
        case MSG_DISK_EJECT:
            if pref.driveSounds && pref.driveEjectSound {
                macAudio.playSound(name: "eject", volume: 0.3)
            }
            refreshStatusBar()
            
        case MSG_DISK_INSERTED,
             MSG_DISK_EJECTED,
             MSG_DISK_UNSAVED,
             MSG_DISK_SAVED,
             MSG_DISK_PROTECTED,
             MSG_DISK_UNPROTECTED:
            refreshStatusBar()

        case MSG_CTRL_AMIGA_AMIGA:
            resetAction(self)
            
        case MSG_SER_IN:
            serialIn += String(UnicodeScalar(msg.data & 0xFF)!)

        case MSG_SER_OUT:
            serialOut += String(UnicodeScalar.init(msg.data & 0xFF)!)

        case MSG_AUTO_SNAPSHOT_TAKEN:
            track("MSG_AUTO_SNAPSHOT_TAKEN")
            mydocument!.autoSnapshots.append(amiga.latestAutoSnapshot)

        case MSG_USER_SNAPSHOT_TAKEN:
            track("MSG_USER_SNAPSHOT_TAKEN")
            mydocument!.userSnapshots.append(amiga.latestUserSnapshot)
            renderer.blendIn(steps: 20)
            
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
