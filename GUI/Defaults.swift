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

@MainActor
extension DefaultsProxy {

    func resetSearchPaths() {

        set("ROM_PATH", UserDefaults.romUrl!.path)
        set("EXT_PATH", UserDefaults.extUrl!.path)
        set("HD0_PATH", UserDefaults.hd0Url!.path)
        set("HD1_PATH", UserDefaults.hd1Url!.path)
        set("HD2_PATH", UserDefaults.hd2Url!.path)
        set("HD3_PATH", UserDefaults.hd3Url!.path)
    }

    func load(url: URL) throws {

        resetSearchPaths()

        let exception = ExceptionWrapper()
        load(url, exception: exception)
        if exception.fault != .OK { throw CoreError(exception) }     

        debug(.defaults, "Successfully loaded user defaults from \(url)")
    }

    func load() {
        
        debug(.defaults, "Loading user defaults")
        
        do {
            let folder = try URL.appSupportFolder()
            let path = folder.appendingPathComponent("vAmiga.ini")
            
            do {
                try load(url: path)
            } catch {
                warn("Failed to load user defaults from file \(path)")
            }
            
        } catch {
            warn("Failed to access application support folder")
        }
    }
    
    func save(url: URL) throws {
        
        let exception = ExceptionWrapper()
        save(url, exception: exception)
        if exception.fault != .OK { throw CoreError(exception) }

        debug(.defaults, "Successfully saved user defaults to \(url)")
    }
    
    func save() {
        
        debug(.defaults, "Saving user defaults")
        
        do {
            let folder = try URL.appSupportFolder()
            let path = folder.appendingPathComponent("vAmiga.ini")
            
            do {
                try save(url: path)
            } catch {
                warn("Failed to save user defaults file \(path)")
            }
            
        } catch {
            warn("Failed to access application support folder")
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

            if let data = Data(base64Encoded: jsonString) {

                if let decoded = try? JSONDecoder().decode(T.self, from: data) {
                    item = decoded
                } else {
                    warn("Failed to decode \(jsonString)")
                }
                return
            }
        }
        warn("Failed to decode jsonString")
    }
}

//
// Paths
//

@MainActor
extension UserDefaults {
    
    static func romUrl(name: String) -> URL? {
        
        let folder = try? URL.appSupportFolder("Roms")
        return folder?.appendingPathComponent(name)
    }

    static func romUrl(fingerprint: Int) -> URL? {

        return romUrl(name: String(format: "%08x", fingerprint) + ".rom")
    }

