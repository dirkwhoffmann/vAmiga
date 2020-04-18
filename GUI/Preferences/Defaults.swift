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
    
    // Registers an item of generic type 'Encodable'
    func register<T: Encodable>(encodableItem item: T, forKey key: String) {
        
        if let data = try? PropertyListEncoder().encode(item) {
            register(defaults: [key: data])
        }
    }

    // Encodes an item of generic type 'Encodable'
    func encode<T: Encodable>(_ item: T, forKey key: String) {
        
        if let encoded = try? PropertyListEncoder().encode(item) {
            track("Encoded \(key) successfully")
            set(encoded, forKey: key)
        } else {
            track("Failed to encode \(key)")
        }
    }
    
    // Encodes an item of generic type 'Decodable'
    func decode<T: Decodable>(_ item: inout T, forKey key: String) {
        
        if let data = data(forKey: key) {
            if let decoded = try? PropertyListDecoder().decode(T.self, from: data) {
                // track("Decoded \(key) successfully")
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

extension UserDefaults {
    
    static func registerUserDefaults() {
        
        track()
        
        UserDefaults.registerRomUserDefaults()
        UserDefaults.registerDevicesUserDefaults()
        UserDefaults.registerVideoUserDefaults()
        UserDefaults.registerGeneralUserDefaults()
        UserDefaults.registerHardwareUserDefaults()
        UserDefaults.registerCompatibilityUserDefaults()
    }
}

extension MyController {
    
    func resetUserDefaults() {
        
        track()
        
        amiga.suspend()
        
        UserDefaults.resetRomUserDefaults()
        UserDefaults.resetHardwareUserDefaults()
        UserDefaults.resetCompatibilityUserDefaults()
        UserDefaults.resetVideoUserDefaults()

        amiga.resume()
    }
    
    func loadUserDefaults() {
        
        track()
        
        amiga.suspend()
        
        prefs.loadGeneralUserDefaults()
        prefs.loadDevicesUserDefaults()

        config.loadRomUserDefaults()
        config.loadVideoUserDefaults()
        config.loadHardwareUserDefaults()
        config.loadCompatibilityUserDefaults()
        
        amiga.resume()
    }
    
    func loadUserDefaults(url: URL, prefixes: [String]) {
        
        if let fileContents = NSDictionary(contentsOf: url) {
            
            if let dict = fileContents as? [String: Any] {
                
                let filteredDict = dict.filter { prefixes.contains(where: $0.0.hasPrefix) }
                
                let defaults = UserDefaults.standard
                defaults.setValuesForKeys(filteredDict)
                
                loadUserDefaults()
            }
        }
    }
    
    func saveUserDefaults() {
        
        track()
        
        prefs.saveGeneralUserDefaults()
        prefs.saveDevicesUserDefaults()

        config.saveRomUserDefaults()
        config.saveVideoUserDefaults()
        config.saveHardwareUserDefaults()
        config.saveCompatibilityUserDefaults()
    }

    func saveUserDefaults(url: URL, prefixes: [String]) {
        
        track()
        
        let dict = UserDefaults.standard.dictionaryRepresentation()
        // let filteredDict = dict.filter { $0.0.hasPrefix("VAMIGA") }
        let filteredDict = dict.filter { prefixes.contains(where: $0.0.hasPrefix) }
        let nsDict = NSDictionary.init(dictionary: filteredDict)
        nsDict.write(to: url, atomically: true)
    }
}

//
// User defaults (General)
//

struct Keys {
    
    // Drives
    static let warpLoad               = "VAMIGA1WarpLoadKey"
    static let driveSounds            = "VAMIGA1DriveSounds"
    static let driveSoundPan          = "VAMIGA1DriveSoundPan"
    static let driveInsertSound       = "VAMIGA1DriveInsertSound"
    static let driveEjectSound        = "VAMIGA1DriveEjectSound"
    static let driveHeadSound         = "VAMIGA1DriveHeadSound"
    static let drivePollSound         = "VAMIGA1DrivePollSound"
    static let driveBlankDiskFormat   = "VAMIGA1DriveBlankDiskFormatKey"

    // Snapshots and screenshots
    static let autoSnapshots          = "VAMIGA1AutoSnapshots"
    static let autoSnapshotInterval   = "VAMIGA1ScreenshotInterval"
    static let autoScreenshots        = "VAMIGA1AutoScreenshots"
    static let autoScreenshotInterval = "VAMIGA1SnapshotInterval"
    static let screenshotSource       = "VAMIGA1ScreenshotSourceKey"
    static let screenshotTarget       = "VAMIGA1ScreenshotTargetKey"
    
    // Fullscreen
    static let keepAspectRatio        = "VAMIGA1FullscreenKeepAspectRatioKey"
    static let exitOnEsc              = "VAMIGA1FullscreenExitOnEscKey"

    // User dialogs
    static let closeWithoutAsking     = "VAMIGA1CloseWithoutAsking"
    static let ejectWithoutAsking     = "VAMIGA1EjectWithoutAsking"
    
    // Miscellaneous
    static let pauseInBackground      = "VAMIGA1PauseInBackground"
}

struct Defaults {
   
    // Drives
    static let warpLoad               = true
    static let driveSounds            = true
    static let driveSoundPan          = 1.0
    static let driveInsertSound       = true
    static let driveEjectSound        = true
    static let driveHeadSound         = true
    static let drivePollSound         = false
    static let driveBlankDiskFormat   = FS_OFS
    
    // Snapshots and Screenshots
    static let autoSnapshots          = true
    static let autoSnapshotInterval   = 20
    static let autoScreenshots        = true
    static let autoScreenshotInterval = 10
    static let screenshotSource       = 0
    static let screenshotTarget       = NSBitmapImageRep.FileType.png

    // Fullscreen
    static let keepAspectRatio        = false
    static let exitOnEsc              = false

    // User dialogs
    
    // Miscellaneous
    static let pauseInBackground      = false
    static let closeWithoutAsking     = false
    static let ejectWithoutAsking     = false
}

extension UserDefaults {
    
    static func registerGeneralUserDefaults() {
        
        let dictionary: [String: Any] = [
            
            Keys.warpLoad: Defaults.warpLoad,
            Keys.driveSounds: Defaults.driveSounds,
            Keys.driveSoundPan: Defaults.driveSoundPan,
            Keys.driveInsertSound: Defaults.driveInsertSound,
            Keys.driveEjectSound: Defaults.driveEjectSound,
            Keys.driveHeadSound: Defaults.driveHeadSound,
            Keys.drivePollSound: Defaults.drivePollSound,
            Keys.driveBlankDiskFormat: Int(Defaults.driveBlankDiskFormat.rawValue),

            Keys.autoSnapshots: Defaults.autoSnapshots,
            Keys.autoSnapshotInterval: Defaults.autoSnapshotInterval,
            Keys.autoScreenshots: Defaults.autoScreenshots,
            Keys.autoScreenshotInterval: Defaults.autoScreenshotInterval,
            Keys.screenshotSource: Defaults.screenshotSource,
            Keys.screenshotTarget: Int(Defaults.screenshotTarget.rawValue),

            Keys.keepAspectRatio: Defaults.keepAspectRatio,
            Keys.exitOnEsc: Defaults.exitOnEsc,
            
            Keys.pauseInBackground: Defaults.pauseInBackground,
            Keys.closeWithoutAsking: Defaults.closeWithoutAsking,
            Keys.ejectWithoutAsking: Defaults.ejectWithoutAsking
        ]
        
        let defaults = UserDefaults.standard
        
        defaults.register(defaults: dictionary)
    }
    
    static func resetGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [Keys.warpLoad,
                    Keys.driveSounds,
                    Keys.driveSoundPan,
                    Keys.driveInsertSound,
                    Keys.driveEjectSound,
                    Keys.driveHeadSound,
                    Keys.drivePollSound,
                    Keys.driveBlankDiskFormat,
                    
                    Keys.autoSnapshots,
                    Keys.autoSnapshotInterval,
                    Keys.autoScreenshots,
                    Keys.autoScreenshotInterval,
                    Keys.screenshotSource,
                    Keys.screenshotTarget,
                    
                    Keys.keepAspectRatio,
                    Keys.exitOnEsc,
                    
                    Keys.pauseInBackground,
                    Keys.closeWithoutAsking,
                    Keys.ejectWithoutAsking
        ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Input Devices)
//

extension Keys {
    
    // Joysticks
    static let joyKeyMap1            = "VAMIGA2JoyKeyMap1"
    static let joyKeyMap2            = "VAMIGA2JoyKeyMap2"
    static let mouseKeyMap           = "VAMIGA2MouseKeyMap"
    static let disconnectJoyKeys     = "VAMIGA2DisconnectKeys"
    static let autofire              = "VAMIGA2Autofire"
    static let autofireBullets       = "VAMIGA2AutofireBullets"
    static let autofireFrequency     = "VAMIGA2AutofireFrequency"
    
    // Mouse
    static let retainMouseKeyComb    = "VAMIGA2RetainMouseKeyComb"
    static let retainMouseWithKeys   = "VAMIGA2RetainMouseWithKeys"
    static let retainMouseByClick    = "VAMIGA2RetainMouseByClick"
    static let retainMouseByEntering = "VAMIGA2RetainMouseByEntering"
    static let releaseMouseKeyComb   = "VAMIGA2ReleaseMouseKeyComb"
    static let releaseMouseWithKeys  = "VAMIGA2ReleaseMouseWithKeys"
    static let releaseMouseByShaking = "VAMIGA2ReleaseMouseByShaking"
}

extension Defaults {
    
    // Emulation keys
    static let joyKeyMap1 = [
        MacKey.init(keyCode: kVK_LeftArrow): PULL_LEFT.rawValue,
        MacKey.init(keyCode: kVK_RightArrow): PULL_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_UpArrow): PULL_UP.rawValue,
        MacKey.init(keyCode: kVK_DownArrow): PULL_DOWN.rawValue,
        MacKey.init(keyCode: kVK_Space): PRESS_FIRE.rawValue
    ]
    static let joyKeyMap2 = [
        MacKey.init(keyCode: kVK_ANSI_S): PULL_LEFT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_D): PULL_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_E): PULL_UP.rawValue,
        MacKey.init(keyCode: kVK_ANSI_X): PULL_DOWN.rawValue,
        MacKey.init(keyCode: kVK_ANSI_C): PRESS_FIRE.rawValue
    ]
    static let mouseKeyMap: [MacKey: UInt32] = [:]
    static let disconnectJoyKeys = true

    // Joysticks
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
    
extension UserDefaults {
    
    static func registerDevicesUserDefaults() {
        
        let dictionary: [String: Any] = [

            // Emulation keys
            Keys.disconnectJoyKeys: Defaults.disconnectJoyKeys,

            // Joysticks
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
        defaults.register(encodableItem: Defaults.mouseKeyMap, forKey: Keys.mouseKeyMap)
    }
    
    static func resetDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.joyKeyMap1,
                     Keys.joyKeyMap2,
                     Keys.mouseKeyMap,
                     
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
    }
}

//
// User defaults (Roms)
//

extension Keys {
    
    static let rom               = "VAMIGA3RomFileKey"
    static let ext               = "VAMIGA3ExtFileKey"
    static let extStart          = "VAMIGA3ExtStartKey"
}

extension Defaults {
    
    static let rom               = URL(fileURLWithPath: "")
    static let ext               = URL(fileURLWithPath: "")
    static let extStart          = 0xE0
}

extension UserDefaults {
    
    static func registerRomUserDefaults() {
        
        let dictionary: [String: Any] = [
            
            Keys.rom: Defaults.rom,
            Keys.ext: Defaults.ext,
            Keys.extStart: Defaults.extStart
            ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }
    
    static func resetRomUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.rom,
                     Keys.ext,
                     Keys.extStart ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Hardware)
//

extension Keys {
    
    // Chipset
    static let agnusRev           = "VAMIGA4AgnusRevKey"
    static let deniseRev          = "VAMIGA4DeniseRevKey"
    static let realTimeClock      = "VAMIGA4RealTimeClockKey"

    // Memory
    static let chipRam            = "VAMIGA4ChipRamKey"
    static let slowRam            = "VAMIGA4SlowRamKey"
    static let fastRam            = "VAMIGA4FastRamKey"

    // Drives
    static let df0Connect         = "VAMIGA4DF0ConnectKey"
    static let df0Type            = "VAMIGA4DF0TypeKey"
    static let df1Connect         = "VAMIGA4DF1ConnectKey"
    static let df1Type            = "VAMIGA4DF1TypeKey"
    static let df2Connect         = "VAMIGA4DF2ConnectKey"
    static let df2Type            = "VAMIGA4DF2TypeKey"
    static let df3Connect         = "VAMIGA4DF3ConnectKey"
    static let df3Type            = "VAMIGA4DF3TypeKey"

    // Ports
    static let gameDevice1        = "VAMIGA4GameDevice1Key"
    static let gameDevice2        = "VAMIGA4GameDevice2Key"
    static let serialDevice       = "VAMIGA4SerialDeviceKey"
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
    }

    static let A500 = ModelDefaults.init(

        agnusRevision: AGNUS_8372,
        deniseRevision: DENISE_8362R8,
        realTimeClock: RTC_NONE,
        chipRam: 512,
        slowRam: 0,
        fastRam: 0,
        driveConnect: [true, false, false, false],
        driveType: [DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD])

    static let A1000 = ModelDefaults.init(

        agnusRevision: AGNUS_8367,
        deniseRevision: DENISE_8362R8,
        realTimeClock: RTC_NONE,
        chipRam: 256,
        slowRam: 0,
        fastRam: 0,
        driveConnect: [true, false, false, false],
        driveType: [DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD])

    static let A2000 = ModelDefaults.init(
        
        agnusRevision: AGNUS_8375,
        deniseRevision: DENISE_8362R8,
        realTimeClock: RTC_M6242B,
        chipRam: 512,
        slowRam: 512,
        fastRam: 0,
        driveConnect: [true, true, false, false],
        driveType: [DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD])
    
    static let gameDevice1  = 0
    static let gameDevice2  = -1
    static let serialDevice = SPD_NONE
}

extension UserDefaults {
    
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

            Keys.gameDevice1: Defaults.gameDevice1,
            Keys.gameDevice2: Defaults.gameDevice2,
            Keys.serialDevice: Defaults.serialDevice.rawValue
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }
        
    static func resetHardwareUserDefaults() {
        
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

                    Keys.gameDevice1,
                    Keys.gameDevice2,
                    Keys.serialDevice ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Compatibility)
//

extension Keys {

    // Graphics
    static let clxSprSpr         = "VAMIGA5ClxSprSpr"
    static let clxSprPlf         = "VAMIGA5ClxSprPlf"
    static let clxPlfPlf         = "VAMIGA5ClxPlfPlf"

    // Audio
    static let samplingMethod    = "VAMIGA5SamplingMethod"
    static let filterActivation  = "VAMIGA5FilterActivation"
    static let filterType        = "VAMIGA5FilterType"

    // Blitter
    static let blitterAccuracy   = "VAMIGA5BlitterAccuracy"

    // Floppy drives
    static let driveSpeed        = "VAMIGA5DriveSpeedKey"
    static let fifoBuffering     = "VAMIGA5FifoBufferingKey"

    // CIAs
    static let todBug            = "VAMIGA5TodBugKey"
}

extension Defaults {

    // Graphics
    static let clxSprSpr         = false
    static let clxSprPlf         = false
    static let clxPlfPlf         = false

    // Audio
    static let samplingMethod    = SMP_NEAREST
    static let filterActivation  = FILTACT_POWER_LED
    static let filterType        = FILT_BUTTERWORTH

    // Blitter
    static let blitterAccuracy   = 0

    // Floppy drives
    static let driveSpeed        = 1
    static let fifoBuffering     = true
    static let todBug            = true
}

extension UserDefaults {

    /*
    struct CompatibilityScheme {

        let agnusRevision: AgnusRevision
        let deniseRevision: DeniseRevision
        let realTimeClock: RTCModel

        let chipRam: Int
        let slowRam: Int
        let fastRam: Int

        let driveConnect: [Bool]
        let driveType: [DriveType]
    }
    */
    
    static func registerCompatibilityUserDefaults() {

        let dictionary: [String: Any] = [

            Keys.clxSprSpr: Defaults.clxSprSpr,
            Keys.clxSprPlf: Defaults.clxSprPlf,
            Keys.clxPlfPlf: Defaults.clxPlfPlf,
            Keys.samplingMethod: Defaults.samplingMethod.rawValue,
            Keys.filterActivation: Defaults.filterActivation.rawValue,
            Keys.filterType: Defaults.filterType.rawValue,
            Keys.blitterAccuracy: Defaults.blitterAccuracy,
            Keys.driveSpeed: Defaults.driveSpeed,
            Keys.fifoBuffering: Defaults.fifoBuffering,
            Keys.todBug: Defaults.todBug
        ]

        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
    }

    static func resetCompatibilityUserDefaults() {

        let defaults = UserDefaults.standard

        let keys = [ Keys.clxSprSpr,
                     Keys.clxSprPlf,
                     Keys.clxPlfPlf,
                     Keys.samplingMethod,
                     Keys.filterActivation,
                     Keys.filterType,
                     Keys.blitterAccuracy,
                     Keys.driveSpeed,
                     Keys.fifoBuffering,
                     Keys.todBug]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Video)
//

extension Keys {
    
    // Colors
    static let palette         = "VAMIGA6PaletteKey"
    static let brightness      = "VAMIGA6BrightnessKey"
    static let contrast        = "VAMIGA6ContrastKey"
    static let saturation      = "VAMIGA6SaturationKey"

    // Geometry
    static let hCenter         = "VAMIGA6HCenter"
    static let vCenter         = "VAMIGA6VCenter"
    static let hZoom           = "VAMIGA6HZoom"
    static let vZoom           = "VAMIGA6VZoom"

    // Upscalers
    static let enhancer        = "VVAMIG6AEnhancerKey"
    static let upscaler        = "VAMIGA6UpscalerKey"

    // GPU options
    static let shaderOptions   = "VAMIGA6ShaderOptionsKey"
}

extension Defaults {
    
    static let palette = COLOR_PALETTE
    static let brightness = Double(50.0)
    static let contrast = Double(100.0)
    static let saturation = Double(50.0)
    
    // Geometry
    static let hCenter = Float(0.1169)
    static let vCenter = Float(0.1683)
    static let hZoom   = Float(0.0454)
    static let vZoom   = Float(0.0349)

    // Upscalers
    static let enhancer = 0
    static let upscaler = 0

    // GPU options
    static let shaderOptions = ShaderDefaultsTFT
}

extension UserDefaults {
    
    static func registerVideoUserDefaults() {
        
        let dictionary: [String: Any] = [
            
            Keys.palette: Int(Defaults.palette.rawValue),
            Keys.brightness: Defaults.brightness,
            Keys.contrast: Defaults.contrast,
            Keys.saturation: Defaults.saturation,

            Keys.hCenter: Defaults.hCenter,
            Keys.vCenter: Defaults.vCenter,
            Keys.hZoom: Defaults.hZoom,
            Keys.vZoom: Defaults.vZoom,

            Keys.enhancer: Defaults.enhancer,
            Keys.upscaler: Defaults.upscaler
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
        defaults.register(encodableItem: Defaults.shaderOptions, forKey: Keys.shaderOptions)
    }

    static func resetVideoUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.palette,
                     Keys.brightness,
                     Keys.contrast,
                     Keys.saturation,
                     
                     Keys.hCenter,
                     Keys.vCenter,
                     Keys.hZoom,
                     Keys.vZoom,

                     Keys.enhancer,
                     Keys.upscaler,

                     Keys.shaderOptions ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}
