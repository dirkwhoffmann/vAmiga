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

extension MyController {
    
    static func registerUserDefaults() {
        
        track()
        
        registerRomUserDefaults()
        registerDevicesUserDefaults()
        registerVideoUserDefaults()
        registerGeneralUserDefaults()
        registerHardwareUserDefaults()
        registerCompatibilityUserDefaults()
    }
    
    func resetUserDefaults() {
        
        track()
        
        amiga.suspend()
        
        resetRomUserDefaults()
        resetDevicesUserDefaults()
        resetVideoUserDefaults()
        resetGeneralUserDefaults()
        resetHardwareUserDefaults()
        resetCompatibilityUserDefaults()
        
        amiga.resume()
    }
    
    func loadUserDefaults() {
        
        track()
        
        amiga.suspend()
        
        loadRomUserDefaults()
        loadDevicesUserDefaults()
        loadVideoUserDefaults()
        loadGeneralUserDefaults()
        loadHardwareUserDefaults()
        loadCompatibilityUserDefaults()
        
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
        
        saveRomUserDefaults()
        saveDevicesUserDefaults()
        saveVideoUserDefaults()
        saveGeneralUserDefaults()
        saveHardwareUserDefaults()
        saveCompatibilityUserDefaults()
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

extension MyController {
    
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

    func resetGeneralUserDefaults() {
        
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
        
        loadGeneralUserDefaults()
    }
    
    func loadGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        prefs.warpLoad = defaults.bool(forKey: Keys.warpLoad)
        prefs.driveSounds = defaults.bool(forKey: Keys.driveSounds)
        prefs.driveSoundPan = defaults.double(forKey: Keys.driveSoundPan)
        prefs.driveInsertSound = defaults.bool(forKey: Keys.driveInsertSound)
        prefs.driveEjectSound = defaults.bool(forKey: Keys.driveEjectSound)
        prefs.driveHeadSound = defaults.bool(forKey: Keys.driveHeadSound)
        prefs.drivePollSound = defaults.bool(forKey: Keys.drivePollSound)
        prefs.driveBlankDiskFormatIntValue = defaults.integer(forKey: Keys.driveBlankDiskFormat)
        
        prefs.autoSnapshots = defaults.bool(forKey: Keys.autoSnapshots)
        prefs.snapshotInterval = defaults.integer(forKey: Keys.autoSnapshotInterval)
        prefs.autoScreenshots = defaults.bool(forKey: Keys.autoScreenshots)
        prefs.screenshotInterval = defaults.integer(forKey: Keys.autoScreenshotInterval)
        prefs.screenshotSource = defaults.integer(forKey: Keys.screenshotSource)
        prefs.screenshotTargetIntValue = defaults.integer(forKey: Keys.screenshotTarget)
    
        prefs.keepAspectRatio = defaults.bool(forKey: Keys.keepAspectRatio)
        prefs.exitOnEsc = defaults.bool(forKey: Keys.exitOnEsc)
        
        prefs.pauseInBackground = defaults.bool(forKey: Keys.pauseInBackground)
        prefs.closeWithoutAsking = defaults.bool(forKey: Keys.closeWithoutAsking)
        prefs.ejectWithoutAsking = defaults.bool(forKey: Keys.ejectWithoutAsking)
        
        amiga.resume()
    }
    
    func saveGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(prefs.warpLoad, forKey: Keys.warpLoad)
        defaults.set(prefs.driveSounds, forKey: Keys.driveSounds)
        defaults.set(prefs.driveSoundPan, forKey: Keys.driveSoundPan)
        defaults.set(prefs.driveInsertSound, forKey: Keys.driveInsertSound)
        defaults.set(prefs.driveEjectSound, forKey: Keys.driveEjectSound)
        defaults.set(prefs.driveHeadSound, forKey: Keys.driveHeadSound)
        defaults.set(prefs.drivePollSound, forKey: Keys.drivePollSound)
        defaults.set(prefs.driveBlankDiskFormatIntValue, forKey: Keys.driveBlankDiskFormat)
        
        defaults.set(prefs.autoSnapshots, forKey: Keys.autoSnapshots)
        defaults.set(prefs.snapshotInterval, forKey: Keys.autoSnapshotInterval)
        defaults.set(prefs.autoScreenshots, forKey: Keys.autoScreenshots)
        defaults.set(prefs.screenshotInterval, forKey: Keys.autoScreenshotInterval)
        defaults.set(prefs.screenshotSource, forKey: Keys.screenshotSource)
        defaults.set(prefs.screenshotTargetIntValue, forKey: Keys.screenshotTarget)
        
        defaults.set(prefs.keepAspectRatio, forKey: Keys.keepAspectRatio)
        defaults.set(prefs.exitOnEsc, forKey: Keys.exitOnEsc)
                
        defaults.set(prefs.pauseInBackground, forKey: Keys.pauseInBackground)
        defaults.set(prefs.closeWithoutAsking, forKey: Keys.closeWithoutAsking)
        defaults.set(prefs.ejectWithoutAsking, forKey: Keys.ejectWithoutAsking)
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
    
extension MyController {
    
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

    func resetDevicesUserDefaults() {
        
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
        
        loadDevicesUserDefaults()
    }
    
    func loadDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard
    
        amiga.suspend()

        // Emulation keys
        defaults.decode(&prefs.keyMaps[0], forKey: Keys.joyKeyMap1)
        defaults.decode(&prefs.keyMaps[1], forKey: Keys.joyKeyMap2)
        defaults.decode(&prefs.keyMaps[2], forKey: Keys.mouseKeyMap)
        prefs.disconnectJoyKeys = defaults.bool(forKey: Keys.disconnectJoyKeys)

        // Joysticks
        prefs.autofire = defaults.bool(forKey: Keys.autofire)
        prefs.autofireBullets = defaults.integer(forKey: Keys.autofireBullets)
        prefs.autofireFrequency = defaults.float(forKey: Keys.autofireFrequency)
 
        // Mouse
        prefs.retainMouseKeyComb = defaults.integer(forKey: Keys.retainMouseKeyComb)
        prefs.retainMouseWithKeys = defaults.bool(forKey: Keys.retainMouseWithKeys)
        prefs.retainMouseByClick = defaults.bool(forKey: Keys.retainMouseByClick)
        prefs.retainMouseByEntering = defaults.bool(forKey: Keys.retainMouseByEntering)
        prefs.releaseMouseKeyComb = defaults.integer(forKey: Keys.releaseMouseKeyComb)
        prefs.releaseMouseWithKeys = defaults.bool(forKey: Keys.releaseMouseWithKeys)
        prefs.releaseMouseByShaking = defaults.bool(forKey: Keys.releaseMouseByShaking)
        
        amiga.resume()
    }
    
    func saveDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard

        // Emulation keys
        defaults.encode(prefs.keyMaps[0], forKey: Keys.joyKeyMap1)
        defaults.encode(prefs.keyMaps[1], forKey: Keys.joyKeyMap2)
        defaults.encode(prefs.keyMaps[2], forKey: Keys.mouseKeyMap)
        defaults.set(prefs.disconnectJoyKeys, forKey: Keys.disconnectJoyKeys)

        // Joysticks
        defaults.set(prefs.autofire, forKey: Keys.autofire)
        defaults.set(prefs.autofireBullets, forKey: Keys.autofireBullets)
        defaults.set(prefs.autofireFrequency, forKey: Keys.autofireFrequency)
        
        // Mouse
        defaults.set(prefs.retainMouseKeyComb, forKey: Keys.retainMouseKeyComb)
        defaults.set(prefs.retainMouseWithKeys, forKey: Keys.retainMouseWithKeys)
        defaults.set(prefs.retainMouseByClick, forKey: Keys.retainMouseByClick)
        defaults.set(prefs.retainMouseByEntering, forKey: Keys.retainMouseByEntering)
        defaults.set(prefs.releaseMouseKeyComb, forKey: Keys.releaseMouseKeyComb)
        defaults.set(prefs.releaseMouseWithKeys, forKey: Keys.releaseMouseWithKeys)
        defaults.set(prefs.releaseMouseByShaking, forKey: Keys.releaseMouseByShaking)
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
            config.romURL = url
            amiga.mem.loadRom(fromFile: config.romURL)
        }
        if let url = defaults.url(forKey: Keys.ext) {
            config.extURL = url
            amiga.mem.loadExt(fromFile: config.extURL)
        }
        config.extStart = defaults.integer(forKey: Keys.extStart)

