// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

enum AutoMountAction : Int, Codable {
    
    case openBrowser = 0
    case flashFirstFile = 1
    case insertIntoDrive8 = 2
    case insertIntoDrive9 = 3
    case insertIntoDatasette = 4
    case attachToExpansionPort = 5
}

protocol MessageReceiver {
    func processMessage(_ msg: Message)
}

class MyController : NSWindowController, MessageReceiver {

    /// Proxy
    /// Implements a bridge between the emulator written in C++ and the
    /// GUI written in Swift. Because Swift cannot interact with C++ directly,
    //  the proxy is written in Objective-C.
    var amiga: AmigaProxy!

    
    /// Proxy object.
    /// Implements a bridge between the emulator written in C++ and the
    /// GUI written in Swift. Because Swift cannot interact with C++ directly,
    //  the proxy is written in Objective-C.
    var c64: C64Proxy!
    
    /// Audio Engine
    var audioEngine: AudioEngine!
    
    /// Game pad manager
    var gamePadManager: GamePadManager!
    
    /// Keyboard controller
    var keyboardcontroller: KeyboardController!

    /// Virtual C64 keyboard (opened as a sheet)
    var virtualKeyboardSheet: VirtualKeyboardController? = nil
    
    /// Preferences controller
    var preferencesController: PreferencesController?

    /// Loop timer
    /// The timer fires 60 times a second and executes all tasks that need to be
    //  done perdiodically (e.g., updating the speedometer and the debug panels)
    var timer: Timer?
    
    // Timer lock
    var timerLock: NSLock!
    
    /// Speedometer to measure clock frequence and frames per second
    var speedometer: Speedometer!
    
    /// Used inside the timer function to fine tune timed events
    var animationCounter = 0
        
    /// Current mouse coordinate
    var mouseXY = NSPoint(x: 0, y: 0)
    
    /// Indicates if mouse is currently hidden
    var hideMouse = false
    
    /// Indicates if a status bar is shown
    var statusBar = true

    /// Small disk icon to be shown in NSMenuItems
    var smallDisk = NSImage.init(named: "diskTemplate")!.resize(width: 16.0, height: 16.0)
    
    // Drive that receives drag and drop inputs
    var dragAndDropDrive : AmigaDriveProxy?
    
    /// Selected game pad slot for joystick in port A
    var inputDevice1 = Defaults.inputDevice1
    
    /// Selected game pad slot for joystick in port B
    var inputDevice2 = Defaults.inputDevice2

    /// Rom file URLs
    var bootRomURL: URL = Defaults.bootRom
    var kickRomURL: URL = Defaults.kickRom

    /// Screenshot resolution (0 = low, 1 = high)
    var screenshotSource = Defaults.screenshotSource
    
    /// Screenshot image format
    var screenshotTarget = Defaults.screenshotTarget
    var screenshotTargetIntValue : Int {
        get { return Int(screenshotTarget.rawValue) }
        set { screenshotTarget = NSBitmapImageRep.FileType(rawValue: UInt(newValue))! }
    }
    
    /// Media file default actions
    var autoMountAction : [String: AutoMountAction] = Defaults.autoMountAction

    /// Media file auto-type enable / disable
    var autoType : [String: Bool] = Defaults.autoType

    /// Media file auto-type text
    var autoTypeText : [String: String] = Defaults.autoTypeText
    
    /// Indicates if the user should be warned if an unsaved document is closed.
    var closeWithoutAsking = Defaults.closeWithoutAsking

    /// Indicates if the user should be warned if an unsaved disk is ejected.
    var ejectWithoutAsking = Defaults.ejectWithoutAsking

    /// Indicates if the emulator should pause when it looses focus.
    var pauseInBackground =  Defaults.pauseInBackground
    
    /// Remembers if the emulator was running or paused when it lost focus.
    /// Needed to implement the pauseInBackground feature.
    var pauseInBackgroundSavedState = false
 
    
    //
    // Outlets
    //
    
    // Main screen
    @IBOutlet weak var metal: MetalView!
    @IBOutlet weak var debugger: NSDrawer!
    
    // Bottom bar
    @IBOutlet weak var powerLED: NSButton!

    @IBOutlet weak var df0LED: NSButton!
    @IBOutlet weak var df0Disk: NSButton!
    @IBOutlet weak var df0DMA: NSProgressIndicator!
    @IBOutlet weak var df1LED: NSButton!
    @IBOutlet weak var df1Disk: NSButton!
    @IBOutlet weak var df1DMA: NSProgressIndicator!

