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
            log(warning: "Failed to encode \(key)")
        }
    }
    
    // Decodes an item of generic type 'Decodable'
    func decode<T: Decodable>(_ item: inout T, forKey key: String) {
        
        if let data = data(forKey: key) {
            if let decoded = try? PropertyListDecoder().decode(T.self, from: data) {
                item = decoded
            } else {
                log(warning: "Failed to decode \(key)")
            }
        }
    }
}

//
// User defaults (all)
//

extension UserDefaults {
    
    static func registerUserDefaults() {
                
        log(level: 2)
        
        registerGeneralUserDefaults()
        registerControlsUserDefaults()
        registerDevicesUserDefaults()
        
        registerRomUserDefaults()
        registerChipsetUserDefaults()
        registerPeripheralsUserDefaults()
        registerCompatibilityUserDefaults()
        registerAudioUserDefaults()
        registerVideoUserDefaults()
        registerGeometryUserDefaults()
    }
}

extension MyController {
    
    func loadUserDefaults() {
        
        log(level: 2)
 
        amiga.suspend()
        
        pref.loadGeneralUserDefaults()
        pref.loadControlsUserDefaults()
        pref.loadDevicesUserDefaults()

        // config.loadRomUserDefaults()
        config.loadChipsetUserDefaults()
        config.loadMemoryUserDefaults()
        config.loadPeripheralsUserDefaults()
        config.loadCompatibilityUserDefaults()
        config.loadAudioUserDefaults()
        config.loadVideoUserDefaults()
        config.loadGeometryUserDefaults()
        
        amiga.resume()
    }
}

//
// User defaults (General)
//

struct Keys {
    
    struct Gen {
                
        // Snapshots
        static let autoSnapshots          = "VAMIGA_GEN_AutoSnapshots"
        static let autoSnapshotInterval   = "VAMIGA_GEN_ScreenshotInterval"

        // Screenshots
        static let screenshotSource       = "VAMIGA_GEN_ScreenshotSource"
        static let screenshotTarget       = "VAMIGA_GEN_ScreenshotTarget"
                
        // Screen captures
        static let ffmpegPath             = "VAMIGA_GEN_ffmpegPath"
        static let captureSource          = "VAMIGA_GEN_Source"
        static let bitRate                = "VAMIGA_GEN_BitRate"
        static let aspectX                = "VAMIGA_GEN_AspectX"
        static let aspectY                = "VAMIGA_GEN_AspectY"
        
        // Fullscreen
        static let keepAspectRatio        = "VAMIGA_GEN_FullscreenKeepAspectRatio"
        static let exitOnEsc              = "VAMIGA_GEN_FullscreenExitOnEsc"
        
        // Warp mode
        static let warpMode               = "VAMIGA_GEN_WarpMode"
        
        // Miscellaneous
        static let ejectWithoutAsking     = "VAMIGA_GEN_EjectWithoutAsking"
        static let detachWithoutAsking    = "VAMIGA_GEN_DetachWithoutAsking"
        static let closeWithoutAsking     = "VAMIGA_GEN_CloseWithoutAsking"
        static let pauseInBackground      = "VAMIGA_GEN_PauseInBackground"
    }
}

struct GeneralDefaults {
            
    // Snapshots
    let autoSnapshots: Bool
    let autoSnapshotInterval: Int

    // Screenshots
    let autoScreenshots: Bool
    let autoScreenshotInterval: Int
    let screenshotSource: Int
    let screenshotTarget: NSBitmapImageRep.FileType
    
    // Captures
    let ffmpegPath: String
    let captureSource: Int
    let bitRate: Int
    let aspectX: Int
    let aspectY: Int
    
    // Fullscreen
    let keepAspectRatio: Bool
    let exitOnEsc: Bool
    
    // Warp mode
    let warpMode: WarpMode

    // Miscellaneous
    let ejectWithoutAsking: Bool
    let detachWithoutAsking: Bool
    let closeWithoutAsking: Bool
    let pauseInBackground: Bool

    //
    // Schemes
    //
    
