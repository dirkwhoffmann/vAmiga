//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

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
        
        c64.suspend()
        
        resetGeneralUserDefaults()
        resetRomUserDefaults()
        resetDevicesUserDefaults()
        resetVideoUserDefaults()
        resetEmulatorUserDefaults()
        resetHardwareUserDefaults()
        
        c64.resume()
    }
    
    func loadUserDefaults() {
        
        track()
        
        c64.suspend()
        
        loadGeneralUserDefaults()
        loadRomUserDefaults()
        loadDevicesUserDefaults()
        loadVideoUserDefaults()
        loadEmulatorUserDefaults()
        loadHardwareUserDefaults()
        
        c64.resume()
    }
    
    func loadUserDefaults(url: URL) {
        
        if let fileContents = NSDictionary(contentsOf: url) {
            
            if let dict = fileContents as? Dictionary<String,Any> {
                
                let filteredDict = dict.filter { $0.0.hasPrefix("VC64") }
                
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
        let filteredDict = dict.filter { $0.0.hasPrefix("VC64") }
        let nsDict = NSDictionary.init(dictionary: filteredDict)
        nsDict.write(to: url, atomically: true)
    }
    
}

//
// User defaults (general)
//

struct VC64Keys {
    
    // Control ports
    static let inputDevice1      = "VC64InputDevice1"
    static let inputDevice2      = "VC64InputDevice2"
}

struct Defaults {
    
    // Control ports
    static let inputDevice1 = -1
    static let inputDevice2 = -1
}

extension MyController {

    static func registerGeneralUserDefaults() {
        
        let dictionary : [String:Any] = [
            
            VC64Keys.inputDevice1: Defaults.inputDevice1,
            VC64Keys.inputDevice2: Defaults.inputDevice2,
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }
    
    func resetGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [ VC64Keys.inputDevice1,
                     VC64Keys.inputDevice2 ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadGeneralUserDefaults()
    }
    
    func loadGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        c64.suspend()
        
        inputDevice1 = defaults.integer(forKey: VC64Keys.inputDevice1)
        inputDevice2 = defaults.integer(forKey: VC64Keys.inputDevice2)
        
        c64.resume()
    }
    
    func saveGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(inputDevice1, forKey: VC64Keys.inputDevice1)
        defaults.set(inputDevice2, forKey: VC64Keys.inputDevice2)
    }
}

//
// User defaults (Roms)
//

extension VC64Keys {
    
    static let bootRom           = "VABootRomFileKey"
    static let kickRom           = "VAKickRomFileKey"
    
    // DEPRECATED
    static let basicRom          = "VC64BasicRomFileKey"
    static let charRom           = "VC64CharRomFileKey"
    static let kernalRom         = "VC64KernelRomFileKey"
    static let vc1541Rom         = "VC64VC1541RomFileKey"
    
}

extension Defaults {
    
    static let bootRom = URL(fileURLWithPath: "")
    static let kickRom = URL(fileURLWithPath: "")
}

extension MyController {
    
    static func registerRomUserDefaults() {
        
        let dictionary : [String:Any] = [
            
            VC64Keys.bootRom: Defaults.bootRom,
            VC64Keys.kickRom: Defaults.kickRom,
            ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }

    func resetRomUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [ VC64Keys.bootRom,
                     VC64Keys.kickRom]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadRomUserDefaults()
    }
    
    func loadRomUserDefaults() {
        
        let defaults = UserDefaults.standard

        c64.suspend()
        loadRom(defaults.url(forKey: VC64Keys.basicRom))
        loadRom(defaults.url(forKey: VC64Keys.charRom))
        loadRom(defaults.url(forKey: VC64Keys.kernalRom))
        loadRom(defaults.url(forKey: VC64Keys.vc1541Rom))
        c64.resume()
        
        amiga.suspend()
        amiga.loadBootRom(fromFile: defaults.url(forKey: VC64Keys.bootRom))
        amiga.loadKickRom(fromFile: defaults.url(forKey: VC64Keys.kickRom))
        amiga.resume()
    }
    
    func saveRomUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(bootRomURL, forKey: VC64Keys.bootRom)
        defaults.set(kickRomURL, forKey: VC64Keys.kickRom)
    }
}


//
// User defaults (Devices)
//

extension VC64Keys {
    
    // Mouse
    static let mouseModel        = "VC64MouseModelKey"

    // Joysticks
    static let disconnectJoyKeys = "VC64DisconnectKeys"
    static let autofire          = "VC64Autofire"
    static let autofireBullets   = "VC64AutofireBullets"
    static let autofireFrequency = "VC64AutofireFrequency"
    static let joyKeyMap1        = "VC64JoyKeyMap1"
    static let joyKeyMap2        = "VC64JoyKeyMap2"
}

