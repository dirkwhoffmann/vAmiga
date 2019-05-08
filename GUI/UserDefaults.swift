// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation
import Carbon.HIToolbox

//
// Convenience extensions to UserDefaults
//

extension UserDefaults {
    
    /// Registers an item of generic type 'Encodable'
    func register<T: Encodable>(encodableItem item: T, forKey key: String) {
        
        if let data = try? PropertyListEncoder().encode(item) {
            register(defaults: [key: data])
        }
    }

    /// Encodes an item of generic type 'Encodable'
    func encode<T: Encodable>(_ item: T, forKey key: String) {
        
        if let encoded = try? PropertyListEncoder().encode(item) {
            track("Encoded \(key) successfully")
            set(encoded, forKey: key)
        } else {
            track("Failed to encode \(key)")
        }
    }
    
    /// Encodes an item of generic type 'Decodable'
    func decode<T: Decodable>(_ item: inout T, forKey key: String) {
        
        if let data = data(forKey: key) {
            if let decoded = try? PropertyListDecoder().decode(T.self, from: data) {
                track("Decoded \(key) successfully")
                item = decoded
            } else {
                track("Failed to decode \(key)")
            }
        }
    }
}

//
// User defaults (all)
//

extension MyController {
    
    static func registerUserDefaults() {
        
        track()
        
        registerGeneralUserDefaults()
        registerRomUserDefaults()
        registerDevicesUserDefaults()
        registerVideoUserDefaults()
        registerEmulatorUserDefaults()
        registerHardwareUserDefaults()
    }
    
    func resetUserDefaults() {
        
        track()
        
        amiga.suspend()
        
        resetGeneralUserDefaults()
        resetRomUserDefaults()
        resetDevicesUserDefaults()
        resetVideoUserDefaults()
        resetEmulatorUserDefaults()
        resetHardwareUserDefaults()
        
        amiga.resume()
    }
    
    func loadUserDefaults() {
        
        track()
        
        amiga.suspend()
        
        loadGeneralUserDefaults()
        loadRomUserDefaults()
        loadDevicesUserDefaults()
        loadVideoUserDefaults()
        loadEmulatorUserDefaults()
        loadHardwareUserDefaults()
        
        amiga.resume()
    }
    
    func loadUserDefaults(url: URL) {
        
        if let fileContents = NSDictionary(contentsOf: url) {
            
            if let dict = fileContents as? Dictionary<String,Any> {
                
                let filteredDict = dict.filter { $0.0.hasPrefix("VAMIGA") }
                
                let defaults = UserDefaults.standard
                defaults.setValuesForKeys(filteredDict)
                
                loadUserDefaults()
            }
        }
    }
    
    func saveUserDefaults() {
        
        track()
        
        saveGeneralUserDefaults()
        saveRomUserDefaults()
        saveDevicesUserDefaults()
        saveVideoUserDefaults()
        saveEmulatorUserDefaults()
        saveHardwareUserDefaults()
    }

    func saveUserDefaults(url: URL) {
        
        track()
        
        let dict = UserDefaults.standard.dictionaryRepresentation()
        let filteredDict = dict.filter { $0.0.hasPrefix("VAMIGA") }
        let nsDict = NSDictionary.init(dictionary: filteredDict)
        nsDict.write(to: url, atomically: true)
    }
    
}

//
// User defaults (general)
//

struct Keys {
    
    // Control ports
    static let inputDevice1      = "VAMIGAInputDevice1"
    static let inputDevice2      = "VAMIGAInputDevice2"
}

struct Defaults {
    
    // Control ports
    static let inputDevice1 = -1
    static let inputDevice2 = -1
}

extension MyController {

    static func registerGeneralUserDefaults() {
        
        let dictionary : [String:Any] = [
            
            Keys.inputDevice1: Defaults.inputDevice1,
            Keys.inputDevice2: Defaults.inputDevice2,
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }
    
    func resetGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [ Keys.inputDevice1,
                     Keys.inputDevice2 ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadGeneralUserDefaults()
    }
    
    func loadGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        setPort1(defaults.integer(forKey: Keys.inputDevice1))
        setPort2(defaults.integer(forKey: Keys.inputDevice2))

        amiga.resume()
    }
    
