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
            set(encoded, forKey: key)
        } else {
            track("Failed to encode \(key)")
        }
    }
    
    // Decodes an item of generic type 'Decodable'
    func decode<T: Decodable>(_ item: inout T, forKey key: String) {
        
        if let data = data(forKey: key) {
            if let decoded = try? PropertyListDecoder().decode(T.self, from: data) {
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
        
        registerGeneralUserDefaults()
        registerDevicesUserDefaults()

        registerRomUserDefaults()
        registerHardwareUserDefaults()
        registerCompatibilityUserDefaults()
        registerAudioUserDefaults()
        registerVideoUserDefaults()
    }
}

extension MyController {
    
    func loadUserDefaults() {
        
        track()
        
        amiga.suspend()
        
        pref.loadEmulatorUserDefaults()
        pref.loadDevicesUserDefaults()

        config.loadRomUserDefaults()
        config.loadMemoryUserDefaults()
        config.loadHardwareUserDefaults()
        config.loadCompatibilityUserDefaults()
        config.loadAudioUserDefaults()
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
    static let driveBlankDiskFormat   = "VAMIGA_GEN_DriveBlankDiskFormat"
    static let ejectWithoutAsking     = "VAMIGA_GEN_EjectWithoutAsking"
    static let driveSounds            = "VAMIGA_GEN_DriveSounds"
    static let driveSoundPan          = "VAMIGA_GEN_DriveSoundPan"
    static let driveInsertSound       = "VAMIGA_GEN_DriveInsertSound"
    static let driveEjectSound        = "VAMIGA_GEN_DriveEjectSound"
    static let driveHeadSound         = "VAMIGA_GEN_DriveHeadSound"
    static let drivePollSound         = "VAMIGA_GEN_DrivePollSound"

    // Snapshots and screenshots
    static let autoSnapshots          = "VAMIGA_GEN_AutoSnapshots"
    static let autoSnapshotInterval   = "VAMIGA_GEN_ScreenshotInterval"
    static let autoScreenshots        = "VAMIGA_GEN_AutoScreenshots"
    static let autoScreenshotInterval = "VAMIGA_GEN_SnapshotInterval"
    static let screenshotSource       = "VAMIGA_GEN_ScreenshotSource"
    static let screenshotTarget       = "VAMIGA_GEN_ScreenshotTarget"
    
    // Fullscreen
    static let keepAspectRatio        = "VAMIGA_GEN_FullscreenKeepAspectRatio"
    static let exitOnEsc              = "VAMIGA_GEN_FullscreenExitOnEsc"

    // User dialogs
    static let closeWithoutAsking     = "VAMIGA_GEN_CloseWithoutAsking"
    
    // Warp mode
    static let warpMode               = "VAMIGA_GEN_WarpMode"

    // Miscellaneous
    static let pauseInBackground      = "VAMIGA_GEN_PauseInBackground"
}

struct EmulatorDefaults {
    
    // Drives
    let driveBlankDiskFormat: FileSystemType
    let ejectWithoutAsking: Bool
    let driveSounds: Bool
    let driveSoundPan: Double
    let driveInsertSound: Bool
    let driveEjectSound: Bool
    let driveHeadSound: Bool
    let drivePollSound: Bool
    
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
    
    // Warp mode
    let warpMode: WarpMode

    // Miscellaneous
    let pauseInBackground: Bool
    let closeWithoutAsking: Bool

    //
    // Schemes
    //
    
    static let std = EmulatorDefaults.init(
        
        driveBlankDiskFormat: .FS_OFS,
        ejectWithoutAsking: false,
        driveSounds: true,
        driveSoundPan: 1.0,
        driveInsertSound: true,
        driveEjectSound: true,
        driveHeadSound: true,
        drivePollSound: false,
        
        autoSnapshots: false,
        autoSnapshotInterval: 20,
        autoScreenshots: false,
        autoScreenshotInterval: 10,
        screenshotSource: 0,
        screenshotTarget: .png,
        
        keepAspectRatio: false,
        exitOnEsc: true,
        
        warpMode: .auto,

        pauseInBackground: false,
        closeWithoutAsking: false
    )
}

extension UserDefaults {
    
    static func registerGeneralUserDefaults() {
    
        let defaults = EmulatorDefaults.std
        let dictionary: [String: Any] = [
            
            Keys.driveBlankDiskFormat: Int(defaults.driveBlankDiskFormat.rawValue),
            Keys.ejectWithoutAsking: defaults.ejectWithoutAsking,
            Keys.driveSounds: defaults.driveSounds,
            Keys.driveSoundPan: defaults.driveSoundPan,
            Keys.driveInsertSound: defaults.driveInsertSound,
            Keys.driveEjectSound: defaults.driveEjectSound,
            Keys.driveHeadSound: defaults.driveHeadSound,
            Keys.drivePollSound: defaults.drivePollSound,

            Keys.autoSnapshots: defaults.autoSnapshots,
            Keys.autoSnapshotInterval: defaults.autoSnapshotInterval,
            Keys.autoScreenshots: defaults.autoScreenshots,
            Keys.autoScreenshotInterval: defaults.autoScreenshotInterval,
            Keys.screenshotSource: defaults.screenshotSource,
            Keys.screenshotTarget: Int(defaults.screenshotTarget.rawValue),

            Keys.keepAspectRatio: defaults.keepAspectRatio,
            Keys.exitOnEsc: defaults.exitOnEsc,
            
            Keys.warpMode: Int(defaults.warpMode.rawValue),

            Keys.pauseInBackground: defaults.pauseInBackground,
            Keys.closeWithoutAsking: defaults.closeWithoutAsking
        ]
        
        let userDefaults = UserDefaults.standard
        
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [ Keys.driveBlankDiskFormat,
                     Keys.ejectWithoutAsking,
                     Keys.driveSounds,
                     Keys.driveSoundPan,
                     Keys.driveInsertSound,
                     Keys.driveEjectSound,
                     Keys.driveHeadSound,
                     Keys.drivePollSound,
                     
                     Keys.autoSnapshots,
                     Keys.autoSnapshotInterval,
                     Keys.autoScreenshots,
                     Keys.autoScreenshotInterval,
                     Keys.screenshotSource,
                     Keys.screenshotTarget,
                     
                     Keys.keepAspectRatio,
                     Keys.exitOnEsc,
                     
                     Keys.warpMode,
                     
                     Keys.pauseInBackground,
                     Keys.closeWithoutAsking
        ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Input Devices)
//

extension Keys {
            
    // Joysticks
    static let joyKeyMap1            = "VAMIGA_DEV_JoyKeyMap1"
    static let joyKeyMap2            = "VAMIGA_DEV_JoyKeyMap2"
    static let mouseKeyMap           = "VAMIGA_DEV_MouseKeyMap"
    static let disconnectJoyKeys     = "VAMIGA_DEV_DisconnectKeys"
    static let autofire              = "VAMIGA_DEV_Autofire"
    static let autofireBullets       = "VAMIGA_DEV_AutofireBullets"
    static let autofireFrequency     = "VAMIGA_DEV_AutofireFrequency"
    
    // Mouse
    static let retainMouseKeyComb    = "VAMIGA_DEV_RetainMouseKeyComb"
    static let retainMouseWithKeys   = "VAMIGA_DEV_RetainMouseWithKeys"
    static let retainMouseByClick    = "VAMIGA_DEV_RetainMouseByClick"
    static let retainMouseByEntering = "VAMIGA_DEV_RetainMouseByEntering"
    static let releaseMouseKeyComb   = "VAMIGA_DEV_ReleaseMouseKeyComb"
    static let releaseMouseWithKeys  = "VAMIGA_DEV_ReleaseMouseWithKeys"
    static let releaseMouseByShaking = "VAMIGA_DEV_ReleaseMouseByShaking"
}

struct DevicesDefaults {
    
    // Emulation keys
    let joyKeyMap1: [MacKey: Int]
    let joyKeyMap2: [MacKey: Int]
    let mouseKeyMap: [MacKey: Int]
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
        
        MacKey.init(keyCode: kVK_LeftArrow): GamePadAction.PULL_LEFT.rawValue,
        MacKey.init(keyCode: kVK_RightArrow): GamePadAction.PULL_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_UpArrow): GamePadAction.PULL_UP.rawValue,
        MacKey.init(keyCode: kVK_DownArrow): GamePadAction.PULL_DOWN.rawValue,
        MacKey.init(keyCode: kVK_Space): GamePadAction.PRESS_FIRE.rawValue
    ]
    static let stdKeyMap2 = [
        
        MacKey.init(keyCode: kVK_ANSI_S): GamePadAction.PULL_LEFT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_D): GamePadAction.PULL_RIGHT.rawValue,
        MacKey.init(keyCode: kVK_ANSI_E): GamePadAction.PULL_UP.rawValue,
        MacKey.init(keyCode: kVK_ANSI_X): GamePadAction.PULL_DOWN.rawValue,
        MacKey.init(keyCode: kVK_ANSI_C): GamePadAction.PRESS_FIRE.rawValue
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
    
    static let extStart          = "VAMIGA_ROM_ExtStart"
}

struct RomDefaults {
    
    let extStart: Int
    
    static let std = RomDefaults.init(
        
        extStart: 0xE0
    )
}

extension UserDefaults {
    
    static func romUrl(name: String) -> URL? {
        
        do {
            let folder = try URL.appSupportFolder("Roms")
            return folder.appendingPathComponent(name)
        } catch {
            return nil
        }
    }
    
    static var womUrl: URL? { return romUrl(name: "wom.bin") }
    static var romUrl: URL? { return romUrl(name: "rom.bin") }
    static var extUrl: URL? { return romUrl(name: "ext.bin") }

    static func registerRomUserDefaults() {
        
        let defaults = RomDefaults.std
        let dictionary: [String: Any] = [
            
            Keys.extStart: defaults.extStart
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetRomUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.extStart ]

        for key in keys { defaults.removeObject(forKey: key) }
        
        // Delete previously saved Rom files
        let fm = FileManager.default
        
        if let url = womUrl {
            track("Deleting Wom")
            try? fm.removeItem(at: url)
        }
        
        if let url = romUrl {
            track("Deleting Rom")
            try? fm.removeItem(at: url)
        }
        
        if let url = extUrl {
            track("Deleting Ext")
            try? fm.removeItem(at: url)
        }
    }
}

//
// User defaults (Hardware)
//

extension Keys {
        
    // Chipset
    static let agnusRev           = "VAMIGA_HW_AgnusRev"
    static let deniseRev          = "VAMIGA_HW_DeniseRev"
    static let realTimeClock      = "VAMIGA_HW_RealTimeClock"

    // Memory
    static let chipRam            = "VAMIGA_HW_ChipRam"
    static let slowRam            = "VAMIGA_HW_SlowRam"
    static let fastRam            = "VAMIGA_HW_FastRam"

    // Drives
    static let df0Connect         = "VAMIGA_HW_DF0Connect"
    static let df0Type            = "VAMIGA_HW_DF0Type"
    static let df1Connect         = "VAMIGA_HW_DF1Connect"
    static let df1Type            = "VAMIGA_HW_DF1Type"
    static let df2Connect         = "VAMIGA_HW_DF2Connect"
    static let df2Type            = "VAMIGA_HW_DF2Type"
    static let df3Connect         = "VAMIGA_HW_DF3Connect"
    static let df3Type            = "VAMIGA_HW_DF3Type"

    // Ports
    static let gameDevice1        = "VAMIGA_HW_GameDevice1"
    static let gameDevice2        = "VAMIGA_HW_GameDevice2"
    static let serialDevice       = "VAMIGA_HW_SerialDevice"
}

struct HardwareDefaults {
    
    var agnusRev: AgnusRevision
    var deniseRev: DeniseRevision
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
        
        agnusRev: .AGNUS_ECS_1MB,
        deniseRev: .DENISE_OCS,
        realTimeClock: .OKI,
        
        chipRam: 512,
        slowRam: 512,
        fastRam: 0,
        
        driveConnect: [true, false, false, false],
        driveType: [.DRIVE_35_DD, .DRIVE_35_DD, .DRIVE_35_DD, .DRIVE_35_DD],
        
        gameDevice1: 0,
        gameDevice2: -1,
        serialDevice: .SPD_NONE
    )
    
    static let A1000 = HardwareDefaults.init(
        
        agnusRev: .AGNUS_OCS,
        deniseRev: .DENISE_OCS,
        realTimeClock: .NONE,
        
        chipRam: 256,
        slowRam: 0,
        fastRam: 0,
        
        driveConnect: [true, false, false, false],
        driveType: [.DRIVE_35_DD, .DRIVE_35_DD, .DRIVE_35_DD, .DRIVE_35_DD],
        
        gameDevice1: 0,
        gameDevice2: -1,
        serialDevice: .SPD_NONE
    )
    
    static let A2000 = HardwareDefaults.init(
        
        agnusRev: .AGNUS_ECS_2MB,
        deniseRev: .DENISE_OCS,
        realTimeClock: .OKI,
        
        chipRam: 512,
        slowRam: 512,
        fastRam: 0,
        
        driveConnect: [true, true, false, false],
        driveType: [.DRIVE_35_DD, .DRIVE_35_DD, .DRIVE_35_DD, .DRIVE_35_DD],
        
        gameDevice1: 0,
        gameDevice2: -1,
        serialDevice: .SPD_NONE
    )
}

extension UserDefaults {
    
    static func registerHardwareUserDefaults() {
        
        let defaults = HardwareDefaults.A500
        let dictionary: [String: Any] = [
            
            Keys.agnusRev: defaults.agnusRev.rawValue,
            Keys.deniseRev: defaults.deniseRev.rawValue,
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
// User defaults (Memory)
//

extension Keys {

    // Chipset features
    static let slowRamMirror = "VAMIGA_MEM_SlowRamMirror"
}

struct MemoryDefaults {
    
    // Chipset features
    let slowRamMirror: Bool
    
    //
    // Schemes
    //
    
    static let std = MemoryDefaults.init(
        
        slowRamMirror: true
    )
}

extension UserDefaults {

    static func registerMemoryUserDefaults() {

        let defaults = MemoryDefaults.std
        let dictionary: [String: Any] = [

            Keys.slowRamMirror: defaults.slowRamMirror
        ]

        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetMemoryUserDefaults() {

        let userDefaults = UserDefaults.standard
        
        let keys = [ Keys.slowRamMirror ]

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Compatibility)
//

extension Keys {
    
    // Graphics
    static let clxSprSpr         = "VAMIGA_COM_ClxSprSpr"
    static let clxSprPlf         = "VAMIGA_COM_ClxSprPlf"
    static let clxPlfPlf         = "VAMIGA_COM_ClxPlfPlf"

    // Blitter
    static let blitterAccuracy   = "VAMIGA_COM_BlitterAccuracy"

    // CIAs
    static let todBug            = "VAMIGA_COM_TodBug"
    static let eClockSyncing     = "VAMIGA_COM_EClockSyncing"

    // Floppy drives
    static let driveSpeed        = "VAMIGA_COM_DriveSpeed"
    static let asyncFifo         = "VAMIGA_COM_AsyncFifo"
    static let lockDskSync       = "VAMIGA_COM_LockDskSync"
    static let autoDskSync       = "VAMIGA_COM_AutoDskSync"

    // Keyboard
    static let accurateKeyboard  = "VAMIGA_COM_AccurateKeyboard"
}

struct CompatibilityDefaults {
    
    // Graphics
    let clxSprSpr: Bool
    let clxSprPlf: Bool
    let clxPlfPlf: Bool
    
    // Blitter
    let blitterAccuracy: Int
    
    // CIAs
    let todBug: Bool
    let eClockSyncing: Bool
    
    // Floppy drives
    let driveSpeed: Int
    let asyncFifo: Bool
    let lockDskSync: Bool
    let autoDskSync: Bool
    
    // Keyboard
    let accurateKeyboard: Bool

    //
    // Schemes
    //
    
    static let std = CompatibilityDefaults.init(
         
         clxSprSpr: false,
         clxSprPlf: false,
         clxPlfPlf: false,
                  
         blitterAccuracy: 2,

         todBug: true,
         eClockSyncing: true,
         
         driveSpeed: 1,
         asyncFifo: true,
         lockDskSync: false,
         autoDskSync: false,

         accurateKeyboard: true
     )
    
    static let accurate = CompatibilityDefaults.init(
        
        clxSprSpr: true,
        clxSprPlf: true,
        clxPlfPlf: true,
                
        blitterAccuracy: 2,
        
        todBug: true,
        eClockSyncing: true,

        driveSpeed: 1,
        asyncFifo: true,
        lockDskSync: false,
        autoDskSync: false,
        
        accurateKeyboard: true
    )

    static let accelerated = CompatibilityDefaults.init(
        
        clxSprSpr: false,
        clxSprPlf: false,
        clxPlfPlf: false,
                
        blitterAccuracy: 0,
        
        todBug: true,
        eClockSyncing: false,

        driveSpeed: -1,
        asyncFifo: false,
        lockDskSync: false,
        autoDskSync: false,
        
        accurateKeyboard: false
    )
}

extension UserDefaults {

    static func registerCompatibilityUserDefaults() {

        let defaults = CompatibilityDefaults.std
        let dictionary: [String: Any] = [

            Keys.clxSprSpr: defaults.clxSprSpr,
            Keys.clxSprPlf: defaults.clxSprPlf,
            Keys.clxPlfPlf: defaults.clxPlfPlf,
            Keys.blitterAccuracy: defaults.blitterAccuracy,
            Keys.todBug: defaults.todBug,
            Keys.eClockSyncing: defaults.eClockSyncing,
            Keys.driveSpeed: defaults.driveSpeed,
            Keys.asyncFifo: defaults.asyncFifo,
            Keys.lockDskSync: defaults.lockDskSync,
            Keys.autoDskSync: defaults.autoDskSync,
            Keys.accurateKeyboard: defaults.accurateKeyboard
        ]

        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetCompatibilityUserDefaults() {

        let userDefaults = UserDefaults.standard

        let keys = [ Keys.clxSprSpr,
                     Keys.clxSprPlf,
                     Keys.clxPlfPlf,
                     Keys.blitterAccuracy,
                     Keys.todBug,
                     Keys.eClockSyncing,
                     Keys.driveSpeed,
                     Keys.asyncFifo,
                     Keys.lockDskSync,
                     Keys.autoDskSync,
                     Keys.accurateKeyboard ]

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Audio)
//

extension Keys {
    
    // In
    static let vol0               = "VAMIGA_AUD_Volume0"
    static let vol1               = "VAMIGA_AUD_Volume1"
    static let vol2               = "VAMIGA_AUD_Volume2"
    static let vol3               = "VAMIGA_AUD_Volume3"
    static let pan0               = "VAMIGA_AUD_Pan0"
    static let pan1               = "VAMIGA_AUD_Pan1"
    static let pan2               = "VAMIGA_AUD_Pan2"
    static let pan3               = "VAMIGA_AUD_Pan3"

    // Out
    static let volL               = "VAMIGA_AUD_VolumeL"
    static let volR               = "VAMIGA_AUD_VolumeR"
    static let samplingMethod     = "VAMIGA_AUD_SamplingMethod"
    static let filterType         = "VAMIGA_AUD_FilterType"
    static let filterAlwaysOn     = "VAMIGA_AUD_FilterAlwaysOn"
}

struct AudioDefaults {
    
    // In
    let vol0: Int
    let vol1: Int
    let vol2: Int
    let vol3: Int
    let pan0: Int
    let pan1: Int
    let pan2: Int
    let pan3: Int
    
    // Out
    let volL: Int
    let volR: Int
    let samplingMethod: SamplingMethod
    let filterType: FilterType
    let filterAlwaysOn: Bool
    
    //
    // Schemes
    //
    
    static let std = AudioDefaults.init(
        
        vol0: 400,
        vol1: 400,
        vol2: 400,
        vol3: 400,
        pan0: 70,
        pan1: 30,
        pan2: 30,
        pan3: 70,
        
        volL: 250,
        volR: 250,
        samplingMethod: .SMP_NONE,
        filterType: .FILT_BUTTERWORTH,
        filterAlwaysOn: false
    )
}

extension UserDefaults {

    static func registerAudioUserDefaults() {

        let defaults = AudioDefaults.std
        let dictionary: [String: Any] = [

            Keys.vol0: defaults.vol0,
            Keys.vol1: defaults.vol1,
            Keys.vol2: defaults.vol2,
            Keys.vol3: defaults.vol3,
            Keys.pan0: defaults.pan0,
            Keys.pan1: defaults.pan1,
            Keys.pan2: defaults.pan2,
            Keys.pan3: defaults.pan3,
            
            Keys.volL: defaults.volL,
            Keys.volR: defaults.volR,
            Keys.samplingMethod: Int(defaults.samplingMethod.rawValue),
            Keys.filterType: Int(defaults.filterType.rawValue),
            Keys.filterAlwaysOn: defaults.filterAlwaysOn
        ]

        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetAudioUserDefaults() {

        let userDefaults = UserDefaults.standard
        
        let keys = [ Keys.vol0,
                     Keys.vol1,
                     Keys.vol2,
                     Keys.vol3,
                     Keys.pan0,
                     Keys.pan1,
                     Keys.pan2,
                     Keys.pan3,
                     
                     Keys.volL,
                     Keys.volR,
                     Keys.samplingMethod,
                     Keys.filterType,
                     Keys.filterAlwaysOn]

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Video)
//

extension Keys {
    
    // Colors
    static let palette            = "VAMIGA_VID_Palette"
    static let brightness         = "VAMIGA_VID_Brightness"
    static let contrast           = "VAMIGA_VID_Contrast"
    static let saturation         = "VAMIGA_VID_Saturation"

    // Geometry
    static let hCenter            = "VAMIGA_VID_HCenter"
    static let vCenter            = "VAMIGA_VID_VCenter"
    static let hZoom              = "VAMIGA_VID_HZoom"
    static let vZoom              = "VAMIGA_VID_VZoom"

    // Upscalers
    static let enhancer           = "VVAMIG_VID_Enhancer"
    static let upscaler           = "VAMIGA_VID_Upscaler"

    // Shader options
    static let blur               = "VAMIGA_VID_Blur"
    static let blurRadius         = "VAMIGA_VID_BlurRadius"
    static let bloom              = "VAMIGA_VID_Bloom"
    static let bloomRadius        = "VAMIGA_VID_BloonRadius"
    static let bloomBrightness    = "VAMIGA_VID_BloomBrightness"
    static let bloomWeight        = "VAMIGA_VID_BloomWeight"
    static let flicker            = "VAMIGA_VID_Flicker"
    static let flickerWeight      = "VAMIGA_VID_FlickerWeight"
    static let dotMask            = "VAMIGA_VID_DotMask"
    static let dotMaskBrightness  = "VAMIGA_VID_DotMaskBrightness"
    static let scanlines          = "VAMIGA_VID_Scanlines"
    static let scanlineBrightness = "VAMIGA_VID_ScanlineBrightness"
    static let scanlineWeight     = "VAMIGA_VID_ScanlineWeight"
    static let disalignment       = "VAMIGA_VID_Disalignment"
    static let disalignmentH      = "VAMIGA_VID_DisalignmentH"
    static let disalignmentV      = "VAMIGA_VID_DisalignmentV"
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
    
    // TFT monitor appearance with a texture cutout similar to UAE
    static let tft = VideoDefaults.init(
        
        palette: Palette.COLOR_PALETTE,
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
        blurRadius: 0,
        bloom: 0,
        bloomRadius: 1.0,
        bloomBrightness: 0.4,
        bloomWeight: 1.21,
        flicker: 1,
        flickerWeight: 0.25,
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
        
        palette: Palette.COLOR_PALETTE,
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
        blurRadius: 1.5,
        bloom: 1,
        bloomRadius: 1.0,
        bloomBrightness: 0.4,
        bloomWeight: 1.21,
        flicker: 1,
        flickerWeight: 0.25,
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
                     Keys.disalignmentV
        ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}