extension Defaults {
    
    // Mouse
    static let mouseModel        = MOUSE1350
    
    // Joysticks
    static let disconnectJoyKeys = true
    static let autofire          = false
    static let autofireBullets   = -3
    static let autofireFrequency = Float(2.5)
    
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
}
    
extension MyController {
    
    static func registerDevicesUserDefaults() {
        
        let dictionary : [String:Any] = [
            VC64Keys.mouseModel: Int(Defaults.mouseModel.rawValue),
            VC64Keys.disconnectJoyKeys: Defaults.disconnectJoyKeys,
            VC64Keys.autofire: Defaults.autofire,
            VC64Keys.autofireBullets: Defaults.autofireBullets,
            VC64Keys.autofireFrequency: Defaults.autofireFrequency
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
        defaults.register(encodableItem: Defaults.joyKeyMap1, forKey: VC64Keys.joyKeyMap1)
        defaults.register(encodableItem: Defaults.joyKeyMap2, forKey: VC64Keys.joyKeyMap2)
    }

    func resetDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [ VC64Keys.mouseModel,
                     VC64Keys.disconnectJoyKeys,
                     VC64Keys.autofire,
                     VC64Keys.autofireBullets,
                     VC64Keys.autofireFrequency,
                     
                     VC64Keys.joyKeyMap1,
                     VC64Keys.joyKeyMap2
                     ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadDevicesUserDefaults()
    }
    
    func loadDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
    
        c64.suspend()
        
        c64.mouse.setModel(defaults.integer(forKey: VC64Keys.mouseModel))
        keyboardcontroller.disconnectJoyKeys = defaults.bool(forKey: VC64Keys.disconnectJoyKeys)
        c64.port1.setAutofire(defaults.bool(forKey: VC64Keys.autofire))
        c64.port2.setAutofire(defaults.bool(forKey: VC64Keys.autofire))
        c64.port1.setAutofireBullets(defaults.integer(forKey: VC64Keys.autofireBullets))
        c64.port2.setAutofireBullets(defaults.integer(forKey: VC64Keys.autofireBullets))
        c64.port1.setAutofireFrequency(defaults.float(forKey: VC64Keys.autofireFrequency))
        c64.port2.setAutofireFrequency(defaults.float(forKey: VC64Keys.autofireFrequency))
        defaults.decode(&gamePadManager.gamePads[0]!.keyMap, forKey: VC64Keys.joyKeyMap1)
        defaults.decode(&gamePadManager.gamePads[1]!.keyMap, forKey: VC64Keys.joyKeyMap2)
 
        c64.resume()
    }
    
    func saveDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(c64.mouse.model(), forKey: VC64Keys.mouseModel)
        defaults.set(keyboardcontroller.disconnectJoyKeys, forKey: VC64Keys.disconnectJoyKeys)
        defaults.set(c64.port1.autofire(), forKey: VC64Keys.autofire)
        defaults.set(c64.port1.autofireBullets(), forKey: VC64Keys.autofireBullets)
        defaults.set(c64.port1.autofireFrequency(), forKey: VC64Keys.autofireFrequency)
        defaults.encode(gamePadManager.gamePads[0]!.keyMap, forKey: VC64Keys.joyKeyMap1)
        defaults.encode(gamePadManager.gamePads[1]!.keyMap, forKey: VC64Keys.joyKeyMap2)
    }
}

//
// User defaults (Video)
//

extension VC64Keys {
    
    static let palette         = "VC64PaletteKey"
    static let brightness      = "VC64BrightnessKey"
    static let contrast        = "VC64ContrastKey"
    static let saturation      = "VC64SaturationKey"
    static let upscaler        = "VC64UpscalerKey"

    // Geometry
    static let keepAspectRatio = "VC64FullscreenKeepAspectRatioKey"
    static let eyeX            = "VC64EyeX"
    static let eyeY            = "VC64EyeY"
    static let eyeZ            = "VC64EyeZ"
    
    // GPU options
    static let shaderOptions   = "VC64ShaderOptionsKey"
}

extension Defaults {
    
    static let palette = COLOR_PALETTE
    static let brightness = Double(50.0)
    static let contrast = Double(100.0)
    static let saturation = Double(50.0)
    static let upscaler = 0
    
    // Geometry
    static let keepAspectRatio = false
    static let eyeX = Float(0.0)
    static let eyeY = Float(0.0)
    static let eyeZ = Float(0.0)
    
