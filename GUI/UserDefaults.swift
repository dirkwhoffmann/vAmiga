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

struct Keys {
    
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
        
        c64.suspend()
        
        inputDevice1 = defaults.integer(forKey: Keys.inputDevice1)
        inputDevice2 = defaults.integer(forKey: Keys.inputDevice2)
        
        c64.resume()
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

        c64.suspend()
        loadRom(defaults.url(forKey: Keys.basicRom))
        loadRom(defaults.url(forKey: Keys.charRom))
        loadRom(defaults.url(forKey: Keys.kernalRom))
        loadRom(defaults.url(forKey: Keys.vc1541Rom))
        c64.resume()
        
        amiga.suspend()
        amiga.loadBootRom(fromFile: defaults.url(forKey: Keys.bootRom))
        amiga.loadKickRom(fromFile: defaults.url(forKey: Keys.kickRom))
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
            Keys.mouseModel: Int(Defaults.mouseModel.rawValue),
            Keys.disconnectJoyKeys: Defaults.disconnectJoyKeys,
            Keys.autofire: Defaults.autofire,
            Keys.autofireBullets: Defaults.autofireBullets,
            Keys.autofireFrequency: Defaults.autofireFrequency
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
        defaults.register(encodableItem: Defaults.joyKeyMap1, forKey: Keys.joyKeyMap1)
        defaults.register(encodableItem: Defaults.joyKeyMap2, forKey: Keys.joyKeyMap2)
    }

    func resetDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [ Keys.mouseModel,
                     Keys.disconnectJoyKeys,
                     Keys.autofire,
                     Keys.autofireBullets,
                     Keys.autofireFrequency,
                     
                     Keys.joyKeyMap1,
                     Keys.joyKeyMap2
                     ]
        {
            defaults.removeObject(forKey: key)
        }
        
        loadDevicesUserDefaults()
    }
    
    func loadDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
    
        c64.suspend()
        
        c64.mouse.setModel(defaults.integer(forKey: Keys.mouseModel))
        keyboardcontroller.disconnectJoyKeys = defaults.bool(forKey: Keys.disconnectJoyKeys)
        c64.port1.setAutofire(defaults.bool(forKey: Keys.autofire))
        c64.port2.setAutofire(defaults.bool(forKey: Keys.autofire))
        c64.port1.setAutofireBullets(defaults.integer(forKey: Keys.autofireBullets))
        c64.port2.setAutofireBullets(defaults.integer(forKey: Keys.autofireBullets))
        c64.port1.setAutofireFrequency(defaults.float(forKey: Keys.autofireFrequency))
        c64.port2.setAutofireFrequency(defaults.float(forKey: Keys.autofireFrequency))
        defaults.decode(&gamePadManager.gamePads[0]!.keyMap, forKey: Keys.joyKeyMap1)
        defaults.decode(&gamePadManager.gamePads[1]!.keyMap, forKey: Keys.joyKeyMap2)
 
        c64.resume()
    }
    
    func saveDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(c64.mouse.model(), forKey: Keys.mouseModel)
        defaults.set(keyboardcontroller.disconnectJoyKeys, forKey: Keys.disconnectJoyKeys)
        defaults.set(c64.port1.autofire(), forKey: Keys.autofire)
        defaults.set(c64.port1.autofireBullets(), forKey: Keys.autofireBullets)
        defaults.set(c64.port1.autofireFrequency(), forKey: Keys.autofireFrequency)
        defaults.encode(gamePadManager.gamePads[0]!.keyMap, forKey: Keys.joyKeyMap1)
        defaults.encode(gamePadManager.gamePads[1]!.keyMap, forKey: Keys.joyKeyMap2)
    }
}

//
// User defaults (Video)
//

extension Keys {
    
    static let palette         = "VC64PaletteKey"
    static let brightness      = "VC64BrightnessKey"
    static let contrast        = "VC64ContrastKey"
    static let saturation      = "VC64SaturationKey"
    static let upscaler        = "VC64UpscalerKey"