    func saveGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(inputDevice1, forKey: Keys.inputDevice1)
        defaults.set(inputDevice2, forKey: Keys.inputDevice2)
    }
}

//
// User defaults (Roms)
//

extension Keys {
    
    static let bootRom           = "VAMIGABootRomFileKey"
    static let kickRom           = "VAMIGAKickRomFileKey"
}

extension Defaults {
    
    static let bootRom = URL(fileURLWithPath: "")
    static let kickRom = URL(fileURLWithPath: "")
}

extension MyController {
    
    static func registerRomUserDefaults() {
        
        let dictionary : [String:Any] = [
            
            Keys.bootRom: Defaults.bootRom,
            Keys.kickRom: Defaults.kickRom,
            ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }

    func resetRomUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [ Keys.bootRom,
                     Keys.kickRom]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadRomUserDefaults()
    }
    
    func loadRomUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        if let url = defaults.url(forKey: Keys.bootRom) {
            bootRomURL = url
            amiga.mem.loadBootRom(fromFile: bootRomURL)
        }
        if let url = defaults.url(forKey: Keys.kickRom) {
            kickRomURL = url
            amiga.mem.loadKickRom(fromFile: kickRomURL)
        }
        amiga.resume()
    }
    
    func saveRomUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(bootRomURL, forKey: Keys.bootRom)
        defaults.set(kickRomURL, forKey: Keys.kickRom)
    }
}


//
// User defaults (Devices)
//

extension Keys {
    
    // Joysticks
    static let joyKeyMap1            = "VAMIGAJoyKeyMap1"
    static let joyKeyMap2            = "VAMIGAJoyKeyMap2"
    static let disconnectJoyKeys     = "VAMIGADisconnectKeys"
    static let autofire              = "VAMIGAAutofire"
    static let autofireBullets       = "VAMIGAAutofireBullets"
    static let autofireFrequency     = "VAMIGAAutofireFrequency"
    
    // Mouse
    static let retainMouseKeyComb    = "VAMIGARetainMouseKeyComb"
    static let retainMouseWithKeys   = "VAMIGARetainMouseWithKeys"
    static let retainMouseByClick    = "VAMIGARetainMouseByClick"
    static let retainMouseByEntering = "VAMIGARetainMouseByEntering"
    static let releaseMouseKeyComb   = "VAMIGAReleaseMouseKeyComb"
    static let releaseMouseWithKeys  = "VAMIGAReleaseMouseWithKeys"
    static let releaseMouseByShaking = "VAMIGAReleaseMouseByShaking"
}

extension Defaults {
    
    // Joysticks
    static let joyKeyMap1 = [
        MacKey.init(keyCode: kVK_LeftArrow):  JOYSTICK_LEFT.rawValue,
        MacKey.init(keyCode: kVK_RightArrow): JOYSTICK_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_DownArrow):  JOYSTICK_UP.rawValue,
        MacKey.init(keyCode: kVK_UpArrow):    JOYSTICK_DOWN.rawValue,
        MacKey.init(keyCode: kVK_Space):      JOYSTICK_FIRE.rawValue
    ]
    static let joyKeyMap2 = [
        MacKey.init(keyCode: kVK_ANSI_S):     JOYSTICK_LEFT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_D):     JOYSTICK_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_E):     JOYSTICK_UP.rawValue,
        MacKey.init(keyCode: kVK_ANSI_X):     JOYSTICK_DOWN.rawValue,
        MacKey.init(keyCode: kVK_ANSI_C):     JOYSTICK_FIRE.rawValue
    ]
    
    static let disconnectJoyKeys = true
    static let autofire          = false
    static let autofireBullets   = -3
    static let autofireFrequency = Float(2.5)
    
    // Mouse
    static let retainMouseKeyComb    = 0
    static let retainMouseWithKeys   = true
    static let retainMouseByClick    = true
    static let retainMouseByEntering = false
    static let releaseMouseKeyComb   = 0
    static let releaseMouseWithKeys  = true
    static let releaseMouseByShaking = true
}
    
extension MyController {
    