    static func romUrl(crc32: UInt32) -> URL? {

        return romUrl(fingerprint: Int(crc32))
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

@MainActor
extension DefaultsProxy {
    
    func registerUserDefaults() {
        
        debug(.defaults, "Registering user defaults")
        
        registerGeneralUserDefaults()
        registerCapturesUserDefaults()
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

@MainActor
extension Preferences {

    func applyUserDefaults() {
        
        debug(.defaults)
        
        applyGeneralUserDefaults()
        applyCapturesUserDefaults()
        applyControlsUserDefaults()
        applyDevicesUserDefaults()
    }
}

@MainActor
extension Configuration {
        
    func applyUserDefaults() {
    
        debug(.defaults)
        
        applyHardwareUserDefaults()
        applyPeripheralsUserDefaults()
        applyPerformanceUserDefaults()
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
        static let snapshotStorage        = "General.SnapshotStorage"
        static let autoSnapshots          = "General.AutoSnapshots"
        static let autoSnapshotInterval   = "General.ScreenshotInterval"
        
        // Fullscreen
        static let keepAspectRatio        = "General.FullscreenKeepAspectRatio"
        static let exitOnEsc              = "General.FullscreenExitOnEsc"

        // Miscellaneous
        static let ejectWithoutAsking     = "General.EjectWithoutAsking"
        static let detachWithoutAsking    = "General.DetachWithoutAsking"
        static let closeWithoutAsking     = "General.CloseWithoutAsking"
        static let pauseInBackground      = "General.PauseInBackground"
    }
}

@MainActor
extension DefaultsProxy {
    
    func registerGeneralUserDefaults() {
        
        debug(.defaults)

        // Snapshots
        register(Keys.Gen.snapshotStorage, 512)
        register(Keys.Gen.autoSnapshots, false)
        register(Keys.Gen.autoSnapshotInterval, 20)
                
        // Fullscreen
        register(Keys.Gen.keepAspectRatio, false)
        register(Keys.Gen.exitOnEsc, true)

        // Misc
        register(Keys.Gen.ejectWithoutAsking, false)
        register(Keys.Gen.detachWithoutAsking, false)
        register(Keys.Gen.closeWithoutAsking, false)
        register(Keys.Gen.pauseInBackground, false)
    }
    
    func removeGeneralUserDefaults() {
        
        debug(.defaults)
        
        let keys = [ Keys.Gen.snapshotStorage,
                     Keys.Gen.autoSnapshots,
                     Keys.Gen.autoSnapshotInterval,
                                          
                     Keys.Gen.keepAspectRatio,
                     Keys.Gen.exitOnEsc,

                     Keys.Gen.ejectWithoutAsking,
                     Keys.Gen.detachWithoutAsking,
                     Keys.Gen.closeWithoutAsking,
                     Keys.Gen.pauseInBackground
        ]
        
        for key in keys { removeKey(key) }
    }
}

@MainActor
extension Preferences {

    func saveGeneralUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        defaults.set(Keys.Gen.snapshotStorage, snapshotStorage)
        defaults.set(Keys.Gen.autoSnapshots, autoSnapshots)
        defaults.set(Keys.Gen.autoSnapshotInterval, snapshotInterval)
                
        defaults.set(Keys.Gen.keepAspectRatio, keepAspectRatio)
        defaults.set(Keys.Gen.exitOnEsc, exitOnEsc)

        defaults.set(Keys.Gen.ejectWithoutAsking, ejectWithoutAsking)
        defaults.set(Keys.Gen.detachWithoutAsking, detachWithoutAsking)
        defaults.set(Keys.Gen.closeWithoutAsking, closeWithoutAsking)
        defaults.set(Keys.Gen.pauseInBackground, pauseInBackground)
        
        defaults.save()
    }
    
    func applyGeneralUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        snapshotStorage = defaults.int(Keys.Gen.snapshotStorage)
        autoSnapshots = defaults.bool(Keys.Gen.autoSnapshots)
        snapshotInterval = defaults.int(Keys.Gen.autoSnapshotInterval)
                
        keepAspectRatio = defaults.bool(Keys.Gen.keepAspectRatio)
        exitOnEsc = defaults.bool(Keys.Gen.exitOnEsc)

        ejectWithoutAsking = defaults.bool(Keys.Gen.ejectWithoutAsking)
        detachWithoutAsking = defaults.bool(Keys.Gen.detachWithoutAsking)
        closeWithoutAsking = defaults.bool(Keys.Gen.closeWithoutAsking)
        pauseInBackground = defaults.bool(Keys.Gen.pauseInBackground)
    }
}

//
// User defaults (Captures)
//

@MainActor
extension Keys {
    
    struct Cap {
                
        // Screenshots
        static let screenshotFormat       = "General.ScreenshotFormat"
        static let screenshotSource       = "General.ScreenshotSource"
        static let screenshotCutout       = "General.ScreenshotCutout"
        static let screenshotWidth        = "General.ScreenshotWidth"
        static let screenshotHeight       = "General.ScreenshotHeight"

        // Videos
        static let ffmpegPath             = "General.ffmpegPath"
        static let captureSource          = "General.Source"
        static let bitRate                = "General.BitRate"
        static let aspectX                = "General.AspectX"
        static let aspectY                = "General.AspectY"
    }
}

@MainActor
extension DefaultsProxy {
    
    func registerCapturesUserDefaults() {
        
        debug(.defaults)
        
        // Screenshots
        register(Keys.Cap.screenshotFormat, NSBitmapImageRep.FileType.png.rawValue)
        register(Keys.Cap.screenshotSource, 0)
        register(Keys.Cap.screenshotCutout, 0)
        register(Keys.Cap.screenshotWidth, 1200)
        register(Keys.Cap.screenshotHeight, 900)

        // Videos
        register(Keys.Cap.ffmpegPath, "")
        register(Keys.Cap.captureSource, 0)
        register(Keys.Cap.bitRate, 2048)
        register(Keys.Cap.aspectX, 768)
        register(Keys.Cap.aspectY, 702)
    }
    
    func removeCapturesUserDefaults() {
        
        debug(.defaults)
        
        let keys = [ Keys.Cap.screenshotFormat,
                     Keys.Cap.screenshotSource,
                     Keys.Cap.screenshotCutout,
                     Keys.Cap.screenshotWidth,
                     Keys.Cap.screenshotHeight,

                     Keys.Cap.ffmpegPath,
                     Keys.Cap.captureSource,
                     Keys.Cap.bitRate,
                     Keys.Cap.aspectX,
                     Keys.Cap.aspectY,
        ]
        
        for key in keys { removeKey(key) }
    }
}

@MainActor
extension Preferences {

    func saveCapturesUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        defaults.set(Keys.Cap.screenshotFormat, screenshotFormatIntValue)
        defaults.set(Keys.Cap.screenshotSource, screenshotSourceIntValue)
        defaults.set(Keys.Cap.screenshotCutout, screenshotCutoutIntValue)
        defaults.set(Keys.Cap.screenshotWidth, screenshotWidth)
        defaults.set(Keys.Cap.screenshotHeight, screenshotHeight)

        defaults.set(Keys.Cap.ffmpegPath, ffmpegPath)
        defaults.set(Keys.Cap.captureSource, captureSourceIntValue)
        defaults.set(Keys.Cap.bitRate, bitRate)
        defaults.set(Keys.Cap.aspectX, aspectX)
        defaults.set(Keys.Cap.aspectY, aspectY)
                
        defaults.save()
    }
    
    func applyCapturesUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
                
        screenshotFormatIntValue = defaults.int(Keys.Cap.screenshotFormat)
        screenshotSourceIntValue = defaults.int(Keys.Cap.screenshotSource)
        screenshotCutoutIntValue = defaults.int(Keys.Cap.screenshotCutout)
        screenshotWidth = defaults.int(Keys.Cap.screenshotWidth)
        screenshotHeight = defaults.int(Keys.Cap.screenshotHeight)

        ffmpegPath = defaults.string(Keys.Cap.ffmpegPath)
        captureSourceIntValue = defaults.int(Keys.Cap.captureSource)
        bitRate = defaults.int(Keys.Cap.bitRate)
        aspectX = defaults.int(Keys.Cap.aspectX)
        aspectY = defaults.int(Keys.Cap.aspectY)
    }
}

