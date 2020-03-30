// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

enum WarpMode {

    case auto
    case on
    case off
}

protocol MessageReceiver {
    func processMessage(_ msg: Message)
}

class MyController: NSWindowController, MessageReceiver {

    // Proxy
    // Implements a bridge between the emulator written in C++ and the
    // GUI written in Swift. Because Swift cannot interact with C++ directly,
    // the proxy is written in Objective-C.
    var amiga: AmigaProxy!

    // Inspector of this emulator instance
    var inspector: Inspector?

    // Monitor of this emulator instance
    var monitor: Monitor?

    // Preferences controller
    var preferencesController: PreferencesController?

    // Audio Engine
    var audioEngine: AudioEngine!
    
    // Game pad manager
    var gamePadManager: GamePadManager!
    
    // Keyboard controller
    var kbController: KBController!

    // Virtual keyboard
    var virtualKeyboardSheet: VKBController?
    
    // Loop timer
    // The timer fires 60 times a second and executes all tasks that need to be
    //  done perdiodically (e.g., updating the speedometer and the debug panels)
    var timer: Timer?
    
    // Timer lock
    var timerLock: NSLock!

    // Screenshot timer
    // The timer is started when a new disk is inserted. It periodically
    // captures screenshots and stores them in the Application Support folder.
    var screenshotTimer: Timer?
    
    // Counts the number of screenshots taken
    var screenshotCounter = 0
    
    // Speedometer to measure clock frequence and frames per second
    var speedometer: Speedometer!
    
    // Used inside the timer function to fine tune timed events
    var animationCounter = 0
        
    // Current mouse coordinate
    var mouseXY = NSPoint(x: 0, y: 0)
    
    // Indicates if mouse is currently hidden DEPRECATED
    var hideMouse = false

    // Indicates if a status bar is shown
    var statusBar = true

    // Small disk icon to be shown in NSMenuItems
    var smallDisk = NSImage.init(named: "diskTemplate")!.resize(width: 16.0, height: 16.0)
    
    // Drive that receives drag and drop inputs
    var dragAndDropDrive: DriveProxy?

    // Serial input and output
    var serialIn = ""
    var serialOut = ""

    // Warp mode
    var warpMode = WarpMode.auto { didSet { updateWarp() } }
    
    //
    // Preferences items
    //
    
    // General
    
    // Selected game pad slot for joystick in port A
    var inputDevice1 = Defaults.inputDevice1
    
    // Selected game pad slot for joystick in port B
    var inputDevice2 = Defaults.inputDevice2

    // Rom preferences
    
    // Rom URLs
    var romURL: URL = Defaults.rom
    var extURL: URL = Defaults.ext

    // Devices preferences
    var disconnectJoyKeys: Bool {
        get { return kbController.disconnectJoyKeys }
        set {
            kbController.disconnectJoyKeys = newValue
        }
    }
    var autofire: Bool {
        get { return amiga.joystick1.autofire() }
        set {
            amiga.joystick1.setAutofire(newValue)
            amiga.joystick2.setAutofire(newValue)
        }
    }
    var autofireBullets: Int {
        get { return amiga.joystick1.autofireBullets() }
        set {
            amiga.joystick1.setAutofireBullets(newValue)
            amiga.joystick2.setAutofireBullets(newValue)
        }
    }
    var autofireFrequency: Float {
        get { return amiga.joystick1.autofireFrequency() }
        set {
            amiga.joystick1.setAutofireFrequency(newValue)
            amiga.joystick2.setAutofireFrequency(newValue)
        }
    }
    var keyMap0: [MacKey: UInt32]? {
        get { return gamePadManager.gamePads[0]?.keyMap }
        set { gamePadManager.gamePads[0]?.keyMap = newValue }
    }
    var keyMap1: [MacKey: UInt32]? {
        get { return gamePadManager.gamePads[1]?.keyMap }
        set { gamePadManager.gamePads[1]?.keyMap = newValue }
    }
 
    // Video preferences
 
