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

    // Fullscreen
    var keepAspectRatio = false
    var exitOnEsc = false

    // Misc
    var ejectWithoutAsking = false
    var detachWithoutAsking = false
    var closeWithoutAsking = false
    var pauseInBackground = false

    //
    // Captures
    //

    // Snapshots
    var snapshotCompressor = Compressor.NONE
    var snapshotCompressorIntValue: Int {
        get { return Int(snapshotCompressor.rawValue) }
        set { snapshotCompressor = Compressor(rawValue: newValue) ?? snapshotCompressor }
    }
    var snapshotAutoDelete: Bool = true

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

    //
    // Controls
    //
    
    // Emulation keys
    var keyMaps: [[MacKey: Int]] = [ [:], [:], [:] ]

    // Joystick
    var autofire: Bool! {
        didSet {
            for emu in myAppDelegate.proxies {
                emu?.set(.JOY_AUTOFIRE, enable: autofire)
            }
        }
    }
    var autofireBursts: Bool! {
        didSet {
            for emu in myAppDelegate.proxies {
                emu?.set(.JOY_AUTOFIRE_BURSTS, enable: autofireBursts)
            }
        }
    }
    var autofireBullets: Int! {
        didSet {
            for emu in myAppDelegate.proxies {
                emu?.set(.JOY_AUTOFIRE_BULLETS, value: autofireBullets)
            }
        }
    }
    var autofireFrequency: Int! {
        didSet {
            autofireFrequency = autofireFrequency.clamped(4, 50)
            for emu in myAppDelegate.proxies {
                emu?.set(.JOY_AUTOFIRE_DELAY, value: autofireFrequency)
            }
        }
    }

    // Keyboard
    var disconnectJoyKeys: Bool!
    var amigaKeysCombEnable: Bool!
    var amigaKeysComb: Int!

    // Mouse
    var retainMouseKeyComb: Int! // DEPRECATED
    var retainMouseByClick: Bool!
    var retainMouseByEntering: Bool!
    var releaseMouseKeyComb: Int! // DEPRECATED
    var releaseMouseByShaking: Bool!
}
