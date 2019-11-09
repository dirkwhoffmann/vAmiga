// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

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
        registerCompatibilityUserDefaults()
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
        resetCompatibilityUserDefaults()
        
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
        loadCompatibilityUserDefaults()
        
        amiga.resume()
    }
    
    func loadUserDefaults(url: URL) {
        
        if let fileContents = NSDictionary(contentsOf: url) {
            
            if let dict = fileContents as? [String: Any] {
                
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
        saveCompatibilityUserDefaults()
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
    static let inputDevice1      = "VAMIGAInputDevice1Key"
    static let inputDevice2      = "VAMIGAInputDevice2Key"
}

struct Defaults {
    
    // Control ports
    static let inputDevice1 = -1
    static let inputDevice2 = -1

    // Keyboard
    static let kbStyle = KBStyle.wide
    static let kbLayout = KBLayout.us
}

extension MyController {

    static func registerGeneralUserDefaults() {
        
        let dictionary: [String: Any] = [
            
            Keys.inputDevice1: Defaults.inputDevice1,
            Keys.inputDevice2: Defaults.inputDevice2
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }
    
    func resetGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.inputDevice1,
                     Keys.inputDevice2
        ]

         for key in keys { defaults.removeObject(forKey: key) }
        
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
    
    static let rom               = "VAMIGARomFileKey"
    static let ext               = "VAMIGAExtFileKey"
    static let extStart          = "VAMIGAExtStartKey"
}

extension Defaults {
    
    static let rom               = URL(fileURLWithPath: "")
    static let ext               = URL(fileURLWithPath: "")
    static let extStart          = 0xE0
}

extension MyController {
    
    static func registerRomUserDefaults() {
        
        let dictionary: [String: Any] = [
            
            Keys.rom: Defaults.rom,
            Keys.ext: Defaults.ext,
            Keys.extStart: Defaults.extStart
            ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }

    func resetRomUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.rom,
                     Keys.ext,
                     Keys.extStart ]

        for key in keys { defaults.removeObject(forKey: key) }
        
        loadRomUserDefaults()
    }
    
    func loadRomUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        if let url = defaults.url(forKey: Keys.rom) {
            romURL = url
            amiga.mem.loadRom(fromFile: romURL)
        }
        if let url = defaults.url(forKey: Keys.ext) {
            extURL = url
            amiga.mem.loadExt(fromFile: extURL)
        }
        amiga.configure(VA_EXT_START, value: defaults.integer(forKey: Keys.extStart))

        amiga.resume()
    }
    
    func saveRomUserDefaults() {
        
        let config = amiga.config()
        let defaults = UserDefaults.standard

        defaults.set(romURL, forKey: Keys.rom)
        defaults.set(extURL, forKey: Keys.ext)
        defaults.set(config.mem.extStart, forKey: Keys.extStart)
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
        MacKey.init(keyCode: kVK_LeftArrow): JOYSTICK_LEFT.rawValue,
        MacKey.init(keyCode: kVK_RightArrow): JOYSTICK_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_UpArrow): JOYSTICK_UP.rawValue,
        MacKey.init(keyCode: kVK_DownArrow): JOYSTICK_DOWN.rawValue,
        MacKey.init(keyCode: kVK_Space): JOYSTICK_FIRE.rawValue
    ]
    static let joyKeyMap2 = [
        MacKey.init(keyCode: kVK_ANSI_S): JOYSTICK_LEFT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_D): JOYSTICK_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_E): JOYSTICK_UP.rawValue,
        MacKey.init(keyCode: kVK_ANSI_X): JOYSTICK_DOWN.rawValue,
        MacKey.init(keyCode: kVK_ANSI_C): JOYSTICK_FIRE.rawValue
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
        
        let dictionary: [String: Any] = [

            // Joysticks
            Keys.disconnectJoyKeys: Defaults.disconnectJoyKeys,
            Keys.autofire: Defaults.autofire,
            Keys.autofireBullets: Defaults.autofireBullets,
            Keys.autofireFrequency: Defaults.autofireFrequency,
            
            // Mouse
            Keys.retainMouseKeyComb: Defaults.retainMouseKeyComb,
            Keys.retainMouseWithKeys: Defaults.retainMouseWithKeys,
            Keys.retainMouseByClick: Defaults.retainMouseByClick,
            Keys.retainMouseByEntering: Defaults.retainMouseByEntering,
            Keys.releaseMouseKeyComb: Defaults.releaseMouseKeyComb,
            Keys.releaseMouseWithKeys: Defaults.releaseMouseWithKeys,
            Keys.releaseMouseByShaking: Defaults.releaseMouseByShaking
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
        defaults.register(encodableItem: Defaults.joyKeyMap1, forKey: Keys.joyKeyMap1)
        defaults.register(encodableItem: Defaults.joyKeyMap2, forKey: Keys.joyKeyMap2)
    }

    func resetDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.joyKeyMap1,
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
                     Keys.releaseMouseByShaking ]

        for key in keys { defaults.removeObject(forKey: key) }
        
        loadDevicesUserDefaults()
    }
    
    func loadDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
    
        amiga.suspend()
        
        // Joysticks
        defaults.decode(&gamePadManager.gamePads[0]!.keyMap, forKey: Keys.joyKeyMap1)
        defaults.decode(&gamePadManager.gamePads[1]!.keyMap, forKey: Keys.joyKeyMap2)
        keyboardcontroller.disconnectJoyKeys = defaults.bool(forKey: Keys.disconnectJoyKeys)
        amiga.joystick1.setAutofire(defaults.bool(forKey: Keys.autofire))
        amiga.joystick2.setAutofire(defaults.bool(forKey: Keys.autofire))
        amiga.joystick1.setAutofireBullets(defaults.integer(forKey: Keys.autofireBullets))
        amiga.joystick2.setAutofireBullets(defaults.integer(forKey: Keys.autofireBullets))
        amiga.joystick1.setAutofireFrequency(defaults.float(forKey: Keys.autofireFrequency))
        amiga.joystick2.setAutofireFrequency(defaults.float(forKey: Keys.autofireFrequency))
 
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
        defaults.set(amiga.joystick1.autofire(), forKey: Keys.autofire)
        defaults.set(amiga.joystick1.autofireBullets(), forKey: Keys.autofireBullets)
        defaults.set(amiga.joystick1.autofireFrequency(), forKey: Keys.autofireFrequency)
        
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
        
        let dictionary: [String: Any] = [
            
            Keys.palette: Int(Defaults.palette.rawValue),
            Keys.brightness: Defaults.brightness,
            Keys.contrast: Defaults.contrast,
            Keys.saturation: Defaults.saturation,
            Keys.enhancer: Defaults.enhancer,
            Keys.upscaler: Defaults.upscaler,

            Keys.eyeX: Defaults.eyeX,
            Keys.eyeY: Defaults.eyeY,
            Keys.eyeZ: Defaults.eyeZ
            ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
        defaults.register(encodableItem: Defaults.shaderOptions, forKey: Keys.shaderOptions)
    }
    
    func resetVideoUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.palette,
                     Keys.brightness,
                     Keys.contrast,
                     Keys.saturation,
                     Keys.enhancer,
                     Keys.upscaler,

                     Keys.eyeX,
                     Keys.eyeY,
                     Keys.eyeZ,
                     
                     Keys.shaderOptions ]

        for key in keys { defaults.removeObject(forKey: key) }
        
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
}

