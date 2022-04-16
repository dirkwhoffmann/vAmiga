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
// Proxy extensions
//

extension PropertiesProxy {

    func load(url: URL) throws {
        
        let exception = ExceptionWrapper()
        load(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func load() {
        
        log("Loading user defaults")
        
        do {
            let folder = try URL.appSupportFolder()
            let path = folder.appendingPathComponent("vAmiga.ini")
            
            do {
                try load(url: path)
                log("Successfully loaded user defaults from file \(path)")
            } catch {
                log("Failed to load user defaults from file \(path)")
            }
            
        } catch {
            log("Failed to access application support folder")
        }
    }
    
    func save(url: URL) throws {
        
        let exception = ExceptionWrapper()
        save(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func save() {
        
        log("Saving user defaults")
        
        do {
            let folder = try URL.appSupportFolder()
            let path = folder.appendingPathComponent("vAmiga.ini")
            
            do {
                try save(url: path)
                log("Successfully saved user defaults to file \(path)")
            } catch {
                log("Failed to save user defaults file \(path)")
            }
            
        } catch {
            log("Failed to access application support folder")
        }
    }

    func register(_ key: String, _ val: String) { register(key, value: val) }
    func register(_ key: String, _ val: Bool) { register(key, value: val ? "1" : "0") }
    func register(_ key: String, _ val: Int) { register(key, value: "\(val)") }
    func register(_ key: String, _ val: Int32) { register(key, value: "\(val)") }
    func register(_ key: String, _ val: UInt) { register(key, value: "\(val)") }
    func register(_ key: String, _ val: Float) { register(key, value: "\(val)") }
    func register(_ key: String, _ val: Double) { register(key, value: "\(val)") }

    func set(_ key: String, _ val: String) { setKey(key, value: val) }
    func set(_ key: String, _ val: Bool) { setKey(key, value: val ? "1" : "0") }
    func set(_ key: String, _ val: Int) { setKey(key, value: "\(val)") }
    func set(_ key: String, _ val: Int32) { setKey(key, value: "\(val)") }
    func set(_ key: String, _ val: UInt) { setKey(key, value: "\(val)") }
    func set(_ key: String, _ val: Float) { setKey(key, value: "\(val)") }
    func set(_ key: String, _ val: Double) { setKey(key, value: "\(val)") }

    func getBool(_ key: String) -> Bool { return getInt(key) != 0 }
    func getInt32(_ key: String) -> Int32 { return Int32(getInt(key)) }
    func getFloat(_ key: String) -> Float { return (getString(key) as NSString).floatValue }
    func getDouble(_ key: String) -> Double { return (getString(key) as NSString).doubleValue }

    func set(_ option: Option, value: Bool) {
        set(option, value: value ? 1 : 0)
    }
    func set(_ option: Option, nr: Int, value: Bool) {
        set(option, nr: nr, value: value ? 1 : 0)
    }
    func set(_ option: Option, nr: [NSInteger], value: NSInteger) {
        for n in nr { set(option, nr: n, value: value) }
    }
    func set(_ option: Option, nr: [NSInteger], value: Bool) {
        for n in nr { set(option, nr: n, value: value ? 1 : 0) }
    }
    
    func register<T: Encodable>(_ key: String, encodable item: T) {
        
        let jsonData = try? JSONEncoder().encode(item)
        let jsonString = jsonData?.base64EncodedString() ?? ""
        register(key, jsonString)
    }

    func set<T: Encodable>(_ key: String, encodable item: T) {
        
        let jsonData = try? JSONEncoder().encode(item)
        let jsonString = jsonData?.base64EncodedString() ?? ""
        set(key, jsonString)
    }

    func decode<T: Decodable>(_ key: String, encodable item: inout T) {
        
        if let jsonString = getString(key) {

            log("Decoding jsonString '\(jsonString)'")
            if let data = Data(base64Encoded: jsonString) {

                if let decoded = try? JSONDecoder().decode(T.self, from: data) {
                    item = decoded
                } else {
                    log(warning: "Failed to decode \(jsonString)")
                }
                return
            }
        }
        log("Failed to decode jsonString")
    }
}

//
// Paths
//

extension UserDefaults {
    
    static func romUrl(name: String) -> URL? {
        
        let folder = try? URL.appSupportFolder("Roms")
        return folder?.appendingPathComponent(name)
    }
    
    static func mediaUrl(name: String) -> URL? {
        
        let folder = try? URL.appSupportFolder("Media")
        return folder?.appendingPathComponent(name)
    }

    static func hdUrl(_ n: Int) -> URL? {
        
        return mediaUrl(name: "hd\(n).hdf")
    }

    static var romUrl: URL? { return romUrl(name: "rom.bin") }
    static var extUrl: URL? { return romUrl(name: "ext.bin") }
    static var hd0Url: URL? { return hdUrl(0) }
    static var hd1Url: URL? { return hdUrl(1) }
    static var hd2Url: URL? { return hdUrl(2) }
    static var hd3Url: URL? { return hdUrl(3) }
}

//
// User defaults (all)
//

extension PropertiesProxy {
    
    func registerUserDefaults() {
        
        log(level: 2)
        
        registerGeneralUserDefaults()
        registerControlsUserDefaults()
        registerDevicesUserDefaults()

        registerChipsetUserDefaults()
        registerMemoryUserDefaults()
        registerPeripheralsUserDefaults()
        registerCompatibilityUserDefaults()
        registerAudioUserDefaults()
        registerVideoUserDefaults()
    }
}

extension Preferences {

    func applyUserDefaults() {
        
        log(level: 2)
        
        applyGeneralUserDefaults()
        applyControlsUserDefaults()
        applyDevicesUserDefaults()
    }
}

extension Configuration {
        
    func applyUserDefaults() {
    
        log(level: 2)
        
        applyChipsetUserDefaults()
        applyMemoryUserDefaults()
        applyPeripheralsUserDefaults()
        applyCompatibilityUserDefaults()
        applyAudioUserDefaults()
        applyVideoUserDefaults()
    }
}

//
// User defaults (General)
//

struct Keys {
    
    struct Gen {
                
        // Snapshots
        static let autoSnapshots          = "GenAutoSnapshots"
        static let autoSnapshotInterval   = "GenScreenshotInterval"

        // Screenshots
        static let screenshotSource       = "GenScreenshotSource"
        static let screenshotTarget       = "GenScreenshotTarget"
                
        // Screen captures
        static let ffmpegPath             = "GenffmpegPath"
        static let captureSource          = "GenSource"
        static let bitRate                = "GenBitRate"
        static let aspectX                = "GenAspectX"
        static let aspectY                = "GenAspectY"
        
        // Fullscreen
        static let keepAspectRatio        = "GenFullscreenKeepAspectRatio"
        static let exitOnEsc              = "GenFullscreenExitOnEsc"
        
        // Warp mode
        static let warpMode               = "GenWarpMode"
        
        // Miscellaneous
        static let ejectWithoutAsking     = "GenEjectWithoutAsking"
        static let detachWithoutAsking    = "GenDetachWithoutAsking"
        static let closeWithoutAsking     = "GenCloseWithoutAsking"
        static let pauseInBackground      = "GenPauseInBackground"
    }
}

extension PropertiesProxy {
    
    func registerGeneralUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        // Snapshots
        defaults.register(Keys.Gen.autoSnapshots, false)
        defaults.register(Keys.Gen.autoSnapshotInterval, 20)
        
        // Screenshots
        defaults.register(Keys.Gen.screenshotSource, 0)
        defaults.register(Keys.Gen.screenshotTarget, NSBitmapImageRep.FileType.png.rawValue)
        
        // Captures
        defaults.register(Keys.Gen.ffmpegPath, "")
        defaults.register(Keys.Gen.captureSource, 0)
        defaults.register(Keys.Gen.bitRate, 2048)
        defaults.register(Keys.Gen.aspectX, 768)
        defaults.register(Keys.Gen.aspectY, 702)
        
        // Fullscreen
        defaults.register(Keys.Gen.keepAspectRatio, false)
        defaults.register(Keys.Gen.exitOnEsc, true)
        
        // Warp mode
        defaults.register(Keys.Gen.warpMode, WarpMode.off.rawValue)
        
        // Misc
        defaults.register(Keys.Gen.ejectWithoutAsking, false)
        defaults.register(Keys.Gen.detachWithoutAsking, false)
        defaults.register(Keys.Gen.closeWithoutAsking, false)
        defaults.register(Keys.Gen.pauseInBackground, false)
    }
    
    func removeGeneralUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
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
        
        for key in keys { defaults.removeKey(key) }
    }
}

extension Preferences {

    func saveGeneralUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        defaults.set(Keys.Gen.autoSnapshots, autoSnapshots)
        defaults.set(Keys.Gen.autoSnapshotInterval, snapshotInterval)
        
        defaults.set(Keys.Gen.screenshotSource, screenshotSource)
        defaults.set(Keys.Gen.screenshotTarget, screenshotTargetIntValue)
        
        defaults.set(Keys.Gen.ffmpegPath, ffmpegPath)
        defaults.set(Keys.Gen.captureSource, captureSource)
        defaults.set(Keys.Gen.bitRate, bitRate)
        defaults.set(Keys.Gen.aspectX, aspectX)
        defaults.set(Keys.Gen.aspectY, aspectY)
        
        defaults.set(Keys.Gen.keepAspectRatio, keepAspectRatio)
        defaults.set(Keys.Gen.exitOnEsc, exitOnEsc)
        
        defaults.set(Keys.Gen.warpMode, warpModeIntValue)
        
        defaults.set(Keys.Gen.ejectWithoutAsking, ejectWithoutAsking)
        defaults.set(Keys.Gen.detachWithoutAsking, detachWithoutAsking)
        defaults.set(Keys.Gen.closeWithoutAsking, closeWithoutAsking)
        defaults.set(Keys.Gen.pauseInBackground, pauseInBackground)
        
        defaults.save()
    }
    
    func applyGeneralUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        autoSnapshots = defaults.getBool(Keys.Gen.autoSnapshots)
        snapshotInterval = defaults.getInt(Keys.Gen.autoSnapshotInterval)
        
        screenshotSource = defaults.getInt(Keys.Gen.screenshotSource)
        screenshotTargetIntValue = defaults.getInt(Keys.Gen.screenshotTarget)
        
        ffmpegPath = defaults.getString(Keys.Gen.ffmpegPath) ?? ""
        captureSource = defaults.getInt(Keys.Gen.captureSource)
        bitRate = defaults.getInt(Keys.Gen.bitRate)
        aspectX = defaults.getInt(Keys.Gen.aspectX)
        aspectY = defaults.getInt(Keys.Gen.aspectY)
        
        keepAspectRatio = defaults.getBool(Keys.Gen.keepAspectRatio)
        exitOnEsc = defaults.getBool(Keys.Gen.exitOnEsc)
        
        warpModeIntValue = defaults.getInt(Keys.Gen.warpMode)
        
        ejectWithoutAsking = defaults.getBool(Keys.Gen.ejectWithoutAsking)
        detachWithoutAsking = defaults.getBool(Keys.Gen.detachWithoutAsking)
        closeWithoutAsking = defaults.getBool(Keys.Gen.closeWithoutAsking)
        pauseInBackground = defaults.getBool(Keys.Gen.pauseInBackground)
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

extension PropertiesProxy {
    
    func registerControlsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        let emptyMap: [MacKey: Int] = [:]

        let stdKeyMap1: [MacKey: Int] = [
            
            MacKey(keyCode: kVK_LeftArrow): GamePadAction.PULL_LEFT.rawValue,
            MacKey(keyCode: kVK_RightArrow): GamePadAction.PULL_RIGHT.rawValue,
            MacKey(keyCode: kVK_UpArrow): GamePadAction.PULL_UP.rawValue,
            MacKey(keyCode: kVK_DownArrow): GamePadAction.PULL_DOWN.rawValue,
            MacKey(keyCode: kVK_Space): GamePadAction.PRESS_FIRE.rawValue
        ]
        
        let stdKeyMap2 = [
            
            MacKey(keyCode: kVK_ANSI_S): GamePadAction.PULL_LEFT.rawValue,
            MacKey(keyCode: kVK_ANSI_D): GamePadAction.PULL_RIGHT.rawValue,
            MacKey(keyCode: kVK_ANSI_E): GamePadAction.PULL_UP.rawValue,
            MacKey(keyCode: kVK_ANSI_X): GamePadAction.PULL_DOWN.rawValue,
            MacKey(keyCode: kVK_ANSI_C): GamePadAction.PRESS_FIRE.rawValue
        ]
        
        // Emulation keys
        defaults.register(Keys.Con.mouseKeyMap, encodable: emptyMap)
        defaults.register(Keys.Con.joyKeyMap1, encodable: stdKeyMap1)
        defaults.register(Keys.Con.joyKeyMap2, encodable: stdKeyMap2)
        defaults.register(Keys.Con.disconnectJoyKeys, true)
        
        // Joysticks
        defaults.register(Keys.Con.autofire, false)
        defaults.register(Keys.Con.autofireBullets, -3)
        defaults.register(Keys.Con.autofireFrequency, 2.5)

        // Mouse
        defaults.register(Keys.Con.retainMouseKeyComb, 0)
        defaults.register(Keys.Con.retainMouseWithKeys, true)
        defaults.register(Keys.Con.retainMouseByClick, true)
        defaults.register(Keys.Con.retainMouseByEntering, false)
        defaults.register(Keys.Con.releaseMouseKeyComb, 0)
        defaults.register(Keys.Con.releaseMouseWithKeys, true)
        defaults.register(Keys.Con.releaseMouseByShaking, true)
    }
    
    func removeControlsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
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

        for key in keys { defaults.removeKey(key) }
    }
}