//
// User defaults (Controls)
//

@MainActor
extension Keys {
    
    struct Con {
        
        // Emulation keys
        static let mouseKeyMap           = "Controls.MouseKeyMap"
        static let joyKeyMap1            = "Controls.JoyKeyMap1"
        static let joyKeyMap2            = "Controls.JoyKeyMap2"
        static let disconnectJoyKeys     = "Controls.DisconnectKeys"
        
        // Joysticks
        static let autofire              = "Controls.Autofire"
        static let autofireBursts        = "Controls.AutofireBursts"
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

@MainActor
extension DefaultsProxy {
    
    func registerControlsUserDefaults() {
        
        debug(.defaults)

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
        register(Keys.Con.autofireBursts, false)
        register(Keys.Con.autofireBullets, 3)
        register(Keys.Con.autofireFrequency, 25)

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
        
        debug(.defaults)
        
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

@MainActor
extension Preferences {

    func saveControlsUserDefaults() {
    
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
                
        defaults.encode(Keys.Con.mouseKeyMap, keyMaps[0])
        defaults.encode(Keys.Con.joyKeyMap1, keyMaps[1])
        defaults.encode(Keys.Con.joyKeyMap2, keyMaps[2])
        defaults.set(Keys.Con.disconnectJoyKeys, disconnectJoyKeys)

        defaults.set(Keys.Con.autofire, autofire)
        defaults.set(Keys.Con.autofireBursts, autofireBursts)
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
           
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        defaults.decode(Keys.Con.mouseKeyMap, &keyMaps[0])
        defaults.decode(Keys.Con.joyKeyMap1, &keyMaps[1])
        defaults.decode(Keys.Con.joyKeyMap2, &keyMaps[2])
        disconnectJoyKeys = defaults.bool(Keys.Con.disconnectJoyKeys)
        
        autofire = defaults.bool(Keys.Con.autofire)
        autofireBursts = defaults.bool(Keys.Con.autofireBursts)
        autofireBullets = defaults.int(Keys.Con.autofireBullets)
        autofireFrequency = defaults.int(Keys.Con.autofireFrequency)

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

@MainActor
extension Keys {
    
    struct Dev {

        static let schemes            = "Devices.Schemes"
    }
}

@MainActor
extension DefaultsProxy {
    
    func registerDevicesUserDefaults() {

    }
    
    func removeDevicesUserDefaults() {

    }
}

@MainActor
extension Preferences {

    func saveDevicesUserDefaults() {
    
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
                        
        defaults.save()
    }
    
    func applyDevicesUserDefaults() {
           
        debug(.defaults)
    }
}

//
// User Defaults (Roms)
//

@MainActor
extension Configuration {

    func saveRomUserDefaults() throws {

        debug(.defaults)

        let defaults = EmulatorProxy.defaults!
        let fm = FileManager.default
        var url: URL?

        amiga.suspend()

        defaults.set(.MEM_EXT_START, extStart)
        defaults.save()
        
        do {

            // Kickstart
            url = UserDefaults.romUrl
            if url == nil { throw CoreError(.FILE_CANT_WRITE) }
            try? fm.removeItem(at: url!)
            if amiga.mem.info.hasRom { try amiga.mem.saveRom(url!) }

            // Kickstart extension
            url = UserDefaults.extUrl
            if url == nil { throw CoreError(.FILE_CANT_WRITE) }
            try? fm.removeItem(at: url!)
            if amiga.mem.info.hasExt { try amiga.mem.saveExt(url!) }

        } catch {

            amiga.resume()
            throw error
        }

        amiga.resume()
    }
}

//
// User defaults (Hardware)
//

@MainActor
extension DefaultsProxy {

    func registerHardwareUserDefaults() {

        registerChipsetUserDefaults()
        registerMemoryUserDefaults()
    }

    func removeHardwareUserDefaults() {

        removeChipsetUserDefaults()
        removeMemoryUserDefaults()
    }
}

@MainActor
extension Configuration {

    func applyHardwareUserDefaults() {

        applyChipsetUserDefaults()
        applyMemoryUserDefaults()
    }

    func saveHardwareUserDefaults() {

        saveChipsetUserDefaults()
        saveMemoryUserDefaults()
    }
}

//
// User defaults (Hardware::Chipset)
//

@MainActor
extension DefaultsProxy {
    
    func registerChipsetUserDefaults() {
    
        debug(.defaults)
        // No GUI related items in this sections
    }

    func removeChipsetUserDefaults() {

        debug(.defaults)

        remove(.AMIGA_VIDEO_FORMAT)
        remove(.CPU_REVISION)
        remove(.CPU_OVERCLOCKING)
        remove(.AGNUS_REVISION)
        remove(.DENISE_REVISION)
        remove(.CIA_REVISION, [0, 1])
        remove(.RTC_MODEL)
    }
}

@MainActor
extension Configuration {

    func applyChipsetUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        amiga.suspend()

        machineType = defaults.get(.AMIGA_VIDEO_FORMAT)
        cpuRev = defaults.get(.CPU_REVISION)
        cpuSpeed = defaults.get(.CPU_OVERCLOCKING)
        agnusRev = defaults.get(.AGNUS_REVISION)
        deniseRev = defaults.get(.DENISE_REVISION)
        ciaRev = defaults.get(.CIA_REVISION)
        rtClock = defaults.get(.RTC_MODEL)
        
        amiga.resume()
    }

    func saveChipsetUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        amiga.suspend()

        defaults.set(.AMIGA_VIDEO_FORMAT, machineType)
        defaults.set(.CPU_REVISION, cpuRev)
        defaults.set(.CPU_OVERCLOCKING, cpuSpeed)
        defaults.set(.AGNUS_REVISION, agnusRev)
        defaults.set(.DENISE_REVISION, deniseRev)
        defaults.set(.CIA_REVISION, [0,1], ciaRev)
        defaults.set(.RTC_MODEL, rtClock)
        defaults.save()

        amiga.resume()
    }
}

//
// User defaults (Hardware::Memory)
//

@MainActor
extension DefaultsProxy {
    
    func registerMemoryUserDefaults() {
    
        debug(.defaults)
        // No GUI related items in this sections
    }
    
    func removeMemoryUserDefaults() {
        
        debug(.defaults)

        remove(.MEM_CHIP_RAM)
        remove(.MEM_SLOW_RAM)
        remove(.MEM_FAST_RAM)
        remove(.MEM_RAM_INIT_PATTERN)
        remove(.MEM_BANKMAP)
        remove(.MEM_UNMAPPING_TYPE)
    }
}

@MainActor
extension Configuration {

    func saveMemoryUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        amiga.suspend()
        
        defaults.set(.MEM_CHIP_RAM, chipRam)
        defaults.set(.MEM_SLOW_RAM, slowRam)
        defaults.set(.MEM_FAST_RAM, fastRam)
        defaults.set(.MEM_RAM_INIT_PATTERN, ramInitPattern)
        defaults.set(.MEM_BANKMAP, bankMap)
        defaults.set(.MEM_UNMAPPING_TYPE, unmappingType)
        defaults.save()
        
        amiga.resume()
    }
    
    func applyMemoryUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        amiga.suspend()

        chipRam = defaults.get(.MEM_CHIP_RAM)
        slowRam = defaults.get(.MEM_SLOW_RAM)
        fastRam = defaults.get(.MEM_FAST_RAM)
        ramInitPattern = defaults.get(.MEM_RAM_INIT_PATTERN)
        bankMap = defaults.get(.MEM_BANKMAP)
        unmappingType = defaults.get(.MEM_UNMAPPING_TYPE)

        amiga.resume()
    }
}

//
// User defaults (Peripherals)
//

@MainActor
extension Keys {
    