extension MyController {
    
    static func registerEmulatorUserDefaults() {
        
        let dictionary: [String: Any] = [
            
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
    }

    func resetEmulatorUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [Keys.warpLoad,
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
                    Keys.autoSnapshotInterval ]

        for key in keys { defaults.removeObject(forKey: key) }
        
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
    
    // Chipset
    static let agnusRev           = "VAMIGAAgnusRevKey"
    static let deniseRev          = "VAMIGADeniseRevKey"
    static let realTimeClock      = "VAMIGARealTimeClockKey"

    // Memory
    static let chipRam            = "VAMIGAChipRamKey"
    static let slowRam            = "VAMIGASlowRamKey"
    static let fastRam            = "VAMIGAFastRamKey"

    // Drives
    static let df0Connect         = "VAMIGADF0ConnectKey"
    static let df0Type            = "VAMIGADF0TypeKey"
    static let df1Connect         = "VAMIGADF1ConnectKey"
    static let df1Type            = "VAMIGADF1TypeKey"
    static let df2Connect         = "VAMIGADF2ConnectKey"
    static let df2Type            = "VAMIGADF2TypeKey"
    static let df3Connect         = "VAMIGADF3ConnectKey"
    static let df3Type            = "VAMIGADF3TypeKey"

    // Ports
    static let serialDevice       = "VAMIGASerialDeviceKey"
}

extension Defaults {

