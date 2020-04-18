// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* All user-customizable items are managed by two different classes.
 *
 * Preferences
 * This class stores all items that belong to the application level. There is
 * a single object of this class and the stored values apply to all emulator
 * instances.
 *
 * Configuration
 * This class stores all items that are specific to an individual emulator
 * instance. Each instance keeps its own object of this class.
 */

class Preferences {
    
    var parent: MyAppDelegate!
    /*
    var amiga: AmigaProxy { return parent.amiga }
    var renderer: Renderer { return parent.renderer }
    var gamePadManager: GamePadManager { return parent.gamePadManager }
    var kbController: KBController { return parent.kbController }
    */
    
    //
    // General
    //
        
    var warpLoad = Defaults.warpLoad {
        didSet { for c in parent.controllers { c.updateWarp() } }
    }
    var driveSounds = Defaults.driveSounds
    var driveSoundPan = Defaults.driveSoundPan
    var driveInsertSound = Defaults.driveInsertSound
    var driveEjectSound = Defaults.driveEjectSound
    var driveHeadSound = Defaults.driveHeadSound
    var drivePollSound = Defaults.drivePollSound
    var driveBlankDiskFormat = Defaults.driveBlankDiskFormat
    
    var driveBlankDiskFormatIntValue: Int {
        get { return Int(driveBlankDiskFormat.rawValue) }
        set { driveBlankDiskFormat = FileSystemType.init(newValue) }
    }
    var keepAspectRatio = Defaults.keepAspectRatio
    var exitOnEsc = Defaults.exitOnEsc
    
    var closeWithoutAsking = Defaults.closeWithoutAsking
    var ejectWithoutAsking = Defaults.ejectWithoutAsking
    var pauseInBackground = Defaults.pauseInBackground
    
    // Remembers if the emulator was running or paused when it lost focus.
    // Needed to implement the pauseInBackground feature.
    var pauseInBackgroundSavedState = false
    
    var autoSnapshots = Defaults.autoSnapshots
    var snapshotInterval = 0 {
        didSet { for c in parent.controllers { c.startSnapshotTimer() } }
    }
    var autoScreenshots = Defaults.autoScreenshots
    var screenshotInterval = 0 {
        didSet { for c in parent.controllers { c.startScreenshotTimer() } }
    }
    var screenshotSource = Defaults.screenshotSource
    var screenshotTarget = Defaults.screenshotTarget
    var screenshotTargetIntValue: Int {
        get { return Int(screenshotTarget.rawValue) }
        set { screenshotTarget = NSBitmapImageRep.FileType(rawValue: UInt(newValue))! }
    }
    
    //
    // Devices preferences
    //
    
    var disconnectJoyKeys = Defaults.disconnectJoyKeys
    
