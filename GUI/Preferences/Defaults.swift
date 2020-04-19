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
        config.loadHardwareUserDefaults()
        config.loadCompatibilityUserDefaults()
        config.loadVideoUserDefaults()

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

struct GeneralDefaults {
    
    // Drives
    let warpLoad: Bool
    let driveSounds: Bool
    let driveSoundPan: Double
    let driveInsertSound: Bool
    let driveEjectSound: Bool
    let driveHeadSound: Bool
    let drivePollSound: Bool
    let driveBlankDiskFormat: FileSystemType
    
    // Snapshots and Screenshots
    let autoSnapshots: Bool
    let autoSnapshotInterval: Int
    let autoScreenshots: Bool
    let autoScreenshotInterval: Int
    let screenshotSource: Int
    let screenshotTarget: NSBitmapImageRep.FileType
    
    // Fullscreen
    let keepAspectRatio: Bool
    let exitOnEsc: Bool
    
    // Miscellaneous
    let pauseInBackground: Bool
    let closeWithoutAsking: Bool
    let ejectWithoutAsking: Bool

    //
    // Schemes
    //
    
    static let std = GeneralDefaults.init(
        
        warpLoad: true,
        driveSounds: true,
        driveSoundPan: 1.0,
        driveInsertSound: true,
        driveEjectSound: true,
        driveHeadSound: true,
        drivePollSound: false,
        driveBlankDiskFormat: FS_OFS,
        
        autoSnapshots: false,
        autoSnapshotInterval: 20,
        autoScreenshots: false,
        autoScreenshotInterval: 10,
        screenshotSource: 0,
        screenshotTarget: .png,
        
        keepAspectRatio: false,
        exitOnEsc: true,
        
        pauseInBackground: false,
        closeWithoutAsking: false,
        ejectWithoutAsking: false
    )
}

extension UserDefaults {
    
    static func registerGeneralUserDefaults() {
    
        let defaults = GeneralDefaults.std
        let dictionary: [String: Any] = [
            
            Keys.warpLoad: defaults.warpLoad,
            Keys.driveSounds: defaults.driveSounds,
            Keys.driveSoundPan: defaults.driveSoundPan,
            Keys.driveInsertSound: defaults.driveInsertSound,
            Keys.driveEjectSound: defaults.driveEjectSound,
            Keys.driveHeadSound: defaults.driveHeadSound,
            Keys.drivePollSound: defaults.drivePollSound,
            Keys.driveBlankDiskFormat: Int(defaults.driveBlankDiskFormat.rawValue),

            Keys.autoSnapshots: defaults.autoSnapshots,
            Keys.autoSnapshotInterval: defaults.autoSnapshotInterval,
            Keys.autoScreenshots: defaults.autoScreenshots,
            Keys.autoScreenshotInterval: defaults.autoScreenshotInterval,
            Keys.screenshotSource: defaults.screenshotSource,
            Keys.screenshotTarget: Int(defaults.screenshotTarget.rawValue),

            Keys.keepAspectRatio: defaults.keepAspectRatio,
            Keys.exitOnEsc: defaults.exitOnEsc,
            
            Keys.pauseInBackground: defaults.pauseInBackground,
            Keys.closeWithoutAsking: defaults.closeWithoutAsking,
            Keys.ejectWithoutAsking: defaults.ejectWithoutAsking
        ]
        
        let userDefaults = UserDefaults.standard
        
        userDefaults.register(defaults: dictionary)
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

struct DevicesDefaults {
    
    // Emulation keys
    let joyKeyMap1: [MacKey: UInt32]
    let joyKeyMap2: [MacKey: UInt32]
    let mouseKeyMap: [MacKey: UInt32]
    let disconnectJoyKeys: Bool
    
    // Joysticks
    let autofire: Bool
    let autofireBullets: Int
    let autofireFrequency: Float
    
    // Mouse
    let retainMouseKeyComb: Int
    let retainMouseWithKeys: Bool
    let retainMouseByClick: Bool
    let retainMouseByEntering: Bool
    let releaseMouseKeyComb: Int
    let releaseMouseWithKeys: Bool
    let releaseMouseByShaking: Bool
    
    //
    // Schemes
    //
    
    static let stdKeyMap1 = [
        
        MacKey.init(keyCode: kVK_LeftArrow): PULL_LEFT.rawValue,
        MacKey.init(keyCode: kVK_RightArrow): PULL_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_UpArrow): PULL_UP.rawValue,
        MacKey.init(keyCode: kVK_DownArrow): PULL_DOWN.rawValue,
        MacKey.init(keyCode: kVK_Space): PRESS_FIRE.rawValue
    ]
    static let stdKeyMap2 = [
        
        MacKey.init(keyCode: kVK_ANSI_S): PULL_LEFT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_D): PULL_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_E): PULL_UP.rawValue,
        MacKey.init(keyCode: kVK_ANSI_X): PULL_DOWN.rawValue,
        MacKey.init(keyCode: kVK_ANSI_C): PRESS_FIRE.rawValue
    ]
    
    static let std = DevicesDefaults.init(
        
        joyKeyMap1: stdKeyMap1,
        joyKeyMap2: stdKeyMap2,
        mouseKeyMap: [:],
        disconnectJoyKeys: true,
        
        autofire: false,
        autofireBullets: -3,
        autofireFrequency: 2.5,
        
        retainMouseKeyComb: 0,
        retainMouseWithKeys: true,
        retainMouseByClick: true,
        retainMouseByEntering: false,
        releaseMouseKeyComb: 0,
        releaseMouseWithKeys: true,
        releaseMouseByShaking: true
    )
}

