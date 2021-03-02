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
        registerControlsUserDefaults()
        registerDevicesUserDefaults()
        registerCaptureUserDefaults()
        
        registerRomUserDefaults()
        registerHardwareUserDefaults()
        registerPeripheralsUserDefaults()
        registerCompatibilityUserDefaults()
        registerAudioUserDefaults()
        registerVideoUserDefaults()
    }
}

extension MyController {
    
    func loadUserDefaults() {
        
        track()
        
        amiga.suspend()
        
        pref.loadGeneralUserDefaults()
        pref.loadControlsUserDefaults()
        pref.loadDevicesUserDefaults()
        pref.loadCaptureUserDefaults()

        config.loadRomUserDefaults()
        config.loadHardwareUserDefaults()
        config.loadPeripheralsUserDefaults()
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
    
    struct Gen {
        
        // Drives
        static let blankDiskFormat        = "VAMIGA_GEN_BlankDiskFormat"
        static let bootBlock              = "VAMIGA_GEN_BootBlock"
        static let ejectWithoutAsking     = "VAMIGA_GEN_EjectWithoutAsking"
        static let driveSounds            = "VAMIGA_GEN_DriveSounds"
        static let driveSoundPan          = "VAMIGA_GEN_DriveSoundPan"
        static let driveInsertSound       = "VAMIGA_GEN_DriveInsertSound"
        static let driveEjectSound        = "VAMIGA_GEN_DriveEjectSound"
        static let driveHeadSound         = "VAMIGA_GEN_DriveHeadSound"
        static let drivePollSound         = "VAMIGA_GEN_DrivePollSound"
        
        // Fullscreen
        static let keepAspectRatio        = "VAMIGA_GEN_FullscreenKeepAspectRatio"
        static let exitOnEsc              = "VAMIGA_GEN_FullscreenExitOnEsc"
        
        // Warp mode
        static let warpMode               = "VAMIGA_GEN_WarpMode"
        
        // Miscellaneous
        static let pauseInBackground      = "VAMIGA_GEN_PauseInBackground"
        static let closeWithoutAsking     = "VAMIGA_GEN_CloseWithoutAsking"
    }
}

struct GeneralDefaults {
    
    // Drives
    let blankDiskFormat: FSVolumeType
    let bootBlock: Int
    let ejectWithoutAsking: Bool
    let driveSounds: Bool
    let driveSoundPan: Double
    let driveInsertSound: Bool
    let driveEjectSound: Bool
    let driveHeadSound: Bool
    let drivePollSound: Bool
        
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
    
    static let std = GeneralDefaults.init(
        
        blankDiskFormat: .OFS,
        bootBlock: 0,
        ejectWithoutAsking: false,
        driveSounds: true,
        driveSoundPan: 1.0,
        driveInsertSound: true,
        driveEjectSound: true,
        driveHeadSound: true,
        drivePollSound: false,
                
        keepAspectRatio: false,
        exitOnEsc: true,
        
        warpMode: .off,

        pauseInBackground: false,
        closeWithoutAsking: false
    )
}

extension UserDefaults {
    
