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

        set("ROM_PATH", UserDefaults.romUrl!.path)
        set("EXT_PATH", UserDefaults.extUrl!.path)
        set("HD0_PATH", UserDefaults.hd0Url!.path)
        set("HD1_PATH", UserDefaults.hd1Url!.path)
        set("HD2_PATH", UserDefaults.hd2Url!.path)
        set("HD3_PATH", UserDefaults.hd3Url!.path)

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

    func register(_ key: String, _ val: String) {
        register(key, value: val)
    }
    func register(_ key: String, _ val: Bool) {
        register(key, value: val ? "1" : "0")
    }
    func register(_ key: String, _ val: Int) {
        register(key, value: "\(val)")
    }
    func register(_ key: String, _ val: UInt) {
        register(key, value: "\(val)")
    }
    func register(_ key: String, _ val: Float) {
        register(key, value: "\(val)")
    }
    func register(_ key: String, _ val: Double) {
        register(key, value: "\(val)")
    }
    func remove(_ option: Option, _ nr: Int) {
        remove(option, nr: nr)
    }
    func remove(_ option: Option, _ nr: [Int]) {
        for n in nr { remove(option, nr: n) }
    }
    func set(_ key: String, _ val: String) {
        setKey(key, value: val)
    }
    func set(_ key: String, _ val: Bool) {
        setKey(key, value: val ? "1" : "0")
    }
    func set(_ key: String, _ val: Int) {
        setKey(key, value: "\(val)")
    }
    func set(_ key: String, _ val: UInt) {
        setKey(key, value: "\(val)")
    }
    func set(_ key: String, _ val: Float) {
        setKey(key, value: "\(val)")
    }
    func set(_ key: String, _ val: Double) {
        setKey(key, value: "\(val)")
    }
    func set(_ option: Option, _ val: Int) {
        setOpt(option, value: val)
    }
    func set(_ option: Option, _ val: Bool) {
        setOpt(option, value: val ? 1 : 0)
    }
    func set(_ option: Option, _ nr: Int, _ val: Int) {
        setOpt(option, nr: nr, value: val)
    }
    func set(_ option: Option, _ nr: Int, _ val: Bool) {
        setOpt(option, nr: nr, value: val ? 1 : 0)
    }
    func set(_ option: Option, _ nr: [Int], _ val: Int) {
        for n in nr { setOpt(option, nr: n, value: val) }
    }
    func set(_ option: Option, _ nr: [Int], _ val: Bool) {
        for n in nr { setOpt(option, nr: n, value: val ? 1 : 0) }
    }
    func get(_ option: Option) -> Int {
        return getOpt(option)
    }
    func get(_ option: Option, _ nr: Int) -> Int {
        return getOpt(option, nr: nr)
    }
    func string(_ key: String) -> String {
        return getString(key) ?? ""
    }
    func bool(_ key: String) -> Bool {
        return getInt(key) != 0
    }
    func int(_ key: String) -> Int {
        return getInt(key)
    }
    func float(_ key: String) -> Float {
        return (getString(key) as NSString).floatValue
    }
    func double(_ key: String) -> Double {
        return (getString(key) as NSString).doubleValue
    }

    func register<T: Encodable>(_ key: String, encodable item: T) {
        
        let jsonData = try? JSONEncoder().encode(item)
        let jsonString = jsonData?.base64EncodedString() ?? ""
        register(key, jsonString)
    }

    func encode<T: Encodable>(_ key: String, _ item: T) {
        
        let jsonData = try? JSONEncoder().encode(item)
        let jsonString = jsonData?.base64EncodedString() ?? ""
        set(key, jsonString)
    }

    func decode<T: Decodable>(_ key: String, _ item: inout T) {
        
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
        
        log("Registering user defaults")
        
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
        static let autoSnapshots          = "General.AutoSnapshots"
        static let autoSnapshotInterval   = "General.ScreenshotInterval"

        // Screenshots
        static let screenshotSource       = "General.ScreenshotSource"
        static let screenshotTarget       = "General.ScreenshotTarget"
                
        // Screen captures
        static let ffmpegPath             = "General.ffmpegPath"
        static let captureSource          = "General.Source"
        static let bitRate                = "General.BitRate"
        static let aspectX                = "General.AspectX"
        static let aspectY                = "General.AspectY"
        
        // Fullscreen
        static let keepAspectRatio        = "General.FullscreenKeepAspectRatio"
        static let exitOnEsc              = "General.FullscreenExitOnEsc"
        
        // Warp mode
        static let warpMode               = "General.WarpMode"
        
        // Miscellaneous
        static let ejectWithoutAsking     = "General.EjectWithoutAsking"
        static let detachWithoutAsking    = "General.DetachWithoutAsking"
        static let closeWithoutAsking     = "General.CloseWithoutAsking"
        static let pauseInBackground      = "General.PauseInBackground"
    }
}