    var enhancer: Int {
        get { return renderer.enhancer }
        set { renderer.enhancer = newValue }
    }
    var upscaler: Int {
        get { return renderer.upscaler }
        set { renderer.upscaler = newValue }
    }
    var palette: Int {
        get { return Int(amiga.denise.palette()) }
        set { amiga.denise.setPalette(Palette(newValue)) }
    }
    var brightness: Double {
        get { return amiga.denise.brightness() }
        set { amiga.denise.setBrightness(newValue) }
    }
    var contrast: Double {
        get { return amiga.denise.contrast() }
        set { amiga.denise.setContrast(newValue) }
    }
    var saturation: Double {
        get { return amiga.denise.saturation() }
        set { amiga.denise.setSaturation(newValue) }
    }
    var blur: Int32 {
        get { return renderer.shaderOptions.blur }
        set { renderer.shaderOptions.blur = newValue }
    }
    var blurRadius: Float {
        get { return renderer.shaderOptions.blurRadius }
        set { renderer.shaderOptions.blurRadius = newValue }
    }
    var bloom: Int32 {
        get { return renderer.shaderOptions.bloom }
        set { renderer.shaderOptions.bloom = newValue }
    }
    var bloomRadius: Float {
        get { return renderer.shaderOptions.bloomRadius }
        set { renderer.shaderOptions.bloomRadius = newValue }
    }
    var bloomBrightness: Float {
        get { return renderer.shaderOptions.bloomBrightness }
        set { renderer.shaderOptions.bloomBrightness = newValue }
    }
    var bloomWeight: Float {
        get { return renderer.shaderOptions.bloomWeight }
        set { renderer.shaderOptions.bloomWeight = newValue }
    }
    var flicker: Int32 {
        get { return renderer.shaderOptions.flicker }
        set { renderer.shaderOptions.flicker = newValue }
    }
    var flickerWeight: Float {
        get { return renderer.shaderOptions.flickerWeight }
        set { renderer.shaderOptions.flickerWeight = newValue }
    }
    var dotMask: Int32 {
        get { return renderer.shaderOptions.dotMask }
        set { renderer.shaderOptions.dotMask = newValue }
    }
    var dotMaskBrightness: Float {
        get { return renderer.shaderOptions.dotMaskBrightness }
        set { renderer.shaderOptions.dotMaskBrightness = newValue }
    }
    var scanlines: Int32 {
        get { return renderer.shaderOptions.scanlines }
        set { renderer.shaderOptions.scanlines = newValue }
    }
    var scanlineBrightness: Float {
        get { return renderer.shaderOptions.scanlineBrightness }
        set { renderer.shaderOptions.scanlineBrightness = newValue }
    }
    var scanlineWeight: Float {
        get { return renderer.shaderOptions.scanlineWeight }
        set { renderer.shaderOptions.scanlineWeight = newValue }
    }
    var disalignment: Int32 {
        get { return renderer.shaderOptions.disalignment }
        set { renderer.shaderOptions.disalignment = newValue }
    }
    var disalignmentH: Float {
        get { return renderer.shaderOptions.disalignmentH }
        set { renderer.shaderOptions.disalignmentH = newValue }
    }
    var disalignmentV: Float {
        get { return renderer.shaderOptions.disalignmentV }
        set { renderer.shaderOptions.disalignmentV = newValue }
    }
    
    //
    // Emulator preferences
    //
    
    // var alwaysWarp = false { didSet { updateWarp() } }
    
    var warpLoad = Defaults.warpLoad { didSet { updateWarp() } }
    var driveNoise = Defaults.driveNoise
    var driveNoiseNoPoll = Defaults.driveNoiseNoPoll
    var driveBlankDiskFormat = Defaults.driveBlankDiskFormat
    var driveBlankDiskFormatIntValue: Int {
        get { return Int(driveBlankDiskFormat.rawValue) }
        set { driveBlankDiskFormat = FileSystemType.init(newValue) }
    }
    var screenshotSource = Defaults.screenshotSource
    var screenshotTarget = Defaults.screenshotTarget
    var screenshotTargetIntValue: Int {
        get { return Int(screenshotTarget.rawValue) }
        set { screenshotTarget = NSBitmapImageRep.FileType(rawValue: UInt(newValue))! }
    }
    var keepAspectRatio: Bool {
        get { return renderer.keepAspectRatio }
        set { renderer.keepAspectRatio = newValue }
    }
    var exitOnEsc: Bool {
        get { return kbController.exitOnEsc }
        set { kbController.exitOnEsc = newValue }
    }
    var closeWithoutAsking = Defaults.closeWithoutAsking
    var ejectWithoutAsking = Defaults.ejectWithoutAsking
    var pauseInBackground = Defaults.pauseInBackground
    
    // Remembers if the emulator was running or paused when it lost focus.
    // Needed to implement the pauseInBackground feature.
    var pauseInBackgroundSavedState = false
    
    var takeAutoSnapshots: Bool {
        get { return amiga.takeAutoSnapshots() }
        set { amiga.setTakeAutoSnapshots(newValue) }
    }
    var snapshotInterval: Int {
        get { return amiga.snapshotInterval() }
        set { amiga.setSnapshotInterval(newValue) }
    }
  
