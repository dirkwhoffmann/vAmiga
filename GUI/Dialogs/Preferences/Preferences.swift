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

@MainActor
class Preferences {
    
    //
    // General
    //

    // Workspaces
    var workspaceCompression = false {
        didSet {
            for emu in myAppDelegate.proxies {
                emu.set(.AMIGA_WS_COMPRESS, enable: workspaceCompression)
            }
        }
    }
    
    // Snapshots
    var autoSnapshots = false {
        didSet {
            for emu in myAppDelegate.proxies {
                emu.set(.AMIGA_SNAP_AUTO, enable: autoSnapshots)
            }
        }
    }
    var snapshotInterval = 0 {
        didSet {
            for emu in myAppDelegate.proxies {
                emu.set(.AMIGA_SNAP_DELAY, value: snapshotInterval)
            }
        }
    }

    // Snapshots
    var snapshotStorage = 0 {
        didSet { for c in myAppDelegate.controllers {
            c.mydocument.snapshots.maxSize = snapshotStorage * 1024 * 1024 }
        }
    }

    // Screenshots
    var screenshotFormat = NSBitmapImageRep.FileType.png
    var screenshotFormatIntValue: Int {
        get { return Int(screenshotFormat.rawValue) }
        set { screenshotFormat = NSBitmapImageRep.FileType(rawValue: UInt(newValue)) ?? screenshotFormat }
    }

    var screenshotSource = ScreenshotSource.emulator
    var screenshotSourceIntValue: Int {
        get { return Int(screenshotSource.rawValue) }
        set { screenshotSource = ScreenshotSource(rawValue: newValue) ?? screenshotSource }
    }

    var screenshotCutout = ScreenshotCutout.visible
    var screenshotCutoutIntValue: Int {
        get { return Int(screenshotCutout.rawValue) }
        set { screenshotCutout = ScreenshotCutout(rawValue: newValue) ?? screenshotCutout }
    }

    var screenshotWidth = 1200 {
        didSet {
            screenshotWidth = max(screenshotWidth, 0)
            screenshotWidth = min(screenshotWidth, TextureSize.merged.width)
        }
    }

    var screenshotHeight = 900 {
        didSet {
            screenshotHeight = max(screenshotHeight, 0)
            screenshotHeight = min(screenshotHeight, TextureSize.merged.height)
        }
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
                amiga.set(.JOY_AUTOFIRE, enable: autofire)
            }
        }
    }
    var autofireBursts: Bool! {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.set(.JOY_AUTOFIRE_BURSTS, enable: autofireBursts)
            }
        }
    }
    var autofireBullets: Int! {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.set(.JOY_AUTOFIRE_BULLETS, value: autofireBullets)
            }
        }
    }
    var autofireFrequency: Int! {
        didSet {
            autofireFrequency = autofireFrequency.clamped(4, 50)
            for amiga in myAppDelegate.proxies {
                amiga.set(.JOY_AUTOFIRE_DELAY, value: autofireFrequency)
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