    static func registerDevicesUserDefaults() {
        
        let dictionary : [String:Any] = [

            // Joysticks
            Keys.disconnectJoyKeys:     Defaults.disconnectJoyKeys,
            Keys.autofire:              Defaults.autofire,
            Keys.autofireBullets:       Defaults.autofireBullets,
            Keys.autofireFrequency:     Defaults.autofireFrequency,
            
            // Mouse
            Keys.retainMouseKeyComb:    Defaults.retainMouseKeyComb,
            Keys.retainMouseWithKeys:   Defaults.retainMouseWithKeys,
            Keys.retainMouseByClick:    Defaults.retainMouseByClick,
            Keys.retainMouseByEntering: Defaults.retainMouseByEntering,
            Keys.releaseMouseKeyComb:   Defaults.releaseMouseKeyComb,
            Keys.releaseMouseWithKeys:  Defaults.releaseMouseWithKeys,
            Keys.releaseMouseByShaking: Defaults.releaseMouseByShaking
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
        defaults.register(encodableItem: Defaults.joyKeyMap1, forKey: Keys.joyKeyMap1)
        defaults.register(encodableItem: Defaults.joyKeyMap2, forKey: Keys.joyKeyMap2)
    }

    func resetDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [ Keys.joyKeyMap1,
                     Keys.joyKeyMap2,
                     
                     Keys.disconnectJoyKeys,
                     Keys.autofire,
                     Keys.autofireBullets,
                     Keys.autofireFrequency,
                     
                     Keys.retainMouseKeyComb,
                     Keys.retainMouseWithKeys,
                     Keys.retainMouseByClick,
                     Keys.retainMouseByEntering,
                     Keys.releaseMouseKeyComb,
                     Keys.releaseMouseWithKeys,
                     Keys.releaseMouseByShaking
            ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadDevicesUserDefaults()
    }
    
    func loadDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
    
        amiga.suspend()
        
        // Joysticks
        defaults.decode(&gamePadManager.gamePads[0]!.keyMap, forKey: Keys.joyKeyMap1)
        defaults.decode(&gamePadManager.gamePads[1]!.keyMap, forKey: Keys.joyKeyMap2)
        keyboardcontroller.disconnectJoyKeys = defaults.bool(forKey: Keys.disconnectJoyKeys)
        amiga.controlPort1.setAutofire(defaults.bool(forKey: Keys.autofire))
        amiga.controlPort2.setAutofire(defaults.bool(forKey: Keys.autofire))
        amiga.controlPort1.setAutofireBullets(defaults.integer(forKey: Keys.autofireBullets))
        amiga.controlPort2.setAutofireBullets(defaults.integer(forKey: Keys.autofireBullets))
        amiga.controlPort1.setAutofireFrequency(defaults.float(forKey: Keys.autofireFrequency))
        amiga.controlPort2.setAutofireFrequency(defaults.float(forKey: Keys.autofireFrequency))
 
        // Mouse
        metal.retainMouseKeyComb    = defaults.integer(forKey: Keys.retainMouseKeyComb)
        metal.retainMouseWithKeys   = defaults.bool(forKey: Keys.retainMouseWithKeys)
        metal.retainMouseByClick    = defaults.bool(forKey: Keys.retainMouseByClick)
        metal.retainMouseByEntering = defaults.bool(forKey: Keys.retainMouseByEntering)
        metal.releaseMouseKeyComb    = defaults.integer(forKey: Keys.releaseMouseKeyComb)
        metal.releaseMouseWithKeys  = defaults.bool(forKey: Keys.releaseMouseWithKeys)
        metal.releaseMouseByShaking = defaults.bool(forKey: Keys.releaseMouseByShaking)
        
        amiga.resume()
    }
    
    func saveDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        // Joysticks
        defaults.encode(gamePadManager.gamePads[0]!.keyMap, forKey: Keys.joyKeyMap1)
        defaults.encode(gamePadManager.gamePads[1]!.keyMap, forKey: Keys.joyKeyMap2)
        defaults.set(keyboardcontroller.disconnectJoyKeys, forKey: Keys.disconnectJoyKeys)
        defaults.set(amiga.controlPort1.autofire(), forKey: Keys.autofire)
        defaults.set(amiga.controlPort1.autofireBullets(), forKey: Keys.autofireBullets)
        defaults.set(amiga.controlPort1.autofireFrequency(), forKey: Keys.autofireFrequency)
        