    struct Per {

        // Ports
        static let gameDevice1        = "Peripherals.GameDevice1"
        static let gameDevice2        = "Peripherals.GameDevice2"
    }
}

@MainActor
extension DefaultsProxy {
    
    func registerPeripheralsUserDefaults() {

        debug(.defaults)
        
        // Port assignments
        register(Keys.Per.gameDevice1, 0)
        register(Keys.Per.gameDevice2, -1)
    }

    func removePeripheralsUserDefaults() {
        
        debug(.defaults)
        
        remove(.DRIVE_CONNECT, [0, 1, 2, 3])
        remove(.DRIVE_TYPE, [0, 1, 2, 3])
        remove(.DRIVE_RPM, [0, 1, 2, 3])
        remove(.HDC_CONNECT, [0, 1, 2, 3])
        remove(.HDR_TYPE, [0, 1, 2, 3])
        remove(.SER_DEVICE)
        remove(.SRV_PORT, nr: ServerType.SER.rawValue)
        remove(.JOY_AUTOFIRE, [0, 1])
        remove(.JOY_AUTOFIRE_BURSTS, [0, 1])
        remove(.JOY_AUTOFIRE_BULLETS, [0, 1])
        remove(.JOY_AUTOFIRE_DELAY, [0, 1])
        removeKey(Keys.Per.gameDevice1)
        removeKey(Keys.Per.gameDevice2)
    }
}
 
@MainActor
extension Configuration {

    func savePeripheralsUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        amiga.suspend()
        
        defaults.set(.DRIVE_CONNECT, 0, df0Connected)
        defaults.set(.DRIVE_CONNECT, 1, df1Connected)
        defaults.set(.DRIVE_CONNECT, 2, df2Connected)
        defaults.set(.DRIVE_CONNECT, 3, df3Connected)
        
        defaults.set(.DRIVE_TYPE, 0, df0Type)
        defaults.set(.DRIVE_TYPE, 1, df1Type)
        defaults.set(.DRIVE_TYPE, 2, df2Type)
        defaults.set(.DRIVE_TYPE, 3, df3Type)

        defaults.set(.DRIVE_RPM, 0, df0Rpm)
        defaults.set(.DRIVE_RPM, 1, df1Rpm)
        defaults.set(.DRIVE_RPM, 2, df2Rpm)
        defaults.set(.DRIVE_RPM, 3, df3Rpm)

        defaults.set(.HDC_CONNECT, 0, hd0Connected)
        defaults.set(.HDC_CONNECT, 1, hd1Connected)
        defaults.set(.HDC_CONNECT, 2, hd2Connected)
        defaults.set(.HDC_CONNECT, 3, hd3Connected)

        defaults.set(.HDR_TYPE, 0, hd0Type)
        defaults.set(.HDR_TYPE, 1, hd1Type)
        defaults.set(.HDR_TYPE, 2, hd2Type)
        defaults.set(.HDR_TYPE, 3, hd3Type)