    static func registerGeneralUserDefaults() {
    
        let defaults = GeneralDefaults.std
        let dictionary: [String: Any] = [
            
            Keys.Gen.blankDiskFormat: Int(defaults.blankDiskFormat.rawValue),
            Keys.Gen.bootBlock: defaults.bootBlock,
            Keys.Gen.ejectWithoutAsking: defaults.ejectWithoutAsking,
            Keys.Gen.driveSounds: defaults.driveSounds,
            Keys.Gen.driveSoundPan: defaults.driveSoundPan,
            Keys.Gen.driveInsertSound: defaults.driveInsertSound,
            Keys.Gen.driveEjectSound: defaults.driveEjectSound,
            Keys.Gen.driveHeadSound: defaults.driveHeadSound,
            Keys.Gen.drivePollSound: defaults.drivePollSound,

            Keys.Gen.keepAspectRatio: defaults.keepAspectRatio,
            Keys.Gen.exitOnEsc: defaults.exitOnEsc,
            
            Keys.Gen.warpMode: Int(defaults.warpMode.rawValue),

            Keys.Gen.pauseInBackground: defaults.pauseInBackground,
            Keys.Gen.closeWithoutAsking: defaults.closeWithoutAsking
        ]
        
        let userDefaults = UserDefaults.standard
        
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [ Keys.Gen.blankDiskFormat,
                     Keys.Gen.bootBlock,
                     Keys.Gen.ejectWithoutAsking,
                     Keys.Gen.driveSounds,
                     Keys.Gen.driveSoundPan,
                     Keys.Gen.driveInsertSound,
                     Keys.Gen.driveEjectSound,
                     Keys.Gen.driveHeadSound,
                     Keys.Gen.drivePollSound,
                                          
                     Keys.Gen.keepAspectRatio,
                     Keys.Gen.exitOnEsc,
                     
                     Keys.Gen.warpMode,
                     
                     Keys.Gen.pauseInBackground,
                     Keys.Gen.closeWithoutAsking
        ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Controls)
//

extension Keys {
    
    struct Con {
        
        // Emulation keys
        static let mouseKeyMap           = "VAMIGA_CON_MouseKeyMap"
        static let joyKeyMap1            = "VAMIGA_CON_JoyKeyMap1"
        static let joyKeyMap2            = "VAMIGA_CON_JoyKeyMap2"
        static let disconnectJoyKeys     = "VAMIGA_CON_DisconnectKeys"
        
        // Joysticks
        static let autofire              = "VAMIGA_CON_Autofire"
        static let autofireBullets       = "VAMIGA_CON_AutofireBullets"
        static let autofireFrequency     = "VAMIGA_CON_AutofireFrequency"
        
        // Mouse
        static let retainMouseKeyComb    = "VAMIGA_CON_RetainMouseKeyComb"
        static let retainMouseWithKeys   = "VAMIGA_CON_RetainMouseWithKeys"
        static let retainMouseByClick    = "VAMIGA_CON_RetainMouseByClick"
        static let retainMouseByEntering = "VAMIGA_CON_RetainMouseByEntering"
        static let releaseMouseKeyComb   = "VAMIGA_CON_ReleaseMouseKeyComb"
        static let releaseMouseWithKeys  = "VAMIGA_CON_ReleaseMouseWithKeys"
        static let releaseMouseByShaking = "VAMIGA_CON_ReleaseMouseByShaking"
    }
}

struct ControlsDefaults {
    
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
    
    static let std = ControlsDefaults.init(
        
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
    
    static func registerControlsUserDefaults() {
        
        let defaults = ControlsDefaults.std
        let dictionary: [String: Any] = [

            // Emulation keys
            Keys.Con.disconnectJoyKeys: defaults.disconnectJoyKeys,

            // Joysticks
            Keys.Con.autofire: defaults.autofire,
            Keys.Con.autofireBullets: defaults.autofireBullets,
            Keys.Con.autofireFrequency: defaults.autofireFrequency,
            
            // Mouse
            Keys.Con.retainMouseKeyComb: defaults.retainMouseKeyComb,
            Keys.Con.retainMouseWithKeys: defaults.retainMouseWithKeys,
            Keys.Con.retainMouseByClick: defaults.retainMouseByClick,
            Keys.Con.retainMouseByEntering: defaults.retainMouseByEntering,
            Keys.Con.releaseMouseKeyComb: defaults.releaseMouseKeyComb,
            Keys.Con.releaseMouseWithKeys: defaults.releaseMouseWithKeys,
            Keys.Con.releaseMouseByShaking: defaults.releaseMouseByShaking
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
        userDefaults.register(encodableItem: defaults.joyKeyMap1, forKey: Keys.Con.joyKeyMap1)
        userDefaults.register(encodableItem: defaults.joyKeyMap2, forKey: Keys.Con.joyKeyMap2)
        userDefaults.register(encodableItem: defaults.mouseKeyMap, forKey: Keys.Con.mouseKeyMap)
    }
    
    static func resetControlsUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.Con.joyKeyMap1,
                     Keys.Con.joyKeyMap2,
                     Keys.Con.mouseKeyMap,
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

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Devices)
//

extension Keys {
    
    struct Dev {

        static let schemes            = "VAMIGA_DEV_Schemes"

        // Mapping schemes (DEPRECATED)
        static let leftStickScheme1   = "VAMIGA_DEV_LeftStickScheme1"
        static let rightStickScheme1  = "VAMIGA_DEV_RightStickScheme1"
        static let hatSwitchScheme1   = "VAMIGA_DEV_HatSwitchScheme1"

        static let leftStickScheme2   = "VAMIGA_DEV_LeftStickScheme2"
        static let rightStickScheme2  = "VAMIGA_DEV_RightStickScheme2"
        static let hatSwitchScheme2   = "VAMIGA_DEV_HatSwitchScheme2"
    }
}

struct DevicesDefaults {
    
    // Mapping schemes
    let leftStickScheme1: Int
    let rightStickScheme1: Int
    let hatSwitchScheme1: Int

    let leftStickScheme2: Int
    let rightStickScheme2: Int
    let hatSwitchScheme2: Int

    static let std = DevicesDefaults.init(
        
        leftStickScheme1: 0,
        rightStickScheme1: 0,
        hatSwitchScheme1: 0,
        
        leftStickScheme2: 0,
        rightStickScheme2: 0,
        hatSwitchScheme2: 0
    )
}

extension UserDefaults {
    
    static func registerDevicesUserDefaults() {
        
        let defaults = DevicesDefaults.std
        let dictionary: [String: Any] = [

            // Mapping schemes
            Keys.Dev.leftStickScheme1: defaults.leftStickScheme1,
            Keys.Dev.rightStickScheme1: defaults.rightStickScheme1,
            Keys.Dev.hatSwitchScheme1: defaults.hatSwitchScheme1,

            Keys.Dev.leftStickScheme2: defaults.leftStickScheme2,
            Keys.Dev.rightStickScheme2: defaults.rightStickScheme2,
            Keys.Dev.hatSwitchScheme2: defaults.hatSwitchScheme2
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetDevicesUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.Dev.leftStickScheme1,
                     Keys.Dev.rightStickScheme1,
                     Keys.Dev.hatSwitchScheme1,
                     
                     Keys.Dev.leftStickScheme2,
                     Keys.Dev.rightStickScheme2,
                     Keys.Dev.hatSwitchScheme2 ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Captures)
//

extension Keys {
    
    struct Cap {
        
        // Screenshots
        static let autoScreenshots        = "VAMIGA_CAP_AutoScreenshots"
        static let autoScreenshotInterval = "VAMIGA_CAP_SnapshotInterval"
        static let screenshotSource       = "VAMIGA_CAP_ScreenshotSource"
        static let screenshotTarget       = "VAMIGA_CAP_ScreenshotTarget"
        
        // Snapshots
        static let autoSnapshots          = "VAMIGA_CAP_AutoSnapshots"
        static let autoSnapshotInterval   = "VAMIGA_CAP_ScreenshotInterval"
        
        // Screen captures
        static let captureSource          = "VAMIGA_CAP_Source"
        static let bitRate                = "VAMIGA_CAP_BitRate"
        static let aspectX                = "VAMIGA_CAP_AspectX"
        static let aspectY                = "VAMIGA_CAP_AspectY"
    }
}

struct CaptureDefaults {

    // Screenshots
    let autoScreenshots: Bool
    let autoScreenshotInterval: Int
    let screenshotSource: Int
    let screenshotTarget: NSBitmapImageRep.FileType

    // Snapshots
    let autoSnapshots: Bool
    let autoSnapshotInterval: Int
    
    // Captures
    let captureSource: Int
    let bitRate: Int
    let aspectX: Int
    let aspectY: Int
    
    //
    // Schemes
    //
    
    static let std = CaptureDefaults.init(
                
        autoScreenshots: false,
        autoScreenshotInterval: 10,
        screenshotSource: 0,
        screenshotTarget: .png,

        autoSnapshots: false,
        autoSnapshotInterval: 20,

        captureSource: 0,
        bitRate: 2048,
        aspectX: 768,
        aspectY: 702
    )
}

extension UserDefaults {
    
    static func registerCaptureUserDefaults() {
    
        let defaults = CaptureDefaults.std
        let dictionary: [String: Any] = [
            
            Keys.Cap.autoScreenshots: defaults.autoScreenshots,
            Keys.Cap.autoScreenshotInterval: defaults.autoScreenshotInterval,
            Keys.Cap.screenshotSource: defaults.screenshotSource,
            Keys.Cap.screenshotTarget: Int(defaults.screenshotTarget.rawValue),

            Keys.Cap.autoSnapshots: defaults.autoSnapshots,
            Keys.Cap.autoSnapshotInterval: defaults.autoSnapshotInterval,

            Keys.Cap.captureSource: defaults.captureSource,
            Keys.Cap.bitRate: defaults.bitRate,
            Keys.Cap.aspectX: defaults.aspectX,
            Keys.Cap.aspectY: defaults.aspectY
        ]
        
        let userDefaults = UserDefaults.standard
        
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetCaptureUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [ Keys.Cap.autoScreenshots,
                     Keys.Cap.autoScreenshotInterval,
                     Keys.Cap.screenshotSource,
                     Keys.Cap.screenshotTarget,
                     
                     Keys.Cap.autoSnapshots,
                     Keys.Cap.autoSnapshotInterval,
                     
                     Keys.Cap.captureSource,
                     Keys.Cap.bitRate,
                     Keys.Cap.aspectX,
                     Keys.Cap.aspectY
        ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Roms)
//

extension Keys {

    struct Rom {
        
        static let extStart          = "VAMIGA_ROM_ExtStart"
    }
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
            let folder = try URL.appSupportFolder("Roms", create: true)
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
            
            Keys.Rom.extStart: defaults.extStart
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetRomUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.Rom.extStart ]

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
    
    struct Hrw {
        
        // Chipset
        static let agnusRev           = "VAMIGA_HW_AgnusRev"
        static let slowRamMirror      = "VAMIGA_HW_SlowRamMirror"
        static let deniseRev          = "VAMIGA_HW_DeniseRev"
        static let borderBlank        = "VAMIGA_HW_BorderBlank"
        static let ciaRev             = "VAMIGA_HW_CiaRev"
        static let todBug             = "VAMIGA_HW_TodBug"
        static let realTimeClock      = "VAMIGA_HW_RealTimeClock"
        
        // Memory
        static let chipRam            = "VAMIGA_HW_ChipRam"
        static let slowRam            = "VAMIGA_HW_SlowRam"
        static let fastRam            = "VAMIGA_HW_FastRam"
        static let ramInitPattern     = "VAMIGA_HW_RamInitPattern"
        
        static let bankMap            = "VAMIGA_HW_BankMap"
        static let unmappingType      = "VAMIGA_HW_UnmappingType"
    }
}

struct HardwareDefaults {
    
    var agnusRev: AgnusRevision
    let slowRamMirror: Bool
    var deniseRev: DeniseRevision
    var borderBlank: Bool
    var ciaRev: CIARevision
    var todBug: Bool
    var realTimeClock: RTCRevision
    
    var chipRam: Int
    var slowRam: Int
    var fastRam: Int
    var ramInitPattern: RamInitPattern

    var bankMap: BankMap
    var unmappingType: UnmappedMemory
    
    //
    // Schemes
    //
    
    static let A500 = HardwareDefaults.init(
        
        agnusRev: .ECS_1MB,
        slowRamMirror: true,
        deniseRev: .OCS,
        borderBlank: false,
        ciaRev: ._8520_DIP,
        todBug: true,
        realTimeClock: .OKI,

        chipRam: 512,
        slowRam: 512,
        fastRam: 0,
        ramInitPattern: .ALL_ZEROES,

        bankMap: .A500,
        unmappingType: .FLOATING
    )
    
    static let A1000 = HardwareDefaults.init(
        
        agnusRev: .OCS,
        slowRamMirror: true,
        deniseRev: .OCS,
        borderBlank: false,
        ciaRev: ._8520_DIP,
        todBug: true,
        realTimeClock: .NONE,
        
        chipRam: 256,
        slowRam: 0,
        fastRam: 0,
        ramInitPattern: .ALL_ZEROES,

        bankMap: .A1000,
        unmappingType: .FLOATING
    )
    
    static let A2000 = HardwareDefaults.init(
        
        agnusRev: .ECS_2MB,
        slowRamMirror: true,
        deniseRev: .OCS,
        borderBlank: false,
        ciaRev: ._8520_DIP,
        todBug: true,
        realTimeClock: .OKI,
        
        chipRam: 512,
        slowRam: 512,
        fastRam: 0,
        ramInitPattern: .ALL_ZEROES,

        bankMap: .A2000B,
        unmappingType: .FLOATING
    )
}

extension UserDefaults {
    
    static func registerHardwareUserDefaults() {
        
        let defaults = HardwareDefaults.A500
        let dictionary: [String: Any] = [
            
            Keys.Hrw.agnusRev: defaults.agnusRev.rawValue,
            Keys.Hrw.slowRamMirror: defaults.slowRamMirror,
            Keys.Hrw.deniseRev: defaults.deniseRev.rawValue,
            Keys.Hrw.borderBlank: defaults.borderBlank,
            Keys.Hrw.ciaRev: defaults.ciaRev.rawValue,
            Keys.Hrw.todBug: defaults.todBug,
            Keys.Hrw.realTimeClock: defaults.realTimeClock.rawValue,

            Keys.Hrw.chipRam: defaults.chipRam,
            Keys.Hrw.slowRam: defaults.slowRam,
            Keys.Hrw.fastRam: defaults.fastRam,
            Keys.Hrw.ramInitPattern: defaults.ramInitPattern.rawValue,

            Keys.Hrw.bankMap: defaults.bankMap.rawValue,
            Keys.Hrw.unmappingType: defaults.unmappingType.rawValue
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }
        
    static func resetHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [Keys.Hrw.agnusRev,
                    Keys.Hrw.slowRamMirror,
                    Keys.Hrw.deniseRev,
                    Keys.Hrw.borderBlank,
                    Keys.Hrw.ciaRev,
                    Keys.Hrw.todBug,
                    Keys.Hrw.realTimeClock,

                    Keys.Hrw.chipRam,
                    Keys.Hrw.slowRam,
                    Keys.Hrw.fastRam,
                    Keys.Hrw.ramInitPattern,

                    Keys.Hrw.bankMap,
                    Keys.Hrw.unmappingType
                ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Peripherals)
//

extension Keys {
    
    struct Per {
        
        // Drives
        static let df0Connect         = "VAMIGA_PER_DF0Connect"
        static let df1Connect         = "VAMIGA_PER_DF1Connect"
        static let df2Connect         = "VAMIGA_PER_DF2Connect"
        static let df3Connect         = "VAMIGA_PER_DF3Connect"
        static let df0Type            = "VAMIGA_PER_DF0Type"
        static let df1Type            = "VAMIGA_PER_DF1Type"
        static let df2Type            = "VAMIGA_PER_DF2Type"
        static let df3Type            = "VAMIGA_PER_DF3Type"
        
        // Drive volumes
        static let df0Pan             = "VAMIGA_PER_DF0Pan"
        static let df1Pan             = "VAMIGA_PER_DF1Pan"
        static let df2Pan             = "VAMIGA_PER_DF2Pan"
        static let df3Pan             = "VAMIGA_PER_DF3Pan"
        static let stepVolume         = "VAMIGA_PER_StepVolume"
        static let pollVolume         = "VAMIGA_PER_PollVolume"
        static let insertVolume       = "VAMIGA_PER_InsertVolume"
        static let ejectVolume        = "VAMIGA_PER_EjectVolume"

        // Ports
        static let gameDevice1        = "VAMIGA_PER_GameDevice1"
        static let gameDevice2        = "VAMIGA_PER_GameDevice2"
        static let serialDevice       = "VAMIGA_PER_SerialDevice"
    }
}

struct PeripheralsDefaults {
        
    var driveConnect: [Bool]
    var driveType: [DriveType]

    var drivePan: [Int]
    var stepVolume: Int
    var pollVolume: Int
    var insertVolume: Int
    var ejectVolume: Int
    
    var gameDevice1: Int
    var gameDevice2: Int
    var serialDevice: SerialPortDevice
        
    //
    // Schemes
    //
    
    static let std = PeripheralsDefaults.init(
        
        driveConnect: [true, false, false, false],
        driveType: [.DD_35, .DD_35, .DD_35, .DD_35],
        
        drivePan: [100, 300, 100, 300],
        stepVolume: 50,
        pollVolume: 0,
        insertVolume: 50,
        ejectVolume: 50,
            
        gameDevice1: 0,
        gameDevice2: -1,
        serialDevice: .NONE
    )
}

extension UserDefaults {

    static func registerPeripheralsUserDefaults() {

        let defaults = PeripheralsDefaults.std
        let dictionary: [String: Any] = [
            
            Keys.Per.df0Connect: defaults.driveConnect[0],
            Keys.Per.df1Connect: defaults.driveConnect[1],
            Keys.Per.df2Connect: defaults.driveConnect[2],
            Keys.Per.df3Connect: defaults.driveConnect[3],
            Keys.Per.df0Type: defaults.driveType[0].rawValue,
            Keys.Per.df1Type: defaults.driveType[1].rawValue,
            Keys.Per.df2Type: defaults.driveType[2].rawValue,
            Keys.Per.df3Type: defaults.driveType[3].rawValue,

            Keys.Per.df0Pan: defaults.drivePan[0],
            Keys.Per.df1Pan: defaults.drivePan[1],
            Keys.Per.df2Pan: defaults.drivePan[2],
            Keys.Per.df3Pan: defaults.drivePan[3],
            Keys.Per.stepVolume: defaults.stepVolume,
            Keys.Per.pollVolume: defaults.pollVolume,
            Keys.Per.insertVolume: defaults.insertVolume,
            Keys.Per.ejectVolume: defaults.ejectVolume,

            Keys.Per.gameDevice1: defaults.gameDevice1,
            Keys.Per.gameDevice2: defaults.gameDevice2,
            Keys.Per.serialDevice: defaults.serialDevice.rawValue
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetPeripheralsUserDefaults() {

        let userDefaults = UserDefaults.standard
        
        let keys = [ Keys.Per.df0Connect,
                     Keys.Per.df1Connect,
                     Keys.Per.df2Connect,
                     Keys.Per.df3Connect,
                     Keys.Per.df0Type,
                     Keys.Per.df1Type,
                     Keys.Per.df2Type,
                     Keys.Per.df3Type,
                     
                     Keys.Per.df0Pan,
                     Keys.Per.df1Pan,
                     Keys.Per.df2Pan,
                     Keys.Per.df3Pan,
                     Keys.Per.stepVolume,
                     Keys.Per.pollVolume,
                     Keys.Per.insertVolume,
                     Keys.Per.ejectVolume,

                     Keys.Per.gameDevice1,
                     Keys.Per.gameDevice2,
                     Keys.Per.serialDevice
        ]

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Compatibility)
//

extension Keys {
    
    struct Com {
        
        // Blitter
        static let blitterAccuracy   = "VAMIGA_COM_BlitterAccuracy"
        
        // Timing
        static let eClockSyncing     = "VAMIGA_COM_EClockSyncing"
        static let slowRamDelay      = "VAMIGA_COM_SlowRamDelay"
        
        // Graphics
        static let clxSprSpr         = "VAMIGA_COM_ClxSprSpr"
        static let clxSprPlf         = "VAMIGA_COM_ClxSprPlf"
        static let clxPlfPlf         = "VAMIGA_COM_ClxPlfPlf"
        
        // Floppy drives
        static let driveSpeed        = "VAMIGA_COM_DriveSpeed"
        static let mechanicalDelays  = "VAMIGA_COM_MechanicalDelays"
        static let lockDskSync       = "VAMIGA_COM_LockDskSync"
        static let autoDskSync       = "VAMIGA_COM_AutoDskSync"
        
        // Keyboard
        static let accurateKeyboard  = "VAMIGA_COM_AccurateKeyboard"
    }
}

struct CompatibilityDefaults {
    
    let blitterAccuracy: Int
    
    let eClockSyncing: Bool
    let slowRamDelay: Bool
    
    let clxSprSpr: Bool
    let clxSprPlf: Bool
    let clxPlfPlf: Bool
    
    let driveSpeed: Int
    let mechanicalDelays: Bool
    let lockDskSync: Bool
    let autoDskSync: Bool
    
    let accurateKeyboard: Bool

    //
    // Schemes
    //
    
    static let std = CompatibilityDefaults.init(
        
        blitterAccuracy: 2,
        
        eClockSyncing: true,
        slowRamDelay: true,
        
        clxSprSpr: false,
        clxSprPlf: false,
        clxPlfPlf: false,
        
        driveSpeed: 1,
        mechanicalDelays: true,
        lockDskSync: false,
        autoDskSync: false,
        
        accurateKeyboard: true
     )
    
    static let accurate = CompatibilityDefaults.init(
        
        blitterAccuracy: 2,

        eClockSyncing: true,
        slowRamDelay: true,

        clxSprSpr: true,
        clxSprPlf: true,
        clxPlfPlf: true,
                        
        driveSpeed: 1,
        mechanicalDelays: true,
        lockDskSync: false,
        autoDskSync: false,
        
        accurateKeyboard: true
    )

    static let accelerated = CompatibilityDefaults.init(
        
        blitterAccuracy: 0,

        eClockSyncing: false,
        slowRamDelay: false,

        clxSprSpr: false,
        clxSprPlf: false,
        clxPlfPlf: false,
                
        driveSpeed: -1,
        mechanicalDelays: false,
        lockDskSync: false,
        autoDskSync: false,
        
        accurateKeyboard: false
    )
}

extension UserDefaults {

    static func registerCompatibilityUserDefaults() {

        let defaults = CompatibilityDefaults.std
        let dictionary: [String: Any] = [

            Keys.Com.blitterAccuracy: defaults.blitterAccuracy,

            Keys.Com.eClockSyncing: defaults.eClockSyncing,
            Keys.Com.slowRamDelay: defaults.slowRamDelay,

            Keys.Com.clxSprSpr: defaults.clxSprSpr,
            Keys.Com.clxSprPlf: defaults.clxSprPlf,
            Keys.Com.clxPlfPlf: defaults.clxPlfPlf,
                                    
            Keys.Com.driveSpeed: defaults.driveSpeed,
            Keys.Com.mechanicalDelays: defaults.mechanicalDelays,
            Keys.Com.lockDskSync: defaults.lockDskSync,
            Keys.Com.autoDskSync: defaults.autoDskSync,
            
            Keys.Com.accurateKeyboard: defaults.accurateKeyboard
        ]

        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetCompatibilityUserDefaults() {

        let userDefaults = UserDefaults.standard

        let keys = [ Keys.Com.blitterAccuracy,
                     
                     Keys.Com.eClockSyncing,
                     Keys.Com.slowRamDelay,

                     Keys.Com.clxSprSpr,
                     Keys.Com.clxSprPlf,
                     Keys.Com.clxPlfPlf,
                     
                     Keys.Com.driveSpeed,
                     Keys.Com.mechanicalDelays,
                     Keys.Com.lockDskSync,
                     Keys.Com.autoDskSync,
                     
                     Keys.Com.accurateKeyboard ]

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Audio)
//

extension Keys {
    
    struct Aud {
        
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
        
        vol0: 100,
        vol1: 100,
        vol2: 100,
        vol3: 100,
        pan0: 170,
        pan1: 30,
        pan2: 30,
        pan3: 170,
        
        volL: 50,
        volR: 50,
        samplingMethod: .NONE,
        filterType: .BUTTERWORTH,
        filterAlwaysOn: false
    )
    
    static let stereo = AudioDefaults.init(
        
        vol0: 100,
        vol1: 100,
        vol2: 100,
        vol3: 100,
        pan0: 150,
        pan1: 50,
        pan2: 50,
        pan3: 150,
        
        volL: 50,
        volR: 50,
        samplingMethod: .NONE,
        filterType: .BUTTERWORTH,
        filterAlwaysOn: false
    )

    static let mono = AudioDefaults.init(
        
        vol0: 100,
        vol1: 100,
        vol2: 100,
        vol3: 100,
        pan0: 0,
        pan1: 0,
        pan2: 0,
        pan3: 0,
        
        volL: 50,
        volR: 50,
        samplingMethod: .NONE,
        filterType: .BUTTERWORTH,
        filterAlwaysOn: false
    )
}

extension UserDefaults {

    static func registerAudioUserDefaults() {

        let defaults = AudioDefaults.std
        let dictionary: [String: Any] = [

            Keys.Aud.vol0: defaults.vol0,
            Keys.Aud.vol1: defaults.vol1,
            Keys.Aud.vol2: defaults.vol2,
            Keys.Aud.vol3: defaults.vol3,
            Keys.Aud.pan0: defaults.pan0,
            Keys.Aud.pan1: defaults.pan1,
            Keys.Aud.pan2: defaults.pan2,
            Keys.Aud.pan3: defaults.pan3,
            
            Keys.Aud.volL: defaults.volL,
            Keys.Aud.volR: defaults.volR,
            Keys.Aud.samplingMethod: Int(defaults.samplingMethod.rawValue),
            Keys.Aud.filterType: Int(defaults.filterType.rawValue),
            Keys.Aud.filterAlwaysOn: defaults.filterAlwaysOn
        ]

        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetAudioUserDefaults() {

        let userDefaults = UserDefaults.standard
        
        let keys = [ Keys.Aud.vol0,
                     Keys.Aud.vol1,
                     Keys.Aud.vol2,
                     Keys.Aud.vol3,
                     Keys.Aud.pan0,
                     Keys.Aud.pan1,
                     Keys.Aud.pan2,
                     Keys.Aud.pan3,
                     
                     Keys.Aud.volL,
                     Keys.Aud.volR,
                     Keys.Aud.samplingMethod,
                     Keys.Aud.filterType,
                     Keys.Aud.filterAlwaysOn]

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Video)
//

extension Keys {
    
    struct Vid {
        
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
        
        palette: Palette.COLOR,
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
        
        palette: Palette.COLOR,
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
            
            Keys.Vid.palette: Int(defaults.palette.rawValue),
            Keys.Vid.brightness: defaults.brightness,
            Keys.Vid.contrast: defaults.contrast,
            Keys.Vid.saturation: defaults.saturation,

            Keys.Vid.hCenter: defaults.hCenter,
            Keys.Vid.vCenter: defaults.vCenter,
            Keys.Vid.hZoom: defaults.hZoom,
            Keys.Vid.vZoom: defaults.vZoom,

            Keys.Vid.enhancer: defaults.enhancer,
            Keys.Vid.upscaler: defaults.upscaler,
            
            Keys.Vid.blur: defaults.blur,
            Keys.Vid.blurRadius: defaults.blurRadius,
            Keys.Vid.bloom: defaults.bloom,
            Keys.Vid.bloomRadius: defaults.bloomRadius,
            Keys.Vid.bloomBrightness: defaults.bloomBrightness,
            Keys.Vid.bloomWeight: defaults.bloomWeight,
            Keys.Vid.flicker: defaults.flicker,
            Keys.Vid.flickerWeight: defaults.flickerWeight,
            Keys.Vid.dotMask: defaults.dotMask,
            Keys.Vid.dotMaskBrightness: defaults.dotMaskBrightness,
            Keys.Vid.scanlines: defaults.scanlines,
            Keys.Vid.scanlineBrightness: defaults.scanlineBrightness,
            Keys.Vid.scanlineWeight: defaults.scanlineWeight,
            Keys.Vid.disalignment: defaults.disalignment,
            Keys.Vid.disalignmentH: defaults.disalignmentH,
            Keys.Vid.disalignmentV: defaults.disalignmentV
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetVideoUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.Vid.palette,
                     Keys.Vid.brightness,
                     Keys.Vid.contrast,
                     Keys.Vid.saturation,
                     
                     Keys.Vid.hCenter,
                     Keys.Vid.vCenter,
                     Keys.Vid.hZoom,
                     Keys.Vid.vZoom,

                     Keys.Vid.enhancer,
                     Keys.Vid.upscaler,

                     Keys.Vid.blur,
                     Keys.Vid.blurRadius,
                     Keys.Vid.bloom,
                     Keys.Vid.bloomRadius,
                     Keys.Vid.bloomBrightness,
                     Keys.Vid.bloomWeight,
                     Keys.Vid.flicker,
                     Keys.Vid.flickerWeight,
                     Keys.Vid.dotMask,
                     Keys.Vid.dotMaskBrightness,
                     Keys.Vid.scanlines,
                     Keys.Vid.scanlineBrightness,
                     Keys.Vid.scanlineWeight,
                     Keys.Vid.disalignment,
                     Keys.Vid.disalignmentH,
                     Keys.Vid.disalignmentV
        ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}