        amiga.resume()
    }
    
    func saveRomUserDefaults() {
        
        let hwconfig = amiga.config()
        let defaults = UserDefaults.standard

        defaults.set(config.romURL, forKey: Keys.rom)
        defaults.set(config.extURL, forKey: Keys.ext)
        defaults.set(hwconfig.mem.extStart, forKey: Keys.extStart)
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

            Keys.gameDevice1: Defaults.gameDevice1,
            Keys.gameDevice2: Defaults.gameDevice2,
            Keys.serialDevice: Defaults.serialDevice.rawValue
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

                    Keys.gameDevice1,
                    Keys.gameDevice2,
                    Keys.serialDevice ]

        for key in keys { defaults.removeObject(forKey: key) }
        
        loadHardwareUserDefaults()
    }
    
    func loadHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        config.agnusRev = defaults.integer(forKey: Keys.agnusRev)
        config.deniseRev = defaults.integer(forKey: Keys.deniseRev)
        config.rtClock = defaults.integer(forKey: Keys.realTimeClock)

        config.chipRam = defaults.integer(forKey: Keys.chipRam)
        config.slowRam = defaults.integer(forKey: Keys.slowRam)
        config.fastRam = defaults.integer(forKey: Keys.fastRam)

        track("chip = \(defaults.integer(forKey: Keys.chipRam))")
        
        config.driveSpeed = defaults.integer(forKey: Keys.driveSpeed)
        config.df0Connected = defaults.bool(forKey: Keys.df0Connect)
        config.df1Connected = defaults.bool(forKey: Keys.df1Connect)
        config.df2Connected = defaults.bool(forKey: Keys.df2Connect)
        config.df3Connected = defaults.bool(forKey: Keys.df3Connect)
        config.df0Type = defaults.integer(forKey: Keys.df0Type)
        config.df1Type = defaults.integer(forKey: Keys.df1Type)
        config.df2Type = defaults.integer(forKey: Keys.df2Type)
        config.df3Type = defaults.integer(forKey: Keys.df3Type)

        config.gameDevice1 = defaults.integer(forKey: Keys.gameDevice1)
        config.gameDevice2 = defaults.integer(forKey: Keys.gameDevice2)
        config.serialDevice = defaults.integer(forKey: Keys.serialDevice)

        amiga.resume()
    }

    func saveHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard

        defaults.set(config.agnusRev, forKey: Keys.agnusRev)
        defaults.set(config.deniseRev, forKey: Keys.deniseRev)
        defaults.set(config.rtClock, forKey: Keys.realTimeClock)

        defaults.set(config.chipRam, forKey: Keys.chipRam)
        defaults.set(config.slowRam, forKey: Keys.slowRam)
        defaults.set(config.fastRam, forKey: Keys.fastRam)

        defaults.set(config.driveSpeed, forKey: Keys.driveSpeed)
        defaults.set(config.df0Connected, forKey: Keys.df0Connect)
        defaults.set(config.df1Connected, forKey: Keys.df1Connect)
        defaults.set(config.df2Connected, forKey: Keys.df2Connect)
        defaults.set(config.df3Connected, forKey: Keys.df3Connect)
        defaults.set(config.df0Type, forKey: Keys.df0Type)
        defaults.set(config.df1Type, forKey: Keys.df1Type)
        defaults.set(config.df2Type, forKey: Keys.df2Type)
        defaults.set(config.df3Type, forKey: Keys.df3Type)

        defaults.set(config.gameDevice1, forKey: Keys.gameDevice1)
        defaults.set(config.gameDevice2, forKey: Keys.gameDevice2)
        defaults.set(config.serialDevice, forKey: Keys.serialDevice)
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