        // Mouse
        defaults.set(metal.retainMouseKeyComb, forKey: Keys.retainMouseKeyComb)
        defaults.set(metal.retainMouseWithKeys, forKey: Keys.retainMouseWithKeys)
        defaults.set(metal.retainMouseByClick, forKey: Keys.retainMouseByClick)
        defaults.set(metal.retainMouseByEntering, forKey: Keys.retainMouseByEntering)
        defaults.set(metal.releaseMouseKeyComb, forKey: Keys.releaseMouseKeyComb)
        defaults.set(metal.releaseMouseWithKeys, forKey: Keys.releaseMouseWithKeys)
        defaults.set(metal.releaseMouseByShaking, forKey: Keys.releaseMouseByShaking)
    }
}

//
// User defaults (Video)
//

extension Keys {
    
    static let palette         = "VAMIGAPaletteKey"
    static let brightness      = "VAMIGABrightnessKey"
    static let contrast        = "VAMIGAContrastKey"
    static let saturation      = "VAMIGASaturationKey"
    static let enhancer        = "VVAMIGAEnhancerKey"
    static let upscaler        = "VAMIGAUpscalerKey"

    // Geometry
    static let eyeX            = "VAMIGAEyeX"
    static let eyeY            = "VAMIGAEyeY"
    static let eyeZ            = "VAMIGAEyeZ"
    
    // GPU options
    static let shaderOptions   = "VAMIGAShaderOptionsKey"
}

extension Defaults {
    
    static let palette = COLOR_PALETTE
    static let brightness = Double(50.0)
    static let contrast = Double(100.0)
    static let saturation = Double(50.0)
    static let enhancer = 0
    static let upscaler = 0
    
    // Geometry
    static let eyeX = Float(0.0)
    static let eyeY = Float(0.0)
    static let eyeZ = Float(0.0)
    
    // GPU options
    static let shaderOptions = ShaderDefaultsTFT
}

extension MyController {
    
    static func registerVideoUserDefaults() {
        
        let dictionary : [String:Any] = [
            
            Keys.palette: Int(Defaults.palette.rawValue),
            Keys.brightness: Defaults.brightness,
            Keys.contrast: Defaults.contrast,
            Keys.saturation: Defaults.saturation,
            Keys.enhancer: Defaults.enhancer,
            Keys.upscaler: Defaults.upscaler,

            Keys.eyeX: Defaults.eyeX,
            Keys.eyeY: Defaults.eyeY,
            Keys.eyeZ: Defaults.eyeZ,
            ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
        defaults.register(encodableItem: Defaults.shaderOptions, forKey: Keys.shaderOptions)
    }
    
    func resetVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [ Keys.palette,
                     Keys.brightness,
                     Keys.contrast,
                     Keys.saturation,
                     Keys.enhancer,
                     Keys.upscaler,

                     Keys.eyeX,
                     Keys.eyeY,
                     Keys.eyeZ,
                     
                     Keys.shaderOptions
            ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadVideoUserDefaults()
    }
    
    func loadVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        metal.enhancer = defaults.integer(forKey: Keys.enhancer)
        metal.upscaler = defaults.integer(forKey: Keys.upscaler)
        palette = defaults.integer(forKey: Keys.palette)
        brightness = defaults.double(forKey: Keys.brightness)
        contrast = defaults.double(forKey: Keys.contrast)
        saturation = defaults.double(forKey: Keys.saturation)
        eyeX = defaults.float(forKey: Keys.eyeX)
        eyeY = defaults.float(forKey: Keys.eyeY)
        eyeZ = defaults.float(forKey: Keys.eyeZ)
        
        defaults.decode(&metal.shaderOptions, forKey: Keys.shaderOptions)
        metal.buildDotMasks()
 
        amiga.resume()
    }
    
    func saveVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(metal.enhancer, forKey: Keys.enhancer)
        defaults.set(metal.upscaler, forKey: Keys.upscaler)
        defaults.set(palette, forKey: Keys.palette)
        defaults.set(brightness, forKey: Keys.brightness)
        defaults.set(contrast, forKey: Keys.contrast)
        defaults.set(saturation, forKey: Keys.saturation)
        
        defaults.set(eyeX, forKey: Keys.eyeX)
        defaults.set(eyeY, forKey: Keys.eyeY)
        defaults.set(eyeZ, forKey: Keys.eyeZ)
  
        defaults.encode(metal.shaderOptions, forKey: Keys.shaderOptions)
    }
}