        defaults.set(.SER_DEVICE, serialDevice)
        defaults.set(.SRV_PORT, ServerType.SER.rawValue, serialDevicePort)

        defaults.set(.JOY_AUTOFIRE, [0, 1], autofire)
        defaults.set(.JOY_AUTOFIRE_BURSTS, [0, 1], autofireBursts)
        defaults.set(.JOY_AUTOFIRE_BULLETS, [0, 1], autofireBullets)
        defaults.set(.JOY_AUTOFIRE_DELAY, [0, 1], autofireDelay)

        defaults.set(Keys.Per.gameDevice1, gameDevice1)
        defaults.set(Keys.Per.gameDevice2, gameDevice2)

        defaults.save()
        
        amiga.resume()
    }

    func applyPeripheralsUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        amiga.suspend()

        df0Connected = defaults.get(.DRIVE_CONNECT, 0) != 0
        df1Connected = defaults.get(.DRIVE_CONNECT, 1) != 0
        df2Connected = defaults.get(.DRIVE_CONNECT, 2) != 0
        df3Connected = defaults.get(.DRIVE_CONNECT, 3) != 0

        df0Type = defaults.get(.DRIVE_TYPE, 0)
        df1Type = defaults.get(.DRIVE_TYPE, 1)
        df2Type = defaults.get(.DRIVE_TYPE, 2)
        df3Type = defaults.get(.DRIVE_TYPE, 3)

        df0Rpm = defaults.get(.DRIVE_RPM, 0)
        df1Rpm = defaults.get(.DRIVE_RPM, 1)
        df2Rpm = defaults.get(.DRIVE_RPM, 2)
        df3Rpm = defaults.get(.DRIVE_RPM, 3)

        hd0Connected = defaults.get(.HDC_CONNECT, 0) != 0
        hd1Connected = defaults.get(.HDC_CONNECT, 1) != 0
        hd2Connected = defaults.get(.HDC_CONNECT, 2) != 0
        hd3Connected = defaults.get(.HDC_CONNECT, 3) != 0

        hd0Type = defaults.get(.HDR_TYPE, 0)
        hd1Type = defaults.get(.HDR_TYPE, 1)
        hd2Type = defaults.get(.HDR_TYPE, 2)
        hd3Type = defaults.get(.HDR_TYPE, 3)

        serialDevice = defaults.get(.SER_DEVICE)
        serialDevicePort = defaults.get(.SRV_PORT, ServerType.SER.rawValue)

        gameDevice1 = defaults.int(Keys.Per.gameDevice1)
        gameDevice2 = defaults.int(Keys.Per.gameDevice2)

        autofire = defaults.get(.JOY_AUTOFIRE, 0) != 0
        autofireBursts = defaults.get(.JOY_AUTOFIRE_BURSTS, 0) != 0
        autofireBullets = defaults.get(.JOY_AUTOFIRE_BULLETS, 0)
        autofireDelay = defaults.get(.JOY_AUTOFIRE_DELAY, 0)

        amiga.resume()
    }
}

//
// User defaults (Performance)
//

@MainActor
extension DefaultsProxy {

    func registerPerformanceUserDefaults() {

        debug(.defaults)
        // No GUI related items in this sections
    }

    func removePerformanceUserDefaults() {

        debug(.defaults)

        remove(.AMIGA_WARP_MODE)
        remove(.AMIGA_WARP_BOOT)
        remove(.AMIGA_VSYNC)
        remove(.AMIGA_SPEED_BOOST)
        remove(.AMIGA_RUN_AHEAD)
        remove(.DENISE_CLX_SPR_SPR)
        remove(.DENISE_CLX_SPR_PLF)
        remove(.DENISE_CLX_PLF_PLF)
        remove(.DENISE_FRAME_SKIPPING)
        remove(.CIA_IDLE_SLEEP, [0, 1])
        remove(.AUD_FASTPATH)
        remove(.AMIGA_WS_COMPRESSION)
        remove(.AMIGA_SNAP_COMPRESSOR)
    }
}

@MainActor
extension Configuration {

    func applyPerformanceUserDefaults() {

        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        amiga.suspend()

        warpBoot = defaults.get(.AMIGA_WARP_BOOT)
        warpMode = defaults.get(.AMIGA_WARP_MODE)
        speedBoost = defaults.get(.AMIGA_SPEED_BOOST)
        vsync = defaults.get(.AMIGA_VSYNC) != 0
        runAhead = defaults.get(.AMIGA_RUN_AHEAD)
        frameSkipping = defaults.get(.DENISE_FRAME_SKIPPING)
        ciaIdleSleep = defaults.get(.CIA_IDLE_SLEEP) != 0
        audioFastPath = defaults.get(.AUD_FASTPATH) != 0
        wsCompressor = defaults.get(.AMIGA_WS_COMPRESSION)
        snapCompressor = defaults.get(.AMIGA_SNAP_COMPRESSOR)

        amiga.resume()
    }

