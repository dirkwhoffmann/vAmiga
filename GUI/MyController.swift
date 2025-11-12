// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor 
protocol MessageReceiver {
    
    func process(message: Message)
}

@MainActor
class MyController: NSWindowController, MessageReceiver {
    
    var pref: Preferences { return myAppDelegate.pref }
    
    // Reference to the connected document
    var mydocument: MyDocument!
    var initialized: Bool { return mydocument != nil }
    
    // File panels
    let myOpenPanel = MyOpenPanel()
    let mySavePanel = MySavePanel()
    
    // Emulator proxy (bridge between the Swift frontend and the C++ backend)
    var emu: EmulatorProxy? { return mydocument.emu }
    
    // Media manager (handles the import and export of media files)
    var mm: MediaManager { return mydocument.mm }
    
    // Auxiliary windows of this emulator instance
    var inspectors: [Inspector] = []
    var dashboards: [Dashboard] = []
    
    // Settings panel
    var settings: SettingsWindowController? { myAppDelegate.settingsController }
    
    // Snapshot and screenshot browsers
    var snapshotBrowser: SnapshotViewer?
    var screenshotBrowser: ScreenshotViewer?
    
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
    
    // Speedometer to measure clock frequence and frames per second
    var speedometer = Speedometer()
    
    // Indicates if the CPU is halted
    var jammed: Bool { emu?.cpu.info.halt ?? false }
    
    // Remembers if audio is muted
    var muted = false
    
    // Indicates if a status bar is shown
    var statusBar = true
    
    // Information message shown in the status bar
    var infoText: String?
    var infoText2: String?
    
    // Pictograms for being used in NSMenuItems (MOVE TO AppDelegate)
    static let iconSize = CGSize(width: 16, height: 16)
    var smallDisk = NSImage(named: "diskTemplate")!.resize(size: iconSize)
    var smallHdr = NSImage(named: "hdrTemplate")!.resize(size: iconSize)
    
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
    
    @IBOutlet weak var haltIcon: NSButton!
    @IBOutlet weak var trackIcon: NSButton!
    @IBOutlet weak var serverIcon: NSButton!
    @IBOutlet weak var muteIcon: NSButton!
    
    @IBOutlet weak var warpIcon: NSButton!
    @IBOutlet weak var activityType: NSPopUpButton!
    @IBOutlet weak var activityInfo: NSTextField!
    @IBOutlet weak var activityBar: NSLevelIndicator!
    @IBOutlet weak var speedStepper: NSStepper!
    
    // Toolbar
    var toolbar: MyToolbar { (window?.toolbar as? MyToolbar)! }
    
    // Quick-access references
    var ledSlot: [NSButton]!
    var cylSlot: [NSTextField]!
    var iconSlot: [NSButton]!
    
    // Slot assignments
    var drv: [Int?] = Array(repeating: nil, count: 8)
    var drvLED: [NSButton?] = Array(repeating: nil, count: 8)
    var drvCyl: [NSTextField?] = Array(repeating: nil, count: 8)
    var drvIcon: [NSButton?] = Array(repeating: nil, count: 8)
}

extension MyController {
    
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
    
    override open func windowDidLoad() {
        
        if !initialized { commonInit() }
    }
    
    func commonInit() {
        
        debug(.lifetime)
        assert(!initialized, "Double-initialization of MyController")
        
        mydocument = document as? MyDocument
        config = Configuration(with: self)
        macAudio = MacAudio(with: self)
        
        ledSlot = [ ledSlot0, ledSlot1, letSlot2, ledSlot3 ]
        cylSlot = [ cylSlot0, cylSlot1, cylSlot2, cylSlot3 ]
        iconSlot = [ iconSlot0, iconSlot1, iconSlot2, iconSlot3 ]
        
        // Create toolbar
        window?.toolbar = MyToolbar(controller: self)
        
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
        
        // Launch the emulator
        launch()
        
        // Apply all GUI related user defaults
        pref.applyUserDefaults()
        config.applyUserDefaults()
        
        do {
            
            // Press the virtual power switch
            emu?.powerOn()
            
            // Start emulation
            try emu?.run()
            
        } catch {
            
            // Switch off
            emu?.powerOff()
            
            // Open the onboarding agent
            renderer.onboarding.open(delay: 1.0)
        }
        
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
        
        // Adjust size
        // autoResizeWindow(self)
        adjustWindowSize()
        window?.windowController?.shouldCascadeWindows = false // true ?!
        
        // Enable auto-save for window coordinates
        // let name = NSWindow.FrameAutosaveName("dirkwhoffmann.de.vAmiga.window")
        // window?.setFrameAutosaveName(name)
        
        // Enable fullscreen mode
        window?.collectionBehavior = .fullScreenPrimary
    }
    