//
// User defaults (Emulator)
//

extension Keys {
    
    // Drives
    static let warpLoad             = "VAMIGAWarpLoadKey"
    static let driveNoise           = "VAMIGADriveNoiseKey"
    static let driveNoiseNoPoll     = "VAMIGADriveNoiseNoPollKey"
    static let driveBlankDiskFormat = "VAMIGADriveBlankDiskFormatKey"

    // Screenshots
    static let screenshotSource     = "VAMIGAScreenshotSourceKey"
    static let screenshotTarget     = "VAMIGAScreenshotTargetKey"
    
    // Fullscreen
    static let keepAspectRatio      = "VAMIGAFullscreenKeepAspectRatioKey"
    static let exitOnEsc            = "VAMIGAFullscreenExitOnEscKey"

    // User dialogs
    static let closeWithoutAsking   = "VAMIGACloseWithoutAsking"
    static let ejectWithoutAsking   = "VAMIGAEjectWithoutAsking"
    
    // Miscellaneous
    static let pauseInBackground    = "VAMIGAPauseInBackground"
    static let autoSnapshots        = "VAMIGAAutoSnapshots"
    static let autoSnapshotInterval = "VAMIGASnapshotInterval"
    
    // Media files
    static let autoMountAction      = "VAMIGAAutoMountAction"
    static let autoType             = "VAMIGAAutoType"
    static let autoTypeText         = "VAMIGAAutoTypeText"
}

extension Defaults {
   
    // Drives
    static let warpLoad             = true
    static let driveNoise           = true
    static let driveNoiseNoPoll     = true
    static let driveBlankDiskFormat = FS_OFS
    
    // Screenshots
    static let screenshotSource     = 0
    static let screenshotTarget     = NSBitmapImageRep.FileType.png
    
    // Fullscreen
    static let keepAspectRatio      = false
    static let exitOnEsc            = false

    // User dialogs
    static let closeWithoutAsking   = false
    static let ejectWithoutAsking   = false
    
    // Miscellaneous
    static let pauseInBackground    = false
    static let autoSnapshots        = true
    static let autoSnapshotInterval = 3
    
    // Media files
    static let autoMountAction      = [ "D64": AutoMountAction.openBrowser,
                                        "PRG": AutoMountAction.openBrowser,
                                        "T64": AutoMountAction.openBrowser,
                                        "TAP": AutoMountAction.openBrowser,
                                        "CRT": AutoMountAction.openBrowser,]
    static let autoType             = [ "D64": true,
                                        "PRG": true,
                                        "T64": true,
                                        "TAP": true,
                                        "CRT": false]
    static let autoTypeText         = [ "D64": "LOAD \"*\",8,1:",
                                        "PRG": "RUN",
                                        "T64": "RUN",
                                        "TAP": "LOAD",
                                        "CRT": ""]
}

extension MyController {
    
    static func registerEmulatorUserDefaults() {
        
        let dictionary : [String:Any] = [
            
            Keys.warpLoad: Defaults.warpLoad,
            Keys.driveNoise: Defaults.driveNoise,
            Keys.driveNoiseNoPoll: Defaults.driveNoiseNoPoll,
            Keys.driveBlankDiskFormat: Int(Defaults.driveBlankDiskFormat.rawValue),

            Keys.screenshotSource: Defaults.screenshotSource,
            Keys.screenshotTarget: Int(Defaults.screenshotTarget.rawValue),

            Keys.keepAspectRatio: Defaults.keepAspectRatio,
            Keys.exitOnEsc: Defaults.exitOnEsc,
            
            Keys.closeWithoutAsking: Defaults.closeWithoutAsking,
            Keys.ejectWithoutAsking: Defaults.ejectWithoutAsking,

            Keys.pauseInBackground: Defaults.pauseInBackground,
            Keys.autoSnapshots: Defaults.autoSnapshots,
            Keys.autoSnapshotInterval: Defaults.autoSnapshotInterval
        ]
        
        let defaults = UserDefaults.standard
        
        defaults.register(defaults: dictionary)
        
        defaults.register(encodableItem: Defaults.autoMountAction, forKey: Keys.autoMountAction)
        defaults.register(encodableItem: Defaults.autoType, forKey: Keys.autoType)
        defaults.register(encodableItem: Defaults.autoTypeText, forKey: Keys.autoTypeText)
    }