    func savePerformanceUserDefaults() {

        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        amiga.suspend()

        defaults.set(.AMIGA_WARP_MODE, warpMode)
        defaults.set(.AMIGA_WARP_BOOT, warpBoot)
        defaults.set(.AMIGA_VSYNC, vsync)
        defaults.set(.AMIGA_SPEED_BOOST, speedBoost)
        defaults.set(.AMIGA_RUN_AHEAD, runAhead)
        defaults.set(.DENISE_FRAME_SKIPPING, frameSkipping)
        defaults.set(.CIA_IDLE_SLEEP, [0,1], ciaIdleSleep)
        defaults.set(.AUD_FASTPATH, audioFastPath)
        defaults.set(.AMIGA_WS_COMPRESSION, wsCompressor)
        defaults.set(.AMIGA_SNAP_COMPRESSOR, snapCompressor)
        defaults.save()

        amiga.resume()
    }
}

//
// User defaults (Compatibility)
//

@MainActor
extension DefaultsProxy {
    
    func registerCompatibilityUserDefaults() {

        debug(.defaults)
        // No GUI related items in this sections
    }
    
    func removeCompatibilityUserDefaults() {
        
        debug(.defaults)
        
        remove(.BLITTER_ACCURACY)
        remove(.AGNUS_PTR_DROPS)
        remove(.CIA_TODBUG, [0, 1])
        remove(.CIA_ECLOCK_SYNCING, [0, 1])
        remove(.DC_SPEED)
        remove(.DC_LOCK_DSKSYNC)
        remove(.DC_AUTO_DSKSYNC)
        remove(.DRIVE_MECHANICS, [ 0, 1, 2, 3])
        remove(.KBD_ACCURACY)
        remove(.DENISE_CLX_PLF_PLF)
        remove(.DENISE_CLX_SPR_PLF)
        remove(.DENISE_CLX_SPR_SPR)
        remove(.MEM_SLOW_RAM_DELAY)
        remove(.MEM_SLOW_RAM_MIRROR)
    }
}

@MainActor
extension Configuration {

    func saveCompatibilityUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        amiga.suspend()
        
        defaults.set(.BLITTER_ACCURACY, blitterAccuracy)
        defaults.set(.AGNUS_PTR_DROPS, ptrDrops)
        defaults.set(.CIA_TODBUG, [0,1], todBug)
        defaults.set(.CIA_ECLOCK_SYNCING, [0,1], eClockSyncing)
        defaults.set(.DC_SPEED, driveSpeed)
        defaults.set(.DC_LOCK_DSKSYNC, lockDskSync)
        defaults.set(.DC_AUTO_DSKSYNC, autoDskSync)
        defaults.set(.DRIVE_MECHANICS, [0, 1, 2, 3], driveMechanics)
        defaults.set(.KBD_ACCURACY, accurateKeyboard)
        defaults.set(.DENISE_CLX_SPR_SPR, clxSprSpr)
        defaults.set(.DENISE_CLX_SPR_PLF, clxSprPlf)
        defaults.set(.DENISE_CLX_PLF_PLF, clxPlfPlf)
        defaults.set(.MEM_SLOW_RAM_DELAY, slowRamDelay)
        defaults.set(.MEM_SLOW_RAM_MIRROR, slowRamMirror)
        defaults.save()
        
        amiga.resume()
    }

    func applyCompatibilityUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        amiga.suspend()
        
        blitterAccuracy = defaults.get(.BLITTER_ACCURACY)
        ptrDrops = defaults.get(.AGNUS_PTR_DROPS) != 0
        todBug = defaults.get(.CIA_TODBUG) != 0
        eClockSyncing = defaults.get(.CIA_ECLOCK_SYNCING) != 0
        driveSpeed = defaults.get(.DC_SPEED)
        lockDskSync = defaults.get(.DC_LOCK_DSKSYNC) != 0
        autoDskSync = defaults.get(.DC_AUTO_DSKSYNC) != 0
        driveMechanics = defaults.get(.DRIVE_MECHANICS, 0)
        accurateKeyboard = defaults.get(.KBD_ACCURACY) != 0
        clxSprSpr = defaults.get(.DENISE_CLX_SPR_SPR) != 0
        clxSprPlf = defaults.get(.DENISE_CLX_SPR_PLF) != 0
        clxPlfPlf = defaults.get(.DENISE_CLX_PLF_PLF) != 0
        slowRamDelay = defaults.get(.MEM_SLOW_RAM_DELAY) != 0
        slowRamMirror = defaults.get(.MEM_SLOW_RAM_MIRROR) != 0

        amiga.resume()
    }
}

//
// User defaults (Audio)
//

@MainActor
extension DefaultsProxy {
    
    func registerAudioUserDefaults() {

        debug(.defaults)
        // No GUI related items in this sections
    }
    
    func removeAudioUserDefaults() {
        
        debug(.defaults)
            
        remove(.DRIVE_PAN, [0, 1, 2, 3])
        remove(.HDR_PAN, [0, 1, 2, 3])
        remove(.AUD_PAN0)
        remove(.AUD_PAN1)
        remove(.AUD_PAN2)
        remove(.AUD_PAN3)
        remove(.AUD_VOL0)
        remove(.AUD_VOL1)
        remove(.AUD_VOL2)
        remove(.AUD_VOL3)
        remove(.AUD_VOLL)
        remove(.AUD_VOLR)
        remove(.DRIVE_STEP_VOLUME, [0, 1, 2, 3])
        remove(.DRIVE_POLL_VOLUME, [0, 1, 2, 3])
        remove(.DRIVE_INSERT_VOLUME, [0, 1, 2, 3])
        remove(.DRIVE_EJECT_VOLUME, [0, 1, 2, 3])
        remove(.AUD_FILTER_TYPE)
        remove(.AUD_SAMPLING_METHOD)
        remove(.AUD_BUFFER_SIZE)
        remove(.AUD_ASR)
    }
}