extension PropertiesProxy {
    
    func registerGeneralUserDefaults() {
        
        log(level: 2)
        
        // Snapshots
        register(Keys.Gen.autoSnapshots, false)
        register(Keys.Gen.autoSnapshotInterval, 20)
        
        // Screenshots
        register(Keys.Gen.screenshotSource, 0)
        register(Keys.Gen.screenshotTarget, NSBitmapImageRep.FileType.png.rawValue)
        
        // Captures
        register(Keys.Gen.ffmpegPath, "")
        register(Keys.Gen.captureSource, 0)
        register(Keys.Gen.bitRate, 2048)
        register(Keys.Gen.aspectX, 768)
        register(Keys.Gen.aspectY, 702)
        
        // Fullscreen
        register(Keys.Gen.keepAspectRatio, false)
        register(Keys.Gen.exitOnEsc, true)
        
        // Warp mode
        register(Keys.Gen.warpMode, WarpMode.off.rawValue)
        
        // Misc
        register(Keys.Gen.ejectWithoutAsking, false)
        register(Keys.Gen.detachWithoutAsking, false)
        register(Keys.Gen.closeWithoutAsking, false)
        register(Keys.Gen.pauseInBackground, false)
    }
    
    func removeGeneralUserDefaults() {
        
        log(level: 2)
        
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
        
        for key in keys { removeKey(key) }
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
        
        autoSnapshots = defaults.bool(Keys.Gen.autoSnapshots)
        snapshotInterval = defaults.int(Keys.Gen.autoSnapshotInterval)
        
        screenshotSource = defaults.int(Keys.Gen.screenshotSource)
        screenshotTargetIntValue = defaults.int(Keys.Gen.screenshotTarget)
        
        ffmpegPath = defaults.string(Keys.Gen.ffmpegPath)
        captureSource = defaults.int(Keys.Gen.captureSource)
        bitRate = defaults.int(Keys.Gen.bitRate)
        aspectX = defaults.int(Keys.Gen.aspectX)
        aspectY = defaults.int(Keys.Gen.aspectY)
        
        keepAspectRatio = defaults.bool(Keys.Gen.keepAspectRatio)
        exitOnEsc = defaults.bool(Keys.Gen.exitOnEsc)
        
        warpModeIntValue = defaults.int(Keys.Gen.warpMode)
        
        ejectWithoutAsking = defaults.bool(Keys.Gen.ejectWithoutAsking)
        detachWithoutAsking = defaults.bool(Keys.Gen.detachWithoutAsking)
        closeWithoutAsking = defaults.bool(Keys.Gen.closeWithoutAsking)
        pauseInBackground = defaults.bool(Keys.Gen.pauseInBackground)
    }
}

//
// User defaults (Controls)
//

extension Keys {
    
    struct Con {
        
        // Emulation keys
        static let mouseKeyMap           = "Controls.MouseKeyMap"
        static let joyKeyMap1            = "Controls.JoyKeyMap1"
        static let joyKeyMap2            = "Controls.JoyKeyMap2"
        static let disconnectJoyKeys     = "Controls.DisconnectKeys"
        
        // Joysticks
        static let autofire              = "Controls.Autofire"
        static let autofireBullets       = "Controls.AutofireBullets"
        static let autofireFrequency     = "Controls.AutofireFrequency"
        
        // Mouse
        static let retainMouseKeyComb    = "Controls.RetainMouseKeyComb"
        static let retainMouseWithKeys   = "Controls.RetainMouseWithKeys"
        static let retainMouseByClick    = "Controls.RetainMouseByClick"
        static let retainMouseByEntering = "Controls.RetainMouseByEntering"
        static let releaseMouseKeyComb   = "Controls.ReleaseMouseKeyComb"
        static let releaseMouseWithKeys  = "Controls.ReleaseMouseWithKeys"
        static let releaseMouseByShaking = "Controls.ReleaseMouseByShaking"
    }
}