extension Preferences {

    func saveControlsUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
                
        defaults.set(Keys.Con.mouseKeyMap, encodable: keyMaps[0])
        defaults.set(Keys.Con.joyKeyMap1, encodable: keyMaps[1])
        defaults.set(Keys.Con.joyKeyMap2, encodable: keyMaps[2])
        defaults.set(Keys.Con.disconnectJoyKeys, disconnectJoyKeys)

        defaults.set(Keys.Con.autofire, autofire)
        defaults.set(Keys.Con.autofireBullets, autofireBullets)
        defaults.set(Keys.Con.autofireFrequency, autofireFrequency)

        defaults.set(Keys.Con.retainMouseKeyComb, retainMouseKeyComb)
        defaults.set(Keys.Con.retainMouseWithKeys, retainMouseWithKeys)
        defaults.set(Keys.Con.retainMouseByClick, retainMouseByClick)
        defaults.set(Keys.Con.retainMouseByEntering, retainMouseByEntering)
        defaults.set(Keys.Con.releaseMouseKeyComb, releaseMouseKeyComb)
        defaults.set(Keys.Con.releaseMouseWithKeys, releaseMouseWithKeys)
        defaults.set(Keys.Con.releaseMouseByShaking, releaseMouseByShaking)
        
        defaults.save()
    }
    
    func applyControlsUserDefaults() {
           
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        defaults.decode(Keys.Con.mouseKeyMap, encodable: &keyMaps[0])
        defaults.decode(Keys.Con.joyKeyMap1, encodable: &keyMaps[1])
        defaults.decode(Keys.Con.joyKeyMap2, encodable: &keyMaps[2])
        disconnectJoyKeys = defaults.getBool(Keys.Con.disconnectJoyKeys)
        
        autofire = defaults.getBool(Keys.Con.autofire)
        autofireBullets = defaults.getInt(Keys.Con.autofireBullets)
        autofireFrequency = defaults.getDouble(Keys.Con.autofireFrequency)
        
        retainMouseKeyComb = defaults.getInt(Keys.Con.retainMouseKeyComb)
        retainMouseWithKeys = defaults.getBool(Keys.Con.retainMouseWithKeys)
        retainMouseByClick = defaults.getBool(Keys.Con.retainMouseByClick)
        retainMouseByEntering = defaults.getBool(Keys.Con.retainMouseByEntering)
        releaseMouseKeyComb = defaults.getInt(Keys.Con.releaseMouseKeyComb)
        releaseMouseWithKeys = defaults.getBool(Keys.Con.releaseMouseWithKeys)
        releaseMouseByShaking = defaults.getBool(Keys.Con.releaseMouseByShaking)
    }
}