@MainActor
extension Configuration {
    
    func saveAudioUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        amiga.suspend()
        
        defaults.set(.AUD_VOL0, vol0)
        defaults.set(.AUD_VOL1, vol1)
        defaults.set(.AUD_VOL2, vol2)
        defaults.set(.AUD_VOL3, vol3)
        defaults.set(.AUD_PAN0, pan0)
        defaults.set(.AUD_PAN1, pan1)
        defaults.set(.AUD_PAN2, pan2)
        defaults.set(.AUD_PAN3, pan3)
        defaults.set(.AUD_VOLL, volL)
        defaults.set(.AUD_VOLR, volR)
        defaults.set(.DRIVE_PAN, 0, df0Pan)
        defaults.set(.DRIVE_PAN, 1, df1Pan)
        defaults.set(.DRIVE_PAN, 2, df2Pan)
        defaults.set(.DRIVE_PAN, 3, df3Pan)
        defaults.set(.HDR_PAN, 0, hd0Pan)
        defaults.set(.HDR_PAN, 1, hd1Pan)
        defaults.set(.HDR_PAN, 2, hd2Pan)
        defaults.set(.HDR_PAN, 3, hd3Pan)
        defaults.set(.DRIVE_STEP_VOLUME, [0, 1, 2, 3], stepVolume)
        defaults.set(.DRIVE_POLL_VOLUME, [0, 1, 2, 3], pollVolume)
        defaults.set(.DRIVE_INSERT_VOLUME, [0, 1, 2, 3], insertVolume)
        defaults.set(.DRIVE_EJECT_VOLUME, [0, 1, 2, 3], ejectVolume)
        defaults.set(.AUD_FILTER_TYPE, filterType)
        defaults.set(.AUD_SAMPLING_METHOD, samplingMethod)
        defaults.set(.AUD_BUFFER_SIZE, audioBufferSize)
        defaults.set(.AUD_ASR, asr)

        defaults.save()
        
        amiga.resume()
    }
    
    func applyAudioUserDefaults() {
        
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!

        amiga.suspend()

        vol0 = defaults.get(.AUD_VOL0)
        vol1 = defaults.get(.AUD_VOL1)
        vol2 = defaults.get(.AUD_VOL2)
        vol3 = defaults.get(.AUD_VOL3)

        pan0 = defaults.get(.AUD_PAN0)
        pan1 = defaults.get(.AUD_PAN1)
        pan2 = defaults.get(.AUD_PAN2)
        pan3 = defaults.get(.AUD_PAN3)

        df0Pan = defaults.get(.DRIVE_PAN, 0)
        df1Pan = defaults.get(.DRIVE_PAN, 1)
        df2Pan = defaults.get(.DRIVE_PAN, 2)
        df3Pan = defaults.get(.DRIVE_PAN, 3)

        hd0Pan = defaults.get(.HDR_PAN, 0)
        hd1Pan = defaults.get(.HDR_PAN, 1)
        hd2Pan = defaults.get(.HDR_PAN, 2)
        hd3Pan = defaults.get(.HDR_PAN, 3)

        volL = defaults.get(.AUD_VOLL)
        volR = defaults.get(.AUD_VOLR)
        stepVolume = defaults.get(.DRIVE_STEP_VOLUME, 0)
        pollVolume = defaults.get(.DRIVE_POLL_VOLUME, 0)
        insertVolume = defaults.get(.DRIVE_INSERT_VOLUME, 0)
        ejectVolume = defaults.get(.DRIVE_EJECT_VOLUME, 0)
        filterType = defaults.get(.AUD_FILTER_TYPE)
        
        samplingMethod = defaults.get(.AUD_SAMPLING_METHOD)
        audioBufferSize = defaults.get(.AUD_BUFFER_SIZE, audioBufferSize)
        asr = defaults.get(.AUD_ASR, asr)

        amiga.resume()
    }
}

//
// User defaults (Video)
//

@MainActor
extension Keys {
    
    struct Vid {

        // Geometry
        static let zoom               = "Geometry.Zoom"
        static let hZoom              = "Geometry.HZoom"
        static let vZoom              = "Geometry.VZoom"
        static let center             = "Geometry.Center"
        static let hCenter            = "Geometry.HCenter"
        static let vCenter            = "Geometry.VCenter"

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

@MainActor
extension DefaultsProxy {
    
    func registerVideoUserDefaults() {

        debug(.defaults)
        
        registerColorUserDefaults()
        registerGeometryUserDefaults()
        registerShaderUserDefaults()
    }

    func registerColorUserDefaults() {
        
        debug(.defaults)
        // No GUI related keys in this category
    }

    func registerGeometryUserDefaults() {

        debug(.defaults)
        // No GUI related keys in this category

        
        register(Keys.Vid.zoom, 2)
        register(Keys.Vid.hZoom, 1.0)
        register(Keys.Vid.vZoom, 0.27)
        register(Keys.Vid.center, 1)
        register(Keys.Vid.hCenter, 0.6)
        register(Keys.Vid.vCenter, 0.47)
        
    }
    