    struct ModelDefaults {

        let agnusRevision: AgnusRevision
        let deniseRevision: DeniseRevision
        let realTimeClock: RTCModel

        let chipRam: Int
        let slowRam: Int
        let fastRam: Int

        let driveConnect: [Bool]
        let driveType: [DriveType]

        let serialDevice: SerialPortDevice
    }

    static let A500 = ModelDefaults.init(

        agnusRevision: AGNUS_8372,
        deniseRevision: DENISE_8362R8,
        realTimeClock: RTC_NONE,
        chipRam: 512,
        slowRam: 0,
        fastRam: 0,
        driveConnect: [true, false, false, false],
        driveType: [DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD],
        serialDevice: SPD_NONE)

    static let A1000 = ModelDefaults.init(

        agnusRevision: AGNUS_8367,
        deniseRevision: DENISE_8362R8,
        realTimeClock: RTC_NONE,
        chipRam: 256,
        slowRam: 0,
        fastRam: 0,
        driveConnect: [true, false, false, false],
        driveType: [DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD],
        serialDevice: SPD_NONE)

    static let A2000 = ModelDefaults.init(
        
        agnusRevision: AGNUS_8375,
        deniseRevision: DENISE_8362R8,
        realTimeClock: RTC_M6242B,
        chipRam: 512,
        slowRam: 512,
        fastRam: 0,
        driveConnect: [true, true, false, false],
        driveType: [DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD],
        serialDevice: SPD_NONE)
}

extension MyController {
    
    static func registerHardwareUserDefaults() {
        
        let defaultModel = Defaults.A500
        
        let dictionary: [String: Any] = [
            
            Keys.agnusRev: defaultModel.agnusRevision.rawValue,
            Keys.deniseRev: defaultModel.deniseRevision.rawValue,
            Keys.realTimeClock: defaultModel.realTimeClock.rawValue,

            Keys.chipRam: defaultModel.chipRam,
            Keys.slowRam: defaultModel.slowRam,
            Keys.fastRam: defaultModel.fastRam,

            Keys.df0Connect: defaultModel.driveConnect[0],
            Keys.df0Type: defaultModel.driveType[0].rawValue,
            Keys.df1Connect: defaultModel.driveConnect[1],
            Keys.df1Type: defaultModel.driveType[1].rawValue,
            Keys.df2Connect: defaultModel.driveConnect[2],
            Keys.df2Type: defaultModel.driveType[2].rawValue,
            Keys.df3Connect: defaultModel.driveConnect[3],
            Keys.df3Type: defaultModel.driveType[3].rawValue,

            Keys.serialDevice: defaultModel.serialDevice.rawValue
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }
    
    func resetHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [Keys.agnusRev,
                    Keys.deniseRev,
                    Keys.realTimeClock,

                    Keys.chipRam,
                    Keys.slowRam,
                    Keys.fastRam,

                    Keys.driveSpeed,
                    Keys.df0Connect,
                    Keys.df0Type,
                    Keys.df1Connect,
                    Keys.df1Type,
                    Keys.df2Connect,
                    Keys.df2Type,
                    Keys.df3Connect,
                    Keys.df3Type,

                    Keys.serialDevice ]

        for key in keys { defaults.removeObject(forKey: key) }
        
        loadHardwareUserDefaults()
    }
    