//
// User defaults (Devices)
//

extension Keys {
    
    struct Dev {

        static let schemes            = "VAMIGA_DEV_Schemes"

        // Mapping schemes (DEPRECATED)
        /*
        static let leftStickScheme1   = "VAMIGA_DEV_LeftStickScheme1"
        static let rightStickScheme1  = "VAMIGA_DEV_RightStickScheme1"
        static let hatSwitchScheme1   = "VAMIGA_DEV_HatSwitchScheme1"

        static let leftStickScheme2   = "VAMIGA_DEV_LeftStickScheme2"
        static let rightStickScheme2  = "VAMIGA_DEV_RightStickScheme2"
        static let hatSwitchScheme2   = "VAMIGA_DEV_HatSwitchScheme2"
        */
    }
}

extension PropertiesProxy {
    
    func registerDevicesUserDefaults() {
        
        /*
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        // Mapping schemes
        defaults.register(Keys.Dev.leftStickScheme1, 0)
        defaults.register(Keys.Dev.rightStickScheme1, 0)
        defaults.register(Keys.Dev.hatSwitchScheme1, 0)

        defaults.register(Keys.Dev.leftStickScheme2, 0)
        defaults.register(Keys.Dev.rightStickScheme2, 0)
        defaults.register(Keys.Dev.hatSwitchScheme2, 0)
        */
    }
    
