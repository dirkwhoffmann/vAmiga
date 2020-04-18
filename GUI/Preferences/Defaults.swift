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
    }
    
    static let general = GeneralDefaults.init(
        
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
        ejectWithoutAsking: false)
}

extension UserDefaults {
    
    static func registerGeneralUserDefaults() {
        
        let dictionary: [String: Any] = [
            
            Keys.warpLoad: Defaults.general.warpLoad,
            Keys.driveSounds: Defaults.general.driveSounds,
            Keys.driveSoundPan: Defaults.general.driveSoundPan,
            Keys.driveInsertSound: Defaults.general.driveInsertSound,
            Keys.driveEjectSound: Defaults.general.driveEjectSound,
            Keys.driveHeadSound: Defaults.general.driveHeadSound,
            Keys.drivePollSound: Defaults.general.drivePollSound,
            Keys.driveBlankDiskFormat: Int(Defaults.general.driveBlankDiskFormat.rawValue),

            Keys.autoSnapshots: Defaults.general.autoSnapshots,
            Keys.autoSnapshotInterval: Defaults.general.autoSnapshotInterval,
            Keys.autoScreenshots: Defaults.general.autoScreenshots,
            Keys.autoScreenshotInterval: Defaults.general.autoScreenshotInterval,
            Keys.screenshotSource: Defaults.general.screenshotSource,
            Keys.screenshotTarget: Int(Defaults.general.screenshotTarget.rawValue),

            Keys.keepAspectRatio: Defaults.general.keepAspectRatio,
            Keys.exitOnEsc: Defaults.general.exitOnEsc,
            
            Keys.pauseInBackground: Defaults.general.pauseInBackground,
            Keys.closeWithoutAsking: Defaults.general.closeWithoutAsking,
            Keys.ejectWithoutAsking: Defaults.general.ejectWithoutAsking
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
    }
    
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
    
    static let devices = DevicesDefaults.init(
        
        joyKeyMap1: joyKeyMap1,
        joyKeyMap2: joyKeyMap2,
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
        
        let dictionary: [String: Any] = [

            // Emulation keys
            Keys.disconnectJoyKeys: Defaults.devices.disconnectJoyKeys,

            // Joysticks
            Keys.autofire: Defaults.devices.autofire,
            Keys.autofireBullets: Defaults.devices.autofireBullets,
            Keys.autofireFrequency: Defaults.devices.autofireFrequency,
            
            // Mouse
            Keys.retainMouseKeyComb: Defaults.devices.retainMouseKeyComb,
            Keys.retainMouseWithKeys: Defaults.devices.retainMouseWithKeys,
            Keys.retainMouseByClick: Defaults.devices.retainMouseByClick,
            Keys.retainMouseByEntering: Defaults.devices.retainMouseByEntering,
            Keys.releaseMouseKeyComb: Defaults.devices.releaseMouseKeyComb,
            Keys.releaseMouseWithKeys: Defaults.devices.releaseMouseWithKeys,
            Keys.releaseMouseByShaking: Defaults.devices.releaseMouseByShaking
        ]
        
        let defaults = UserDefaults.standard
        defaults.register(defaults: dictionary)
        defaults.register(encodableItem: Defaults.devices.joyKeyMap1, forKey: Keys.joyKeyMap1)
        defaults.register(encodableItem: Defaults.devices.joyKeyMap2, forKey: Keys.joyKeyMap2)
        defaults.register(encodableItem: Defaults.devices.mouseKeyMap, forKey: Keys.mouseKeyMap)
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
    
    struct RomDefaults {
        
        let rom: URL
        let ext: URL
        let extStart: Int
    }
    
    static let rom = RomDefaults.init(
        
        rom: URL(fileURLWithPath: ""),
        ext: URL(fileURLWithPath: ""),
        extStart: 0xE0
    )
}

extension UserDefaults {
    
    static func registerRomUserDefaults() {
        
        let dictionary: [String: Any] = [
            
            Keys.rom: Defaults.rom.rom,
            Keys.ext: Defaults.rom.ext,
            Keys.extStart: Defaults.rom.extStart
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

    struct HwDefaults {

        let agnusRevision: AgnusRevision
        let deniseRevision: DeniseRevision
        let realTimeClock: RTCModel

        let chipRam: Int
        let slowRam: Int
        let fastRam: Int

        let driveConnect: [Bool]
        let driveType: [DriveType]
        
        let gameDevice1: Int
        let gameDevice2: Int
        let serialDevice: SerialPortDevice
    }

    static let A500 = HwDefaults.init(

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

    static let A1000 = HwDefaults.init(

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

    static let A2000 = HwDefaults.init(
        
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

            Keys.gameDevice1: defaultModel.gameDevice1,
            Keys.gameDevice2: defaultModel.gameDevice2,
            Keys.serialDevice: defaultModel.serialDevice.rawValue
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

struct CompDefaults {
    
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
    
    static let accurate = CompDefaults.init(
        
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
    
    static let std = CompDefaults.init(
        
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
    
    static let accelerated = CompDefaults.init(
        
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

        let defaults = CompDefaults.std
        
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