    func loadHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        amiga.configure(VA_AGNUS_REVISION, value: defaults.integer(forKey: Keys.agnusRev))
        amiga.configure(VA_DENISE_REVISION, value: defaults.integer(forKey: Keys.deniseRev))
        amiga.configure(VA_RT_CLOCK, enable: defaults.bool(forKey: Keys.realTimeClock))

        amiga.configure(VA_CHIP_RAM, value: defaults.integer(forKey: Keys.chipRam))
        amiga.configure(VA_SLOW_RAM, value: defaults.integer(forKey: Keys.slowRam))
        amiga.configure(VA_FAST_RAM, value: defaults.integer(forKey: Keys.fastRam))

        amiga.configure(VA_DRIVE_SPEED, value: defaults.integer(forKey: Keys.driveSpeed))
        amiga.configureDrive(0, connected: defaults.bool(forKey: Keys.df0Connect))
        amiga.configureDrive(0, type: defaults.integer(forKey: Keys.df0Type))
        amiga.configureDrive(1, connected: defaults.bool(forKey: Keys.df1Connect))
        amiga.configureDrive(1, type: defaults.integer(forKey: Keys.df1Type))
        amiga.configureDrive(2, connected: defaults.bool(forKey: Keys.df2Connect))
        amiga.configureDrive(2, type: defaults.integer(forKey: Keys.df2Type))
        amiga.configureDrive(3, connected: defaults.bool(forKey: Keys.df3Connect))
        amiga.configureDrive(3, type: defaults.integer(forKey: Keys.df3Type))

        amiga.configure(VA_SERIAL_DEVICE, value: defaults.integer(forKey: Keys.serialDevice))
        amiga.resume()
    }

    func saveHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        let config = amiga.config()
        let dc = config.diskController

        defaults.set(config.agnus.revision.rawValue, forKey: Keys.agnusRev)
        defaults.set(config.denise.revision.rawValue, forKey: Keys.deniseRev)
        defaults.set(config.rtc.model.rawValue, forKey: Keys.realTimeClock)

        defaults.set(config.mem.chipRamSize / 1024, forKey: Keys.chipRam)
        defaults.set(config.mem.slowRamSize / 1024, forKey: Keys.slowRam)
        defaults.set(config.mem.fastRamSize / 1024, forKey: Keys.fastRam)

        defaults.set(config.df0.speed, forKey: Keys.driveSpeed)
        defaults.set(dc.connected.0, forKey: Keys.df0Connect)
        defaults.set(dc.connected.1, forKey: Keys.df1Connect)
        defaults.set(dc.connected.2, forKey: Keys.df2Connect)
        defaults.set(dc.connected.3, forKey: Keys.df3Connect)
        defaults.set(config.df0.type.rawValue, forKey: Keys.df0Type)
        defaults.set(config.df1.type.rawValue, forKey: Keys.df1Type)
        defaults.set(config.df2.type.rawValue, forKey: Keys.df2Type)
        defaults.set(config.df3.type.rawValue, forKey: Keys.df3Type)

        defaults.set(config.serialPort.device.rawValue, forKey: Keys.serialDevice)
    }
}

//
// User defaults (Compatibility)
//

extension Keys {

    // CPU
    static let cpuEngine         = "VAMIGACpuEngine"
    static let cpuSpeed          = "VAMIGACpuSpeed"

    // Graphics
    static let clxSprSpr         = "VAMIGAClxSprSpr"
    static let clxSprPlf         = "VAMIGAClxSprPlf"
    static let clxPlfPlf         = "VAMIGAClxPlfPlf"

    // Audio
    static let filterActivation  = "VAMIGAFilterActivation"
    static let filterType        = "VAMIGAFilterType"

    // Blitter
    static let blitterAccuracy   = "VAMIGABlitterAccuracy"

    // Floppy drives
    static let driveSpeed        = "VAMIGADriveSpeedKey"
    static let fifoBuffering     = "VAMIGAFifoBufferingKey"
}

extension Defaults {

    // CPU
    static let cpuEngine         = CPU_MUSASHI
    static let cpuSpeed          = 1