    func removeDevicesUserDefaults() {
        
        /*
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        let keys = [ Keys.Dev.leftStickScheme1,
                     Keys.Dev.rightStickScheme1,
                     Keys.Dev.hatSwitchScheme1,
                     
                     Keys.Dev.leftStickScheme2,
                     Keys.Dev.rightStickScheme2,
                     Keys.Dev.hatSwitchScheme2 ]

        for key in keys { defaults.removeKey(key) }
        */
    }
}

extension Preferences {

    func saveDevicesUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
                        
        defaults.save()
    }
    
    func applyDevicesUserDefaults() {
           
        log(level: 2)
    }
}

//
// User defaults (Roms)
//

//
// User defaults (Chipset)
//

extension PropertiesProxy {
    
    func registerChipsetUserDefaults() {
    
    }

    func removeChipsetUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!
                     
        defaults.remove(.AGNUS_REVISION)
        defaults.remove(.DENISE_REVISION)
        defaults.remove(.CIA_REVISION)
        defaults.remove(.RTC_MODEL)
    }
}

extension Configuration {

    func applyChipsetUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        agnusRev = defaults.get(.AGNUS_REVISION)
        deniseRev = defaults.get(.DENISE_REVISION)
        ciaRev = defaults.get(.CIA_REVISION)
        rtClock = defaults.get(.RTC_MODEL)
        
        amiga.resume()
    }

    func saveChipsetUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        defaults.set(.AGNUS_REVISION, value: agnusRev)
        defaults.set(.DENISE_REVISION, value: deniseRev)
        defaults.set(.CIA_REVISION, value: ciaRev)
        defaults.set(.RTC_MODEL, value: rtClock)
        defaults.save()

        amiga.resume()
    }
}

//
// User defaults (Memory)
//

extension PropertiesProxy {
    
    func registerMemoryUserDefaults() {
    
    }
    
    func removeMemoryUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        defaults.remove(.CHIP_RAM)
        defaults.remove(.SLOW_RAM)
        defaults.remove(.FAST_RAM)
        defaults.remove(.RAM_INIT_PATTERN)
        defaults.remove(.BANKMAP)
        defaults.remove(.UNMAPPING_TYPE)
        defaults.remove(.SLOW_RAM_DELAY)
        defaults.remove(.SLOW_RAM_MIRROR)
    }
}

extension Configuration {

    func saveMemoryUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
        
        defaults.set(.CHIP_RAM, value: chipRam)
        defaults.set(.SLOW_RAM, value: slowRam)
        defaults.set(.FAST_RAM, value: fastRam)
        defaults.set(.RAM_INIT_PATTERN, value: ramInitPattern)
        defaults.set(.BANKMAP, value: bankMap)
        defaults.set(.UNMAPPING_TYPE, value: unmappingType)
        defaults.set(.SLOW_RAM_DELAY, value: slowRamDelay)
        defaults.set(.SLOW_RAM_MIRROR, value: slowRamMirror)
        defaults.save()
        
