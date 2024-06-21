// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Carbon.HIToolbox

/* This class stores all emulator settings that belong to the application level.
 * There is a single object of this class stored in the application delegate.
 * The object is shared among all emulator instances.
 *
 * See class "Configuration" for instance specific settings.
 */

public enum CaptureSource: Int {

    case visible = 0
    case entire = 1
}

class Preferences {
    
    //
    // General
    //
      
    // Snapshots
    var autoSnapshots = false {
        didSet {
            for emu in myAppDelegate.proxies {
                emu.set(.AMIGA_SNAPSHOTS, enable: autoSnapshots)
            }
        }
    }
    var snapshotInterval = 0 {
        didSet {
            for emu in myAppDelegate.proxies {
                emu.set(.AMIGA_SNAPSHOT_DELAY, value: snapshotInterval)
            }
        }
    }


    // Snapshots
    var snapshotStorage = 0 {
        didSet { for c in myAppDelegate.controllers {
            c.mydocument.snapshots.maxSize = snapshotStorage * 1024 * 1024 }
        }
    }
    /*
    var autoSnapshots = false {
        didSet { for c in myAppDelegate.controllers {
            c.validateSnapshotTimer() }
        }
    }
    var snapshotInterval = 0 {
        didSet { for c in myAppDelegate.controllers {
            c.validateSnapshotTimer() }
        }
    }
    */

    // Screenshots
    var screenshotSource = 0
    var screenshotTarget = NSBitmapImageRep.FileType.png
    var screenshotTargetIntValue: Int {
        get { return Int(screenshotTarget.rawValue) }
        set { screenshotTarget = NSBitmapImageRep.FileType(rawValue: UInt(newValue))! }
    }

    // Screen captures
    var ffmpegPath = "" {
        didSet {
            for proxy in myAppDelegate.proxies {
                proxy.recorder.path = ffmpegPath
            }
        }
    }
    var captureSource: CaptureSource = .visible
    var captureSourceIntValue: Int {
        get { return Int(captureSource.rawValue) }
        set { captureSource = CaptureSource(rawValue: Int(newValue)) ?? .visible }
    }

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
        
    // Fullscreen
    var keepAspectRatio = false
    var exitOnEsc = false

    // Misc
    var ejectWithoutAsking = false
    var detachWithoutAsking = false
    var closeWithoutAsking = false
    var pauseInBackground = false

    //
    // Controls
    //
    
    // Emulation keys
    var keyMaps: [[MacKey: Int]] = [ [:], [:], [:] ]

    // Joystick
    var disconnectJoyKeys: Bool!
    var autofire: Bool! {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.configure(.AUTOFIRE_ENABLE, enable: autofire)
            }
        }
    }
    var autofireBullets: Int! {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.configure(.AUTOFIRE_BULLETS, value: autofireBullets)
            }
        }
    }
    var autofireFrequency: Double! {
        didSet {
            autofireFrequency = autofireFrequency.clamped(1, 4)
            let autofireDelay = Int(50.0 / autofireFrequency)
            for amiga in myAppDelegate.proxies {
                amiga.configure(.AUTOFIRE_DELAY, value: autofireDelay)
            }
        }
    }

    // Mouse
    var retainMouseKeyComb: Int!
    var retainMouseWithKeys: Bool!
    var retainMouseByClick: Bool!
    var retainMouseByEntering: Bool!
    var releaseMouseKeyComb: Int!
    var releaseMouseWithKeys: Bool!
    var releaseMouseByShaking: Bool!
}