    // Graphics
    static let clxSprSpr         = false
    static let clxSprPlf         = false
    static let clxPlfPlf         = false

    // Audio
    static let filterActivation  = FILTACT_POWER_LED
    static let filterType        = FILT_BUTTERWORTH

    // Blitter
    static let blitterAccuracy   = 0

    // Floppy drives
    static let driveSpeed        = 1
    static let fifoBuffering     = true
}

extension MyController {

    static func registerCompatibilityUserDefaults() {

        let dictionary: [String: Any] = [

            Keys.cpuEngine: Defaults.cpuEngine.rawValue,
            Keys.cpuSpeed: Defaults.cpuSpeed,
            Keys.clxSprSpr: Defaults.clxSprSpr,
            Keys.clxSprPlf: Defaults.clxSprPlf,
            Keys.clxPlfPlf: Defaults.clxPlfPlf,
            Keys.filterActivation: Defaults.filterActivation.rawValue,
            Keys.filterType: Defaults.filterType.rawValue,
            Keys.blitterAccuracy: Defaults.blitterAccuracy,
            Keys.driveSpeed: Defaults.driveSpeed,
            Keys.fifoBuffering: Defaults.fifoBuffering
        ]

        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }

    func resetCompatibilityUserDefaults() {

        let defaults = UserDefaults.standard

        let keys = [ Keys.cpuEngine,
                     Keys.cpuSpeed,
                     Keys.clxSprSpr,
                     Keys.clxSprPlf,
                     Keys.clxPlfPlf,
                     Keys.filterActivation,
                     Keys.filterType,
                     Keys.blitterAccuracy,
                     Keys.driveSpeed,
                     Keys.fifoBuffering ]

        for key in keys { defaults.removeObject(forKey: key) }

        loadCompatibilityUserDefaults()
    }

    func loadCompatibilityUserDefaults() {

        let defaults = UserDefaults.standard

        amiga.suspend()

        amiga.configure(VA_CPU_ENGINE, value: defaults.integer(forKey: Keys.cpuEngine))
        amiga.configure(VA_CPU_SPEED, value: defaults.integer(forKey: Keys.cpuSpeed))
        amiga.configure(VA_CLX_SPR_SPR, enable: defaults.bool(forKey: Keys.clxSprSpr))
        amiga.configure(VA_CLX_SPR_PLF, enable: defaults.bool(forKey: Keys.clxSprPlf))
        amiga.configure(VA_CLX_PLF_PLF, enable: defaults.bool(forKey: Keys.clxPlfPlf))
        amiga.configure(VA_FILTER_ACTIVATION, value: defaults.integer(forKey: Keys.filterActivation))
        amiga.configure(VA_FILTER_TYPE, value: defaults.integer(forKey: Keys.filterType))
        amiga.configure(VA_BLITTER_ACCURACY, value: defaults.integer(forKey: Keys.blitterAccuracy))
        amiga.configure(VA_DRIVE_SPEED, value: defaults.integer(forKey: Keys.driveSpeed))
        amiga.configure(VA_FIFO_BUFFERING, enable: defaults.bool(forKey: Keys.fifoBuffering))

        amiga.resume()
    }

    func saveCompatibilityUserDefaults() {

        let defaults = UserDefaults.standard
        let config = amiga.config()

        defaults.set(config.cpuEngine.rawValue, forKey: Keys.cpuEngine)
        defaults.set(config.cpuSpeed, forKey: Keys.cpuSpeed)
        defaults.set(config.denise.clxSprSpr, forKey: Keys.clxSprSpr)
        defaults.set(config.denise.clxSprPlf, forKey: Keys.clxSprPlf)
        defaults.set(config.denise.clxPlfPlf, forKey: Keys.clxPlfPlf)
        defaults.set(config.audio.filterActivation.rawValue, forKey: Keys.filterActivation)
        defaults.set(config.audio.filterType.rawValue, forKey: Keys.filterType)
        defaults.set(config.blitter.accuracy, forKey: Keys.blitterAccuracy)
        defaults.set(config.diskController.useFifo, forKey: Keys.fifoBuffering)
    }
}