extension MyController {

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

    func resetCompatibilityUserDefaults() {

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

        loadCompatibilityUserDefaults()
    }

    func loadCompatibilityUserDefaults() {

        let defaults = UserDefaults.standard

        amiga.suspend()

        config.clxSprSpr = defaults.bool(forKey: Keys.clxSprSpr)
        config.clxSprPlf = defaults.bool(forKey: Keys.clxSprPlf)
        config.clxPlfPlf = defaults.bool(forKey: Keys.clxPlfPlf)
        config.samplingMethod = defaults.integer(forKey: Keys.samplingMethod)
        config.filterActivation = defaults.integer(forKey: Keys.filterActivation)
        config.filterType = defaults.integer(forKey: Keys.filterType)
        config.blitterAccuracy = defaults.integer(forKey: Keys.blitterAccuracy)
        config.driveSpeed = defaults.integer(forKey: Keys.driveSpeed)
        config.fifoBuffering = defaults.bool(forKey: Keys.fifoBuffering)
        config.todBug = defaults.bool(forKey: Keys.todBug)

        amiga.resume()
    }

    func saveCompatibilityUserDefaults() {

        let defaults = UserDefaults.standard
        let config = amiga.config()

        defaults.set(config.denise.clxSprSpr, forKey: Keys.clxSprSpr)
        defaults.set(config.denise.clxSprPlf, forKey: Keys.clxSprPlf)
        defaults.set(config.denise.clxPlfPlf, forKey: Keys.clxPlfPlf)
        defaults.set(config.audio.samplingMethod.rawValue, forKey: Keys.samplingMethod)
        defaults.set(config.audio.filterActivation.rawValue, forKey: Keys.filterActivation)
        defaults.set(config.audio.filterType.rawValue, forKey: Keys.filterType)
        defaults.set(config.blitter.accuracy, forKey: Keys.blitterAccuracy)
        defaults.set(config.diskController.useFifo, forKey: Keys.fifoBuffering)
        defaults.set(config.ciaA.todBug, forKey: Keys.todBug)
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

extension MyController {
    
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
    
    func resetVideoUserDefaults() {
        
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
        
        loadVideoUserDefaults()
    }
    
    func loadVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        config.palette = defaults.integer(forKey: Keys.palette)
        config.brightness = defaults.double(forKey: Keys.brightness)
        config.contrast = defaults.double(forKey: Keys.contrast)
        config.saturation = defaults.double(forKey: Keys.saturation)

        config.hCenter = defaults.float(forKey: Keys.hCenter)
        config.vCenter = defaults.float(forKey: Keys.vCenter)
        config.hZoom = defaults.float(forKey: Keys.hZoom)
        config.vZoom = defaults.float(forKey: Keys.vZoom)

        config.enhancer = defaults.integer(forKey: Keys.enhancer)
        config.upscaler = defaults.integer(forKey: Keys.upscaler)

        defaults.decode(&renderer.shaderOptions, forKey: Keys.shaderOptions)

        renderer.updateTextureRect()
        renderer.buildDotMasks()
        
        amiga.resume()
    }
    
    func saveVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        defaults.set(config.palette, forKey: Keys.palette)
        defaults.set(config.brightness, forKey: Keys.brightness)
        defaults.set(config.contrast, forKey: Keys.contrast)
        defaults.set(config.saturation, forKey: Keys.saturation)

        defaults.set(config.hCenter, forKey: Keys.hCenter)
        defaults.set(config.vCenter, forKey: Keys.vCenter)
        defaults.set(config.hZoom, forKey: Keys.hZoom)
        defaults.set(config.vZoom, forKey: Keys.vZoom)

        defaults.set(config.enhancer, forKey: Keys.enhancer)
        defaults.set(config.upscaler, forKey: Keys.upscaler)

        defaults.encode(renderer.shaderOptions, forKey: Keys.shaderOptions)
    }
}