    // GPU options
    static let shaderOptions = ShaderDefaultsTFT
}

extension MyController {
    
    static func registerVideoUserDefaults() {
        
        let dictionary : [String:Any] = [
            
            VC64Keys.palette: Int(Defaults.palette.rawValue),
            VC64Keys.brightness: Defaults.brightness,
            VC64Keys.contrast: Defaults.contrast,
            VC64Keys.saturation: Defaults.saturation,
            VC64Keys.upscaler: Defaults.upscaler,

            VC64Keys.keepAspectRatio: Defaults.keepAspectRatio,
            VC64Keys.eyeX: Defaults.eyeX,
            VC64Keys.eyeY: Defaults.eyeY,
            VC64Keys.eyeZ: Defaults.eyeZ,
            ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
        defaults.register(encodableItem: Defaults.shaderOptions, forKey: VC64Keys.shaderOptions)
    }
    
    func resetVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [ VC64Keys.palette,
                     VC64Keys.brightness,
                     VC64Keys.contrast,
                     VC64Keys.saturation,
                     VC64Keys.upscaler,
                     
                     VC64Keys.keepAspectRatio,
                     VC64Keys.eyeX,
                     VC64Keys.eyeY,
                     VC64Keys.eyeZ,
                     
                     VC64Keys.shaderOptions
            ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadVideoUserDefaults()
    }
    
    func loadVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        c64.suspend()
        
        metalScreen.upscaler = defaults.integer(forKey: VC64Keys.upscaler)
        c64.vic.setVideoPalette(defaults.integer(forKey: VC64Keys.palette))
        c64.vic.setBrightness(defaults.double(forKey: VC64Keys.brightness))
        c64.vic.setContrast(defaults.double(forKey: VC64Keys.contrast))
        c64.vic.setSaturation(defaults.double(forKey: VC64Keys.saturation))

        metalScreen.keepAspectRatio = defaults.bool(forKey: VC64Keys.keepAspectRatio)
        metalScreen.setEyeX(defaults.float(forKey: VC64Keys.eyeX))
        metalScreen.setEyeY(defaults.float(forKey: VC64Keys.eyeY))
        metalScreen.setEyeZ(defaults.float(forKey: VC64Keys.eyeZ))
        
        defaults.decode(&metalScreen.shaderOptions, forKey: VC64Keys.shaderOptions)
        metalScreen.buildDotMasks()
 
        c64.resume()
    }
    
    func saveVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(metalScreen.upscaler, forKey: VC64Keys.upscaler)
        defaults.set(c64.vic.videoPalette(), forKey: VC64Keys.palette)
        defaults.set(c64.vic.brightness(), forKey: VC64Keys.brightness)
        defaults.set(c64.vic.contrast(), forKey: VC64Keys.contrast)
        defaults.set(c64.vic.saturation(), forKey: VC64Keys.saturation)
        
        defaults.set(metalScreen.keepAspectRatio, forKey: VC64Keys.keepAspectRatio)
        defaults.set(metalScreen.eyeX(), forKey: VC64Keys.eyeX)
        defaults.set(metalScreen.eyeY(), forKey: VC64Keys.eyeY)
        defaults.set(metalScreen.eyeZ(), forKey: VC64Keys.eyeZ)
  
        defaults.encode(metalScreen.shaderOptions, forKey: VC64Keys.shaderOptions)
    }
}

//
// User defaults (Emulator)
//

extension VC64Keys {
    
    // Drives
    static let warpLoad             = "VC64WarpLoadKey"
    static let driveNoise           = "VC64DriveNoiseKey"
    
    // Screenshots
    static let screenshotSource     = "VC64ScreenshotSourceKey"
    static let screenshotTarget     = "VC64ScreenshotTargetKey"
    
    // User dialogs
    static let closeWithoutAsking   = "VC64CloseWithoutAsking"
    static let ejectWithoutAsking   = "VC64EjectWithoutAsking"
    
    // Miscellaneous
    static let pauseInBackground    = "VC64PauseInBackground"
    static let autoSnapshots        = "VC64AutoSnapshots"
    static let autoSnapshotInterval = "VC64SnapshotInterval"
    
    // Media files
    static let autoMountAction      = "VC64AutoMountAction"
    static let autoType             = "VC64AutoType"
    static let autoTypeText         = "VC64AutoTypeText"
}

extension Defaults {
   
    // Drives
    static let warpLoad             = true
    static let driveNoise           = true
    
    // Screenshots
    static let screenshotSource     = 0
    static let screenshotTarget     = NSBitmapImageRep.FileType.png
    
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
            