    func resetEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [Keys.warpLoad,
                    Keys.driveNoise,
                    Keys.driveNoiseNoPoll,
                    Keys.driveBlankDiskFormat,
                    
                    Keys.screenshotSource,
                    Keys.screenshotTarget,
                    
                    Keys.keepAspectRatio,
                    Keys.exitOnEsc,
                    
                    Keys.closeWithoutAsking,
                    Keys.ejectWithoutAsking,
                    
                    Keys.pauseInBackground,
                    Keys.autoSnapshots,
                    Keys.autoSnapshotInterval,
                    
                    Keys.autoMountAction,
                    Keys.autoType,
                    Keys.autoTypeText
                    ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadEmulatorUserDefaults()
    }
    
    func loadEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        warpLoad = defaults.bool(forKey: Keys.warpLoad)
        driveNoise = defaults.bool(forKey: Keys.driveNoise)
        driveNoiseNoPoll = defaults.bool(forKey: Keys.driveNoiseNoPoll)
        driveBlankDiskFormatIntValue = defaults.integer(forKey: Keys.driveBlankDiskFormat)
        
        screenshotSource = defaults.integer(forKey: Keys.screenshotSource)
        screenshotTargetIntValue = defaults.integer(forKey: Keys.screenshotTarget)
    
        metal.keepAspectRatio = defaults.bool(forKey: Keys.keepAspectRatio)
        keyboardcontroller.exitOnEsc = defaults.bool(forKey: Keys.exitOnEsc)
        
        closeWithoutAsking = defaults.bool(forKey: Keys.closeWithoutAsking)
        ejectWithoutAsking = defaults.bool(forKey: Keys.ejectWithoutAsking)

        pauseInBackground = defaults.bool(forKey: Keys.pauseInBackground)
        amiga.setTakeAutoSnapshots(defaults.bool(forKey: Keys.autoSnapshots))
        amiga.setSnapshotInterval(defaults.integer(forKey: Keys.autoSnapshotInterval))
        
        amiga.resume()
    }
    
    func saveEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(warpLoad, forKey: Keys.warpLoad)
        defaults.set(driveNoise, forKey: Keys.driveNoise)
        defaults.set(driveNoiseNoPoll, forKey: Keys.driveNoiseNoPoll)
        defaults.set(driveBlankDiskFormatIntValue, forKey: Keys.driveBlankDiskFormat)
        
        defaults.set(screenshotSource, forKey: Keys.screenshotSource)
        defaults.set(screenshotTargetIntValue, forKey: Keys.screenshotTarget)
        
        defaults.set(metal.keepAspectRatio, forKey: Keys.keepAspectRatio)
        defaults.set(keyboardcontroller.exitOnEsc, forKey: Keys.exitOnEsc)
        
        defaults.set(closeWithoutAsking, forKey: Keys.closeWithoutAsking)
        defaults.set(ejectWithoutAsking, forKey: Keys.ejectWithoutAsking)
        
        defaults.set(pauseInBackground, forKey: Keys.pauseInBackground)
        defaults.set(amiga.takeAutoSnapshots(), forKey: Keys.autoSnapshots)
        defaults.set(amiga.snapshotInterval(), forKey: Keys.autoSnapshotInterval)
    }
}

//
// User defaults (Hardware)
//

extension Keys {
    
    // Machine
    static let amigaModel         = "VAMIGAAmigaModelKey"
    static let layout             = "VAMIGAKeyboardLayoutKey"
    
    // Memory
    static let chipRam            = "VAMIGAChipRamKey"
    static let slowRam            = "VAMIGASlowRamKey"
    static let fastRam            = "VAMIGAFastRamKey"

    // Drives
    static let df0Connect         = "VAMIGADF0ConnectKey"
    static let df0Type            = "VAMIGADF0TypeKey"
    static let df0Speed           = "VAMIGADF0SpeedKey"
    static let df1Connect         = "VAMIGADF1ConnectKey"
    static let df1Type            = "VAMIGADF1TypeKey"
    static let df1Speed           = "VAMIGADF1SpeedKey"
    static let df2Connect         = "VAMIGADF2ConnectKey"
    static let df2Type            = "VAMIGADF2TypeKey"
    static let df2Speed           = "VAMIGADF2SpeedKey"
    static let df3Connect         = "VAMIGADF3ConnectKey"
    static let df3Type            = "VAMIGADF3TypeKey"
    static let df3Speed           = "VAMIGADF3SpeedKey"