        amiga.resume()
    }
    
    func applyMemoryUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        chipRam = defaults.get(.CHIP_RAM)
        slowRam = defaults.get(.SLOW_RAM)
        fastRam = defaults.get(.FAST_RAM)
        ramInitPattern = defaults.get(.RAM_INIT_PATTERN)
        bankMap = defaults.get(.BANKMAP)
        unmappingType = defaults.get(.UNMAPPING_TYPE)
        slowRamDelay = defaults.get(.SLOW_RAM_DELAY) != 0
        slowRamMirror = defaults.get(.SLOW_RAM_MIRROR) != 0
        
        amiga.resume()
    }
}

//
// User defaults (Peripherals)
//

extension Keys {
    
    struct Per {

        // Ports
        static let gameDevice1        = "VAMIGA_PER_GameDevice1"
        static let gameDevice2        = "VAMIGA_PER_GameDevice2"
    }
}

extension PropertiesProxy {
    
    func registerPeripheralsUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        // Port assignments
        defaults.register(Keys.Per.gameDevice1, 0)
        defaults.register(Keys.Per.gameDevice2, -1)
    }

    func removePeripheralsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        for i in 0 ... 3 {
            
            defaults.remove(.DRIVE_CONNECT, nr: i)
            defaults.remove(.DRIVE_TYPE, nr: i)
            defaults.remove(.HDC_CONNECT, nr: i)
            defaults.remove(.HDR_TYPE, nr: i)
        }
        
        defaults.remove(.SERIAL_DEVICE)
        defaults.remove(.SRV_PORT)
        defaults.removeKey(Keys.Per.gameDevice1)
        defaults.removeKey(Keys.Per.gameDevice2)
    }
}
 
extension Configuration {

    func savePeripheralsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()
        
        defaults.set(.DRIVE_CONNECT, nr: 0, value: df0Connected)
        defaults.set(.DRIVE_CONNECT, nr: 1, value: df1Connected)
        defaults.set(.DRIVE_CONNECT, nr: 2, value: df2Connected)
        defaults.set(.DRIVE_CONNECT, nr: 3, value: df3Connected)
        
        defaults.set(.DRIVE_TYPE, nr: 0, value: df0Type)
        defaults.set(.DRIVE_TYPE, nr: 1, value: df1Type)
        defaults.set(.DRIVE_TYPE, nr: 2, value: df2Type)
        defaults.set(.DRIVE_TYPE, nr: 3, value: df3Type)

        defaults.set(.HDC_CONNECT, nr: 0, value: hd0Connected)
        defaults.set(.HDC_CONNECT, nr: 1, value: hd1Connected)
        defaults.set(.HDC_CONNECT, nr: 2, value: hd2Connected)
        defaults.set(.HDC_CONNECT, nr: 3, value: hd3Connected)

        defaults.set(.HDR_TYPE, nr: 0, value: hd0Type)
        defaults.set(.HDR_TYPE, nr: 1, value: hd1Type)
        defaults.set(.HDR_TYPE, nr: 2, value: hd2Type)
        defaults.set(.HDR_TYPE, nr: 3, value: hd3Type)

        defaults.set(.SERIAL_DEVICE, value: serialDevice)
        defaults.set(.SRV_PORT, value: serialDevicePort)

        defaults.set(Keys.Per.gameDevice1, gameDevice1)
        defaults.set(Keys.Per.gameDevice2, gameDevice2)
        
        defaults.save()
        
        amiga.resume()
    }

    func applyPeripheralsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        df0Connected = defaults.get(.DRIVE_CONNECT, nr: 0) != 0
        df1Connected = defaults.get(.DRIVE_CONNECT, nr: 1) != 0
        df2Connected = defaults.get(.DRIVE_CONNECT, nr: 2) != 0
        df3Connected = defaults.get(.DRIVE_CONNECT, nr: 3) != 0

        df0Type = defaults.get(.DRIVE_TYPE, nr: 0)
        df1Type = defaults.get(.DRIVE_TYPE, nr: 1)
        df2Type = defaults.get(.DRIVE_TYPE, nr: 2)
        df3Type = defaults.get(.DRIVE_TYPE, nr: 3)

        hd0Connected = defaults.get(.HDC_CONNECT, nr: 0) != 0
        hd1Connected = defaults.get(.HDC_CONNECT, nr: 1) != 0
        hd2Connected = defaults.get(.HDC_CONNECT, nr: 2) != 0
        hd3Connected = defaults.get(.HDC_CONNECT, nr: 3) != 0

        hd0Type = defaults.get(.HDR_TYPE, nr: 0)
        hd1Type = defaults.get(.HDR_TYPE, nr: 1)
        hd2Type = defaults.get(.HDR_TYPE, nr: 2)
        hd3Type = defaults.get(.HDR_TYPE, nr: 3)

        serialDevice = defaults.get(.SERIAL_DEVICE)
        serialDevicePort = defaults.get(.SRV_PORT, nr: ServerType.SER.rawValue)

        gameDevice1 = defaults.getInt(Keys.Per.gameDevice1)
        gameDevice2 = defaults.getInt(Keys.Per.gameDevice2)

        amiga.resume()
    }
}

//
// User defaults (Compatibility)
//

extension PropertiesProxy {
    
    func registerCompatibilityUserDefaults() {

        log(level: 2)
    }
    