    func launch() {
        
        do {
            
            // Pass in command line arguments as a RetroShell script
            var script = ""
            for arg in myAppDelegate.argv where arg.hasPrefix("-") {
                script = script + arg.dropFirst() + "\n"
            }
            emu?.retroShell.execute(script)
            
            if BuildSettings.msgCallback {
                
                // Convert 'self' to a void pointer
                let myself = UnsafeRawPointer(Unmanaged.passUnretained(self).toOpaque())
                
                try emu?.launch(myself) { (ptr, msg: Message) in
                    
                    // Convert void pointer back to 'self'
                    let myself = Unmanaged<MyController>.fromOpaque(ptr!).takeUnretainedValue()
                    
                    // Process message in the main thread
                    Task { @MainActor in myself.process(message: msg) }
                }
                
            } else {
                
                try emu?.launch()
            }
            
        } catch {
            
            // Something terrible happened
            mydocument.showLaunchAlert(error: error)
        }
    }
    
    //
    // Timer and message processing
    //
    
    func update(frames: Int64) {
        
        guard let emu = emu else { return }
        
        if frames % 5 == 0 {
            
            // Animate the inspectors
            for inspector in inspectors { inspector.continuousRefresh() }
            
            // Animate the dashboards
            for dashboard in dashboards { dashboard.continuousRefresh() }
        }
        
        // Do less times...
        if frames % 16 == 0 {
            
            updateSpeedometer()
        }
        
        // Do less times...
        if frames % 32 == 0 {
            
            if pref.closeWithoutAsking {
                needsSaving = false
            } else {
                needsSaving =
                emu.df0.getFlag(.MODIFIED) ||
                emu.df1.getFlag(.MODIFIED) ||
                emu.df2.getFlag(.MODIFIED) ||
                emu.df3.getFlag(.MODIFIED) ||
                emu.hd0.getFlag(.MODIFIED) ||
                emu.hd1.getFlag(.MODIFIED) ||
                emu.hd2.getFlag(.MODIFIED) ||
                emu.hd3.getFlag(.MODIFIED)
            }
        }
        
        // Do lesser times...
        if frames % 256 == 0 {
            
            // Let the cursor disappear in fullscreen mode
            if renderer.fullscreen &&
                CGEventSource.secondsSinceLastEventType(.combinedSessionState,
                                                        eventType: .mouseMoved) > 1.0 {
                NSCursor.setHiddenUntilMouseMoves(true)
            }
        }
    }
    