    @IBOutlet weak var cmdLock: NSButton!

    @IBOutlet weak var clockSpeed: NSTextField!
    @IBOutlet weak var clockSpeedBar: NSLevelIndicator!
    @IBOutlet weak var warpIcon: NSButton!
    
    // Toolbar
    @IBOutlet weak var controlPort1: NSPopUpButton!
    @IBOutlet weak var controlPort2: NSPopUpButton!
    @IBOutlet weak var pauseTbItem: NSToolbarItem!
    @IBOutlet weak var snapshotSegCtrl: NSSegmentedControl!

    // Debug panel (commons)
    var hex = true
    @IBOutlet weak var debugPanel: NSTabView!
    @IBOutlet weak var dezHexSelector: NSMatrix!
    @IBOutlet weak var stopAndGoButton: NSButton!
    @IBOutlet weak var stepIntoButton: NSButton!
    @IBOutlet weak var stepOverButton: NSButton!
    
    // Debug panel (CPU)
    @IBOutlet weak var cpuTableView: CpuTableView!
    @IBOutlet weak var cpuTraceView: CpuTraceView!
    @IBOutlet weak var pc: NSTextField!
    @IBOutlet weak var sp: NSTextField!
    @IBOutlet weak var a: NSTextField!
    @IBOutlet weak var x: NSTextField!
    @IBOutlet weak var y: NSTextField!
    @IBOutlet weak var nflag: NSButton!
    @IBOutlet weak var zflag: NSButton!
    @IBOutlet weak var cflag: NSButton!
    @IBOutlet weak var iflag: NSButton!
    @IBOutlet weak var bflag: NSButton!
    @IBOutlet weak var dflag: NSButton!
    @IBOutlet weak var vflag: NSButton!
    @IBOutlet weak var breakAt: NSTextField!
    
    // Debug panel (Memory)
    @IBOutlet weak var memTableView: MemTableView!
    
    // Debug panel (CIA)
    @IBOutlet weak var ciaSelector: NSSegmentedControl!
    
    @IBOutlet weak var ciaPA: NSTextField!
    @IBOutlet weak var ciaPAbinary: NSTextField!
    @IBOutlet weak var ciaPRA: NSTextField!
    @IBOutlet weak var ciaDDRA: NSTextField!
    
    @IBOutlet weak var ciaPB: NSTextField!
    @IBOutlet weak var ciaPBbinary: NSTextField!
    @IBOutlet weak var ciaPRB: NSTextField!
    @IBOutlet weak var ciaDDRB: NSTextField!
    
    @IBOutlet weak var ciaTimerA: NSTextField!
    @IBOutlet weak var ciaLatchA: NSTextField!
    @IBOutlet weak var ciaRunningA: NSButton!
    @IBOutlet weak var ciaToggleA: NSButton!
    @IBOutlet weak var ciaPBoutA: NSButton!
    @IBOutlet weak var ciaOneShotA: NSButton!
    
    @IBOutlet weak var ciaTimerB: NSTextField!
    @IBOutlet weak var ciaLatchB: NSTextField!
    @IBOutlet weak var ciaRunningB: NSButton!
    @IBOutlet weak var ciaToggleB: NSButton!
    @IBOutlet weak var ciaPBoutB: NSButton!
    @IBOutlet weak var ciaOneShotB: NSButton!
    
    @IBOutlet weak var todHours: NSTextField!
    @IBOutlet weak var todMinutes: NSTextField!
    @IBOutlet weak var todSeconds: NSTextField!
    @IBOutlet weak var todTenth: NSTextField!
    @IBOutlet weak var todIntEnable: NSButton!
    @IBOutlet weak var alarmHours: NSTextField!
    @IBOutlet weak var alarmMinutes: NSTextField!
    @IBOutlet weak var alarmSeconds: NSTextField!
    @IBOutlet weak var alarmTenth: NSTextField!
    
    @IBOutlet weak var ciaIcr: NSTextField!
    @IBOutlet weak var ciaIcrBinary: NSTextField!
    @IBOutlet weak var ciaImr: NSTextField!
    @IBOutlet weak var ciaImrBinary: NSTextField!
    @IBOutlet weak var ciaIntLineLow: NSButton!
    
