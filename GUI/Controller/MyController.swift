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

    // Reference to the connected document
    var mydocument: MyDocument!
    
    // Amiga proxy
    // Implements a bridge between the emulator written in C++ and the
    // GUI written in Swift. Because Swift cannot interact with C++ directly,
    // the proxy is written in Objective-C.
    var amiga: AmigaProxy!

    // Inspector of this emulator instance
    var inspector: Inspector?

    // Monitor of this emulator instance
    var monitor: Monitor?

    // Configuration panel of this emulator instance
    var configurator: ConfigController?
    
    // Snapshot and screenshot browsers
    var snapshotBrowser: SnapshotDialog?
    var screenshotBrowser: ScreenshotDialog?

    // Emulator configuration
    var config: Configuration!
    
    // Audio Engine
    var macAudio: MacAudio!
     
    // Game pad manager
    var gamePadManager: GamePadManager!
    
    // Keyboard controller
    var kbController: KBController!

    // Virtual keyboard
    var virtualKeyboard: VKBController?
    
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
    var warp = false

    // Current mouse coordinate
    var mouseXY = NSPoint(x: 0, y: 0)
    
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

    // Application preferences
    var prefs: Preferences { return myAppDelegate.prefs }
         
    //
    // Timers
    //
        
    func startSnapshotTimer() {
        
        if prefs.snapshotInterval > 0 {
            
            snapshotTimer?.invalidate()
            snapshotTimer =
                Timer.scheduledTimer(timeInterval: TimeInterval(prefs.snapshotInterval),
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
        
        if prefs.snapshotInterval > 0 {
            
            screenshotTimer?.invalidate()
            screenshotTimer =
                Timer.scheduledTimer(timeInterval: TimeInterval(prefs.screenshotInterval),
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
        
        switch prefs.warpMode {
        case .auto: warp = amiga.diskController.spinning()
        case .off: warp = false
        case .on: warp = true
        }
        
        if warp != amiga.warp() {
            warp ? amiga.warpOn() : amiga.warpOff()
            refreshStatusBar()
        }
    }
    
    // Returns the icon of the sand clock in the bottom bar
    var hourglassIcon: NSImage? {
        switch prefs.warpMode {
        case .auto where amiga.warp():
            return NSImage.init(named: "hourglass3Template")
        case .auto:
            return NSImage.init(named: "hourglass1Template")
        case .off:
            return NSImage.init(named: "warpLockOffTemplate")
        case .on:
            return NSImage.init(named: "warpLockOnTemplate")
        }
    }
    
    //
    // Outlets
    //
    
    // Main screen
    @IBOutlet weak var metal: MetalView!

    var renderer: Renderer!

    // Bottom bar
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
            if newValue && !prefs.closeWithoutAsking {
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
        
        config = Configuration.init(with: self)
        
        // Create audio engine
        macAudio = MacAudio.init(with: self)
        
        mydocument = document as? MyDocument
    }

    override open func windowDidLoad() {
 
        track()
        
        // Reset mouse coordinates
        mouseXY = NSPoint.zero
        
        // Create keyboard controller
        kbController = KBController(parent: self)
        assert(kbController != nil, "Failed to create keyboard controller")

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

        // Check if the Amiga is ready to power on
        if amiga.isReady() {

            // Power on the Amiga
            amiga.powerOn()

            // Process attachment (if any)
            mydocument.mountAmigaAttachment()

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
        if inspector?.window?.isVisible == true { inspector!.triggerRefresh() }
 
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
        
        speedometer.updateWith(cycle: amiga.cpu.cycles(), frame: renderer.frames)
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
        
        let counts = [
            dma.bus.accumulated.0,
            dma.bus.accumulated.1,
            dma.bus.accumulated.2,
            dma.bus.accumulated.3,
            dma.bus.accumulated.4,
            dma.bus.accumulated.5,
            dma.bus.accumulated.6,
            dma.bus.accumulated.7,
            dma.bus.accumulated.8
        ]
                
        let copDMA = Float(counts[Int(BUS_COPPER.rawValue)]) / (313*120)
        let bltDMA = Float(counts[Int(BUS_BLITTER.rawValue)]) / (313*120)
        let dskDMA = Float(counts[Int(BUS_DISK.rawValue)]) / (313*3)
        let audDMA = Float(counts[Int(BUS_AUDIO.rawValue)]) / (313*4)
        let sprDMA = Float(counts[Int(BUS_SPRITE.rawValue)]) / (313*16)
        let bplDMA = Float(counts[Int(BUS_BITPLANE.rawValue)]) / 39330
        
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

        if prefs.autoSnapshots { takeAutoSnapshot() }
    }
    
    @objc func screenshotTimerFunc() {
        
        if prefs.autoScreenshots { takeAutoScreenshot() }
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
            mydocument.setBootDiskID(amiga.df0.fnv())
            inspector?.fullRefresh()

        case MSG_MUTE_ON:
            muted = true
            refreshStatusBar()
            
        case MSG_MUTE_OFF:
            muted = false
            refreshStatusBar()

        case MSG_WARP_ON:
            warp = true
            refreshStatusBar()

        case MSG_WARP_OFF:
            warp = false
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

        case MSG_MEM_LAYOUT:
            inspector?.fullRefresh()

        case MSG_DRIVE_CONNECT:
            switch msg.data {
            case 0: myAppDelegate.df0Menu.isHidden = false
            case 1: myAppDelegate.df1Menu.isHidden = false
            case 2: myAppDelegate.df2Menu.isHidden = false
            case 3: myAppDelegate.df3Menu.isHidden = false
            default: fatalError()
            }
            refreshStatusBar()
            
        case MSG_DRIVE_DISCONNECT:
            switch msg.data {
            case 0: myAppDelegate.df0Menu.isHidden = true
            case 1: myAppDelegate.df1Menu.isHidden = true
            case 2: myAppDelegate.df2Menu.isHidden = true
            case 3: myAppDelegate.df3Menu.isHidden = true
            default: fatalError()
            }
            
            // Remove drop target status from the disconnect drive
            if dragAndDropDrive === amiga.df(msg.data) {
                dragAndDropDrive = nil
            }
            
            refreshStatusBar()

        case MSG_DRIVE_SELECT:
            refreshStatusBar(writing: nil)

        case MSG_DRIVE_READ:
            refreshStatusBar(writing: false)
            
        case MSG_DRIVE_WRITE:
            refreshStatusBar(writing: true)

        case MSG_DRIVE_LED_ON:
            refreshStatusBar(drive: msg.data, led: true)
            
        case MSG_DRIVE_LED_OFF:
            refreshStatusBar(drive: msg.data, led: false)

        case MSG_DRIVE_MOTOR_ON:
            refreshStatusBar(drive: msg.data, motor: true)
            updateWarp()

        case MSG_DRIVE_MOTOR_OFF:
            refreshStatusBar(drive: msg.data, motor: false)
            updateWarp()

        case MSG_DRIVE_HEAD:
            if prefs.driveSounds && prefs.driveHeadSound {
                macAudio.playSound(name: "drive_head", volume: 0.3)
            }
            refreshStatusBar(drive: msg.data >> 8, cyclinder: msg.data % 0xFF)
  
        case MSG_DRIVE_HEAD_POLL:
            if prefs.driveSounds && prefs.drivePollSound {
                macAudio.playSound(name: "drive_head", volume: 0.3)
            }
            refreshStatusBar(drive: msg.data >> 8, cyclinder: msg.data % 0xFF)

        case MSG_DISK_INSERT:
            if prefs.driveSounds && prefs.driveInsertSound {
                macAudio.playSound(name: "insert", volume: 0.3)
            }
            if msg.data == 0 { mydocument.setBootDiskID(amiga.df0.fnv()) }
            refreshStatusBar()
            
        case MSG_DISK_EJECT:
            if prefs.driveSounds && prefs.driveEjectSound {
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
            mydocument!.autoSnapshots.append(amiga.latestAutoSnapshot())

        case MSG_USER_SNAPSHOT_TAKEN:
            track("MSG_USER_SNAPSHOT_TAKEN")
            mydocument!.userSnapshots.append(amiga.latestUserSnapshot())
            renderer.blendIn(steps: 20)
            
        default:
            track("Unknown message: \(msg)")
            assert(false)
        }
    }

    //
    // Dialogs
    //
    
    //
    // Keyboard events
    //

    // Keyboard events are handled by the emulator window.
    // If they are handled here, some keys such as 'TAB' don't trigger an event.

    //
    //  Game pad events
    //

    // Feeds game pad actions into the Amiga's first control port
    func emulateEventsOnGamePort1(_ events: [GamePadAction]) -> Bool {
        
        for event in events {
            amiga.joystick1.trigger(event)
            amiga.mouse.trigger(event)
        }
        return events != []
    }
    
    // Feeds game pad actions into the Amiga's second control port
    func emulateEventsOnGamePort2(_ events: [GamePadAction]) -> Bool {
        
        for event in events {
            amiga.joystick2.trigger(event)
            amiga.mouse.trigger(event)
        }
        return events != []
    }

    // Feeds game pad actions into the port associated with a certain game pad
    @discardableResult
    func emulateEventsOnGamePort(slot: Int, events: [GamePadAction]) -> Bool {
        
        if slot == config.gameDevice1 {
            return emulateEventsOnGamePort1(events)
        }
        if slot == config.gameDevice2 {
            return emulateEventsOnGamePort2(events)
        }
        return false
    }
    
    //
    // Action methods (status bar)
    //
    
    @IBAction func warpAction(_ sender: Any!) {

        track()

        switch prefs.warpMode {
        case .auto: prefs.warpMode = .off
        case .off: prefs.warpMode = .on
        case .on: prefs.warpMode = .auto
        }

        refreshStatusBar()
    }
}