    func removeCompatibilityUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        defaults.remove(.BLITTER_ACCURACY)
        defaults.remove(.TODBUG)
        defaults.remove(.ECLOCK_SYNCING)
        defaults.remove(.CLX_SPR_SPR)
        defaults.remove(.CLX_SPR_PLF)
        defaults.remove(.CLX_PLF_PLF)
        defaults.remove(.DRIVE_SPEED)
        defaults.remove(.EMULATE_MECHANICS)
        defaults.remove(.LOCK_DSKSYNC)
        defaults.remove(.AUTO_DSKSYNC)
        defaults.remove(.ACCURATE_KEYBOARD)
    }
}

extension Configuration {

    func saveCompatibilityUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
        
        defaults.set(.BLITTER_ACCURACY, value: blitterAccuracy)
        defaults.set(.TODBUG, value: todBug)
        defaults.set(.ECLOCK_SYNCING, value: eClockSyncing)
        defaults.set(.CLX_SPR_SPR, value: clxSprSpr)
        defaults.set(.CLX_SPR_PLF, value: clxSprPlf)
        defaults.set(.CLX_PLF_PLF, value: clxPlfPlf)
        defaults.set(.DRIVE_SPEED, value: driveSpeed)
        defaults.set(.EMULATE_MECHANICS, value: mechanicalDelays)
        defaults.set(.LOCK_DSKSYNC, value: lockDskSync)
        defaults.set(.AUTO_DSKSYNC, value: autoDskSync)
        defaults.set(.ACCURATE_KEYBOARD, value: accurateKeyboard)
        defaults.save()
        
        amiga.resume()
    }

    func applyCompatibilityUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
        
        blitterAccuracy = defaults.get(.BLITTER_ACCURACY)
        todBug = defaults.get(.TODBUG) != 0
        eClockSyncing = defaults.get(.ECLOCK_SYNCING) != 0
        clxSprSpr = defaults.get(.CLX_SPR_SPR) != 0
        clxSprPlf = defaults.get(.CLX_SPR_PLF) != 0
        clxPlfPlf = defaults.get(.CLX_PLF_PLF) != 0
        driveSpeed = defaults.get(.DRIVE_SPEED)
        mechanicalDelays = defaults.get(.EMULATE_MECHANICS, nr: 0) != 0
        lockDskSync = defaults.get(.LOCK_DSKSYNC) != 0
        autoDskSync = defaults.get(.AUTO_DSKSYNC) != 0
        accurateKeyboard = defaults.get(.ACCURATE_KEYBOARD) != 0
        
        amiga.resume()
    }
}

//
// User defaults (Audio)
//

extension PropertiesProxy {
    
    func registerAudioUserDefaults() {

        log(level: 2)
    }
    
    func removeAudioUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        for i in 0 ... 3 {
            
            defaults.remove(.AUDVOL, nr: i)
            defaults.remove(.AUDPAN, nr: i)
            defaults.remove(.DRIVE_PAN, nr: i)
            defaults.remove(.HDR_PAN, nr: i)
        }
        
        defaults.remove(.AUDVOLL)
        defaults.remove(.AUDVOLR)
        defaults.remove(.SAMPLING_METHOD)
        defaults.remove(.STEP_VOLUME)
        defaults.remove(.POLL_VOLUME)
        defaults.remove(.INSERT_VOLUME)
        defaults.remove(.EJECT_VOLUME)
        defaults.remove(.FILTER_TYPE)
        defaults.remove(.FILTER_ALWAYS_ON)
    }
}

extension Configuration {
    
    func saveAudioUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()
        
        defaults.set(.AUDVOL, nr: 0, value: vol0)
        defaults.set(.AUDVOL, nr: 1, value: vol1)
        defaults.set(.AUDVOL, nr: 2, value: vol2)
        defaults.set(.AUDVOL, nr: 3, value: vol3)
        defaults.set(.AUDPAN, nr: 0, value: pan0)
        defaults.set(.AUDPAN, nr: 1, value: pan1)
        defaults.set(.AUDPAN, nr: 2, value: pan2)
        defaults.set(.AUDPAN, nr: 3, value: pan3)
        defaults.set(.AUDVOLL, value: volL)
        defaults.set(.AUDVOLR, value: volR)
        defaults.set(.SAMPLING_METHOD, value: samplingMethod)
        defaults.set(.DRIVE_PAN, nr: 0, value: df0Pan)
        defaults.set(.DRIVE_PAN, nr: 1, value: df1Pan)
        defaults.set(.DRIVE_PAN, nr: 2, value: df2Pan)
        defaults.set(.DRIVE_PAN, nr: 3, value: df3Pan)
        defaults.set(.HDR_PAN, nr: 0, value: hd0Pan)
        defaults.set(.HDR_PAN, nr: 1, value: hd1Pan)
        defaults.set(.HDR_PAN, nr: 2, value: hd2Pan)
        defaults.set(.HDR_PAN, nr: 3, value: hd3Pan)
        defaults.set(.STEP_VOLUME, value: stepVolume)
        defaults.set(.POLL_VOLUME, value: pollVolume)
        defaults.set(.INSERT_VOLUME, value: insertVolume)
        defaults.set(.EJECT_VOLUME, value: ejectVolume)
        defaults.set(.FILTER_TYPE, value: filterType)
        defaults.set(.FILTER_ALWAYS_ON, value: filterAlwaysOn)
        defaults.save()
        