    // Geometry
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
        
        c64.suspend()
        
        metal.upscaler = defaults.integer(forKey: Keys.upscaler)
        c64.vic.setVideoPalette(defaults.integer(forKey: Keys.palette))
        c64.vic.setBrightness(defaults.double(forKey: Keys.brightness))
        c64.vic.setContrast(defaults.double(forKey: Keys.contrast))
        c64.vic.setSaturation(defaults.double(forKey: Keys.saturation))

        metal.setEyeX(defaults.float(forKey: Keys.eyeX))
        metal.setEyeY(defaults.float(forKey: Keys.eyeY))
        metal.setEyeZ(defaults.float(forKey: Keys.eyeZ))
        
        defaults.decode(&metal.shaderOptions, forKey: Keys.shaderOptions)
        metal.buildDotMasks()
 
        c64.resume()
    }
    
    func saveVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(metal.upscaler, forKey: Keys.upscaler)
        defaults.set(c64.vic.videoPalette(), forKey: Keys.palette)
        defaults.set(c64.vic.brightness(), forKey: Keys.brightness)
        defaults.set(c64.vic.contrast(), forKey: Keys.contrast)
        defaults.set(c64.vic.saturation(), forKey: Keys.saturation)
        
        defaults.set(metal.eyeX(), forKey: Keys.eyeX)
        defaults.set(metal.eyeY(), forKey: Keys.eyeY)
        defaults.set(metal.eyeZ(), forKey: Keys.eyeZ)
  
        defaults.encode(metal.shaderOptions, forKey: Keys.shaderOptions)
    }
}

//
// User defaults (Emulator)
//

extension Keys {
    
    // Drives
    static let warpLoad             = "VAWarpLoadKey"
    static let driveNoise           = "VADriveNoiseKey"
    
    // Screenshots
    static let screenshotSource     = "VAScreenshotSourceKey"
    static let screenshotTarget     = "VAScreenshotTargetKey"
    
    // Fullscreen
    static let keepAspectRatio      = "VAFullscreenKeepAspectRatioKey"
    static let exitOnEsc            = "VAFullscreenExitOnEscKey"

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
            
        c64.suspend()
        amiga.suspend()
        
        amiga.setWarpLoad(defaults.bool(forKey: Keys.warpLoad))
        c64.drive1.setSendSoundMessages(defaults.bool(forKey: Keys.driveNoise))
        c64.drive2.setSendSoundMessages(defaults.bool(forKey: Keys.driveNoise))
    
        screenshotSource = defaults.integer(forKey: Keys.screenshotSource)
        screenshotTargetIntValue = defaults.integer(forKey: Keys.screenshotTarget)
    
        metal.keepAspectRatio = defaults.bool(forKey: Keys.keepAspectRatio)
        keyboardcontroller.exitOnEsc = defaults.bool(forKey: Keys.exitOnEsc)
        
        closeWithoutAsking = defaults.bool(forKey: Keys.closeWithoutAsking)
        ejectWithoutAsking = defaults.bool(forKey: Keys.ejectWithoutAsking)

        pauseInBackground = defaults.bool(forKey: Keys.pauseInBackground)
        amiga.setTakeAutoSnapshots(defaults.bool(forKey: Keys.autoSnapshots))
        amiga.setSnapshotInterval(defaults.integer(forKey: Keys.autoSnapshotInterval))
        
        defaults.decode(&autoMountAction, forKey: Keys.autoMountAction)
        defaults.decode(&autoType, forKey: Keys.autoType)
        defaults.decode(&autoTypeText, forKey: Keys.autoTypeText)
        