    // Extensions
    static let realTimeClock      = "VAMIGARealTimeClockKey"
    
    // Compatibility
    static let exactBlitter       = "VAMIGAAccurateBlitterKey"
    static let fifoBuffering      = "VAMIGAFifoBufferingKey"
}

extension Defaults {
    
    struct a500 {
        
        static let amigaModel        = A500
        static let layout            = Layout.us
        
        static let chipRam           = 512
        static let slowRam           = 0
        static let fastRam           = 0

        static let df0Connect        = true
        static let df0Type           = DRIVE_35_DD
        static let df0Speed          = 1
        static let df1Connect        = false
        static let df1Type           = DRIVE_35_DD
        static let df1Speed          = 1
        static let df2Connect        = false
        static let df2Type           = DRIVE_35_DD
        static let df2Speed          = 1
        static let df3Connect        = false
        static let df3Type           = DRIVE_35_DD
        static let df3Speed          = 1

        static let realTimeClock     = false
        
        static let exactBlitter      = false
        static let fifoBuffering     = true
    }
    
    struct a1000 {
        
        static let amigaModel        = A1000
        static let layout            = Layout.us
        
        static let chipRam           = 256
        static let slowRam           = 0
        static let fastRam           = 0
        
        static let df0Connect        = true
        static let df0Type           = DRIVE_35_DD
        static let df0Speed          = 1
        static let df1Connect        = false
        static let df1Type           = DRIVE_35_DD
        static let df1Speed          = 1
        static let df2Connect        = false
        static let df2Type           = DRIVE_35_DD
        static let df2Speed          = 1
        static let df3Connect        = false
        static let df3Type           = DRIVE_35_DD
        static let df3Speed          = 1

        static let realTimeClock     = false
        
        static let exactBlitter      = false
        static let fifoBuffering     = true
    }
    
    struct a2000 {
        
        static let amigaModel        = A2000
        static let layout            = Layout.us
        
        static let chipRam           = 512
        static let slowRam           = 512
        static let fastRam           = 0
        
        static let df0Connect        = true
        static let df0Type           = DRIVE_35_DD
        static let df0Speed          = 1
        static let df1Connect        = true
        static let df1Type           = DRIVE_35_DD
        static let df1Speed          = 1
        static let df2Connect        = false
        static let df2Type           = DRIVE_35_DD
        static let df2Speed          = 1
        static let df3Connect        = false
        static let df3Type           = DRIVE_35_DD
        static let df3Speed          = 1

        static let realTimeClock     = true
        
        static let exactBlitter      = false
        static let fifoBuffering     = true
    }
}

extension MyController {
    
