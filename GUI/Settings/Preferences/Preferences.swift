// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* Preferences
 *
 * This class stores all emulator settings that belong to the application level.
 * There is a single object of this class stored in the application delegate.
 * The object is shared among all emulator instances.
 *
 * See class "Configuration" for instance specific settings.
 */

class Preferences {
    
    //
    // General
    //
        
    // Floppy
    var blankDiskFormat = PeripheralsDefaults.std.blankDiskFormat
    var blankDiskFormatIntValue: Int {
        get { return Int(blankDiskFormat.rawValue) }
        set { blankDiskFormat = FSVolumeType.init(rawValue: newValue)! }
    }
    var bootBlock = PeripheralsDefaults.std.bootBlock
    
    // Fullscreen
    var keepAspectRatio = GeneralDefaults.std.keepAspectRatio
    var exitOnEsc = GeneralDefaults.std.exitOnEsc
            
    // Warp mode
    var warpMode = GeneralDefaults.std.warpMode {
        didSet { for c in myAppDelegate.controllers { c.updateWarp() } }
    }
    var warpModeIntValue: Int {
        get { return Int(warpMode.rawValue) }
        set { warpMode = WarpMode.init(rawValue: newValue)! }
    }
    
    // Misc
    var ejectWithoutAsking = GeneralDefaults.std.ejectWithoutAsking
    var closeWithoutAsking = GeneralDefaults.std.closeWithoutAsking
    var pauseInBackground = GeneralDefaults.std.pauseInBackground

    //
    // Controls
    //
    
    // Emulation keys
    var keyMaps = [ ControlsDefaults.std.mouseKeyMap,
                    ControlsDefaults.std.joyKeyMap1,
                    ControlsDefaults.std.joyKeyMap2 ]
    