        amiga.reset()
        c64.resume()
    }
    
    func saveEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(c64.warpLoad(), forKey: Keys.warpLoad)
        defaults.set(c64.drive1.sendSoundMessages(), forKey: Keys.driveNoise)
        defaults.set(c64.drive2.sendSoundMessages(), forKey: Keys.driveNoise)

        defaults.set(screenshotSource, forKey: Keys.screenshotSource)
        defaults.set(screenshotTargetIntValue, forKey: Keys.screenshotTarget)
        
        defaults.set(metal.keepAspectRatio, forKey: Keys.keepAspectRatio)
        defaults.set(keyboardcontroller.exitOnEsc, forKey: Keys.exitOnEsc)
        
        defaults.set(closeWithoutAsking, forKey: Keys.closeWithoutAsking)
        defaults.set(ejectWithoutAsking, forKey: Keys.ejectWithoutAsking)
        
        defaults.set(pauseInBackground, forKey: Keys.pauseInBackground)
        defaults.set(amiga.takeAutoSnapshots(), forKey: Keys.autoSnapshots)
        defaults.set(amiga.snapshotInterval(), forKey: Keys.autoSnapshotInterval)
        
        defaults.encode(autoMountAction, forKey: Keys.autoMountAction)
        defaults.encode(autoType, forKey: Keys.autoType)
        defaults.encode(autoTypeText, forKey: Keys.autoTypeText)
    }
}

//
// User defaults (Hardware)
//

extension Keys {
    
    // Machine
    static let amigaModel     = "VAAmigaModelKey"
    static let layout         = "VAKeyboardLayoutKey"
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
        static let layout         = Layout.us
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
        static let layout         = Layout.us
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
        static let layout         = Layout.us
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
            
            Keys.amigaModel: defaultModel.amigaModel.rawValue,
            Keys.layout: defaultModel.layout.rawValue,
            Keys.realTimeClock: defaultModel.realTimeClock,
            
            Keys.chipRam: defaultModel.chipRam,
            Keys.slowRam: defaultModel.slowRam,
            Keys.fastRam: defaultModel.fastRam,
            
            Keys.df0Connect: defaultModel.df0Connect,
            Keys.df0Type: defaultModel.df0Type.rawValue,
            Keys.df1Connect: defaultModel.df1Connect,
            Keys.df1Type: defaultModel.df1Type.rawValue,
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }
    
    func resetHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        for key in [Keys.amigaModel,
                    Keys.layout,
                    Keys.realTimeClock,
                    
                    Keys.chipRam,
                    Keys.slowRam,
                    Keys.fastRam,

                    Keys.df0Connect,
                    Keys.df0Type,
                    Keys.df1Connect,
                    Keys.df1Type
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
        amiga.configureRealTimeClock(defaults.bool(forKey: Keys.realTimeClock))
    
        amiga.configureChipMemory(defaults.integer(forKey: Keys.chipRam))
        amiga.configureSlowMemory(defaults.integer(forKey: Keys.slowRam))
        amiga.configureFastMemory(defaults.integer(forKey: Keys.fastRam))

        amiga.configureDrive(0, connected: defaults.bool(forKey: Keys.df0Connect))
        amiga.configureDrive(0, type: defaults.integer(forKey: Keys.df0Type))
        amiga.configureDrive(1, connected: defaults.bool(forKey: Keys.df1Connect))
        amiga.configureDrive(1, type: defaults.integer(forKey: Keys.df1Type))
        
        amiga.resume()
    }

    func saveHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        let config = amiga.config()
        
        defaults.set(config.model.rawValue, forKey: Keys.amigaModel)
        defaults.set(config.layout, forKey: Keys.layout)
        defaults.set(config.realTimeClock, forKey: Keys.realTimeClock)

        defaults.set(config.chipRamSize, forKey: Keys.chipRam)
        defaults.set(config.slowRamSize, forKey: Keys.slowRam)
        defaults.set(config.fastRamSize, forKey: Keys.fastRam)

        defaults.set(config.df0.connected, forKey: Keys.df0Connect)
        defaults.set(config.df0.type.rawValue, forKey: Keys.df0Type)
        defaults.set(config.df1.connected, forKey: Keys.df1Connect)
        defaults.set(config.df1.type.rawValue, forKey: Keys.df1Type)
    }
}