    static func registerHardwareUserDefaults() {
        
        let defaultModel = Defaults.a500.self
        
        let dictionary : [String:Any] = [
            
            Keys.amigaModel: defaultModel.amigaModel.rawValue,
            Keys.layout: defaultModel.layout.rawValue,
            
            Keys.chipRam: defaultModel.chipRam,
            Keys.slowRam: defaultModel.slowRam,
            Keys.fastRam: defaultModel.fastRam,
            
            Keys.df0Connect: defaultModel.df0Connect,
            Keys.df0Type: defaultModel.df0Type.rawValue,
            Keys.df0Speed: defaultModel.df0Speed,
            Keys.df1Connect: defaultModel.df1Connect,
            Keys.df1Type: defaultModel.df1Type.rawValue,
            Keys.df1Speed: defaultModel.df1Speed,
            Keys.df2Connect: defaultModel.df2Connect,
            Keys.df2Type: defaultModel.df2Type.rawValue,
            Keys.df2Speed: defaultModel.df2Speed,
            Keys.df3Connect: defaultModel.df3Connect,
            Keys.df3Type: defaultModel.df3Type.rawValue,
            Keys.df3Speed: defaultModel.df3Speed,

            Keys.realTimeClock: defaultModel.realTimeClock,
            
            Keys.fifoBuffering: defaultModel.fifoBuffering,
            Keys.exactBlitter: defaultModel.exactBlitter,
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }
    
    func resetHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [Keys.amigaModel,
                    Keys.layout,
                    
                    Keys.chipRam,
                    Keys.slowRam,
                    Keys.fastRam,

                    Keys.df0Connect,
                    Keys.df0Type,
                    Keys.df0Speed,
                    Keys.df1Connect,
                    Keys.df1Type,
                    Keys.df1Speed,
                    Keys.df2Connect,
                    Keys.df2Type,
                    Keys.df2Speed,
                    Keys.df3Connect,
                    Keys.df3Type,
                    Keys.df3Speed,

                    Keys.realTimeClock,
                    
                    Keys.exactBlitter,
                    Keys.fifoBuffering
            ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadHardwareUserDefaults()
    }
    
    func loadHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        amiga.configureModel(defaults.integer(forKey: Keys.amigaModel))
        amiga.configureLayout(defaults.integer(forKey: Keys.layout))
    
        amiga.configureChipMemory(defaults.integer(forKey: Keys.chipRam))
        amiga.configureSlowMemory(defaults.integer(forKey: Keys.slowRam))
        amiga.configureFastMemory(defaults.integer(forKey: Keys.fastRam))

        amiga.configureDrive(0, connected: defaults.bool(forKey: Keys.df0Connect))
        amiga.configureDrive(0, type: defaults.integer(forKey: Keys.df0Type))
        amiga.configureDrive(0, speed: defaults.integer(forKey: Keys.df0Speed))
        amiga.configureDrive(1, connected: defaults.bool(forKey: Keys.df1Connect))
        amiga.configureDrive(1, type: defaults.integer(forKey: Keys.df1Type))
        amiga.configureDrive(1, speed: defaults.integer(forKey: Keys.df1Speed))
        amiga.configureDrive(2, connected: defaults.bool(forKey: Keys.df2Connect))
        amiga.configureDrive(2, type: defaults.integer(forKey: Keys.df2Type))
        amiga.configureDrive(2, speed: defaults.integer(forKey: Keys.df2Speed))
        amiga.configureDrive(3, connected: defaults.bool(forKey: Keys.df3Connect))
        amiga.configureDrive(3, type: defaults.integer(forKey: Keys.df3Type))
        amiga.configureDrive(3, speed: defaults.integer(forKey: Keys.df3Speed))

        amiga.configureRealTimeClock(defaults.bool(forKey: Keys.realTimeClock))
        
        amiga.configureExactBlitter(defaults.bool(forKey: Keys.exactBlitter))
        amiga.configureFifoBuffering(defaults.bool(forKey: Keys.fifoBuffering))

        amiga.resume()
    }

    func saveHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        let config = amiga.config()
        let memConfig = amiga.memConfig()

        defaults.set(config.model.rawValue, forKey: Keys.amigaModel)
        defaults.set(config.layout, forKey: Keys.layout)

        defaults.set(memConfig.chipRamSize, forKey: Keys.chipRam)
        defaults.set(memConfig.slowRamSize, forKey: Keys.slowRam)
        defaults.set(memConfig.fastRamSize, forKey: Keys.fastRam)

        defaults.set(config.df0.connected, forKey: Keys.df0Connect)
        defaults.set(config.df0.type.rawValue, forKey: Keys.df0Type)
        defaults.set(config.df0.speed, forKey: Keys.df0Speed)
        defaults.set(config.df1.connected, forKey: Keys.df1Connect)
        defaults.set(config.df1.type.rawValue, forKey: Keys.df1Type)
        defaults.set(config.df1.speed, forKey: Keys.df1Speed)
        defaults.set(config.df2.connected, forKey: Keys.df2Connect)
        defaults.set(config.df2.type.rawValue, forKey: Keys.df2Type)
        defaults.set(config.df2.speed, forKey: Keys.df2Speed)
        defaults.set(config.df3.connected, forKey: Keys.df3Connect)
        defaults.set(config.df3.type.rawValue, forKey: Keys.df3Type)
        defaults.set(config.df3.speed, forKey: Keys.df3Speed)

        defaults.set(config.realTimeClock, forKey: Keys.realTimeClock)
        
        defaults.set(config.exactBlitter, forKey: Keys.exactBlitter)
        defaults.set(config.fifoBuffering, forKey: Keys.fifoBuffering)
    }
}