            VC64Keys.warpLoad: Defaults.warpLoad,
            VC64Keys.driveNoise: Defaults.driveNoise,
            
            VC64Keys.screenshotSource: Defaults.screenshotSource,
            VC64Keys.screenshotTarget: Int(Defaults.screenshotTarget.rawValue),

            VC64Keys.closeWithoutAsking: Defaults.closeWithoutAsking,
            VC64Keys.ejectWithoutAsking: Defaults.ejectWithoutAsking,

            VC64Keys.pauseInBackground: Defaults.pauseInBackground,
            VC64Keys.autoSnapshots: Defaults.autoSnapshots,
            VC64Keys.autoSnapshotInterval: Defaults.autoSnapshotInterval
        ]
        
        let defaults = UserDefaults.standard
        
        defaults.register(defaults: dictionary)
        
        defaults.register(encodableItem: Defaults.autoMountAction, forKey: VC64Keys.autoMountAction)
        defaults.register(encodableItem: Defaults.autoType, forKey: VC64Keys.autoType)
        defaults.register(encodableItem: Defaults.autoTypeText, forKey: VC64Keys.autoTypeText)
    }

    func resetEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [VC64Keys.warpLoad,
                    VC64Keys.driveNoise,
                    
                    VC64Keys.screenshotSource,
                    VC64Keys.screenshotTarget,
                    
                    VC64Keys.closeWithoutAsking,
                    VC64Keys.ejectWithoutAsking,
                    
                    VC64Keys.pauseInBackground,
                    VC64Keys.autoSnapshots,
                    VC64Keys.autoSnapshotInterval,
                    
                    VC64Keys.autoMountAction,
                    VC64Keys.autoType,
                    VC64Keys.autoTypeText
                    ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadEmulatorUserDefaults()
    }
    
    func loadEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard
            
        c64.suspend()
        
        c64.setWarpLoad(defaults.bool(forKey: VC64Keys.warpLoad))
        c64.drive1.setSendSoundMessages(defaults.bool(forKey: VC64Keys.driveNoise))
        c64.drive2.setSendSoundMessages(defaults.bool(forKey: VC64Keys.driveNoise))
    
        screenshotSource = defaults.integer(forKey: VC64Keys.screenshotSource)
        screenshotTargetIntValue = defaults.integer(forKey: VC64Keys.screenshotTarget)
    
        closeWithoutAsking = defaults.bool(forKey: VC64Keys.closeWithoutAsking)
        ejectWithoutAsking = defaults.bool(forKey: VC64Keys.ejectWithoutAsking)

        pauseInBackground = defaults.bool(forKey: VC64Keys.pauseInBackground)
        c64.setTakeAutoSnapshots(defaults.bool(forKey: VC64Keys.autoSnapshots))
        c64.setSnapshotInterval(defaults.integer(forKey: VC64Keys.autoSnapshotInterval))
        
        defaults.decode(&autoMountAction, forKey: VC64Keys.autoMountAction)
        defaults.decode(&autoType, forKey: VC64Keys.autoType)
        defaults.decode(&autoTypeText, forKey: VC64Keys.autoTypeText)
        
        c64.resume()
    }
    
    func saveEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(c64.warpLoad(), forKey: VC64Keys.warpLoad)
        defaults.set(c64.drive1.sendSoundMessages(), forKey: VC64Keys.driveNoise)
        defaults.set(c64.drive2.sendSoundMessages(), forKey: VC64Keys.driveNoise)

        defaults.set(screenshotSource, forKey: VC64Keys.screenshotSource)
        defaults.set(screenshotTargetIntValue, forKey: VC64Keys.screenshotTarget)
        
        defaults.set(closeWithoutAsking, forKey: VC64Keys.closeWithoutAsking)
        defaults.set(ejectWithoutAsking, forKey: VC64Keys.ejectWithoutAsking)
        
        defaults.set(pauseInBackground, forKey: VC64Keys.pauseInBackground)
        defaults.set(c64.takeAutoSnapshots(), forKey: VC64Keys.autoSnapshots)
        defaults.set(c64.snapshotInterval(), forKey: VC64Keys.autoSnapshotInterval)
        
        defaults.encode(autoMountAction, forKey: VC64Keys.autoMountAction)
        defaults.encode(autoType, forKey: VC64Keys.autoType)
        defaults.encode(autoTypeText, forKey: VC64Keys.autoTypeText)
    }
}

//
// User defaults (Hardware)
//

extension VC64Keys {
    