    func registerShaderUserDefaults() {

        debug(.defaults)
        // No GUI related keys in this category
        
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

        debug(.defaults)
        
        removeColorUserDefaults()
        removeGeometryUserDefaults()
        removeShaderUserDefaults()
    }
    
    func removeColorUserDefaults() {

        debug(.defaults)

        remove(.MON_PALETTE)
        remove(.MON_BRIGHTNESS)
        remove(.MON_CONTRAST)
        remove(.MON_SATURATION)
    }

    func removeGeometryUserDefaults() {

        debug(.defaults)
        
        remove(.MON_CENTER)
        remove(.MON_HCENTER)
        remove(.MON_VCENTER)
        remove(.MON_ZOOM)
        remove(.MON_HZOOM)
        remove(.MON_VZOOM)
    }
    
    func removeShaderUserDefaults() {

        debug(.defaults)
        
        remove(.MON_ENHANCER)
        remove(.MON_UPSCALER)
        remove(.MON_BLUR)
        remove(.MON_BLUR_RADIUS)
        remove(.MON_BLOOM)
        remove(.MON_BLOOM_RADIUS)
        remove(.MON_BLOOM_BRIGHTNESS)
        remove(.MON_BLOOM_WEIGHT)
        remove(.MON_DOTMASK)
        remove(.MON_DOTMASK_BRIGHTNESS)
        remove(.MON_SCANLINES)
        remove(.MON_SCANLINE_BRIGHTNESS)
        remove(.MON_SCANLINE_WEIGHT)
        remove(.MON_DISALIGNMENT)
        remove(.MON_DISALIGNMENT_H)
        remove(.MON_DISALIGNMENT_V)
        remove(.MON_FLICKER)
        remove(.MON_FLICKER_WEIGHT)
    }
}

@MainActor
extension Configuration {

    func saveVideoUserDefaults() {
        
        debug(.defaults)

        amiga.suspend()

        saveColorUserDefaults()
        saveGeometryUserDefaults()
        saveShaderUserDefaults()

        amiga.resume()
    }
    
    func saveColorUserDefaults() {

        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        amiga.suspend()
        
        defaults.set(.MON_PALETTE, palette)
        defaults.set(.MON_BRIGHTNESS, brightness)
        defaults.set(.MON_CONTRAST, contrast)
        defaults.set(.MON_SATURATION, saturation)

        defaults.save()
        
        amiga.resume()
    }
    
    func saveGeometryUserDefaults() {

        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        amiga.suspend()
                
        defaults.set(.MON_ZOOM, Int(zoom))
        defaults.set(.MON_HZOOM, Int(hZoom))
        defaults.set(.MON_VZOOM, Int(vZoom))
        defaults.set(.MON_CENTER, Int(center))
        defaults.set(.MON_HCENTER, Int(hCenter))
        defaults.set(.MON_VCENTER, Int(vCenter))

        defaults.save()
        
        amiga.resume()
    }
  
    func saveShaderUserDefaults() {

        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
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
        
        debug(.defaults)
        
        applyColorUserDefaults()
        applyGeometryUserDefaults()
        applyShaderUserDefaults()
    }
    
    func applyColorUserDefaults() {
    
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        amiga.suspend()
        
        palette = defaults.get(.MON_PALETTE)
        brightness = defaults.get(.MON_BRIGHTNESS)
        contrast = defaults.get(.MON_CONTRAST)
        saturation = defaults.get(.MON_SATURATION)

        amiga.resume()
    }

    func applyGeometryUserDefaults() {
    
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        amiga.suspend()
          
        zoom = defaults.get(.MON_ZOOM)
        hZoom = defaults.get(.MON_HZOOM)
        vZoom = defaults.get(.MON_VZOOM)
        center = defaults.get(.MON_CENTER)
        hCenter = defaults.get(.MON_HCENTER)
        vCenter = defaults.get(.MON_VCENTER)

        amiga.resume()
    }

    func applyShaderUserDefaults() {
    
        debug(.defaults)
        let defaults = EmulatorProxy.defaults!
        
        amiga.suspend()
                        
        enhancer = defaults.get(.MON_ENHANCER)
        upscaler = defaults.get(.MON_UPSCALER)
        blur = defaults.get(.MON_BLUR)
        blurRadius = defaults.get(.MON_BLUR_RADIUS)
        bloom = defaults.get(.MON_BLOOM)
        bloomRadius = defaults.get(.MON_BLOOM_RADIUS)
        bloomBrightness = defaults.get(.MON_BLOOM_BRIGHTNESS)
        bloomWeight = defaults.get(.MON_BLOOM_WEIGHT)
        flicker = defaults.get(.MON_FLICKER)
        flickerWeight = defaults.get(.MON_FLICKER_WEIGHT)
        dotMask = defaults.get(.MON_DOTMASK)
        dotMaskBrightness = defaults.get(.MON_DOTMASK_BRIGHTNESS)
        scanlines = defaults.get(.MON_SCANLINES)
        scanlineBrightness = defaults.get(.MON_SCANLINE_BRIGHTNESS)
        scanlineWeight = defaults.get(.MON_SCANLINE_WEIGHT)
        disalignment = defaults.get(.MON_DISALIGNMENT)
        disalignmentH = defaults.get(.MON_DISALIGNMENT_H)
        disalignmentV = defaults.get(.MON_DISALIGNMENT_V)
        
        amiga.resume()
    }
}