    // Updates the warp status
    func updateWarp() {

        var warp: Bool

        switch warpMode {
        case .auto: warp = amiga.diskController.spinning() && warpLoad
        case .on: warp = true
        case .off: warp = false
        }

        warp ? amiga.warpOn() : amiga.warpOff()
    }
    
    // Returns the icon of the sand clock in the bottom bar
    var hourglassIcon: NSImage? {
        switch warpMode {
        case .auto:
            return NSImage.init(named: amiga.warp() ? "hourglass3Template" : "hourglass1Template")
        case .on:
            return NSImage.init(named: "warpLockOnTemplate")
        case .off:
            return NSImage.init(named: "warpLockOffTemplate")
        }
    }

    /*
    var hourglassIcon: NSImage? {
        if amiga.warp() {
            if warpMode == .auto {
                return NSImage.init(named: "hourglass2Template")
            } else {
                return NSImage.init(named: "hourglass3Template")
            }
        } else {
            return NSImage.init(named: "hourglass1Template")
        }
    }
    */

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

    @IBOutlet weak var clockSpeed: NSTextField!
    @IBOutlet weak var clockSpeedBar: NSLevelIndicator!
    @IBOutlet weak var warpIcon: NSButton!
    
    // Toolbar
    @IBOutlet weak var toolbar: NSToolbar!
}

extension MyController {

    // Provides the undo manager
    override open var undoManager: UndoManager? { return metal.undoManager }
 
    // Provides the document casted to the correct type
    var mydocument: MyDocument? { return document as? MyDocument }
    