    // Debugger (SID panel)
    @IBOutlet weak var volume: NSTextField!
 
    @IBOutlet weak var waveformView: WaveformView!
    @IBOutlet weak var audioBufferLevel: NSLevelIndicator!
    @IBOutlet weak var audioBufferLevelText: NSTextField!
    @IBOutlet weak var bufferUnderflows: NSTextField!
    @IBOutlet weak var bufferOverflows: NSTextField!
}

extension MyController {

    // Provides the undo manager
    override open var undoManager: UndoManager? {
        get {
            return metal.undoManager
        }
    }
 
    // Provides the document casted to the correct type
    var mydocument: MyDocument {
        get {
            return document as! MyDocument
        }
    }
    
    /// Indicates if the emulator needs saving
    var needsSaving: Bool {
        get {
            return document?.changeCount != 0
        }
        set {
            if (newValue && !closeWithoutAsking) {
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
        
        cpuTableView.c = self
        cpuTraceView.c = self
        memTableView.c = self
        
        // Create audio engine
        audioEngine = AudioEngine.init(withSID: c64.sid)
    }
    
    
    override open func windowDidLoad() {
 
        track()
        
        // Reset mouse coordinates
        mouseXY = NSZeroPoint
        hideMouse = false
        
        // Create keyboard controller
        keyboardcontroller = KeyboardController()
        if (keyboardcontroller == nil) {
            track("Failed to create keyboard controller")
            return
        }

        // Create game pad manager
        gamePadManager = GamePadManager(controller: self)
        if (gamePadManager == nil) {
            track("Failed to create game pad manager")
            return
        }
        
        // Setup toolbar, window, and debugger
        configureToolbar()
        configureWindow()
        setupDebugger()
        
        // Get metal running
        metal.setupMetal()
    
        // Load user defaults
        loadUserDefaults()
        
        // Enable message processing (register callback)
        addListener()

        // Power up. If all Roms are in place, the emulator starts running.
        // Otherwise, it sends a MISSING_ROM message.
        c64.powerUp()
        
        // Create speed monitor and get the timer tunning
        createTimer()
        
        // Update toolbar
        validateToolbarItems()
        
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
        let name = NSWindow.FrameAutosaveName("dirkwhoffmann.de.virtualC64.window")
        window?.setFrameAutosaveName(name)
        
        // Enable fullscreen mode
        window?.collectionBehavior = .fullScreenPrimary
    }
    
    func configureToolbar() {
        
        // Get and resize images
        let cutout = NSMakeRect(2, 0, 28, 28)
        
        var none = NSImage(named: "oxygen_none")
        none = none?.resizeImage(width: 32, height: 32, cutout: cutout)
        var keyset = NSImage(named: "oxygen_keys")
        keyset = keyset?.resizeImage(width: 32, height: 32, cutout: cutout)
        var mouse = NSImage(named: "devMouseTemplate")
        mouse = mouse?.resizeImage(width: 32, height: 32, cutout: cutout)
        var gamepad = NSImage(named: "crystal_gamepad")
        gamepad = gamepad?.resizeImage(width: 32, height: 32, cutout: cutout)
        
        validateJoystickToolbarItems()
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
                let mType = MessageType(rawValue: UInt32(type))
                myself.processMessage(Message(type: mType, data: data))
            }
        }
  
        c64.addListener(myself) { (ptr, type, data) in
            
            // Convert void pointer back to 'self'
            let myself = Unmanaged<MyController>.fromOpaque(ptr!).takeUnretainedValue()
            
            // Process message in the main thread
            DispatchQueue.main.async {
                let mType = MessageType(rawValue: UInt32(type))
                myself.processMessage(Message(type: mType, data: data))
            }
        }
        
        track("Listener is in place")
    }
    
    func createTimer() {
    
        // Create speed monitor
        speedometer = Speedometer()
        
        // Create timer and speedometer
        timerLock = NSLock()
        timer = Timer.scheduledTimer(timeInterval: 1.0/12, // 12 times a second
                                     target: self,
                                     selector: #selector(timerFunc),
                                     userInfo: nil,
                                     repeats: true)
        
        track("GUI timer is up and running")
    }
 
    
    //
    // Timer and message processing
    //
    