    // Machine
    static let amigaModel     = "VAAmigaModelKey"
    static let realTimeClock  = "VARealTimeClockKey"
    
    // Memory
    static let chipRam        = "VAChipRamKey"
    static let slowRam        = "VASlowRamKey"
    static let fastRam        = "VAFastRamKey"

    // Drives
    static let df0Connect     = "VADF0ConnectKey"
    static let df0Type        = "VADF0TypeKey"
    static let df1Connect     = "VADF1ConnectKey"
    static let df1Type        = "VADF1TypeKey"
}

extension Defaults {
    
    struct a500 {
        
        static let amigaModel     = A500
        static let realTimeClock  = false
        
        static let chipRam        = 512
        static let slowRam        = 0
        static let fastRam        = 0

        static let df0Connect     = true
        static let df0Type        = A1010_ORIG
        static let df1Connect     = false
        static let df1Type        = A1010_ORIG
    }
    
    struct a1000 {
        
        static let amigaModel     = A1000
        static let realTimeClock  = false
        
        static let chipRam        = 256
        static let slowRam        = 0
        static let fastRam        = 0
        
        static let df0Connect     = true
        static let df0Type        = A1010_ORIG
        static let df1Connect     = false
        static let df1Type        = A1010_ORIG
    }
    
    struct a2000 {
        
        static let amigaModel     = A2000
        static let realTimeClock  = true
        
        static let chipRam        = 512
        static let slowRam        = 512
        static let fastRam        = 0
        
        static let df0Connect     = true
        static let df0Type        = A1010_ORIG
        static let df1Connect     = true
        static let df1Type        = A1010_ORIG
    }
}

extension MyController {
    
    static func registerHardwareUserDefaults() {
        
        let defaultModel = Defaults.a500.self
        
        let dictionary : [String:Any] = [
            
            VC64Keys.amigaModel: defaultModel.amigaModel.rawValue,
            VC64Keys.realTimeClock: defaultModel.realTimeClock,
            
            VC64Keys.chipRam: defaultModel.chipRam,
            VC64Keys.slowRam: defaultModel.slowRam,
            VC64Keys.fastRam: defaultModel.fastRam,
            
            VC64Keys.df0Connect: defaultModel.df0Connect,
            VC64Keys.df0Type: defaultModel.df0Type.rawValue,
            VC64Keys.df1Connect: defaultModel.df1Connect,
            VC64Keys.df1Type: defaultModel.df1Type.rawValue,
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }
    
    func resetHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [VC64Keys.amigaModel,
                    VC64Keys.realTimeClock,
                    
                    VC64Keys.chipRam,
                    VC64Keys.slowRam,
                    VC64Keys.fastRam,

                    VC64Keys.df0Connect,
                    VC64Keys.df0Type,
                    VC64Keys.df1Connect,
                    VC64Keys.df1Type
            ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadHardwareUserDefaults()
    }
    
    func loadHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        amiga.configureModel(defaults.integer(forKey: VC64Keys.amigaModel))
        amiga.configureRealTimeClock(defaults.bool(forKey: VC64Keys.realTimeClock))
    
        amiga.configureChipMemory(defaults.integer(forKey: VC64Keys.chipRam))
        amiga.configureSlowMemory(defaults.integer(forKey: VC64Keys.slowRam))
        amiga.configureFastMemory(defaults.integer(forKey: VC64Keys.fastRam))

        amiga.configureDrive(0, connected: defaults.bool(forKey: VC64Keys.df0Connect))
        amiga.configureDrive(0, type: defaults.integer(forKey: VC64Keys.df0Type))
        amiga.configureDrive(1, connected: defaults.bool(forKey: VC64Keys.df1Connect))
        amiga.configureDrive(1, type: defaults.integer(forKey: VC64Keys.df1Type))
        
        amiga.resume()
    }

    func saveHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        let config = amiga.config()
        
        defaults.set(config.model, forKey: VC64Keys.amigaModel)
        defaults.set(config.realTimeClock, forKey: VC64Keys.realTimeClock)

        defaults.set(config.chipRamSize, forKey: VC64Keys.chipRam)
        defaults.set(config.slowRamSize, forKey: VC64Keys.slowRam)
        defaults.set(config.fastRamSize, forKey: VC64Keys.fastRam)

        defaults.set(config.df0.connected, forKey: VC64Keys.df0Connect)
        defaults.set(config.df0.type, forKey: VC64Keys.df0Type)
        defaults.set(config.df1.connected, forKey: VC64Keys.df1Connect)
        defaults.set(config.df1.type, forKey: VC64Keys.df1Type)
    }
}