extension PropertiesProxy {
    
    func registerControlsUserDefaults() {
        
        log(level: 2)

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
        register(Keys.Con.mouseKeyMap, encodable: emptyMap)
        register(Keys.Con.joyKeyMap1, encodable: stdKeyMap1)
        register(Keys.Con.joyKeyMap2, encodable: stdKeyMap2)
        register(Keys.Con.disconnectJoyKeys, true)
        
        // Joysticks
        register(Keys.Con.autofire, false)
        register(Keys.Con.autofireBullets, -3)
        register(Keys.Con.autofireFrequency, 2.5)

        // Mouse
        register(Keys.Con.retainMouseKeyComb, 0)
        register(Keys.Con.retainMouseWithKeys, true)
        register(Keys.Con.retainMouseByClick, true)
        register(Keys.Con.retainMouseByEntering, false)
        register(Keys.Con.releaseMouseKeyComb, 0)
        register(Keys.Con.releaseMouseWithKeys, true)
        register(Keys.Con.releaseMouseByShaking, true)
    }
    
    func removeControlsUserDefaults() {
        
        log(level: 2)
        
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

        for key in keys { removeKey(key) }
    }
}

extension Preferences {

    func saveControlsUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
                
        defaults.encode(Keys.Con.mouseKeyMap, keyMaps[0])
        defaults.encode(Keys.Con.joyKeyMap1, keyMaps[1])
        defaults.encode(Keys.Con.joyKeyMap2, keyMaps[2])
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
        
        defaults.decode(Keys.Con.mouseKeyMap, &keyMaps[0])
        defaults.decode(Keys.Con.joyKeyMap1, &keyMaps[1])
        defaults.decode(Keys.Con.joyKeyMap2, &keyMaps[2])
        disconnectJoyKeys = defaults.bool(Keys.Con.disconnectJoyKeys)
        
        autofire = defaults.bool(Keys.Con.autofire)
        autofireBullets = defaults.int(Keys.Con.autofireBullets)
        autofireFrequency = defaults.double(Keys.Con.autofireFrequency)
        
        retainMouseKeyComb = defaults.int(Keys.Con.retainMouseKeyComb)
        retainMouseWithKeys = defaults.bool(Keys.Con.retainMouseWithKeys)
        retainMouseByClick = defaults.bool(Keys.Con.retainMouseByClick)
        retainMouseByEntering = defaults.bool(Keys.Con.retainMouseByEntering)
        releaseMouseKeyComb = defaults.int(Keys.Con.releaseMouseKeyComb)
        releaseMouseWithKeys = defaults.bool(Keys.Con.releaseMouseWithKeys)
        releaseMouseByShaking = defaults.bool(Keys.Con.releaseMouseByShaking)
    }
}

//
// User defaults (Devices)
//

extension Keys {
    
    struct Dev {

        static let schemes            = "DevSchemes"