    var autofire = Defaults.autofire {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.joystick1.setAutofire(autofire)
                amiga.joystick2.setAutofire(autofire)
            }
        }
    }
    var autofireBullets = Defaults.autofireBullets {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.joystick1.setAutofireBullets(autofireBullets)
                amiga.joystick2.setAutofireBullets(autofireBullets)
            }
        }
    }
    var autofireFrequency = Defaults.autofireFrequency {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.joystick1.setAutofireFrequency(autofireFrequency)
                amiga.joystick2.setAutofireFrequency(autofireFrequency)
            }
        }
    }
    
    var keyMaps = [ Defaults.joyKeyMap1, Defaults.joyKeyMap2, Defaults.mouseKeyMap ]

    var retainMouseKeyComb = Defaults.retainMouseKeyComb
    var retainMouseWithKeys = Defaults.retainMouseWithKeys
    var retainMouseByClick = Defaults.retainMouseByClick
    var retainMouseByEntering = Defaults.retainMouseByEntering
    var releaseMouseKeyComb = Defaults.retainMouseKeyComb
    var releaseMouseWithKeys = Defaults.releaseMouseWithKeys
    var releaseMouseByShaking = Defaults.releaseMouseByShaking
 
    init(with delegate: MyAppDelegate) { parent = delegate }
    
    func loadGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
                
        warpLoad = defaults.bool(forKey: Keys.warpLoad)
        driveSounds = defaults.bool(forKey: Keys.driveSounds)
        driveSoundPan = defaults.double(forKey: Keys.driveSoundPan)
        driveInsertSound = defaults.bool(forKey: Keys.driveInsertSound)
        driveEjectSound = defaults.bool(forKey: Keys.driveEjectSound)
        driveHeadSound = defaults.bool(forKey: Keys.driveHeadSound)
        drivePollSound = defaults.bool(forKey: Keys.drivePollSound)
        driveBlankDiskFormatIntValue = defaults.integer(forKey: Keys.driveBlankDiskFormat)
        
        autoSnapshots = defaults.bool(forKey: Keys.autoSnapshots)
        snapshotInterval = defaults.integer(forKey: Keys.autoSnapshotInterval)
        autoScreenshots = defaults.bool(forKey: Keys.autoScreenshots)
        screenshotInterval = defaults.integer(forKey: Keys.autoScreenshotInterval)
        screenshotSource = defaults.integer(forKey: Keys.screenshotSource)
        screenshotTargetIntValue = defaults.integer(forKey: Keys.screenshotTarget)
    
        keepAspectRatio = defaults.bool(forKey: Keys.keepAspectRatio)
        exitOnEsc = defaults.bool(forKey: Keys.exitOnEsc)
        
        pauseInBackground = defaults.bool(forKey: Keys.pauseInBackground)
        closeWithoutAsking = defaults.bool(forKey: Keys.closeWithoutAsking)
        ejectWithoutAsking = defaults.bool(forKey: Keys.ejectWithoutAsking)
    }
    
    func saveGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(warpLoad, forKey: Keys.warpLoad)
        defaults.set(driveSounds, forKey: Keys.driveSounds)
        defaults.set(driveSoundPan, forKey: Keys.driveSoundPan)
        defaults.set(driveInsertSound, forKey: Keys.driveInsertSound)
        defaults.set(driveEjectSound, forKey: Keys.driveEjectSound)
        defaults.set(driveHeadSound, forKey: Keys.driveHeadSound)
        defaults.set(drivePollSound, forKey: Keys.drivePollSound)
        defaults.set(driveBlankDiskFormatIntValue, forKey: Keys.driveBlankDiskFormat)
        
        defaults.set(autoSnapshots, forKey: Keys.autoSnapshots)
        defaults.set(snapshotInterval, forKey: Keys.autoSnapshotInterval)
        defaults.set(autoScreenshots, forKey: Keys.autoScreenshots)
        defaults.set(screenshotInterval, forKey: Keys.autoScreenshotInterval)
        defaults.set(screenshotSource, forKey: Keys.screenshotSource)
        defaults.set(screenshotTargetIntValue, forKey: Keys.screenshotTarget)
        
        defaults.set(keepAspectRatio, forKey: Keys.keepAspectRatio)
        defaults.set(exitOnEsc, forKey: Keys.exitOnEsc)
                
        defaults.set(pauseInBackground, forKey: Keys.pauseInBackground)
        defaults.set(closeWithoutAsking, forKey: Keys.closeWithoutAsking)
        defaults.set(ejectWithoutAsking, forKey: Keys.ejectWithoutAsking)
    }
    
    func loadDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Emulation keys
        defaults.decode(&keyMaps[0], forKey: Keys.joyKeyMap1)
        defaults.decode(&keyMaps[1], forKey: Keys.joyKeyMap2)
        defaults.decode(&keyMaps[2], forKey: Keys.mouseKeyMap)
        disconnectJoyKeys = defaults.bool(forKey: Keys.disconnectJoyKeys)
        
        // Joysticks
        autofire = defaults.bool(forKey: Keys.autofire)
        autofireBullets = defaults.integer(forKey: Keys.autofireBullets)
        autofireFrequency = defaults.float(forKey: Keys.autofireFrequency)
        
        // Mouse
        retainMouseKeyComb = defaults.integer(forKey: Keys.retainMouseKeyComb)
        retainMouseWithKeys = defaults.bool(forKey: Keys.retainMouseWithKeys)
        retainMouseByClick = defaults.bool(forKey: Keys.retainMouseByClick)
        retainMouseByEntering = defaults.bool(forKey: Keys.retainMouseByEntering)
        releaseMouseKeyComb = defaults.integer(forKey: Keys.releaseMouseKeyComb)
        releaseMouseWithKeys = defaults.bool(forKey: Keys.releaseMouseWithKeys)
        releaseMouseByShaking = defaults.bool(forKey: Keys.releaseMouseByShaking)
    }
    
    func saveDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Emulation keys
        defaults.encode(keyMaps[0], forKey: Keys.joyKeyMap1)
        defaults.encode(keyMaps[1], forKey: Keys.joyKeyMap2)
        defaults.encode(keyMaps[2], forKey: Keys.mouseKeyMap)
        defaults.set(disconnectJoyKeys, forKey: Keys.disconnectJoyKeys)
        
        // Joysticks
        defaults.set(autofire, forKey: Keys.autofire)
        defaults.set(autofireBullets, forKey: Keys.autofireBullets)
        defaults.set(autofireFrequency, forKey: Keys.autofireFrequency)
        
        // Mouse
        defaults.set(retainMouseKeyComb, forKey: Keys.retainMouseKeyComb)
        defaults.set(retainMouseWithKeys, forKey: Keys.retainMouseWithKeys)
        defaults.set(retainMouseByClick, forKey: Keys.retainMouseByClick)
        defaults.set(retainMouseByEntering, forKey: Keys.retainMouseByEntering)
        defaults.set(releaseMouseKeyComb, forKey: Keys.releaseMouseKeyComb)
        defaults.set(releaseMouseWithKeys, forKey: Keys.releaseMouseWithKeys)
        defaults.set(releaseMouseByShaking, forKey: Keys.releaseMouseByShaking)
    }
}
