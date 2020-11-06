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
    
    var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }

    //
    // General
    //
        
    // Floppy
    var driveBlankDiskFormat = GeneralDefaults.std.driveBlankDiskFormat
    var driveBlankDiskFormatIntValue: Int {
        get { return Int(driveBlankDiskFormat.rawValue) }
        set { driveBlankDiskFormat = EmptyDiskFormat.init(rawValue: newValue)! }
    }
    var ejectWithoutAsking = GeneralDefaults.std.ejectWithoutAsking
    var driveSounds = GeneralDefaults.std.driveSounds
    var driveSoundPan = GeneralDefaults.std.driveSoundPan
    var driveInsertSound = GeneralDefaults.std.driveInsertSound
    var driveEjectSound = GeneralDefaults.std.driveEjectSound
    var driveHeadSound = GeneralDefaults.std.driveHeadSound
    var drivePollSound = GeneralDefaults.std.drivePollSound
    
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
    var closeWithoutAsking = GeneralDefaults.std.closeWithoutAsking
    var pauseInBackground = GeneralDefaults.std.pauseInBackground

    //
    // Devices
    //
    
    // Emulation keys
    var keyMaps = [ DevicesDefaults.std.mouseKeyMap,
                    DevicesDefaults.std.joyKeyMap1,
                    DevicesDefaults.std.joyKeyMap2 ]
    
    // Joystick
    var disconnectJoyKeys = DevicesDefaults.std.disconnectJoyKeys
    var autofire = DevicesDefaults.std.autofire {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.controlPort1.joystick.autofire = autofire
                amiga.controlPort2.joystick.autofire = autofire
            }
        }
    }
    var autofireBullets = DevicesDefaults.std.autofireBullets {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.controlPort1.joystick.autofireBullets = autofireBullets
                amiga.controlPort2.joystick.autofireBullets = autofireBullets
            }
        }
    }
    var autofireFrequency = DevicesDefaults.std.autofireFrequency {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.controlPort1.joystick.autofireFrequency = autofireFrequency
                amiga.controlPort2.joystick.autofireFrequency = autofireFrequency
            }
        }
    }
    
    // Mouse
    var retainMouseKeyComb = DevicesDefaults.std.retainMouseKeyComb
    var retainMouseWithKeys = DevicesDefaults.std.retainMouseWithKeys
    var retainMouseByClick = DevicesDefaults.std.retainMouseByClick
    var retainMouseByEntering = DevicesDefaults.std.retainMouseByEntering
    var releaseMouseKeyComb = DevicesDefaults.std.retainMouseKeyComb
    var releaseMouseWithKeys = DevicesDefaults.std.releaseMouseWithKeys
    var releaseMouseByShaking = DevicesDefaults.std.releaseMouseByShaking
     
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
        
        // Floppy
        driveBlankDiskFormat = defaults.driveBlankDiskFormat
        ejectWithoutAsking = defaults.ejectWithoutAsking
        driveSounds = defaults.driveSounds
        driveSoundPan = defaults.driveSoundPan
        driveInsertSound = defaults.driveInsertSound
        driveEjectSound = defaults.driveEjectSound
        driveHeadSound = defaults.driveHeadSound
        drivePollSound = defaults.drivePollSound

        // Fullscreen
        keepAspectRatio = defaults.keepAspectRatio
        exitOnEsc = defaults.exitOnEsc
        
        // Warp mode
        warpMode = defaults.warpMode

        // Misc
        pauseInBackground = defaults.pauseInBackground
        closeWithoutAsking = defaults.closeWithoutAsking
    }
    
    func loadGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
           
        // Floppy
        driveBlankDiskFormatIntValue = defaults.integer(forKey: Keys.driveBlankDiskFormat)
        ejectWithoutAsking = defaults.bool(forKey: Keys.ejectWithoutAsking)
        driveSounds = defaults.bool(forKey: Keys.driveSounds)
        driveSoundPan = defaults.double(forKey: Keys.driveSoundPan)
        driveInsertSound = defaults.bool(forKey: Keys.driveInsertSound)
        driveEjectSound = defaults.bool(forKey: Keys.driveEjectSound)
        driveHeadSound = defaults.bool(forKey: Keys.driveHeadSound)
        drivePollSound = defaults.bool(forKey: Keys.drivePollSound)
        
        // Fullscreen
        keepAspectRatio = defaults.bool(forKey: Keys.keepAspectRatio)
        exitOnEsc = defaults.bool(forKey: Keys.exitOnEsc)
    
        // Warp mode
        warpModeIntValue = defaults.integer(forKey: Keys.warpMode)

        // Misc
        pauseInBackground = defaults.bool(forKey: Keys.pauseInBackground)
        closeWithoutAsking = defaults.bool(forKey: Keys.closeWithoutAsking)
    }
    
    func saveGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Floppy
        defaults.set(ejectWithoutAsking, forKey: Keys.ejectWithoutAsking)
        defaults.set(driveSounds, forKey: Keys.driveSounds)
        defaults.set(driveSoundPan, forKey: Keys.driveSoundPan)
        defaults.set(driveInsertSound, forKey: Keys.driveInsertSound)
        defaults.set(driveEjectSound, forKey: Keys.driveEjectSound)
        defaults.set(driveHeadSound, forKey: Keys.driveHeadSound)
        defaults.set(drivePollSound, forKey: Keys.drivePollSound)
        defaults.set(driveBlankDiskFormatIntValue, forKey: Keys.driveBlankDiskFormat)
        
        // Fullscreen
        defaults.set(keepAspectRatio, forKey: Keys.keepAspectRatio)
        defaults.set(exitOnEsc, forKey: Keys.exitOnEsc)
        
        // Warp mode
        defaults.set(warpModeIntValue, forKey: Keys.warpMode)

        // Misc
        defaults.set(pauseInBackground, forKey: Keys.pauseInBackground)
        defaults.set(closeWithoutAsking, forKey: Keys.closeWithoutAsking)
    }
    
    //
    // Devices
    //
    
    func loadDevicesDefaults(_ defaults: DevicesDefaults) {
        
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
    
    func loadDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Emulation keys
        defaults.decode(&keyMaps[0], forKey: Keys.Dev.mouseKeyMap)
        defaults.decode(&keyMaps[1], forKey: Keys.Dev.joyKeyMap1)
        defaults.decode(&keyMaps[2], forKey: Keys.Dev.joyKeyMap2)
        disconnectJoyKeys = defaults.bool(forKey: Keys.Dev.disconnectJoyKeys)
        
        // Joysticks
        autofire = defaults.bool(forKey: Keys.Dev.autofire)
        autofireBullets = defaults.integer(forKey: Keys.Dev.autofireBullets)
        autofireFrequency = defaults.float(forKey: Keys.Dev.autofireFrequency)
        
        // Mouse
        retainMouseKeyComb = defaults.integer(forKey: Keys.Dev.retainMouseKeyComb)
        retainMouseWithKeys = defaults.bool(forKey: Keys.Dev.retainMouseWithKeys)
        retainMouseByClick = defaults.bool(forKey: Keys.Dev.retainMouseByClick)
        retainMouseByEntering = defaults.bool(forKey: Keys.Dev.retainMouseByEntering)
        releaseMouseKeyComb = defaults.integer(forKey: Keys.Dev.releaseMouseKeyComb)
        releaseMouseWithKeys = defaults.bool(forKey: Keys.Dev.releaseMouseWithKeys)
        releaseMouseByShaking = defaults.bool(forKey: Keys.Dev.releaseMouseByShaking)
    }
    
    func saveDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Emulation keys
        defaults.encode(keyMaps[0], forKey: Keys.Dev.mouseKeyMap)
        defaults.encode(keyMaps[1], forKey: Keys.Dev.joyKeyMap1)
        defaults.encode(keyMaps[2], forKey: Keys.Dev.joyKeyMap2)
        defaults.set(disconnectJoyKeys, forKey: Keys.Dev.disconnectJoyKeys)
        
        // Joysticks
        defaults.set(autofire, forKey: Keys.Dev.autofire)
        defaults.set(autofireBullets, forKey: Keys.Dev.autofireBullets)
        defaults.set(autofireFrequency, forKey: Keys.Dev.autofireFrequency)
        
        // Mouse
        defaults.set(retainMouseKeyComb, forKey: Keys.Dev.retainMouseKeyComb)
        defaults.set(retainMouseWithKeys, forKey: Keys.Dev.retainMouseWithKeys)
        defaults.set(retainMouseByClick, forKey: Keys.Dev.retainMouseByClick)
        defaults.set(retainMouseByEntering, forKey: Keys.Dev.retainMouseByEntering)
        defaults.set(releaseMouseKeyComb, forKey: Keys.Dev.releaseMouseKeyComb)
        defaults.set(releaseMouseWithKeys, forKey: Keys.Dev.releaseMouseWithKeys)
        defaults.set(releaseMouseByShaking, forKey: Keys.Dev.releaseMouseByShaking)
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