    // Joystick
    var disconnectJoyKeys = ControlsDefaults.std.disconnectJoyKeys
    var autofire = ControlsDefaults.std.autofire {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.controlPort1.joystick.autofire = autofire
                amiga.controlPort2.joystick.autofire = autofire
            }
        }
    }
    var autofireBullets = ControlsDefaults.std.autofireBullets {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.controlPort1.joystick.autofireBullets = autofireBullets
                amiga.controlPort2.joystick.autofireBullets = autofireBullets
            }
        }
    }
    var autofireFrequency = ControlsDefaults.std.autofireFrequency {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.controlPort1.joystick.autofireFrequency = autofireFrequency
                amiga.controlPort2.joystick.autofireFrequency = autofireFrequency
            }
        }
    }
    
    // Mouse
    var retainMouseKeyComb = ControlsDefaults.std.retainMouseKeyComb
    var retainMouseWithKeys = ControlsDefaults.std.retainMouseWithKeys
    var retainMouseByClick = ControlsDefaults.std.retainMouseByClick
    var retainMouseByEntering = ControlsDefaults.std.retainMouseByEntering
    var releaseMouseKeyComb = ControlsDefaults.std.retainMouseKeyComb
    var releaseMouseWithKeys = ControlsDefaults.std.releaseMouseWithKeys
    var releaseMouseByShaking = ControlsDefaults.std.releaseMouseByShaking
     
    //
    // Devices
    //
        
    //
    // Captures
    //
    
    // Screenshots
    var autoScreenshots = CaptureDefaults.std.autoScreenshots
    var screenshotInterval = 0 {
        didSet { for c in myAppDelegate.controllers { c.startScreenshotTimer() } }
    }
    var screenshotSource = CaptureDefaults.std.screenshotSource
    var screenshotTarget = CaptureDefaults.std.screenshotTarget
    var screenshotTargetIntValue: Int {
        get { return Int(screenshotTarget.rawValue) }
        set { screenshotTarget = NSBitmapImageRep.FileType(rawValue: UInt(newValue))! }
    }

    // Snapshots
    var autoSnapshots = CaptureDefaults.std.autoSnapshots
    var snapshotInterval = 0 {
        didSet { for c in myAppDelegate.controllers { c.startSnapshotTimer() } }
    }

    // Screen captures
    var captureSource = CaptureDefaults.std.captureSource
    var bitRate = 512 {
        didSet {
            if bitRate < 64 { bitRate = 64 }
            if bitRate > 16384 { bitRate = 16384 }
        }
    }
    var aspectX = 768 {
        didSet {
            if aspectX < 1 { aspectX = 1 }
            if aspectX > 999 { aspectX = 999 }
        }
    }
    var aspectY = 702 {
        didSet {
            if aspectY < 1 { aspectY = 1 }
            if aspectY > 999 { aspectY = 999 }
        }
    }
    
    //
    // General
    //
    
    func loadGeneralDefaults(_ defaults: GeneralDefaults) {
                
        // Fullscreen
        keepAspectRatio = defaults.keepAspectRatio
        exitOnEsc = defaults.exitOnEsc
        
        // Warp mode
        warpMode = defaults.warpMode

        // Misc
        ejectWithoutAsking = defaults.ejectWithoutAsking
        pauseInBackground = defaults.pauseInBackground
        closeWithoutAsking = defaults.closeWithoutAsking
    }
    
    func loadGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
           
        // Fullscreen
        keepAspectRatio = defaults.bool(forKey: Keys.Gen.keepAspectRatio)
        exitOnEsc = defaults.bool(forKey: Keys.Gen.exitOnEsc)
    
        // Warp mode
        warpModeIntValue = defaults.integer(forKey: Keys.Gen.warpMode)

        // Misc
        ejectWithoutAsking = defaults.bool(forKey: Keys.Gen.ejectWithoutAsking)
        pauseInBackground = defaults.bool(forKey: Keys.Gen.pauseInBackground)
        closeWithoutAsking = defaults.bool(forKey: Keys.Gen.closeWithoutAsking)
    }
    
    func saveGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
                
        // Fullscreen
        defaults.set(keepAspectRatio, forKey: Keys.Gen.keepAspectRatio)
        defaults.set(exitOnEsc, forKey: Keys.Gen.exitOnEsc)
        
        // Warp mode
        defaults.set(warpModeIntValue, forKey: Keys.Gen.warpMode)

        // Misc
        defaults.set(ejectWithoutAsking, forKey: Keys.Gen.ejectWithoutAsking)
        defaults.set(pauseInBackground, forKey: Keys.Gen.pauseInBackground)
        defaults.set(closeWithoutAsking, forKey: Keys.Gen.closeWithoutAsking)
    }
    
    //
    // Controls
    //
    
    func loadControlsDefaults(_ defaults: ControlsDefaults) {
        
        // Emulation keys
        keyMaps[0] = defaults.mouseKeyMap
        keyMaps[1] = defaults.joyKeyMap1
        keyMaps[2] = defaults.joyKeyMap2
        disconnectJoyKeys = defaults.disconnectJoyKeys
        
        // Joysticks
        autofire = defaults.autofire
        autofireBullets = defaults.autofireBullets
        autofireFrequency = defaults.autofireFrequency
        
        // Mouse
        retainMouseKeyComb = defaults.retainMouseKeyComb
        retainMouseWithKeys = defaults.retainMouseWithKeys
        retainMouseByClick = defaults.retainMouseByClick
        retainMouseByEntering = defaults.retainMouseByEntering
        releaseMouseKeyComb = defaults.releaseMouseKeyComb
        releaseMouseWithKeys = defaults.releaseMouseWithKeys
        releaseMouseByShaking = defaults.releaseMouseByShaking
    }
    
    func loadControlsUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Emulation keys
        defaults.decode(&keyMaps[0], forKey: Keys.Con.mouseKeyMap)
        defaults.decode(&keyMaps[1], forKey: Keys.Con.joyKeyMap1)
        defaults.decode(&keyMaps[2], forKey: Keys.Con.joyKeyMap2)
        disconnectJoyKeys = defaults.bool(forKey: Keys.Con.disconnectJoyKeys)
        
        // Joysticks
        autofire = defaults.bool(forKey: Keys.Con.autofire)
        autofireBullets = defaults.integer(forKey: Keys.Con.autofireBullets)
        autofireFrequency = defaults.float(forKey: Keys.Con.autofireFrequency)
        
        // Mouse
        retainMouseKeyComb = defaults.integer(forKey: Keys.Con.retainMouseKeyComb)
        retainMouseWithKeys = defaults.bool(forKey: Keys.Con.retainMouseWithKeys)
        retainMouseByClick = defaults.bool(forKey: Keys.Con.retainMouseByClick)
        retainMouseByEntering = defaults.bool(forKey: Keys.Con.retainMouseByEntering)
        releaseMouseKeyComb = defaults.integer(forKey: Keys.Con.releaseMouseKeyComb)
        releaseMouseWithKeys = defaults.bool(forKey: Keys.Con.releaseMouseWithKeys)
        releaseMouseByShaking = defaults.bool(forKey: Keys.Con.releaseMouseByShaking)
    }
    
    func saveControlsUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Emulation keys
        defaults.encode(keyMaps[0], forKey: Keys.Con.mouseKeyMap)
        defaults.encode(keyMaps[1], forKey: Keys.Con.joyKeyMap1)
        defaults.encode(keyMaps[2], forKey: Keys.Con.joyKeyMap2)
        defaults.set(disconnectJoyKeys, forKey: Keys.Con.disconnectJoyKeys)
        
        // Joysticks
        defaults.set(autofire, forKey: Keys.Con.autofire)
        defaults.set(autofireBullets, forKey: Keys.Con.autofireBullets)
        defaults.set(autofireFrequency, forKey: Keys.Con.autofireFrequency)
        
        // Mouse
        defaults.set(retainMouseKeyComb, forKey: Keys.Con.retainMouseKeyComb)
        defaults.set(retainMouseWithKeys, forKey: Keys.Con.retainMouseWithKeys)
        defaults.set(retainMouseByClick, forKey: Keys.Con.retainMouseByClick)
        defaults.set(retainMouseByEntering, forKey: Keys.Con.retainMouseByEntering)
        defaults.set(releaseMouseKeyComb, forKey: Keys.Con.releaseMouseKeyComb)
        defaults.set(releaseMouseWithKeys, forKey: Keys.Con.releaseMouseWithKeys)
        defaults.set(releaseMouseByShaking, forKey: Keys.Con.releaseMouseByShaking)
    }

    //
    // Devices
    //
    
    func loadDevicesDefaults(_ defaults: DevicesDefaults) {
        
    }
    
    func loadDevicesUserDefaults() {
        
    }
    
    func saveDevicesUserDefaults() {
        
    }
    
    //
    // Captures
    //
    
    func loadCaptureDefaults(_ defaults: CaptureDefaults) {
        
        // Screenshots
        autoScreenshots = defaults.autoScreenshots
        screenshotInterval = defaults.autoScreenshotInterval
        screenshotSource = defaults.screenshotSource
        screenshotTarget = defaults.screenshotTarget

        // Snapshots
        autoSnapshots = defaults.autoSnapshots
        snapshotInterval = defaults.autoSnapshotInterval

        // Captures
        captureSource = defaults.captureSource
        bitRate = defaults.bitRate
        aspectX = defaults.aspectX
        aspectY = defaults.aspectY
    }
    
    func loadCaptureUserDefaults() {
        
        let defaults = UserDefaults.standard
           
        // Screenshots
        autoScreenshots = defaults.bool(forKey: Keys.Cap.autoScreenshots)
        screenshotInterval = defaults.integer(forKey: Keys.Cap.autoScreenshotInterval)
        screenshotSource = defaults.integer(forKey: Keys.Cap.screenshotSource)
        screenshotTargetIntValue = defaults.integer(forKey: Keys.Cap.screenshotTarget)

        // Snapshots
        autoSnapshots = defaults.bool(forKey: Keys.Cap.autoSnapshots)
        snapshotInterval = defaults.integer(forKey: Keys.Cap.autoSnapshotInterval)

        // Captures
        captureSource = defaults.integer(forKey: Keys.Cap.captureSource)
        bitRate = defaults.integer(forKey: Keys.Cap.bitRate)
        aspectX = defaults.integer(forKey: Keys.Cap.aspectX)
        aspectY = defaults.integer(forKey: Keys.Cap.aspectY)
    }
    
    func saveCaptureUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Screenshots
        defaults.set(autoSnapshots, forKey: Keys.Cap.autoSnapshots)
        defaults.set(snapshotInterval, forKey: Keys.Cap.autoSnapshotInterval)
        defaults.set(autoScreenshots, forKey: Keys.Cap.autoScreenshots)
        defaults.set(screenshotInterval, forKey: Keys.Cap.autoScreenshotInterval)
        defaults.set(screenshotSource, forKey: Keys.Cap.screenshotSource)

        // Snapshots
        defaults.set(autoSnapshots, forKey: Keys.Cap.autoSnapshots)
        defaults.set(snapshotInterval, forKey: Keys.Cap.autoSnapshotInterval)

        // Captures
        defaults.set(captureSource, forKey: Keys.Cap.captureSource)
        defaults.set(bitRate, forKey: Keys.Cap.bitRate)
        defaults.set(aspectX, forKey: Keys.Cap.aspectX)
        defaults.set(aspectY, forKey: Keys.Cap.aspectY)
    }
}
