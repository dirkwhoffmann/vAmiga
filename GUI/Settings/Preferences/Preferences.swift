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

class Preferences {
    
    //
    // General
    //
       
    // Snapshots
    var autoSnapshots = false {
        didSet { for c in myAppDelegate.controllers { c.validateSnapshotTimer() } }
    }
    var snapshotInterval = 0 {
        didSet { for c in myAppDelegate.controllers { c.validateSnapshotTimer() } }
    }

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
            for amiga in myAppDelegate.proxies {
                amiga.recorder.path = ffmpegPath
            }
        }
    }
    var captureSource = 0
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
            
    // Warp mode
    var warpMode = WarpMode.off {
        didSet { for c in myAppDelegate.controllers { c.updateWarp() } }
    }
    var warpModeIntValue: Int {
        get { return Int(warpMode.rawValue) }
        set { warpMode = WarpMode(rawValue: newValue)! }
    }
    
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
    /*
            ControlsDefaults.std.mouseKeyMap,
                    ControlsDefaults.std.joyKeyMap1,
                    ControlsDefaults.std.joyKeyMap2 ]
    */
    
    // Joystick
    var disconnectJoyKeys: Bool!
    var autofire: Bool! {
        didSet {
            for amiga in myAppDelegate.proxies {
                amiga.configure(.AUTOFIRE, enable: autofire)
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
     
    //
    // All
    //
    
    func registerUserDefaults() {
        
        registerGeneralUserDefaults()
        registerControlsUserDefaults()
        registerDevicesUserDefaults()
    }
    
    func applyUserDefaults() {

        applyGeneralUserDefaults()
        applyControlsUserDefaults()
        applyDevicesUserDefaults()
    }
    
    //
    // General
    //
        
    func registerGeneralUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        // Snapshots
        defaults.register(Keys.Gen.autoSnapshots, false)
        defaults.register(Keys.Gen.autoSnapshotInterval, 20)
        
        // Screenshots
        defaults.register(Keys.Gen.screenshotSource, 0)
        defaults.register(Keys.Gen.screenshotTarget, NSBitmapImageRep.FileType.png.rawValue)

        // Captures
        defaults.register(Keys.Gen.ffmpegPath, "")
        defaults.register(Keys.Gen.captureSource, 0)
        defaults.register(Keys.Gen.bitRate, 2048)
        defaults.register(Keys.Gen.aspectX, 768)
        defaults.register(Keys.Gen.aspectY, 702)
        
        // Fullscreen
        defaults.register(Keys.Gen.keepAspectRatio, false)
        defaults.register(Keys.Gen.exitOnEsc, true)
        
        // Warp mode
        defaults.register(Keys.Gen.warpMode, WarpMode.off.rawValue)
        
        // Misc
        defaults.register(Keys.Gen.ejectWithoutAsking, false)
        defaults.register(Keys.Gen.detachWithoutAsking, false)
        defaults.register(Keys.Gen.closeWithoutAsking, false)
        defaults.register(Keys.Gen.pauseInBackground, false)
    }
    
    func removeGeneralUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        let keys = [ Keys.Gen.autoSnapshots,
                     Keys.Gen.autoSnapshotInterval,
                     
                     Keys.Gen.screenshotSource,
                     Keys.Gen.screenshotTarget,
                     
                     Keys.Gen.ffmpegPath,
                     Keys.Gen.captureSource,
                     Keys.Gen.bitRate,
                     Keys.Gen.aspectX,
                     Keys.Gen.aspectY,
            
                     Keys.Gen.keepAspectRatio,
                     Keys.Gen.exitOnEsc,
                     
                     Keys.Gen.warpMode,
                     
                     Keys.Gen.ejectWithoutAsking,
                     Keys.Gen.detachWithoutAsking,
                     Keys.Gen.closeWithoutAsking,
                     Keys.Gen.pauseInBackground
        ]

        for key in keys { defaults.removeKey(key) }
    }
    
    func saveGeneralUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
                
        defaults.set(Keys.Gen.autoSnapshots, autoSnapshots)
        defaults.set(Keys.Gen.autoSnapshotInterval, snapshotInterval)

        defaults.set(Keys.Gen.screenshotSource, screenshotSource)
        defaults.set(Keys.Gen.screenshotTarget, screenshotTargetIntValue)

        defaults.set(Keys.Gen.ffmpegPath, ffmpegPath)
        defaults.set(Keys.Gen.captureSource, captureSource)
        defaults.set(Keys.Gen.bitRate, bitRate)
        defaults.set(Keys.Gen.aspectX, aspectX)
        defaults.set(Keys.Gen.aspectY, aspectY)
        
        defaults.set(Keys.Gen.keepAspectRatio, keepAspectRatio)
        defaults.set(Keys.Gen.exitOnEsc, exitOnEsc)
        
        defaults.set(Keys.Gen.warpMode, warpModeIntValue)

        defaults.set(Keys.Gen.ejectWithoutAsking, ejectWithoutAsking)
        defaults.set(Keys.Gen.detachWithoutAsking, detachWithoutAsking)
        defaults.set(Keys.Gen.closeWithoutAsking, closeWithoutAsking)
        defaults.set(Keys.Gen.pauseInBackground, pauseInBackground)
        
        defaults.save()
    }
    
    func applyGeneralUserDefaults() {
           
        log(level: 2)
        let defaults = AmigaProxy.defaults!
                   
        autoSnapshots = defaults.getBool(Keys.Gen.autoSnapshots)
        snapshotInterval = defaults.getInt(Keys.Gen.autoSnapshotInterval)

        screenshotSource = defaults.getInt(Keys.Gen.screenshotSource)
        screenshotTargetIntValue = defaults.getInt(Keys.Gen.screenshotTarget)

        ffmpegPath = defaults.getString(Keys.Gen.ffmpegPath) ?? ""
        captureSource = defaults.getInt(Keys.Gen.captureSource)
        bitRate = defaults.getInt(Keys.Gen.bitRate)
        aspectX = defaults.getInt(Keys.Gen.aspectX)
        aspectY = defaults.getInt(Keys.Gen.aspectY)
        
        keepAspectRatio = defaults.getBool(Keys.Gen.keepAspectRatio)
        exitOnEsc = defaults.getBool(Keys.Gen.exitOnEsc)
    
        warpModeIntValue = defaults.getInt(Keys.Gen.warpMode)

        ejectWithoutAsking = defaults.getBool(Keys.Gen.ejectWithoutAsking)
        detachWithoutAsking = defaults.getBool(Keys.Gen.detachWithoutAsking)
        closeWithoutAsking = defaults.getBool(Keys.Gen.closeWithoutAsking)
        pauseInBackground = defaults.getBool(Keys.Gen.pauseInBackground)
    }
    
    //
    // Controls
    //

    func registerControlsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        let emptyMap: [MacKey: Int] = [:]

        let stdKeyMap1: [MacKey: Int] = [
            
            MacKey(keyCode: kVK_LeftArrow): GamePadAction.PULL_LEFT.rawValue,
            MacKey(keyCode: kVK_RightArrow): GamePadAction.PULL_RIGHT.rawValue,
            MacKey(keyCode: kVK_UpArrow): GamePadAction.PULL_UP.rawValue,
            MacKey(keyCode: kVK_DownArrow): GamePadAction.PULL_DOWN.rawValue,
            MacKey(keyCode: kVK_Space): GamePadAction.PRESS_FIRE.rawValue
        ]
        
        let stdKeyMap2 = [
            
            MacKey(keyCode: kVK_ANSI_S): GamePadAction.PULL_LEFT.rawValue,
            MacKey(keyCode: kVK_ANSI_D): GamePadAction.PULL_RIGHT.rawValue,
            MacKey(keyCode: kVK_ANSI_E): GamePadAction.PULL_UP.rawValue,
            MacKey(keyCode: kVK_ANSI_X): GamePadAction.PULL_DOWN.rawValue,
            MacKey(keyCode: kVK_ANSI_C): GamePadAction.PRESS_FIRE.rawValue
        ]
        
        // Emulation keys
        defaults.register(Keys.Con.mouseKeyMap, encodable: emptyMap)
        defaults.register(Keys.Con.joyKeyMap1, encodable: stdKeyMap1)
        defaults.register(Keys.Con.joyKeyMap2, encodable: stdKeyMap2)
        defaults.register(Keys.Con.disconnectJoyKeys, true)
        
        // Joysticks
        defaults.register(Keys.Con.autofire, false)
        defaults.register(Keys.Con.autofireBullets, -3)
        defaults.register(Keys.Con.autofireFrequency, 2.5)

        // Mouse
        defaults.register(Keys.Con.retainMouseKeyComb, 0)
        defaults.register(Keys.Con.retainMouseWithKeys, true)
        defaults.register(Keys.Con.retainMouseByClick, true)
        defaults.register(Keys.Con.retainMouseByEntering, false)
        defaults.register(Keys.Con.releaseMouseKeyComb, 0)
        defaults.register(Keys.Con.releaseMouseWithKeys, true)
        defaults.register(Keys.Con.releaseMouseByShaking, true)
    }
    
    func removeControlsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        let keys = [ Keys.Con.mouseKeyMap,
                     Keys.Con.joyKeyMap1,
                     Keys.Con.joyKeyMap2,
                     Keys.Con.disconnectJoyKeys,
                     
                     Keys.Con.autofire,
                     Keys.Con.autofireBullets,
                     Keys.Con.autofireFrequency,
                     
                     Keys.Con.retainMouseKeyComb,
                     Keys.Con.retainMouseWithKeys,
                     Keys.Con.retainMouseByClick,
                     Keys.Con.retainMouseByEntering,
                     Keys.Con.releaseMouseKeyComb,
                     Keys.Con.releaseMouseWithKeys,
                     Keys.Con.releaseMouseByShaking ]

        for key in keys { defaults.removeKey(key) }
    }

    func saveControlsUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
                
        defaults.set(Keys.Con.mouseKeyMap, encodable: keyMaps[0])
        defaults.set(Keys.Con.joyKeyMap1, encodable: keyMaps[1])
        defaults.set(Keys.Con.joyKeyMap2, encodable: keyMaps[2])
        defaults.set(Keys.Con.disconnectJoyKeys, disconnectJoyKeys)

        defaults.set(Keys.Con.autofire, autofire)
        defaults.set(Keys.Con.autofireBullets, autofireBullets)
        defaults.set(Keys.Con.autofireFrequency, autofireFrequency)

        defaults.set(Keys.Con.retainMouseKeyComb, retainMouseKeyComb)
        defaults.set(Keys.Con.retainMouseWithKeys, retainMouseWithKeys)
        defaults.set(Keys.Con.retainMouseByClick, retainMouseByClick)
        defaults.set(Keys.Con.retainMouseByEntering, retainMouseByEntering)
        defaults.set(Keys.Con.releaseMouseKeyComb, releaseMouseKeyComb)
        defaults.set(Keys.Con.releaseMouseWithKeys, releaseMouseWithKeys)
        defaults.set(Keys.Con.releaseMouseByShaking, releaseMouseByShaking)
        
        defaults.save()
    }
    
    func applyControlsUserDefaults() {
           
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        defaults.decode(Keys.Con.mouseKeyMap, encodable: &keyMaps[0])
        defaults.decode(Keys.Con.joyKeyMap1, encodable: &keyMaps[1])
        defaults.decode(Keys.Con.joyKeyMap2, encodable: &keyMaps[2])
        disconnectJoyKeys = defaults.getBool(Keys.Con.disconnectJoyKeys)
        
        autofire = defaults.getBool(Keys.Con.autofire)
        autofireBullets = defaults.getInt(Keys.Con.autofireBullets)
        autofireFrequency = defaults.getDouble(Keys.Con.autofireFrequency)
        
        retainMouseKeyComb = defaults.getInt(Keys.Con.retainMouseKeyComb)
        retainMouseWithKeys = defaults.getBool(Keys.Con.retainMouseWithKeys)
        retainMouseByClick = defaults.getBool(Keys.Con.retainMouseByClick)
        retainMouseByEntering = defaults.getBool(Keys.Con.retainMouseByEntering)
        releaseMouseKeyComb = defaults.getInt(Keys.Con.releaseMouseKeyComb)
        releaseMouseWithKeys = defaults.getBool(Keys.Con.releaseMouseWithKeys)
        releaseMouseByShaking = defaults.getBool(Keys.Con.releaseMouseByShaking)
    }
    
    //
    // Devices
    //
        
    func registerDevicesUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        // Mapping schemes
        defaults.register(Keys.Dev.leftStickScheme1, 0)
        defaults.register(Keys.Dev.rightStickScheme1, 0)
        defaults.register(Keys.Dev.hatSwitchScheme1, 0)

        defaults.register(Keys.Dev.leftStickScheme2, 0)
        defaults.register(Keys.Dev.rightStickScheme2, 0)
        defaults.register(Keys.Dev.hatSwitchScheme2, 0)
    }
    
    func removeDevicesUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        let keys = [ Keys.Dev.leftStickScheme1,
                     Keys.Dev.rightStickScheme1,
                     Keys.Dev.hatSwitchScheme1,
                     
                     Keys.Dev.leftStickScheme2,
                     Keys.Dev.rightStickScheme2,
                     Keys.Dev.hatSwitchScheme2 ]

        for key in keys { defaults.removeKey(key) }
    }
    
    func saveDevicesUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
                        
        defaults.save()
    }
    
    func applyDevicesUserDefaults() {
           
        log(level: 2)
    }
}