        amiga.resume()
    }
    
    func applyAudioUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        vol0 = defaults.get(.AUDVOL, nr: 0)
        vol1 = defaults.get(.AUDVOL, nr: 1)
        vol2 = defaults.get(.AUDVOL, nr: 2)
        vol3 = defaults.get(.AUDVOL, nr: 3)

        pan0 = defaults.get(.AUDPAN, nr: 0)
        pan1 = defaults.get(.AUDPAN, nr: 1)
        pan2 = defaults.get(.AUDPAN, nr: 2)
        pan3 = defaults.get(.AUDPAN, nr: 3)

        df0Pan = defaults.get(.DRIVE_PAN, nr: 0)
        df1Pan = defaults.get(.DRIVE_PAN, nr: 1)
        df2Pan = defaults.get(.DRIVE_PAN, nr: 2)
        df3Pan = defaults.get(.DRIVE_PAN, nr: 3)

        hd0Pan = defaults.get(.HDR_PAN, nr: 0)
        hd1Pan = defaults.get(.HDR_PAN, nr: 1)
        hd2Pan = defaults.get(.HDR_PAN, nr: 2)
        hd3Pan = defaults.get(.HDR_PAN, nr: 3)

        volL = defaults.get(.AUDVOLL)
        volR = defaults.get(.AUDVOLR)
        samplingMethod = defaults.get(.SAMPLING_METHOD)
        stepVolume = defaults.get(.STEP_VOLUME, nr: 0)
        pollVolume = defaults.get(.POLL_VOLUME, nr: 0)
        insertVolume = defaults.get(.INSERT_VOLUME, nr: 0)
        ejectVolume = defaults.get(.EJECT_VOLUME, nr: 0)
        filterType = defaults.get(.FILTER_TYPE)
        filterAlwaysOn = defaults.get(.FILTER_ALWAYS_ON) != 0

        amiga.resume()
    }
}

//
// User defaults (Video)
//

extension Keys {
    
    struct Vid {

        // Geometry
        static let hAutoCenter        = "VAMIGA_VID_HAutoCenter"
        static let vAutoCenter        = "VAMIGA_VID_VAutoCenter"
        static let hCenter            = "VAMIGA_VID_HCenter"
        static let vCenter            = "VAMIGA_VID_VCenter"
        static let hZoom              = "VAMIGA_VID_HZoom"
        static let vZoom              = "VAMIGA_VID_VZoom"
        
        // Monitor
        static let enhancer           = "VVAMIG_VID_Enhancer"
        static let upscaler           = "VAMIGA_VID_Upscaler"
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

extension PropertiesProxy {
    
    func registerVideoUserDefaults() {

        log(level: 2)
        
        registerColorUserDefaults()
        registerGeometryUserDefaults()
        registerShaderUserDefaults()
    }

    func registerColorUserDefaults() {
        
        // No GUI related keys in this category
    }

    func registerGeometryUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!

        defaults.register(Keys.Vid.hAutoCenter, true)
        defaults.register(Keys.Vid.vAutoCenter, true)
        defaults.register(Keys.Vid.hCenter, 0.6)
        defaults.register(Keys.Vid.vCenter, 0.47)
        defaults.register(Keys.Vid.hZoom, 1.0)
        defaults.register(Keys.Vid.vZoom, 0.27)
    }
    
    func registerShaderUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        defaults.register(Keys.Vid.enhancer, 0)
        defaults.register(Keys.Vid.upscaler, 0)
        defaults.register(Keys.Vid.blur, 1)
        defaults.register(Keys.Vid.blurRadius, 0)
        defaults.register(Keys.Vid.bloom, 0)
        defaults.register(Keys.Vid.bloomRadius, 1.0)
        defaults.register(Keys.Vid.bloomBrightness, 0.4)
        defaults.register(Keys.Vid.bloomWeight, 1.21)
        defaults.register(Keys.Vid.flicker, 1)
        defaults.register(Keys.Vid.flickerWeight, 0.25)
        defaults.register(Keys.Vid.dotMask, 0)
        defaults.register(Keys.Vid.dotMaskBrightness, 0.55)
        defaults.register(Keys.Vid.scanlines, 0)
        defaults.register(Keys.Vid.scanlineBrightness, 0.55)
        defaults.register(Keys.Vid.scanlineWeight, 0.11)
        defaults.register(Keys.Vid.disalignment, 0)
        defaults.register(Keys.Vid.disalignmentH, 0.001)
        defaults.register(Keys.Vid.disalignmentV, 0.001)
    }
    
    func removeVideoUserDefaults() {

        log(level: 2)
        
        removeColorUserDefaults()
        removeGeometryUserDefaults()
        removeShaderUserDefaults()
    }
    
    func removeColorUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!

        defaults.remove(.PALETTE)
        defaults.remove(.BRIGHTNESS)
        defaults.remove(.CONTRAST)
        defaults.remove(.SATURATION)
    }

    func removeGeometryUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        let keys = [ Keys.Vid.hAutoCenter,
                     Keys.Vid.vAutoCenter,
                     Keys.Vid.hCenter,
                     Keys.Vid.vCenter,
                     Keys.Vid.hZoom,
                     Keys.Vid.vZoom ]

        for key in keys { defaults.removeKey(key) }
    }
    
    func removeShaderUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        let keys = [ Keys.Vid.enhancer,
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
                     Keys.Vid.disalignmentV ]
        
        for key in keys { defaults.removeKey(key) }
    }
}

