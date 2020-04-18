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
}