    @objc func timerFunc() {

        assert(timerLock != nil)
        timerLock.lock()
 
        animationCounter += 1

        // Do 12 times a second ...
        if (animationCounter % 1) == 0 {
            
            // Refresh debug panel if open
            if c64.isRunning() {
                let state = debugger.state
                if state == NSDrawerState.open || state == NSDrawerState.opening {
                    refresh()
                }
            }
        }
        
        // Do 6 times a second ...
        if (animationCounter % 2) == 0 {
 
        }
        
        // Do 3 times a second ...
        if (animationCounter % 4) == 0 {
            speedometer.updateWith(cycle: amiga.masterClock() / 4, frame: metal.frames)
            let mhz = speedometer.mhz
            let fps = speedometer.fps
            clockSpeed.stringValue = String(format:"%.2f MHz %.0f fps", mhz, fps)
            clockSpeedBar.doubleValue = 10 * mhz
        
            // Let the cursor disappear in fullscreen mode
            if metal.fullscreen &&
                CGEventSource.secondsSinceLastEventType(.combinedSessionState,
                                                        eventType: .mouseMoved) > 1.0 {
                NSCursor.setHiddenUntilMouseMoves(true)
            }
        }
        
        timerLock.unlock()
    }
 
    func processMessage(_ msg: Message) {

        /*
        var drive : AmigaDriveProxy? {
            get {
                assert(msg.data == 0 || msg.data == 1)
                return msg.data == 0 ? amigaProxy?.df0 : amigaProxy?.df1
            }
        }
        */
        
        switch (msg.type) {
    
        case MSG_READY_TO_POWER_ON:
    
            // Start emulator
            c64.run()
            amiga.powerOn()
            
            // Blend in emulator texture
            /*
            if (!metal.drawC64texture) {
                metal.blendIn()
                metal.drawC64texture = true
            }
            */
            
            // Process attachment (if any)
            mydocument.mountAmigaAttachment()
        
        case MSG_RUN:
            
            needsSaving = true
            disableUserEditing()
            validateToolbarItems()
            refresh()
    
        case MSG_PAUSE:
            
            enableUserEditing()
            validateToolbarItems()
            refresh()
    
        case MSG_POWER_ON:
            
            track()
            metal.blendIn()
            powerLED.image = NSImage.init(named: "powerLedOn")
            
        case MSG_POWER_OFF:
            
            track()
            metal.blendOut()
            powerLED.image = NSImage.init(named: "powerLedOff")
            
        case MSG_RESET:
            
            track()
            
        case MSG_DRIVE_CONNECT:
            
            switch (msg.data) {
                
            case 0: myAppDelegate.df0Menu.isHidden = false
            case 1: myAppDelegate.df1Menu.isHidden = false
            default: fatalError()
            }
            
            refreshStatusBar()
            
        case MSG_DRIVE_DISCONNECT:
            
            switch (msg.data) {
            case 0: myAppDelegate.df0Menu.isHidden = true
            case 1: myAppDelegate.df1Menu.isHidden = true
            default: fatalError()
            }
            
            // Remove drop target status from the disconnect drive
            if dragAndDropDrive === amiga.df(msg.data) {
                dragAndDropDrive = nil
            }
            
            refreshStatusBar()
            
        case MSG_DRIVE_DISK_INSERT,
             MSG_DRIVE_DISK_EJECT,
             MSG_DRIVE_DISK_UNSAVED,
             MSG_DRIVE_DISK_SAVED,
             MSG_DRIVE_DISK_PROTECTED,
             MSG_DRIVE_DISK_UNPROTECTED:

            refreshStatusBar()
            
        case MSG_DRIVE_LED_ON:
            
            assert(msg.data == 0 || msg.data == 1)
            let item = (msg.data == 0) ? df0LED : df1LED
            item!.image = NSImage.init(named: "driveLedOn")
            
        case MSG_DRIVE_LED_OFF:
            
            assert(msg.data == 0 || msg.data == 1)
            let item = (msg.data == 0) ? df0LED : df1LED
            item!.image = NSImage.init(named: "driveLedOff")
        
        case MSG_DRIVE_DMA_ON:
            
            assert(msg.data == 0 || msg.data == 1)
            let item = (msg.data == 0) ? df0DMA : df1DMA
            item!.startAnimation(self)

        case MSG_DRIVE_DMA_OFF:
            
            assert(msg.data == 0 || msg.data == 1)
            let item = (msg.data == 0) ? df0DMA : df1DMA
            item!.stopAnimation(self)

        case MSG_DRIVE_HEAD_UP,
             MSG_DRIVE_HEAD_DOWN:
            
            track()
            playSound(name: "drive_click", volume: 1.0)
            
 
            
            
            
        // DEPRECATED MESSAGES BELOW...
        case MSG_BASIC_ROM_LOADED,
             MSG_CHAR_ROM_LOADED,
             MSG_KERNAL_ROM_LOADED,
             MSG_VC1541_ROM_LOADED:
            break
            
        case MSG_ROM_MISSING:
            openPreferences()
            
            /*
            if (romDialogController == nil) {
                track("MSG_ROM_MISSING")
                let nibName = NSNib.Name("RomPrefs")
                romDialogController = RomPrefsController.init(windowNibName: nibName)
                romDialogController!.showSheet(withParent: self)
            }
             */
            
        case MSG_SNAPSHOT_TAKEN:
            break
    
        case MSG_CPU_OK,
             MSG_CPU_SOFT_BREAKPOINT_REACHED:
            break
            
        case MSG_CPU_HARD_BREAKPOINT_REACHED,
             MSG_CPU_ILLEGAL_INSTRUCTION:
            self.debugOpenAction(self)
            refresh()
            
        case MSG_WARP_ON,
             MSG_WARP_OFF,
             MSG_ALWAYS_WARP_ON,
             MSG_ALWAYS_WARP_OFF:

            if c64.alwaysWarp() {
                let name = NSImage.Name("hourglass3Template")
                warpIcon.image = NSImage.init(named: name)
            } else if (c64.warp()) {
                let name = NSImage.Name("hourglass2Template")
                warpIcon.image = NSImage.init(named: name)
            } else {
                let name = NSImage.Name("hourglass1Template")
                warpIcon.image = NSImage.init(named: name)
            }
    
     
        case MSG_KEYMATRIX,
             MSG_CHARSET:
            
            let appDelegate = NSApp.delegate as! MyAppDelegate
            if appDelegate.virtualKeyboard != nil {
                appDelegate.virtualKeyboard?.refresh()
            }
            
            // virtualKeyboard?.refresh()
            virtualKeyboardSheet?.refresh()

        case MSG_VC1541_ATTACHED,
             MSG_VC1541_DETACHED:
            track()
            
        case MSG_VC1541_ATTACHED_SOUND:
            
            // Not sure about the copyright of the following sound:
            // playSound:@"1541_power_on_0" volume:0.2];
            // Sound from Commodore 64 (C64) Preservation Project (c64preservation.com):
            playSound(name: "drive_click", volume: 1.0)
        
        case MSG_VC1541_DETACHED_SOUND:
            
            // Not sure about the copyright of the following sound:
            // playSound:@"1541_track_change_0" volume:0.6];
            // Sound from Commodore 64 (C64) Preservation Project (c64preservation.com):
            playSound(name: "drive_click", volume: 1.0)
    
        case MSG_VC1541_DISK_SOUND:
            
            // playSound:@"1541_door_closed_2" volume:0.2];
            playSound(name: "drive_snatch_uae", volume: 0.1)
            
        case MSG_VC1541_NO_DISK_SOUND:
            
            // playSound:@"1541_door_open_1" volume:0.2];
            playSound(name: "drive_snatch_uae", volume: 0.1)
            
        case MSG_VC1541_HEAD_UP_SOUND:
            
            // Not sure about the copyright of the following sound:
            // playSound:@"1541_track_change_0" volume:0.6];
            // Sound from Commodore 64 (C64) Preservation Project (c64preservation.com):
            playSound(name: "drive_click", volume: 1.0)
            
        case MSG_VC1541_HEAD_DOWN_SOUND:
            
            // Not sure about the copyright of the following sound:
            // playSound:@"1541_track_change_2" volume:1.0];
            // Sound from Commodore 64 (C64) Preservation Project (c64preservation.com):
            playSound(name: "drive_click", volume: 1.0)
            
        case MSG_VC1541_DISK,
             MSG_VC1541_NO_DISK,
             MSG_DISK_SAVED,
             MSG_DISK_UNSAVED,
             MSG_VC1541_RED_LED_ON,
             MSG_VC1541_RED_LED_OFF,
             MSG_VC1541_RED_LED_OFF,
             MSG_IEC_BUS_BUSY,
             MSG_IEC_BUS_IDLE,
             MSG_VC1541_MOTOR_ON,
             MSG_VC1541_MOTOR_OFF,
             MSG_VC1541_HEAD_UP,
             MSG_VC1541_HEAD_DOWN,
             MSG_VC1530_TAPE,
             MSG_VC1530_NO_TAPE,
             MSG_VC1530_PROGRESS,
             MSG_CARTRIDGE,
             MSG_NO_CARTRIDGE,
             MSG_NO_CARTRIDGE,
             MSG_CART_SWITCH:
            break
    
            
        default:
            
            track("Unknown message: \(msg)")
            assert(false)
        }
    }