        // Mapping schemes (DEPRECATED)
        /*
        static let leftStickScheme1   = "DevLeftStickScheme1"
        static let rightStickScheme1  = "DevRightStickScheme1"
        static let hatSwitchScheme1   = "DevHatSwitchScheme1"

        static let leftStickScheme2   = "DevLeftStickScheme2"
        static let rightStickScheme2  = "DevRightStickScheme2"
        static let hatSwitchScheme2   = "DevHatSwitchScheme2"
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
    
        log(level: 2)
        // No GUI related items in this sections
    }

    func removeChipsetUserDefaults() {

        log(level: 2)
                     
        remove(.AGNUS_REVISION)
        remove(.DENISE_REVISION)
        remove(.CIA_REVISION)
        remove(.RTC_MODEL)
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

        defaults.set(.AGNUS_REVISION, agnusRev)
        defaults.set(.DENISE_REVISION, deniseRev)
        defaults.set(.CIA_REVISION, ciaRev)
        defaults.set(.RTC_MODEL, rtClock)
        defaults.save()

        amiga.resume()
    }
}

//
// User defaults (Memory)
//

extension PropertiesProxy {
    
    func registerMemoryUserDefaults() {
    
        log(level: 2)
        // No GUI related items in this sections
    }
    
    func removeMemoryUserDefaults() {
        
        log(level: 2)

        remove(.CHIP_RAM)
        remove(.SLOW_RAM)
        remove(.FAST_RAM)
        remove(.RAM_INIT_PATTERN)
        remove(.BANKMAP)
        remove(.UNMAPPING_TYPE)
        remove(.SLOW_RAM_DELAY)
        remove(.SLOW_RAM_MIRROR)
    }
}

extension Configuration {

    func saveMemoryUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
        
        defaults.set(.CHIP_RAM, chipRam)
        defaults.set(.SLOW_RAM, slowRam)
        defaults.set(.FAST_RAM, fastRam)
        defaults.set(.RAM_INIT_PATTERN, ramInitPattern)
        defaults.set(.BANKMAP, bankMap)
        defaults.set(.UNMAPPING_TYPE, unmappingType)
        defaults.set(.SLOW_RAM_DELAY, slowRamDelay)
        defaults.set(.SLOW_RAM_MIRROR, slowRamMirror)
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
        static let gameDevice1        = "Peripherals.GameDevice1"
        static let gameDevice2        = "Peripherals.GameDevice2"
    }
}

extension PropertiesProxy {
    
    func registerPeripheralsUserDefaults() {

        log(level: 2)
        
        // Port assignments
        register(Keys.Per.gameDevice1, 0)
        register(Keys.Per.gameDevice2, -1)
    }

    func removePeripheralsUserDefaults() {
        
        log(level: 2)
        
        remove(.DRIVE_CONNECT, [0, 1, 2, 3])
        remove(.DRIVE_TYPE, [0, 1, 2, 3])
        remove(.HDC_CONNECT, [0, 1, 2, 3])
        remove(.HDR_TYPE, [0, 1, 2, 3])
        remove(.SERIAL_DEVICE)
        remove(.SRV_PORT, nr: ServerType.SER.rawValue)
        removeKey(Keys.Per.gameDevice1)
        removeKey(Keys.Per.gameDevice2)
    }
}
 
extension Configuration {

    func savePeripheralsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()
        
        defaults.set(.DRIVE_CONNECT, 0, df0Connected)
        defaults.set(.DRIVE_CONNECT, 1, df1Connected)
        defaults.set(.DRIVE_CONNECT, 2, df2Connected)
        defaults.set(.DRIVE_CONNECT, 3, df3Connected)
        
        defaults.set(.DRIVE_TYPE, 0, df0Type)
        defaults.set(.DRIVE_TYPE, 1, df1Type)
        defaults.set(.DRIVE_TYPE, 2, df2Type)
        defaults.set(.DRIVE_TYPE, 3, df3Type)

        defaults.set(.HDC_CONNECT, 0, hd0Connected)
        defaults.set(.HDC_CONNECT, 1, hd1Connected)
        defaults.set(.HDC_CONNECT, 2, hd2Connected)
        defaults.set(.HDC_CONNECT, 3, hd3Connected)

        defaults.set(.HDR_TYPE, 0, hd0Type)
        defaults.set(.HDR_TYPE, 1, hd1Type)
        defaults.set(.HDR_TYPE, 2, hd2Type)
        defaults.set(.HDR_TYPE, 3, hd3Type)

        defaults.set(.SERIAL_DEVICE, serialDevice)
        defaults.set(.SRV_PORT, ServerType.SER.rawValue, serialDevicePort)

        defaults.set(Keys.Per.gameDevice1, gameDevice1)
        defaults.set(Keys.Per.gameDevice2, gameDevice2)
        
        defaults.save()
        
        amiga.resume()
    }

    func applyPeripheralsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        df0Connected = defaults.get(.DRIVE_CONNECT, 0) != 0
        df1Connected = defaults.get(.DRIVE_CONNECT, 1) != 0
        df2Connected = defaults.get(.DRIVE_CONNECT, 2) != 0
        df3Connected = defaults.get(.DRIVE_CONNECT, 3) != 0

        df0Type = defaults.get(.DRIVE_TYPE, 0)
        df1Type = defaults.get(.DRIVE_TYPE, 1)
        df2Type = defaults.get(.DRIVE_TYPE, 2)
        df3Type = defaults.get(.DRIVE_TYPE, 3)

        hd0Connected = defaults.get(.HDC_CONNECT, 0) != 0
        hd1Connected = defaults.get(.HDC_CONNECT, 1) != 0
        hd2Connected = defaults.get(.HDC_CONNECT, 2) != 0
        hd3Connected = defaults.get(.HDC_CONNECT, 3) != 0

        hd0Type = defaults.get(.HDR_TYPE, 0)
        hd1Type = defaults.get(.HDR_TYPE, 1)
        hd2Type = defaults.get(.HDR_TYPE, 2)
        hd3Type = defaults.get(.HDR_TYPE, 3)

        serialDevice = defaults.get(.SERIAL_DEVICE)
        serialDevicePort = defaults.get(.SRV_PORT, ServerType.SER.rawValue)

        gameDevice1 = defaults.int(Keys.Per.gameDevice1)
        gameDevice2 = defaults.int(Keys.Per.gameDevice2)

        amiga.resume()
    }
}

//
// User defaults (Compatibility)
//

extension PropertiesProxy {
    
    func registerCompatibilityUserDefaults() {

        log(level: 2)
        // No GUI related items in this sections
    }
    
    func removeCompatibilityUserDefaults() {
        
        log(level: 2)
        
        remove(.BLITTER_ACCURACY)
        remove(.TODBUG)
        remove(.ECLOCK_SYNCING)
        remove(.CLX_SPR_SPR)
        remove(.CLX_SPR_PLF)
        remove(.CLX_PLF_PLF)
        remove(.DRIVE_SPEED)
        remove(.EMULATE_MECHANICS, [ 0, 1, 2, 3])
        remove(.LOCK_DSKSYNC)
        remove(.AUTO_DSKSYNC)
        remove(.ACCURATE_KEYBOARD)
    }
}

extension Configuration {

    func saveCompatibilityUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
        
        defaults.set(.BLITTER_ACCURACY, blitterAccuracy)
        defaults.set(.TODBUG, todBug)
        defaults.set(.ECLOCK_SYNCING, eClockSyncing)
        defaults.set(.CLX_SPR_SPR, clxSprSpr)
        defaults.set(.CLX_SPR_PLF, clxSprPlf)
        defaults.set(.CLX_PLF_PLF, clxPlfPlf)
        defaults.set(.DRIVE_SPEED, driveSpeed)
        defaults.set(.EMULATE_MECHANICS, [0, 1, 2, 3], mechanicalDelays)
        defaults.set(.LOCK_DSKSYNC, lockDskSync)
        defaults.set(.AUTO_DSKSYNC, autoDskSync)
        defaults.set(.ACCURATE_KEYBOARD, accurateKeyboard)
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
        mechanicalDelays = defaults.get(.EMULATE_MECHANICS, 0) != 0
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
        // No GUI related items in this sections
    }
    
    func removeAudioUserDefaults() {
        
        log(level: 2)
            
        remove(.AUDVOL, [0, 1, 2, 3])
        remove(.AUDPAN, [0, 1, 2, 3])
        remove(.DRIVE_PAN, [0, 1, 2, 3])
        remove(.HDR_PAN, [0, 1, 2, 3])
        remove(.AUDVOLL)
        remove(.AUDVOLR)
        remove(.SAMPLING_METHOD)
        remove(.STEP_VOLUME, [0, 1, 2, 3])
        remove(.POLL_VOLUME, [0, 1, 2, 3])
        remove(.INSERT_VOLUME, [0, 1, 2, 3])
        remove(.EJECT_VOLUME, [0, 1, 2, 3])
        remove(.FILTER_TYPE)
        remove(.FILTER_ALWAYS_ON)
    }
}

extension Configuration {
    
    func saveAudioUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()
        
        defaults.set(.AUDVOL, 0, vol0)
        defaults.set(.AUDVOL, 1, vol1)
        defaults.set(.AUDVOL, 2, vol2)
        defaults.set(.AUDVOL, 3, vol3)
        defaults.set(.AUDPAN, 0, pan0)
        defaults.set(.AUDPAN, 1, pan1)
        defaults.set(.AUDPAN, 2, pan2)
        defaults.set(.AUDPAN, 3, pan3)
        defaults.set(.AUDVOLL, volL)
        defaults.set(.AUDVOLR, volR)
        defaults.set(.SAMPLING_METHOD, samplingMethod)
        defaults.set(.DRIVE_PAN, 0, df0Pan)
        defaults.set(.DRIVE_PAN, 1, df1Pan)
        defaults.set(.DRIVE_PAN, 2, df2Pan)
        defaults.set(.DRIVE_PAN, 3, df3Pan)
        defaults.set(.HDR_PAN, 0, hd0Pan)
        defaults.set(.HDR_PAN, 1, hd1Pan)
        defaults.set(.HDR_PAN, 2, hd2Pan)
        defaults.set(.HDR_PAN, 3, hd3Pan)
        defaults.set(.STEP_VOLUME, [0, 1, 2, 3], stepVolume)
        defaults.set(.POLL_VOLUME, [0, 1, 2, 3], pollVolume)
        defaults.set(.INSERT_VOLUME, [0, 1, 2, 3], insertVolume)
        defaults.set(.EJECT_VOLUME, [0, 1, 2, 3], ejectVolume)
        defaults.set(.FILTER_TYPE, filterType)
        defaults.set(.FILTER_ALWAYS_ON, filterAlwaysOn)
        defaults.save()
        
        amiga.resume()
    }
    
    func applyAudioUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        vol0 = defaults.get(.AUDVOL, 0)
        vol1 = defaults.get(.AUDVOL, 1)
        vol2 = defaults.get(.AUDVOL, 2)
        vol3 = defaults.get(.AUDVOL, 3)

        pan0 = defaults.get(.AUDPAN, 0)
        pan1 = defaults.get(.AUDPAN, 1)
        pan2 = defaults.get(.AUDPAN, 2)
        pan3 = defaults.get(.AUDPAN, 3)

        df0Pan = defaults.get(.DRIVE_PAN, 0)
        df1Pan = defaults.get(.DRIVE_PAN, 1)
        df2Pan = defaults.get(.DRIVE_PAN, 2)
        df3Pan = defaults.get(.DRIVE_PAN, 3)

        hd0Pan = defaults.get(.HDR_PAN, 0)
        hd1Pan = defaults.get(.HDR_PAN, 1)
        hd2Pan = defaults.get(.HDR_PAN, 2)
        hd3Pan = defaults.get(.HDR_PAN, 3)

        volL = defaults.get(.AUDVOLL)
        volR = defaults.get(.AUDVOLR)
        samplingMethod = defaults.get(.SAMPLING_METHOD)
        stepVolume = defaults.get(.STEP_VOLUME, 0)
        pollVolume = defaults.get(.POLL_VOLUME, 0)
        insertVolume = defaults.get(.INSERT_VOLUME, 0)
        ejectVolume = defaults.get(.EJECT_VOLUME, 0)
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
        static let hAutoCenter        = "Geometry.HAutoCenter"
        static let vAutoCenter        = "Geometry.VAutoCenter"
        static let hCenter            = "Geometry.HCenter"
        static let vCenter            = "Geometry.VCenter"
        static let hZoom              = "Geometry.HZoom"
        static let vZoom              = "Geometry.VZoom"
        
        // Shaders
        static let enhancer           = "Shaders.Enhancer"
        static let upscaler           = "Shaders.Upscaler"
        static let blur               = "Shaders.Blur"
        static let blurRadius         = "Shaders.BlurRadius"
        static let bloom              = "Shaders.Bloom"
        static let bloomRadius        = "Shaders.BloonRadius"
        static let bloomBrightness    = "Shaders.BloomBrightness"
        static let bloomWeight        = "Shaders.BloomWeight"
        static let flicker            = "Shaders.Flicker"
        static let flickerWeight      = "Shaders.FlickerWeight"
        static let dotMask            = "Shaders.DotMask"
        static let dotMaskBrightness  = "Shaders.DotMaskBrightness"
        static let scanlines          = "Shaders.Scanlines"
        static let scanlineBrightness = "Shaders.ScanlineBrightness"
        static let scanlineWeight     = "Shaders.ScanlineWeight"
        static let disalignment       = "Shaders.Disalignment"
        static let disalignmentH      = "Shaders.DisalignmentH"
        static let disalignmentV      = "Shaders.DisalignmentV"
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
        
        log(level: 2)
        // No GUI related keys in this category
    }

    func registerGeometryUserDefaults() {

        log(level: 2)

        register(Keys.Vid.hAutoCenter, true)
        register(Keys.Vid.vAutoCenter, true)
        register(Keys.Vid.hCenter, 0.6)
        register(Keys.Vid.vCenter, 0.47)
        register(Keys.Vid.hZoom, 1.0)
        register(Keys.Vid.vZoom, 0.27)
    }
    
    func registerShaderUserDefaults() {

        log(level: 2)
        
        register(Keys.Vid.enhancer, 0)
        register(Keys.Vid.upscaler, 0)
        register(Keys.Vid.blur, 1)
        register(Keys.Vid.blurRadius, 0)
        register(Keys.Vid.bloom, 0)
        register(Keys.Vid.bloomRadius, 1.0)
        register(Keys.Vid.bloomBrightness, 0.4)
        register(Keys.Vid.bloomWeight, 1.21)
        register(Keys.Vid.flicker, 1)
        register(Keys.Vid.flickerWeight, 0.25)
        register(Keys.Vid.dotMask, 0)
        register(Keys.Vid.dotMaskBrightness, 0.55)
        register(Keys.Vid.scanlines, 0)
        register(Keys.Vid.scanlineBrightness, 0.55)
        register(Keys.Vid.scanlineWeight, 0.11)
        register(Keys.Vid.disalignment, 0)
        register(Keys.Vid.disalignmentH, 0.001)
        register(Keys.Vid.disalignmentV, 0.001)
    }
    
    func removeVideoUserDefaults() {

        log(level: 2)
        
        removeColorUserDefaults()
        removeGeometryUserDefaults()
        removeShaderUserDefaults()
    }
    
    func removeColorUserDefaults() {

        log(level: 2)

        remove(.PALETTE)
        remove(.BRIGHTNESS)
        remove(.CONTRAST)
        remove(.SATURATION)
    }

    func removeGeometryUserDefaults() {

        log(level: 2)
        
        let keys = [ Keys.Vid.hAutoCenter,
                     Keys.Vid.vAutoCenter,
                     Keys.Vid.hCenter,
                     Keys.Vid.vCenter,
                     Keys.Vid.hZoom,
                     Keys.Vid.vZoom ]

        for key in keys { removeKey(key) }
    }
    
    func removeShaderUserDefaults() {

        log(level: 2)
        
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
        
        for key in keys { removeKey(key) }
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
        
        defaults.set(.PALETTE, palette)
        defaults.set(.BRIGHTNESS, brightness)
        defaults.set(.CONTRAST, contrast)
        defaults.set(.SATURATION, saturation)
        
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
          
        hAutoCenter = defaults.bool(Keys.Vid.hAutoCenter)
        vAutoCenter = defaults.bool(Keys.Vid.vAutoCenter)
        hCenter = defaults.float(Keys.Vid.hCenter)
        hCenter = defaults.float(Keys.Vid.vCenter)
        hZoom = defaults.float(Keys.Vid.hZoom)
        vZoom = defaults.float(Keys.Vid.vZoom)
                
        amiga.resume()
    }

    func applyShaderUserDefaults() {
    
        log(level: 2)
        let defaults = AmigaProxy.defaults!
        
        amiga.suspend()
                        
        enhancer = defaults.int(Keys.Vid.enhancer)
        upscaler = defaults.int(Keys.Vid.upscaler)
        blur = defaults.int(Keys.Vid.blur)
        blurRadius = defaults.float(Keys.Vid.blurRadius)
        bloom = defaults.int(Keys.Vid.bloom)
        bloomRadius = defaults.float(Keys.Vid.bloomRadius)
        bloomBrightness = defaults.float(Keys.Vid.bloomBrightness)
        bloomWeight = defaults.float(Keys.Vid.bloomWeight)
        flicker = defaults.int(Keys.Vid.flicker)
        flickerWeight = defaults.float(Keys.Vid.flickerWeight)
        dotMask = defaults.int(Keys.Vid.dotMask)
        dotMaskBrightness = defaults.float(Keys.Vid.dotMaskBrightness)
        scanlines = defaults.int(Keys.Vid.scanlines)
        scanlineBrightness = defaults.float(Keys.Vid.scanlineBrightness)
        scanlineWeight = defaults.float(Keys.Vid.scanlineWeight)
        disalignment = defaults.int(Keys.Vid.disalignment)
        disalignmentH = defaults.float(Keys.Vid.disalignmentH)
        disalignmentV = defaults.float(Keys.Vid.disalignmentV)
        
        amiga.resume()
    }
}