    /// Indicates if the emulator needs saving
    var needsSaving: Bool {
        get {
            return document?.changeCount != 0
        }
        set {
            if newValue && !closeWithoutAsking {
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
                
        // Create audio engine
        audioEngine = AudioEngine.init(withPaula: amiga.paula)
    }

    override open func windowDidLoad() {
 
        track()
        
        // Reset mouse coordinates
        mouseXY = NSPoint.zero
        hideMouse = false
        
        // Create keyboard controller
        kbController = KBController(parent: self)
        if kbController == nil {
            track("Failed to create keyboard controller")
            return
        }

        // Create game pad manager
        gamePadManager = GamePadManager(controller: self)
        if gamePadManager == nil {
            track("Failed to create game pad manager")
            return
        }

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
        if amiga.readyToPowerOn() == ERR_OK {

            // Power on the Amiga
            amiga.powerOn()

            // Process attachment (if any)
            mydocument?.mountAmigaAttachment()

            // Launch the emulator thread
            amiga.run()

        } else {

            // Open the Rom dialog
            openPreferences(tab: "Roms")
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
        timer = Timer.scheduledTimer(timeInterval: 1.0/12, // 12 times a second
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

        // Do 6 times a second ...
        if (animationCounter % 2) == 0 {
 
        }
        
        // Do 3 times a second ...
        if (animationCounter % 4) == 0 {
            speedometer.updateWith(cycle: amiga.cpu.cycles(), frame: renderer.frames)
            let mhz = speedometer.mhz
            let fps = speedometer.fps
            clockSpeed.stringValue = String(format: "%.2f MHz %.0f fps", mhz, fps)
            clockSpeedBar.doubleValue = 10 * mhz
        
            // Let the cursor disappear in fullscreen mode
            if renderer.fullscreen &&
                CGEventSource.secondsSinceLastEventType(.combinedSessionState,
                                                        eventType: .mouseMoved) > 1.0 {
                NSCursor.setHiddenUntilMouseMoves(true)
            }
        }
        
        timerLock.unlock()
    }
 
    @objc func screenshotTimerFunc() {
        
        takeScreenshot(auto: true)
        screenshotCounter += 1
        
        // Schedule the next screenshot
        let delay = TimeInterval(4 * screenshotCounter)
        track("Taking next screenshot in \(delay) seconds")
        
        screenshotTimer = Timer.scheduledTimer(timeInterval: delay,
                                               target: self,
                                               selector: #selector(screenshotTimerFunc),
                                               userInfo: nil,
                                               repeats: false)
    }
        
    func processMessage(_ msg: Message) {

        switch msg.type {
    
        case MSG_CONFIG:
            inspector?.fullRefresh()

        case MSG_POWER_ON:

            serialIn = ""
            serialOut = ""
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

            inspector?.fullRefresh()

        case MSG_WARP_ON,
             MSG_WARP_OFF:

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

        case MSG_DRIVE_SELECT,
             MSG_DRIVE_READ,
             MSG_DRIVE_WRITE:
            refreshStatusBar()

        case MSG_DRIVE_LED_ON:
            
            let image = NSImage.init(named: "driveLedOn")
            switch msg.data {
            case 0: df0LED.image = image
            case 1: df1LED.image = image
            case 2: df2LED.image = image
            case 3: df3LED.image = image
            default: fatalError()
            }
            
        case MSG_DRIVE_LED_OFF:
            
            let image = NSImage.init(named: "driveLedOff")
            switch msg.data {
            case 0: df0LED.image = image
            case 1: df1LED.image = image
            case 2: df2LED.image = image
            case 3: df3LED.image = image
            default: fatalError()
            }
            
        case MSG_DRIVE_MOTOR_ON,
             MSG_DRIVE_MOTOR_OFF:
            
            updateWarp()
            refreshStatusBar()

        case MSG_DRIVE_HEAD:

            if driveNoise {
                playSound(name: "drive_click", volume: 1.0)
            }
            refreshStatusBar()
  
        case MSG_DRIVE_HEAD_POLL:
 
            if driveNoise && !driveNoiseNoPoll {
                playSound(name: "drive_click", volume: 1.0)
            }
            refreshStatusBar()
            
        case MSG_DISK_INSERT:

            track()

            if msg.data == 0 { // Df0
                Screenshot.deleteAutoFolder(checksum: amiga.df0.fnv())
                screenshotCounter = 0
                screenshotTimer?.invalidate()
                let initial = TimeInterval(Int.random(in: 4..<8))
                screenshotTimer = Timer.scheduledTimer(timeInterval: initial,
                                                       target: self,
                                                       selector: #selector(screenshotTimerFunc),
                                                       userInfo: nil,
                                                       repeats: false)
            }
            refreshStatusBar()
            
        case MSG_DISK_EJECT,
             MSG_DISK_UNSAVED,
             MSG_DISK_SAVED,
             MSG_DISK_PROTECTED,
             MSG_DISK_UNPROTECTED:

            refreshStatusBar()

        case MSG_SER_IN:
            serialIn += String(UnicodeScalar(msg.data & 0xFF)!)

        case MSG_SER_OUT:
            serialOut += String(UnicodeScalar.init(msg.data & 0xFF)!)

        case MSG_USERSNAPSHOT_LOADED,
             MSG_USERSNAPSHOT_SAVED,
             MSG_AUTOSNAPSHOT_LOADED:
            renderer.blendIn(steps: 20)

        case MSG_AUTOSNAPSHOT_SAVED:
            break
            
        default:
            track("Unknown message: \(msg)")
            assert(false)
        }
    }

    //
    // Dialogs
    //
    
    func openPreferences(tab: String = "") {
        
        if preferencesController == nil {
            let name = NSNib.Name("Preferences")
            preferencesController = PreferencesController.make(parent: self,
                                                               nibName: name)
        }
        preferencesController?.showSheet(tab: tab)
    }
    
    //
    // Keyboard events
    //

    // Keyboard events are handled by the emulator window.
    // If they are handled here, some keys such as 'TAB' don't trigger an event.

    //
    //  Game pad events
    //
    
    // GamePadManager delegation method
    // Returns true, iff a joystick event has been triggered on port A or B
    @discardableResult
    func joystickAction(slot: Int, events: [GamePadAction]) -> Bool {
        
        if slot == inputDevice1 {
            for event in events {
                amiga.joystick1.trigger(event)
                amiga.mouse.trigger(event)
            }
            return true
        }

        if slot == inputDevice2 {
            for event in events {
                amiga.joystick2.trigger(event)
                amiga.mouse.trigger(event)
            }
            return true
        }
        
        return false
    }    

    //
    // Action methods (status bar)
    //
    
    @IBAction func warpAction(_ sender: Any!) {

        track()

        switch warpMode {
        case .auto: warpMode = .on
        case .on: warpMode = .off
        case .off: warpMode = .auto
        }

        refreshStatusBar()
    }

    //
    // Screenshots
    //
    
    func takeScreenshot(auto: Bool) {
        
        let upscaled = screenshotSource > 0
        let checksum = amiga.df0.fnv()
        
        if checksum == 0 { return }
        
        guard let screen = renderer.screenshot(afterUpscaling: upscaled) else {
            track("Failed to create screenshot")
            return
        }
        let screenshot = Screenshot.init(screen: screen, upscaled: upscaled)
        
        var url: URL?
        if auto {
            url = Screenshot.newAutoUrl(checksum: checksum)
        } else {
            url = Screenshot.newUserUrl(checksum: checksum)
        }
        
        if url == nil {
            track("Failed to create URL")
            return
        }
        
        track("Saving screenshot to \(url!.path)")
        try? screenshot.save(url: url!, format: .jpeg)
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