    //
    // Dialogs
    //
    
    func openPreferences() {
        
        if preferencesController == nil {
            let nibName = NSNib.Name("Preferences")
            preferencesController = PreferencesController.init(windowNibName: nibName)
        }
        preferencesController!.showSheet()
    }
    
    //
    // Loading Roms
    //
    
    @discardableResult
    func loadRom(_ url: URL?) -> Bool {
        
        if (url == nil) {
            return false
        }
        
        if c64.loadBasicRom(url!) {
            // basicRomURL = url!
            return true
        }
        if c64.loadCharRom(url!) {
            // charRomURL = url!
            return true
        }
        if c64.loadKernalRom(url!) {
            // kernalRomURL = url!
            return true
        }
        if c64.loadVC1541Rom(url!) {
            // vc1541RomURL = url!
            return true
        }
        
        track("ROM file \(url!) not found")
        return false
    }
    

    //
    // Keyboard events
    //

    // Keyboard events are handled by the emulator window.
    // If they are handled here, some keys such as 'TAB' don't trigger an event.
 
    
    //
    //  Game pad events
    //
    
    /// GamePadManager delegation method
    /// - Returns: true, iff a joystick event has been triggered on port A or B
    @discardableResult
    func joystickEvent(slot: Int, events: [JoystickEvent]) -> Bool {
        
        if (slot == inputDevice1) {
            for event in events { c64.port1.trigger(event) }
            return true
        }

        if (slot == inputDevice2) {
            for event in events { c64.port2.trigger(event) }
            return true
        }
        
        return false
    }    

    
    //
    // Action methods (status bar)
    //
    