extension Configuration {

    func saveVideoUserDefaults() {
        
        log(level: 2)
        
        saveColorUserDefaults()
        saveGeometryUserDefaults()
        saveShaderUserDefaults()
    }
         
    func saveColorUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
        
        defaults.set(.PALETTE, value: palette)
        defaults.set(.BRIGHTNESS, value: brightness)
        defaults.set(.CONTRAST, value: contrast)
        defaults.set(.SATURATION, value: saturation)
        
        defaults.save()
        
        amiga.resume()
    }
    
    func saveGeometryUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
                
        defaults.set(Keys.Vid.hAutoCenter, hAutoCenter)
        defaults.set(Keys.Vid.vAutoCenter, vAutoCenter)
        defaults.set(Keys.Vid.hCenter, hCenter)
        defaults.set(Keys.Vid.vCenter, vCenter)
        defaults.set(Keys.Vid.hZoom, hZoom)
        defaults.set(Keys.Vid.vZoom, vZoom)
        
        defaults.save()
        
        amiga.resume()
    }
  
    func saveShaderUserDefaults() {

        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
                        
        defaults.set(Keys.Vid.enhancer, enhancer)
        defaults.set(Keys.Vid.upscaler, upscaler)
        defaults.set(Keys.Vid.blur, blur)
        defaults.set(Keys.Vid.blurRadius, blurRadius)
        defaults.set(Keys.Vid.bloom, bloom)
        defaults.set(Keys.Vid.bloomRadius, bloomRadius)
        defaults.set(Keys.Vid.bloomBrightness, bloomBrightness)
        defaults.set(Keys.Vid.bloomWeight, bloomWeight)
        defaults.set(Keys.Vid.flicker, flicker)
        defaults.set(Keys.Vid.flickerWeight, flickerWeight)
        defaults.set(Keys.Vid.dotMask, dotMask)
        defaults.set(Keys.Vid.dotMaskBrightness, dotMaskBrightness)
        defaults.set(Keys.Vid.scanlines, scanlines)
        defaults.set(Keys.Vid.scanlineBrightness, scanlineBrightness)
        defaults.set(Keys.Vid.scanlineWeight, scanlineWeight)
        defaults.set(Keys.Vid.disalignment, disalignment)
        defaults.set(Keys.Vid.disalignmentH, disalignmentH)
        defaults.set(Keys.Vid.disalignmentV, disalignmentV)
        
        defaults.save()
        
        amiga.resume()
    }
    
    func applyVideoUserDefaults() {
        
        log(level: 2)
        
        applyColorUserDefaults()
        applyGeometryUserDefaults()
        applyShaderUserDefaults()
    }
    
    func applyColorUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
        
        palette = defaults.get(.PALETTE)
        brightness = defaults.get(.BRIGHTNESS)
        contrast = defaults.get(.CONTRAST)
        saturation = defaults.get(.SATURATION)
        
        amiga.resume()
    }

    func applyGeometryUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
          
        hAutoCenter = defaults.getBool(Keys.Vid.hAutoCenter)
        vAutoCenter = defaults.getBool(Keys.Vid.vAutoCenter)
        hCenter = defaults.getFloat(Keys.Vid.hCenter)
        hCenter = defaults.getFloat(Keys.Vid.vCenter)
        hZoom = defaults.getFloat(Keys.Vid.hZoom)
        vZoom = defaults.getFloat(Keys.Vid.vZoom)
                
        amiga.resume()
    }

    func applyShaderUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
                        
        enhancer = defaults.getInt(Keys.Vid.enhancer)
        upscaler = defaults.getInt(Keys.Vid.upscaler)
        blur = defaults.getInt32(Keys.Vid.blur)
        blurRadius = defaults.getFloat(Keys.Vid.blurRadius)
        bloom = defaults.getInt(Keys.Vid.bloom)
        bloomRadius = defaults.getFloat(Keys.Vid.bloomRadius)
        bloomBrightness = defaults.getFloat(Keys.Vid.bloomBrightness)
        bloomWeight = defaults.getFloat(Keys.Vid.bloomWeight)
        flicker = defaults.getInt32(Keys.Vid.flicker)
        flickerWeight = defaults.getFloat(Keys.Vid.flickerWeight)
        dotMask = defaults.getInt(Keys.Vid.dotMask)
        dotMaskBrightness = defaults.getFloat(Keys.Vid.dotMaskBrightness)
        scanlines = defaults.getInt(Keys.Vid.scanlines)
        scanlineBrightness = defaults.getFloat(Keys.Vid.scanlineBrightness)
        scanlineWeight = defaults.getFloat(Keys.Vid.scanlineWeight)
        disalignment = defaults.getInt32(Keys.Vid.disalignment)
        disalignmentH = defaults.getFloat(Keys.Vid.disalignmentH)
        disalignmentV = defaults.getFloat(Keys.Vid.disalignmentV)
        
        amiga.resume()
    }
}