    func process(message msg: Message) {
        
        MainActor.assertIsolated()
        
        guard let emu = emu else { return }
        var value: Int { return Int(msg.value) }
        var nr: Int { return Int(msg.drive.nr) }
        var cyl: Int { return Int(msg.drive.value) }
        var pc: Int { return Int(msg.cpu.pc) }
        var pcHex: String { return String(format: "%X", pc) }
        var vector: Int { return Int(msg.cpu.vector) }
        var volume: Int { return Int(msg.drive.volume) }
        var pan: Int { return Int(msg.drive.pan) }
        var acceleration: Double { return Double(msg.value == 0 ? 1 : msg.value) }
        var pos: String { return "(\(emu.amiga.info.vpos),\(emu.amiga.info.hpos))" }

        switch msg.type {
            
        case .CONFIG:
            
            refreshStatusBar()
            settings?.refresh()
            
        case .POWER:
            
            if value != 0 {
                
                let delay = document?.fileURL != nil ? 1.5 : 1.5
                renderer.canvas.open(delay: delay)
                
                if let url = mydocument.launchURL {
                    
                    try? mm.mount(url: url, options: [.remember, .force])
                    mydocument.launchURL = nil
                }
                
                virtualKeyboard = nil
                serialIn = ""
                serialOut = ""
            }
            
            clearInfo()
            settings?.refresh()
            
        case .RUN:
            toolbar.updateToolbar()
            refreshStatusBar()
            clearInfo()

        case .PAUSE:
            toolbar.updateToolbar()
            refreshStatusBar()

        case .STEP:
            clearInfo()

        case .RESET:
            clearInfo()

        case .RSH_CLOSE:
            renderer.console.close(delay: 0.25)
            
        case .RSH_UPDATE:
            renderer.console.isDirty = true

        case .RSH_SWITCH:
            break
            
        case .RSH_WAIT:
            renderer.console.isDirty = true
            
        case .RSH_ERROR:
            NSSound.beep()
            renderer.console.isDirty = true
            
        case .RSH_EXPORT:
            break;
            
        case .SHUTDOWN:
            shutDown()
            
        case .ABORT:
            debug(.shutdown, "Aborting with exit code \(value)")
            exit(Int32(value))
            
        case .MUTE:
            muted = value != 0
            refreshStatusBar()
            
        case .EASTER_EGG, .WARP, .TRACK:
            refreshStatusBar()
            
        case .POWER_LED_ON:
            powerLED.image = NSImage(named: "ledRed")
            
        case .POWER_LED_DIM:
            powerLED.image = NSImage(named: "ledBlack")
            
        case .POWER_LED_OFF:
            powerLED.image = NSImage(named: "ledGrey")
            
        case .DMA_DEBUG:
            msg.value != 0 ? renderer.zoomTextureOut() : renderer.zoomTextureIn()
            
        case .VIDEO_FORMAT:
            renderer.canvas.updateTextureRect()
            
        case .OVERCLOCKING:
            speedometer.acceleration = acceleration
            activityBar.maxValue = 140.0 * acceleration
            activityBar.warningValue = 77.0 * acceleration
            activityBar.criticalValue = 105.0 * acceleration
            
        case .COPPERBP_UPDATED, .COPPERWP_UPDATED, .GUARD_UPDATED:
            break

        case .BREAKPOINT_REACHED:
            setInfo("Breakpoint reached", "Interrupted at address \(pcHex)")
            
        case .WATCHPOINT_REACHED:
            setInfo("Watchpoint reached", "Interrupted at address \(pcHex)")
            
        case .CATCHPOINT_REACHED:
            let name = emu.cpu.vectorName(vector)!
            setInfo("Exception vector catched", "Caught vector \(vector) (\(name))")
            
        case .COPPERBP_REACHED:
            setInfo("Copper breakpoint reached", "Interrupted at address \(pcHex)")
            
        case .COPPERWP_REACHED:
            setInfo("Copper watchpoint reached", "Interrupted at address \(pcHex)")
            
        case .SWTRAP_REACHED:
            setInfo("Software trap reached at address \(pc)", "Interrupted at address \(pcHex)")
            
        case .BEAMTRAP_REACHED:
            setInfo("Beamtrap reached", "Interrupted at location \(pos)")
            
        case .EOF_REACHED:
            setInfo("End of frame reached", "Interrupted at location \(pos)")
            
        case .EOL_REACHED:
            setInfo("End of line reached", "Interrupted at location \(pos)")
            
        case .CPU_HALT:
            refreshStatusBar()
            
        case .VIEWPORT:
            renderer.canvas.updateTextureRect(hstrt: Int(msg.viewport.hstrt),
                                              vstrt: Int(msg.viewport.vstrt),
                                              hstop: Int(msg.viewport.hstop),
                                              vstop: Int(msg.viewport.vstop))
            
        case .MEM_LAYOUT:
                break

        case .DRIVE_CONNECT:
            
            if msg.value != 0 {
                
                hideOrShowDriveMenus()
                assignSlots()
                refreshStatusBar()
                
            } else {
                
                hideOrShowDriveMenus()
                assignSlots()
                refreshStatusBar()
            }
            
        case .DRIVE_SELECT:
            refreshStatusBar(writing: nil)
            
        case .DRIVE_READ:
            refreshStatusBar(writing: false)
            
        case .DRIVE_WRITE:
            refreshStatusBar(writing: true)
            
        case .DRIVE_LED:
            refreshStatusBar()
            
        case .DRIVE_MOTOR:
            refreshStatusBar()
            
        case .DRIVE_STEP:
            macAudio.playSound(MacAudio.Sounds.step, volume: volume, pan: pan)
            refreshStatusBar(drive: nr, cylinder: cyl)
            
        case .DRIVE_POLL:
            macAudio.playSound(MacAudio.Sounds.step, volume: volume, pan: pan)
            refreshStatusBar(drive: nr, cylinder: cyl)
            
        case .DISK_INSERT:
            macAudio.playSound(MacAudio.Sounds.insert, volume: volume, pan: pan)
            refreshStatusBar()
            
        case .DISK_EJECT:
            macAudio.playSound(MacAudio.Sounds.eject, volume: volume, pan: pan)
            refreshStatusBar()
            
        case .DISK_PROTECTED:
            refreshStatusBar()
            
        case .HDC_CONNECT:
            
            if msg.value != 0 {
                
                hideOrShowDriveMenus()
                assignSlots()
                refreshStatusBar()
                
            } else {
                
                hideOrShowDriveMenus()
                assignSlots()
                refreshStatusBar()
            }
            
        case .HDC_STATE:
            refreshStatusBar()
            
        case .HDR_STEP:
            macAudio.playSound(MacAudio.Sounds.move, volume: volume, pan: pan)
            refreshStatusBar()
            
        case .HDR_IDLE, .HDR_READ:
            refreshStatusBar()
            
        case .HDR_WRITE:
            refreshStatusBar()
            
        case .MON_SETTING:
            renderer.process(message: msg)
            
        case .CTRL_AMIGA_AMIGA:
            resetAction(self)
            
        case .SER_IN:
            var c = emu.serialPort.readIncomingPrintableByte()
            while c != -1 {
                serialIn += String(UnicodeScalar(UInt8(c)))
                c = emu.serialPort.readIncomingPrintableByte()
            }
            
        case .SER_OUT:
            var c = emu.serialPort.readOutgoingPrintableByte()
            while c != -1 {
                serialOut += String(UnicodeScalar(UInt8(c)))
                c = emu.serialPort.readOutgoingPrintableByte()
            }
            
        case .SNAPSHOT_TAKEN:
            let ptr = msg.snapshot.snapshot
            let proxy = MediaFileProxy.init(ptr)!
            if !mydocument.appendSnapshot(file: proxy) {
                NSSound.beep()
            }
            
        case .SNAPSHOT_RESTORED:
            renderer.flash(steps: 40)
            hideOrShowDriveMenus()
            assignSlots()
            refreshStatusBar()
            
        case .WORKSPACE_SAVED, .WORKSPACE_LOADED:
            break
            
        case .SHAKING:
            metal.lastShake = DispatchTime(uptimeNanoseconds: 0)
            if pref.releaseMouseByShaking {
                metal.releaseMouse()
            }
            
        case .SRV_STATE:
            refreshStatusBar()
            settings?.refresh()

        case .SRV_RECEIVE, .SRV_SEND:
            break
            
        case .ALARM:
            debug(.events, "Received Alarm \(msg.value)")
            
        default:
            warn("Unknown message: \(msg)")
            fatalError()
        }
        
        // Pass message to all open auxiliary panels
        for inspector in inspectors { inspector.processMessage(msg) }
        for dashboard in dashboards { dashboard.processMessage(msg) }
    }
    
    func setInfo(_ text: String?, _ text2: String? = nil) {
        
        infoText = text
        infoText2 = text2
        refreshStatusBar()
    }
    
    func clearInfo() {
        
        infoText = nil
        infoText2 = nil
    }
}