    @IBAction func alwaysWarpAction(_ sender: Any!) {
        
        undoManager?.registerUndo(withTarget: self) {
            targetSelf in targetSelf.alwaysWarpAction(sender)
        }
    
        amiga.setAlwaysWarp(!amiga.alwaysWarp())
        refresh()
    }
    
    
    //
    // Mounting media files
    //
    
    @discardableResult
    func mount(_ item: AnyC64FileProxy?) -> Bool {

        guard let type = item?.type() else { return false }
        
        switch (type) {
            
        case CRT_FILE:
            c64.expansionport.attachCartridgeAndReset(item as? CRTFileProxy)
            return true
            
        case TAP_FILE:
            return c64.datasette.insertTape(item as? TAPFileProxy)
            
        case T64_FILE, D64_FILE,
             PRG_FILE, P00_FILE,
             G64_FILE:
            // We need to take some special care for items that mount as a disk.
            // In that case, the light barrier has to be broken several times.
            // TODO: Use insertDisk for these attachments in future
            changeDisk(item, drive: 1)
            return true
                        
        default:
            track("Unknown attachment type \(type).")
            fatalError()
        }
    }
    
    // Emulates changing a disk including the necessary light barrier breaks
    // If disk is nil, only the ejection is emulated.
    func changeDisk(_ disk: AnyC64FileProxy?, drive nr: Int) {
        
        let drive = c64.drive(nr)!

        DispatchQueue.global().async {
            
            // For a better user experience, we switch on automatically
            // when a disk is inserted.
            if disk != nil {
                self.c64.drive(nr).powerOn()
            }
            
            // Remove old disk if present
            if drive.hasDisk() {
                drive.prepareToEject()
                usleep(300000)
                drive.ejectDisk()
            }
            
            // Insert new disk if provided
            if disk != nil {
                drive.prepareToInsert()
                usleep(300000)
                drive.insertDisk(disk as? AnyArchiveProxy)
            }
        }
    }

    
    //
    // Misc
    //
    
    func playSound(name: String, volume: Float) {
        
        if let s = NSSound.init(named: name) {
            s.volume = volume
            s.play()
        } else {
            track("ERROR: Cannot create NSSound object.")
        }
    }
}