    static let std = GeneralDefaults(
                      
        autoSnapshots: false,
        autoSnapshotInterval: 20,

        autoScreenshots: false,
        autoScreenshotInterval: 10,
        screenshotSource: 0,
        screenshotTarget: .png,

        ffmpegPath: "",
        captureSource: 0,
        bitRate: 2048,
        aspectX: 768,
        aspectY: 702,
        
        keepAspectRatio: false,
        exitOnEsc: true,
        
        warpMode: .off,

        ejectWithoutAsking: false,
        detachWithoutAsking: false,
        closeWithoutAsking: false,
        pauseInBackground: false
    )
}

extension UserDefaults {
    
    static func registerGeneralUserDefaults() {
    
        let defaults = GeneralDefaults.std
        let dictionary: [String: Any] = [
            
            Keys.Gen.autoSnapshots: defaults.autoSnapshots,
            Keys.Gen.autoSnapshotInterval: defaults.autoSnapshotInterval,

            Keys.Gen.screenshotSource: defaults.screenshotSource,
            Keys.Gen.screenshotTarget: Int(defaults.screenshotTarget.rawValue),

            Keys.Gen.ffmpegPath: defaults.ffmpegPath,
            Keys.Gen.captureSource: defaults.captureSource,
            Keys.Gen.bitRate: defaults.bitRate,
            Keys.Gen.aspectX: defaults.aspectX,
            Keys.Gen.aspectY: defaults.aspectY,
            
            Keys.Gen.keepAspectRatio: defaults.keepAspectRatio,
            Keys.Gen.exitOnEsc: defaults.exitOnEsc,
            
            Keys.Gen.warpMode: Int(defaults.warpMode.rawValue),

            Keys.Gen.ejectWithoutAsking: defaults.ejectWithoutAsking,
            Keys.Gen.detachWithoutAsking: defaults.detachWithoutAsking,
            Keys.Gen.pauseInBackground: defaults.pauseInBackground,
            Keys.Gen.closeWithoutAsking: defaults.closeWithoutAsking
        ]
        
        let userDefaults = UserDefaults.standard
        
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [ Keys.Gen.autoSnapshots,
                     Keys.Gen.autoSnapshotInterval,
                     
                     Keys.Gen.screenshotSource,
                     Keys.Gen.screenshotTarget,
                     
                     Keys.Gen.ffmpegPath,
                     Keys.Gen.captureSource,
                     Keys.Gen.bitRate,
                     Keys.Gen.aspectX,
                     Keys.Gen.aspectY,
            
                     Keys.Gen.keepAspectRatio,
                     Keys.Gen.exitOnEsc,
                     
                     Keys.Gen.warpMode,
                     
                     Keys.Gen.ejectWithoutAsking,
                     Keys.Gen.detachWithoutAsking,
                     Keys.Gen.closeWithoutAsking,
                     Keys.Gen.pauseInBackground
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
    let mouseKeyMap: [MacKey: Int]
    let joyKeyMap1: [MacKey: Int]
    let joyKeyMap2: [MacKey: Int]
    let disconnectJoyKeys: Bool
    
    // Joysticks
    let autofire: Bool
    let autofireBullets: Int
    let autofireFrequency: Double
    
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
        
        MacKey(keyCode: kVK_LeftArrow): GamePadAction.PULL_LEFT.rawValue,
        MacKey(keyCode: kVK_RightArrow): GamePadAction.PULL_RIGHT.rawValue,
        MacKey(keyCode: kVK_UpArrow): GamePadAction.PULL_UP.rawValue,
        MacKey(keyCode: kVK_DownArrow): GamePadAction.PULL_DOWN.rawValue,
        MacKey(keyCode: kVK_Space): GamePadAction.PRESS_FIRE.rawValue
    ]
    static let stdKeyMap2 = [
        
        MacKey(keyCode: kVK_ANSI_S): GamePadAction.PULL_LEFT.rawValue,
        MacKey(keyCode: kVK_ANSI_D): GamePadAction.PULL_RIGHT.rawValue,
        MacKey(keyCode: kVK_ANSI_E): GamePadAction.PULL_UP.rawValue,
        MacKey(keyCode: kVK_ANSI_X): GamePadAction.PULL_DOWN.rawValue,
        MacKey(keyCode: kVK_ANSI_C): GamePadAction.PRESS_FIRE.rawValue
    ]
    
    static let std = ControlsDefaults(
        
        mouseKeyMap: [:],
        joyKeyMap1: stdKeyMap1,
        joyKeyMap2: stdKeyMap2,
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
        userDefaults.register(encodableItem: defaults.joyKeyMap2, forKey: Keys.Con.joyKeyMap2)
        userDefaults.register(defaults: dictionary)
        userDefaults.register(encodableItem: defaults.joyKeyMap1, forKey: Keys.Con.joyKeyMap1)
    }
    
    static func resetControlsUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.Con.mouseKeyMap,
                     Keys.Con.joyKeyMap1,
                     Keys.Con.joyKeyMap2,
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

    static let std = DevicesDefaults(
        
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
// User defaults (Roms)
//

extension Keys {

    struct Rom {
        
        static let extStart          = "VAMIGA_ROM_ExtStart"
    }
}

struct RomDefaults {
    
    let extStart: Int
    
    static let std = RomDefaults(
        
        extStart: 0xE0
    )
}

extension UserDefaults {
    
    static func romUrl(name: String) -> URL? {
        
        let folder = try? URL.appSupportFolder("Roms")
        return folder?.appendingPathComponent(name)        
    }
    
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
        
        if let url = romUrl {
            try? fm.removeItem(at: url)
        }
        
        if let url = extUrl {
            try? fm.removeItem(at: url)
        }
    }
}

//
// User defaults (Chipset)
//

extension Keys {
    
    struct Hrw {
        
        // Chipset
        static let agnusRev           = "VAMIGA_HW_AgnusRev"
        static let deniseRev          = "VAMIGA_HW_DeniseRev"
        static let ciaRev             = "VAMIGA_HW_CiaRev"
        static let realTimeClock      = "VAMIGA_HW_RealTimeClock"
        
    }
}

struct HardwareDefaults {
    
    let agnusRev: AgnusRevision
    let deniseRev: DeniseRevision
    let ciaRev: CIARevision
    let realTimeClock: RTCRevision
    
    let chipRam: Int
    let slowRam: Int
    let fastRam: Int
    let ramInitPattern: RamInitPattern

    let bankMap: BankMap
    let unmappingType: UnmappedMemory
    
    //
    // Schemes
    //
    
    static let A500 = HardwareDefaults(
        
        agnusRev: .ECS_1MB,
        deniseRev: .OCS,
        ciaRev: .MOS_8520_DIP,
        realTimeClock: .NONE,
        
        chipRam: 512,
        slowRam: 0,
        fastRam: 0,
        ramInitPattern: .ALL_ZEROES,

        bankMap: .A500,
        unmappingType: .FLOATING
    )
    
    static let A1000 = HardwareDefaults(
        
        agnusRev: .OCS_OLD,
        deniseRev: .OCS,
        ciaRev: .MOS_8520_DIP,
        realTimeClock: .NONE,
        
        chipRam: 256,
        slowRam: 0,
        fastRam: 0,
        ramInitPattern: .ALL_ZEROES,

        bankMap: .A1000,
        unmappingType: .FLOATING
    )
    
    static let A2000 = HardwareDefaults(
        
        agnusRev: .ECS_1MB,
        deniseRev: .OCS,
        ciaRev: .MOS_8520_DIP,
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
    
    static func registerChipsetUserDefaults() {
        
        let defaults = HardwareDefaults.A500
        let dictionary: [String: Any] = [
            
            Keys.Hrw.agnusRev: defaults.agnusRev.rawValue,
            Keys.Hrw.deniseRev: defaults.deniseRev.rawValue,
            Keys.Hrw.ciaRev: defaults.ciaRev.rawValue,
            Keys.Hrw.realTimeClock: defaults.realTimeClock.rawValue
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }
        
    static func resetChipsetUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [ Keys.Hrw.agnusRev,
                     Keys.Hrw.deniseRev,
                     Keys.Hrw.ciaRev,
                     Keys.Hrw.realTimeClock ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Memory)
//

extension Keys {
    
    struct Mem {
                
        // Memory
        static let chipRam            = "VAMIGA_MEM_ChipRam"
        static let slowRam            = "VAMIGA_MEM_SlowRam"
        static let fastRam            = "VAMIGA_MEM_FastRam"
        static let ramInitPattern     = "VAMIGA_MEM_InitPattern"
        
        static let bankMap            = "VAMIGA_MEM_BankMap"
        static let unmappingType      = "VAMIGA_MEM_UnmappingType"

        // Features
        static let slowRamDelay       = "VAMIGA_MEM_SlowRamDelay"
        static let slowRamMirror      = "VAMIGA_MEM_SlowRamMirror"
    }
}

struct MemoryDefaults {
    
    let chipRam: Int
    let slowRam: Int
    let fastRam: Int
    let ramInitPattern: RamInitPattern

    let bankMap: BankMap
    let unmappingType: UnmappedMemory
    
    let slowRamDelay: Bool
    let slowRamMirror: Bool

    //
    // Schemes
    //
    
    static let A500 = MemoryDefaults(
                
        chipRam: 512,
        slowRam: 0,
        fastRam: 0,
        ramInitPattern: .ALL_ZEROES,

        bankMap: .A500,
        unmappingType: .FLOATING,
        
        slowRamDelay: true,
        slowRamMirror: true
    )
    
    static let A1000 = MemoryDefaults(
        
        chipRam: 256,
        slowRam: 0,
        fastRam: 0,
        ramInitPattern: .ALL_ZEROES,

        bankMap: .A1000,
        unmappingType: .FLOATING,
        
        slowRamDelay: true,
        slowRamMirror: true
    )
    
    static let A2000 = MemoryDefaults(
        
        chipRam: 512,
        slowRam: 512,
        fastRam: 0,
        ramInitPattern: .ALL_ZEROES,

        bankMap: .A2000B,
        unmappingType: .FLOATING,
        
        slowRamDelay: true,
        slowRamMirror: true
    )
}

extension UserDefaults {
    
    static func registerMemoryUserDefaults() {
        
        let defaults = MemoryDefaults.A500
        let dictionary: [String: Any] = [
            
            Keys.Mem.chipRam: defaults.chipRam,
            Keys.Mem.slowRam: defaults.slowRam,
            Keys.Mem.fastRam: defaults.fastRam,
            Keys.Mem.ramInitPattern: defaults.ramInitPattern.rawValue,

            Keys.Mem.bankMap: defaults.bankMap.rawValue,
            Keys.Mem.unmappingType: defaults.unmappingType.rawValue,
            
            Keys.Mem.slowRamDelay: defaults.slowRamDelay,
            Keys.Mem.slowRamMirror: defaults.slowRamMirror
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }
        
    static func resetMemoryUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.Mem.chipRam,
                     Keys.Mem.slowRam,
                     Keys.Mem.fastRam,
                     Keys.Mem.ramInitPattern,
                     
                     Keys.Mem.bankMap,
                     Keys.Mem.unmappingType,
                     
                     Keys.Mem.slowRamDelay,
                     Keys.Mem.slowRamMirror ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Peripherals)
//

extension Keys {
    
    struct Per {
        
        // Floppy Drives
        static let df0Connect         = "VAMIGA_PER_DF0Connect"
        static let df1Connect         = "VAMIGA_PER_DF1Connect"
        static let df2Connect         = "VAMIGA_PER_DF2Connect"
        static let df3Connect         = "VAMIGA_PER_DF3Connect"
        static let df0Type            = "VAMIGA_PER_DF0Type"
        static let df1Type            = "VAMIGA_PER_DF1Type"
        static let df2Type            = "VAMIGA_PER_DF2Type"
        static let df3Type            = "VAMIGA_PER_DF3Type"

        // Hard Drives
        static let hd0Connect         = "VAMIGA_PER_HD0Connect"
        static let hd1Connect         = "VAMIGA_PER_HD1Connect"
        static let hd2Connect         = "VAMIGA_PER_HD2Connect"
        static let hd3Connect         = "VAMIGA_PER_HD3Connect"
        static let hd0Type            = "VAMIGA_PER_HD0Type"
        static let hd1Type            = "VAMIGA_PER_HD1Type"
        static let hd2Type            = "VAMIGA_PER_HD2Type"
        static let hd3Type            = "VAMIGA_PER_HD3Type"

        // Ports
        static let gameDevice1        = "VAMIGA_PER_GameDevice1"
        static let gameDevice2        = "VAMIGA_PER_GameDevice2"
        static let serialDevice       = "VAMIGA_PER_SerialDevice"
        static let serialDevicePort   = "VAMIGA_PER_SerialDevicePort"
    }
}

struct PeripheralsDefaults {
        
    var driveConnect: [Bool]
    var driveType: [FloppyDriveType]

    var hardDriveConnect: [Bool]
    var hardDriveType: [HardDriveType]

    var gameDevice1: Int
    var gameDevice2: Int
    var serialDevice: SerialPortDevice
    var serialDevicePort: Int
        
    //
    // Schemes
    //
    
    static let std = PeripheralsDefaults(
        
        driveConnect: [true, false, false, false],
        driveType: [.DD_35, .DD_35, .DD_35, .DD_35],
        hardDriveConnect: [true, false, false, false],
        hardDriveType: [.GENERIC, .GENERIC, .GENERIC, .GENERIC],
        gameDevice1: 0,
        gameDevice2: -1,
        serialDevice: .NONE,
        serialDevicePort: 8080
    )
}

extension UserDefaults {

    static func mediaUrl(name: String) -> URL? {
        
        let folder = try? URL.appSupportFolder("Media")
        return folder?.appendingPathComponent(name)
    }

    static func hdnUrl(_ nr: Int) -> URL? {
        
        return mediaUrl(name: "hd\(nr).hdf")
    }

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

            Keys.Per.hd0Connect: defaults.driveConnect[0],
            Keys.Per.hd1Connect: defaults.driveConnect[1],
            Keys.Per.hd2Connect: defaults.driveConnect[2],
            Keys.Per.hd3Connect: defaults.driveConnect[3],
            Keys.Per.hd0Type: defaults.driveType[0].rawValue,
            Keys.Per.hd1Type: defaults.driveType[1].rawValue,
            Keys.Per.hd2Type: defaults.driveType[2].rawValue,
            Keys.Per.hd3Type: defaults.driveType[3].rawValue,

            Keys.Per.gameDevice1: defaults.gameDevice1,
            Keys.Per.gameDevice2: defaults.gameDevice2,
            Keys.Per.serialDevice: defaults.serialDevice.rawValue,
            Keys.Per.serialDevicePort: defaults.serialDevicePort
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
                     
                     Keys.Per.hd0Connect,
                     Keys.Per.hd1Connect,
                     Keys.Per.hd2Connect,
                     Keys.Per.hd3Connect,
                     Keys.Per.hd0Type,
                     Keys.Per.hd1Type,
                     Keys.Per.hd2Type,
                     Keys.Per.hd3Type,
                     
                     Keys.Per.gameDevice1,
                     Keys.Per.gameDevice2,
                     Keys.Per.serialDevice,
                     Keys.Per.serialDevicePort
        ]

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Compatibility)
//

extension Keys {
    
    /*
    struct Com {
        
        // Blitter
        static let blitterAccuracy   = "VAMIGA_COM_BlitterAccuracy"
        
        // Chipset
        static let borderBlank        = "VAMIGA_COM_BorderBlank"
        static let todBug             = "VAMIGA_COM_TodBug"

        // Timing
        static let eClockSyncing     = "VAMIGA_COM_EClockSyncing"
        
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
    */
}

struct CompatibilityDefaults {
    
    let blitterAccuracy: Int
    
    let borderBlank: Bool
    let todBug: Bool

    let eClockSyncing: Bool
    
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
    
    static let std = CompatibilityDefaults(
        
        blitterAccuracy: 2,
        
        borderBlank: false,
        todBug: true,
            
        eClockSyncing: true,
        
        clxSprSpr: false,
        clxSprPlf: false,
        clxPlfPlf: false,
        
        driveSpeed: 1,
        mechanicalDelays: true,
        lockDskSync: false,
        autoDskSync: false,
        
        accurateKeyboard: true
     )
    
    static let accurate = CompatibilityDefaults(
        
        blitterAccuracy: 2,

        borderBlank: false,
        todBug: true,

        eClockSyncing: true,

        clxSprSpr: true,
        clxSprPlf: true,
        clxPlfPlf: true,
                        
        driveSpeed: 1,
        mechanicalDelays: true,
        lockDskSync: false,
        autoDskSync: false,
        
        accurateKeyboard: true
    )

    static let accelerated = CompatibilityDefaults(
        
        blitterAccuracy: 0,

        borderBlank: false,
        todBug: true,

        eClockSyncing: false,

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

        /*
        let defaults = CompatibilityDefaults.std
        let dictionary: [String: Any] = [

            Keys.Com.blitterAccuracy: defaults.blitterAccuracy,

            Keys.Com.borderBlank: defaults.borderBlank,
            Keys.Com.todBug: defaults.todBug,

            Keys.Com.eClockSyncing: defaults.eClockSyncing,

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
        */
    }

    static func resetCompatibilityUserDefaults() {

        /*
        let userDefaults = UserDefaults.standard

        let keys = [ Keys.Com.blitterAccuracy,

                     Keys.Com.borderBlank,
                     Keys.Com.todBug,

                     Keys.Com.eClockSyncing,

                     Keys.Com.clxSprSpr,
                     Keys.Com.clxSprPlf,
                     Keys.Com.clxPlfPlf,
                     
                     Keys.Com.driveSpeed,
                     Keys.Com.mechanicalDelays,
                     Keys.Com.lockDskSync,
                     Keys.Com.autoDskSync,
                     
                     Keys.Com.accurateKeyboard ]

        for key in keys { userDefaults.removeObject(forKey: key) }
        */
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
        
        // Drive volumes
        static let df0Pan             = "VAMIGA_AUD_DF0Pan"
        static let df1Pan             = "VAMIGA_AUD_DF1Pan"
        static let df2Pan             = "VAMIGA_AUD_DF2Pan"
        static let df3Pan             = "VAMIGA_AUD_DF3Pan"
        static let hd0Pan             = "VAMIGA_AUD_HD0Pan"
        static let hd1Pan             = "VAMIGA_AUD_HD1Pan"
        static let hd2Pan             = "VAMIGA_AUD_HD2Pan"
        static let hd3Pan             = "VAMIGA_AUD_HD3Pan"
        static let stepVolume         = "VAMIGA_AUD_StepVolume"
        static let pollVolume         = "VAMIGA_AUD_PollVolume"
        static let insertVolume       = "VAMIGA_AUD_InsertVolume"
        static let ejectVolume        = "VAMIGA_AUD_EjectVolume"

        // Filter
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

    // Drive
    var drivePan: [Int]
    var hdPan: [Int]
    var stepVolume: Int
    var pollVolume: Int
    var insertVolume: Int
    var ejectVolume: Int

    // Filter
    let filterType: FilterType
    let filterAlwaysOn: Bool

    //
    // Schemes
    //
    
    static let std = AudioDefaults(
        
        vol0: 100,
        vol1: 100,
        vol2: 100,
        vol3: 100,
        pan0: 50,
        pan1: 350,
        pan2: 350,
        pan3: 50,
        
        volL: 50,
        volR: 50,
        samplingMethod: .NONE,
        
        drivePan: [100, 300, 100, 300],
        hdPan: [100, 300, 100, 300],
        stepVolume: 50,
        pollVolume: 0,
        insertVolume: 50,
        ejectVolume: 50,
        
        filterType: .BUTTERWORTH,
        filterAlwaysOn: false
    )
    
    static let stereo = AudioDefaults(
        
        vol0: 100,
        vol1: 100,
        vol2: 100,
        vol3: 100,
        pan0: 100,
        pan1: 300,
        pan2: 300,
        pan3: 100,
        
        volL: 50,
        volR: 50,
        samplingMethod: .NONE,
        
        drivePan: [100, 300, 100, 300],
        hdPan: [100, 300, 100, 300],
        stepVolume: 50,
        pollVolume: 0,
        insertVolume: 50,
        ejectVolume: 50,
        
        filterType: .BUTTERWORTH,
        filterAlwaysOn: false
    )

    static let mono = AudioDefaults(
        
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
        
        drivePan: [100, 300, 100, 300],
        hdPan: [100, 300, 100, 300],
        stepVolume: 50,
        pollVolume: 0,
        insertVolume: 50,
        ejectVolume: 50,
        
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
            
            Keys.Aud.df0Pan: defaults.drivePan[0],
            Keys.Aud.df1Pan: defaults.drivePan[1],
            Keys.Aud.df2Pan: defaults.drivePan[2],
            Keys.Aud.df3Pan: defaults.drivePan[3],
            Keys.Aud.hd0Pan: defaults.hdPan[0],
            Keys.Aud.hd1Pan: defaults.hdPan[1],
            Keys.Aud.hd2Pan: defaults.hdPan[2],
            Keys.Aud.hd3Pan: defaults.hdPan[3],
            Keys.Aud.stepVolume: defaults.stepVolume,
            Keys.Aud.pollVolume: defaults.pollVolume,
            Keys.Aud.insertVolume: defaults.insertVolume,
            Keys.Aud.ejectVolume: defaults.ejectVolume,

            Keys.Aud.filterType: defaults.filterType.rawValue,
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
                     
                     Keys.Aud.df0Pan,
                     Keys.Aud.df1Pan,
                     Keys.Aud.df2Pan,
                     Keys.Aud.df3Pan,
                     Keys.Aud.hd0Pan,
                     Keys.Aud.hd1Pan,
                     Keys.Aud.hd2Pan,
                     Keys.Aud.hd3Pan,
                     Keys.Aud.stepVolume,
                     Keys.Aud.pollVolume,
                     Keys.Aud.insertVolume,
                     Keys.Aud.ejectVolume,

                     Keys.Aud.filterType,
                     Keys.Aud.filterAlwaysOn ]

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
        static let hAutoCenter        = "VAMIGA_VID_HAutoCenter"
        static let vAutoCenter        = "VAMIGA_VID_VAutoCenter"
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
    let brightness: Int
    let contrast: Int
    let saturation: Int
        
    // Upscalers
    let enhancer: Int
    let upscaler: Int
    
    // Shader options
    let blur: Int32
    let blurRadius: Float
    let bloom: Int
    let bloomRadius: Float
    let bloomBrightness: Float
    let bloomWeight: Float
    let flicker: Int32
    let flickerWeight: Float
    let dotMask: Int
    let dotMaskBrightness: Float
    let scanlines: Int
    let scanlineBrightness: Float
    let scanlineWeight: Float
    let disalignment: Int32
    let disalignmentH: Float
    let disalignmentV: Float
    
    //
    // Schemes
    //
    
    // TFT monitor
    static let tft = VideoDefaults(
        
        palette: Palette.COLOR,
        brightness: 50,
        contrast: 100,
        saturation: 50,
        
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
    
    // CRT monitor
    static let crt = VideoDefaults(
        
        palette: Palette.COLOR,
        brightness: 50,
        contrast: 100,
        saturation: 50,
        
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

struct GeometryDefaults {
        
    let hAutoCenter: Bool
    let vAutoCenter: Bool
    let hCenter: Float
    let vCenter: Float
    let hZoom: Float
    let vZoom: Float
    
    static let narrow = GeometryDefaults(
        
        hAutoCenter: true,
        vAutoCenter: true,
        hCenter: 0.6,
        vCenter: 0.47,
        hZoom: 1.0,
        vZoom: 0.27
    )

    static let wide = GeometryDefaults(
        
        hAutoCenter: true,
        vAutoCenter: true,
        hCenter: 0.409,
        vCenter: 0.143,
        hZoom: 0.747,
        vZoom: 0.032
    )

    static let extreme = GeometryDefaults(
        
        hAutoCenter: false,
        vAutoCenter: false,
        hCenter: 0,
        vCenter: 0,
        hZoom: 0,
        vZoom: 0
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

    static func registerGeometryUserDefaults() {
        
        let defaults = GeometryDefaults.wide
        let dictionary: [String: Any] = [
            
            Keys.Vid.hAutoCenter: defaults.hAutoCenter,
            Keys.Vid.vAutoCenter: defaults.vAutoCenter,
            Keys.Vid.hCenter: defaults.hCenter,
            Keys.Vid.vCenter: defaults.vCenter,
            Keys.Vid.hZoom: defaults.hZoom,
            Keys.Vid.vZoom: defaults.vZoom
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
    
    static func resetGeometryUserDefaults() {
        
        let defaults = UserDefaults.standard

        let keys = [ Keys.Vid.hAutoCenter,
                     Keys.Vid.vAutoCenter,
                     Keys.Vid.hCenter,
                     Keys.Vid.vCenter,
                     Keys.Vid.hZoom,
                     Keys.Vid.vZoom
        ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}