extension UserDefaults {
    
    static func registerDevicesUserDefaults() {
        
        let defaults = DevicesDefaults.std
        let dictionary: [String: Any] = [

            // Emulation keys
            Keys.disconnectJoyKeys: defaults.disconnectJoyKeys,

            // Joysticks
            Keys.autofire: defaults.autofire,
            Keys.autofireBullets: defaults.autofireBullets,
            Keys.autofireFrequency: defaults.autofireFrequency,
            
            // Mouse
            Keys.retainMouseKeyComb: defaults.retainMouseKeyComb,
            Keys.retainMouseWithKeys: defaults.retainMouseWithKeys,
            Keys.retainMouseByClick: defaults.retainMouseByClick,
            Keys.retainMouseByEntering: defaults.retainMouseByEntering,
            Keys.releaseMouseKeyComb: defaults.releaseMouseKeyComb,
            Keys.releaseMouseWithKeys: defaults.releaseMouseWithKeys,
            Keys.releaseMouseByShaking: defaults.releaseMouseByShaking
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
        userDefaults.register(encodableItem: defaults.joyKeyMap1, forKey: Keys.joyKeyMap1)
        userDefaults.register(encodableItem: defaults.joyKeyMap2, forKey: Keys.joyKeyMap2)
        userDefaults.register(encodableItem: defaults.mouseKeyMap, forKey: Keys.mouseKeyMap)
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

struct RomDefaults {
    
    let rom: URL
    let ext: URL
    let extStart: Int
    
    static let std = RomDefaults.init(
        
        rom: URL(fileURLWithPath: ""),
        ext: URL(fileURLWithPath: ""),
        extStart: 0xE0
    )
}

extension UserDefaults {
    
    static func registerRomUserDefaults() {
        
        let defaults = RomDefaults.std
        let dictionary: [String: Any] = [
            
            Keys.rom: defaults.rom,
            Keys.ext: defaults.ext,
            Keys.extStart: defaults.extStart
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
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

struct HardwareDefaults {
    
    var agnusRevision: AgnusRevision
    var deniseRevision: DeniseRevision
    var realTimeClock: RTCModel
    
    var chipRam: Int
    var slowRam: Int
    var fastRam: Int
    
    var driveConnect: [Bool]
    var driveType: [DriveType]
    
    var gameDevice1: Int
    var gameDevice2: Int
    var serialDevice: SerialPortDevice
    
    //
    // Schemes
    //
    
    static let A500 = HardwareDefaults.init(
        
        agnusRevision: AGNUS_8372,
        deniseRevision: DENISE_8362R8,
        realTimeClock: RTC_NONE,
        
        chipRam: 512,
        slowRam: 0,
        fastRam: 0,
        
        driveConnect: [true, false, false, false],
        driveType: [DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD],
        
        gameDevice1: 0,
        gameDevice2: -1,
        serialDevice: SPD_NONE
    )
    
    static let A1000 = HardwareDefaults.init(
        
        agnusRevision: AGNUS_8367,
        deniseRevision: DENISE_8362R8,
        realTimeClock: RTC_NONE,
        
        chipRam: 256,
        slowRam: 0,
        fastRam: 0,
        
        driveConnect: [true, false, false, false],
        driveType: [DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD],
        
        gameDevice1: 0,
        gameDevice2: -1,
        serialDevice: SPD_NONE
    )
    
    static let A2000 = HardwareDefaults.init(
        
        agnusRevision: AGNUS_8375,
        deniseRevision: DENISE_8362R8,
        realTimeClock: RTC_M6242B,
        
        chipRam: 512,
        slowRam: 512,
        fastRam: 0,
        
        driveConnect: [true, true, false, false],
        driveType: [DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD, DRIVE_35_DD],
        
        gameDevice1: 0,
        gameDevice2: -1,
        serialDevice: SPD_NONE
    )
}

extension UserDefaults {
    
    static func registerHardwareUserDefaults() {
        
        let defaults = HardwareDefaults.A500
        let dictionary: [String: Any] = [
            
            Keys.agnusRev: defaults.agnusRevision.rawValue,
            Keys.deniseRev: defaults.deniseRevision.rawValue,
            Keys.realTimeClock: defaults.realTimeClock.rawValue,

            Keys.chipRam: defaults.chipRam,
            Keys.slowRam: defaults.slowRam,
            Keys.fastRam: defaults.fastRam,

            Keys.df0Connect: defaults.driveConnect[0],
            Keys.df0Type: defaults.driveType[0].rawValue,
            Keys.df1Connect: defaults.driveConnect[1],
            Keys.df1Type: defaults.driveType[1].rawValue,
            Keys.df2Connect: defaults.driveConnect[2],
            Keys.df2Type: defaults.driveType[2].rawValue,
            Keys.df3Connect: defaults.driveConnect[3],
            Keys.df3Type: defaults.driveType[3].rawValue,

            Keys.gameDevice1: defaults.gameDevice1,
            Keys.gameDevice2: defaults.gameDevice2,
            Keys.serialDevice: defaults.serialDevice.rawValue
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
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

struct CompatibilityDefaults {
    
    // Graphics
    let clxSprSpr: Bool
    let clxSprPlf: Bool
    let clxPlfPlf: Bool
    
    // Audio
    let samplingMethod: SamplingMethod
    let filterActivation: FilterActivation
    let filterType: FilterType
    
    // Blitter
    let blitterAccuracy: Int
    
    // Floppy drives
    let driveSpeed: Int
    let fifoBuffering: Bool
    
    // CIAs
    let todBug: Bool
    
    //
    // Schemes
    //
    
    static let std = CompatibilityDefaults.init(
         
         clxSprSpr: false,
         clxSprPlf: false,
         clxPlfPlf: false,
         
         samplingMethod: SMP_NEAREST,
         filterActivation: FILTACT_POWER_LED,
         filterType: FILT_BUTTERWORTH,
         
         blitterAccuracy: 2,
         
         driveSpeed: 1,
         fifoBuffering: true,
         
         todBug: true
     )
    
    static let accurate = CompatibilityDefaults.init(
        
        clxSprSpr: true,
        clxSprPlf: true,
        clxPlfPlf: true,
        
        samplingMethod: SMP_LINEAR,
        filterActivation: FILTACT_POWER_LED,
        filterType: FILT_BUTTERWORTH,
        
        blitterAccuracy: 2,
        
        driveSpeed: 1,
        fifoBuffering: true,
        
        todBug: true
    )

    static let accelerated = CompatibilityDefaults.init(
        
        clxSprSpr: false,
        clxSprPlf: false,
        clxPlfPlf: false,
        
        samplingMethod: SMP_NONE,
        filterActivation: FILTACT_NEVER,
        filterType: FILT_BUTTERWORTH,
        
        blitterAccuracy: 0,
        
        driveSpeed: -1,
        fifoBuffering: false,
        
        todBug: true
    )
}

extension UserDefaults {

    static func registerCompatibilityUserDefaults() {

        let defaults = CompatibilityDefaults.std
        let dictionary: [String: Any] = [

            Keys.clxSprSpr: defaults.clxSprSpr,
            Keys.clxSprPlf: defaults.clxSprPlf,
            Keys.clxPlfPlf: defaults.clxPlfPlf,
            Keys.samplingMethod: defaults.samplingMethod.rawValue,
            Keys.filterActivation: defaults.filterActivation.rawValue,
            Keys.filterType: defaults.filterType.rawValue,
            Keys.blitterAccuracy: defaults.blitterAccuracy,
            Keys.driveSpeed: defaults.driveSpeed,
            Keys.fifoBuffering: defaults.fifoBuffering,
            Keys.todBug: defaults.todBug
        ]

        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetCompatibilityUserDefaults() {

        let userDefaults = UserDefaults.standard

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

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Video)
//

extension Keys {
    
    // Colors
    static let palette            = "VAMIGA6PaletteKey"
    static let brightness         = "VAMIGA6BrightnessKey"
    static let contrast           = "VAMIGA6ContrastKey"
    static let saturation         = "VAMIGA6SaturationKey"

    // Geometry
    static let hCenter            = "VAMIGA6HCenter"
    static let vCenter            = "VAMIGA6VCenter"
    static let hZoom              = "VAMIGA6HZoom"
    static let vZoom              = "VAMIGA6VZoom"

    // Upscalers
    static let enhancer           = "VVAMIG6AEnhancerKey"
    static let upscaler           = "VAMIGA6UpscalerKey"

    // Shader options
    static let blur               = "VAMIGA6Blur"
    static let blurRadius         = "VAMIGA6BlurRadius"
    static let bloom              = "VAMIGA6Bloom"
    static let bloomRadius        = "VAMIGA6BloonRadius"
    static let bloomBrightness    = "VAMIGA6BloomBrightness"
    static let bloomWeight        = "VAMIGA6BloomWeight"
    static let flicker            = "VAMIGA6Flicker"
    static let flickerWeight      = "VAMIGA6FlickerWeight"
    static let dotMask            = "VAMIGA6DotMask"
    static let dotMaskBrightness  = "VAMIGA6DotMaskBrightness"
    static let scanlines          = "VAMIGA6Scanlines"
    static let scanlineBrightness = "VAMIGA6ScanlineBrightness"
    static let scanlineWeight     = "VAMIGA6ScanlineWeight"
    static let disalignment       = "VAMIGA6Disalignment"
    static let disalignmentH      = "VAMIGA6DisalignmentH"
    static let disalignmentV      = "VAMIGA6DisalignmentV"
}

struct VideoDefaults {
    
    // Colors
    let palette: Palette
    let brightness: Double
    let contrast: Double
    let saturation: Double
    
    // Geometry
    let hCenter: Float
    let vCenter: Float
    let hZoom: Float
    let vZoom: Float
    
    // Upscalers
    let enhancer: Int
    let upscaler: Int
    
    // Shader options
    let blur: Int32
    let blurRadius: Float
    let bloom: Int32
    let bloomRadius: Float
    let bloomBrightness: Float
    let bloomWeight: Float
    let flicker: Int32
    let flickerWeight: Float
    let dotMask: Int32
    let dotMaskBrightness: Float
    let scanlines: Int32
    let scanlineBrightness: Float
    let scanlineWeight: Float
    let disalignment: Int32
    let disalignmentH: Float
    let disalignmentV: Float
    
    //
    // Schemes
    //
    
    // TFT Monior appearance with a texture cutout similar to UAE
    static let tft = VideoDefaults.init(
        
        palette: COLOR_PALETTE,
        brightness: 50.0,
        contrast: 100.0,
        saturation: 50.0,
        
        hCenter: 0.1169,
        vCenter: 0.1683,
        hZoom: 0.0454,
        vZoom: 0.0349,
        
        enhancer: 0,
        upscaler: 0,
        
        blur: 1,
        blurRadius: 0,
        bloom: 0,
        bloomRadius: 1.0,
        bloomBrightness: 0.4,
        bloomWeight: 1.21,
        flicker: 1,
        flickerWeight: 0.5,
        dotMask: 0,
        dotMaskBrightness: 0.7,
        scanlines: 0,
        scanlineBrightness: 0.55,
        scanlineWeight: 0.11,
        disalignment: 0,
        disalignmentH: 0.001,
        disalignmentV: 0.001
    )
    
    // CRT monitor appearance with a texture-cutout closer to the center
    static let crt = VideoDefaults.init(
        
        palette: COLOR_PALETTE,
        brightness: 50.0,
        contrast: 100.0,
        saturation: 50.0,
        
        hCenter: 0.6333,
        vCenter: 0.1683,
        hZoom: 0.0454,
        vZoom: 0.0349,
        
        enhancer: 0,
        upscaler: 0,
        
        blur: 1,
        blurRadius: 1.5,
        bloom: 1,
        bloomRadius: 1.0,
        bloomBrightness: 0.4,
        bloomWeight: 1.21,
        flicker: 1,
        flickerWeight: 0.5,
        dotMask: 1,
        dotMaskBrightness: 0.5,
        scanlines: 2,
        scanlineBrightness: 0.55,
        scanlineWeight: 0.11,
        disalignment: 0,
        disalignmentH: 0.001,
        disalignmentV: 0.001
    )
}

extension UserDefaults {
    
    static func registerVideoUserDefaults() {
        
        let defaults = VideoDefaults.tft
        let dictionary: [String: Any] = [
            
            Keys.palette: Int(defaults.palette.rawValue),
            Keys.brightness: defaults.brightness,
            Keys.contrast: defaults.contrast,
            Keys.saturation: defaults.saturation,

            Keys.hCenter: defaults.hCenter,
            Keys.vCenter: defaults.vCenter,
            Keys.hZoom: defaults.hZoom,
            Keys.vZoom: defaults.vZoom,

            Keys.enhancer: defaults.enhancer,
            Keys.upscaler: defaults.upscaler,
            
            Keys.blur: defaults.blur,
            Keys.blurRadius: defaults.blurRadius,
            Keys.bloom: defaults.bloom,
            Keys.bloomRadius: defaults.bloomRadius,
            Keys.bloomBrightness: defaults.bloomBrightness,
            Keys.bloomWeight: defaults.bloomWeight,
            Keys.flicker: defaults.flicker,
            Keys.flickerWeight: defaults.flickerWeight,
            Keys.dotMask: defaults.dotMask,
            Keys.dotMaskBrightness: defaults.dotMaskBrightness,
            Keys.scanlines: defaults.scanlines,
            Keys.scanlineBrightness: defaults.scanlineBrightness,
            Keys.scanlineWeight: defaults.scanlineWeight,
            Keys.disalignment: defaults.disalignment,
            Keys.disalignmentH: defaults.disalignmentH,
            Keys.disalignmentV: defaults.disalignmentV
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
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

                     Keys.blur,
                     Keys.blurRadius,
                     Keys.bloom,
                     Keys.bloomRadius,
                     Keys.bloomBrightness,
                     Keys.bloomWeight,
                     Keys.flicker,
                     Keys.flickerWeight,
                     Keys.dotMask,
                     Keys.dotMaskBrightness,
                     Keys.scanlines,
                     Keys.scanlineBrightness,
                     Keys.scanlineWeight,
                     Keys.disalignment,
                     Keys.disalignmentH,
                     Keys.disalignmentV ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}
