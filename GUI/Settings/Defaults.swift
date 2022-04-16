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
        
        // registerGeneralUserDefaults()
        // registerControlsUserDefaults()
        registerDevicesUserDefaults()
        
        registerVideoUserDefaults()
        registerGeometryUserDefaults()
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

/*
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
*/
/*
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
*/

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

/*

struct RomDefaults {
    
    let extStart: Int
    
    static let std = RomDefaults(
        
        extStart: 0xE0
    )
}
*/

extension UserDefaults {
    
    static func romUrl(name: String) -> URL? {
        
        let folder = try? URL.appSupportFolder("Roms")
        return folder?.appendingPathComponent(name)        
    }
    
    static var romUrl: URL? { return romUrl(name: "rom.bin") }
    static var extUrl: URL? { return romUrl(name: "ext.bin") }
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

extension UserDefaults {

    static func mediaUrl(name: String) -> URL? {
        
        let folder = try? URL.appSupportFolder("Media")
        return folder?.appendingPathComponent(name)
    }

    static func hdnUrl(_ nr: Int) -> URL? {
        
        return mediaUrl(name: "hd\(nr).hdf")
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
