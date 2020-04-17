// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* Preferences items are managed by two different classes.
 *
 * ApplicationPreferences
 * This class stores all items that belong to the application level. There is
 * a single object of this class and the stored values apply to all emulator
 * instances.
 *
 * EmulatorPreferences
 * This class stores all items that are specific to an individual emulator
 * instance. Each instance keeps its own object of this class.
 */

class ApplicationPreferences {
    
    var parent: MyController!
    var amiga: AmigaProxy { return parent.amiga }
    var renderer: Renderer { return parent.renderer }
    var gamePadManager: GamePadManager { return parent.gamePadManager }
    var kbController: KBController { return parent.kbController }
    
    //
    // General
    //
    
    // Selected game pad slot for joystick in port A
    var inputDevice1 = Defaults.inputDevice1
    
    // Selected game pad slot for joystick in port B
    var inputDevice2 = Defaults.inputDevice2
    
    var warpLoad = Defaults.warpLoad {
        didSet { parent.updateWarp() }
    }
    var driveSounds = Defaults.driveSounds
    var driveSoundPan = Defaults.driveSoundPan
    var driveInsertSound = Defaults.driveInsertSound
    var driveEjectSound = Defaults.driveEjectSound
    var driveHeadSound = Defaults.driveHeadSound
    var drivePollSound = Defaults.drivePollSound
    var driveBlankDiskFormat = Defaults.driveBlankDiskFormat
    
    var driveBlankDiskFormatIntValue: Int {
        get { return Int(driveBlankDiskFormat.rawValue) }
        set { driveBlankDiskFormat = FileSystemType.init(newValue) }
    }
    var keepAspectRatio = Defaults.keepAspectRatio
    var exitOnEsc = Defaults.exitOnEsc
    
    var closeWithoutAsking = Defaults.closeWithoutAsking
    var ejectWithoutAsking = Defaults.ejectWithoutAsking
    var pauseInBackground = Defaults.pauseInBackground
    
    // Remembers if the emulator was running or paused when it lost focus.
    // Needed to implement the pauseInBackground feature.
    var pauseInBackgroundSavedState = false
    
    var autoSnapshots = Defaults.autoSnapshots
    var snapshotInterval = 0 {
        didSet { parent.startSnapshotTimer() }
    }
    var autoScreenshots = Defaults.autoScreenshots
    var screenshotInterval = 0 {
        didSet { parent.startScreenshotTimer() }
    }
    var screenshotSource = Defaults.screenshotSource
    var screenshotTarget = Defaults.screenshotTarget
    var screenshotTargetIntValue: Int {
        get { return Int(screenshotTarget.rawValue) }
        set { screenshotTarget = NSBitmapImageRep.FileType(rawValue: UInt(newValue))! }
    }
    
    //
    // Devices preferences
    //
    
    var disconnectJoyKeys = Defaults.disconnectJoyKeys
    
    var autofire: Bool {
        get { return amiga.joystick1.autofire() }
        set {
            for amiga in myAppDelegate.proxies {
                amiga.joystick1.setAutofire(newValue)
                amiga.joystick2.setAutofire(newValue)
            }
        }
    }
    var autofireBullets: Int {
        get { return myAppDelegate.proxy?.joystick1.autofireBullets() ?? 0 }
        set {
            for amiga in myAppDelegate.proxies {
                amiga.joystick1.setAutofireBullets(newValue)
                amiga.joystick2.setAutofireBullets(newValue)
            }
        }
    }
    var autofireFrequency: Float {
        get { return myAppDelegate.proxy?.joystick1.autofireFrequency() ?? 0 }
        set {
            for amiga in myAppDelegate.proxies {
                amiga.joystick1.setAutofireFrequency(newValue)
                amiga.joystick2.setAutofireFrequency(newValue)
            }
        }
    }
    
    var keyMaps = [ Defaults.joyKeyMap1, Defaults.joyKeyMap2, Defaults.mouseKeyMap ]

    var retainMouseKeyComb = Defaults.retainMouseKeyComb
    var retainMouseWithKeys = Defaults.retainMouseWithKeys
    var retainMouseByClick = Defaults.retainMouseByClick
    var retainMouseByEntering = Defaults.retainMouseByEntering
    var releaseMouseKeyComb = Defaults.retainMouseKeyComb
    var releaseMouseWithKeys = Defaults.releaseMouseWithKeys
    var releaseMouseByShaking = Defaults.releaseMouseByShaking
 
    init(with controller: MyController) {
        
        parent = controller
    }
}

class EmulatorPreferences {

    var parent: MyController!
    var amiga: AmigaProxy { return parent.amiga }
    var renderer: Renderer { return parent.renderer }
    var gamePadManager: GamePadManager { return parent.gamePadManager }
    var kbController: KBController { return parent.kbController }

    //
    // Rom preferences
    //
    
    // Rom URLs
    var romURL: URL = Defaults.rom
    var extURL: URL = Defaults.ext
    
    //
    // Video preferences
    //
    
    var enhancer = Defaults.enhancer
    var upscaler = Defaults.upscaler
    
    var hCenter = Defaults.hCenter
    var vCenter = Defaults.vCenter
    var hZoom = Defaults.hZoom
    var vZoom = Defaults.vZoom
    
    var brightness: Double {
        get { return amiga.denise.brightness() }
        set { amiga.denise.setBrightness(newValue) }
    }
    var contrast: Double {
        get { return amiga.denise.contrast() }
        set { amiga.denise.setContrast(newValue) }
    }
    var saturation: Double {
        get { return amiga.denise.saturation() }
        set { amiga.denise.setSaturation(newValue) }
    }
    var palette: Int {
        get { return Int(amiga.denise.palette()) }
        set { amiga.denise.setPalette(Palette(newValue)) }
    }
    var blur: Int32 {
        get { return renderer.shaderOptions.blur }
        set { renderer.shaderOptions.blur = newValue }
    }
    var blurRadius: Float {
        get { return renderer.shaderOptions.blurRadius }
        set { renderer.shaderOptions.blurRadius = newValue }
    }
    var bloom: Int32 {
        get { return renderer.shaderOptions.bloom }
        set { renderer.shaderOptions.bloom = newValue }
    }
    var bloomRadius: Float {
        get { return renderer.shaderOptions.bloomRadius }
        set { renderer.shaderOptions.bloomRadius = newValue }
    }
    var bloomBrightness: Float {
        get { return renderer.shaderOptions.bloomBrightness }
        set { renderer.shaderOptions.bloomBrightness = newValue }
    }
    var bloomWeight: Float {
        get { return renderer.shaderOptions.bloomWeight }
        set { renderer.shaderOptions.bloomWeight = newValue }
    }
    var flicker: Int32 {
        get { return renderer.shaderOptions.flicker }
        set { renderer.shaderOptions.flicker = newValue }
    }
    var flickerWeight: Float {
        get { return renderer.shaderOptions.flickerWeight }
        set { renderer.shaderOptions.flickerWeight = newValue }
    }
    var dotMask: Int32 {
        get { return renderer.shaderOptions.dotMask }
        set { renderer.shaderOptions.dotMask = newValue }
    }
    var dotMaskBrightness: Float {
        get { return renderer.shaderOptions.dotMaskBrightness }
        set { renderer.shaderOptions.dotMaskBrightness = newValue }
    }
    var scanlines: Int32 {
        get { return renderer.shaderOptions.scanlines }
        set { renderer.shaderOptions.scanlines = newValue }
    }
    var scanlineBrightness: Float {
        get { return renderer.shaderOptions.scanlineBrightness }
        set { renderer.shaderOptions.scanlineBrightness = newValue }
    }
    var scanlineWeight: Float {
        get { return renderer.shaderOptions.scanlineWeight }
        set { renderer.shaderOptions.scanlineWeight = newValue }
    }
    var disalignment: Int32 {
        get { return renderer.shaderOptions.disalignment }
        set { renderer.shaderOptions.disalignment = newValue }
    }
    var disalignmentH: Float {
        get { return renderer.shaderOptions.disalignmentH }
        set { renderer.shaderOptions.disalignmentH = newValue }
    }
    var disalignmentV: Float {
        get { return renderer.shaderOptions.disalignmentV }
        set { renderer.shaderOptions.disalignmentV = newValue }
    }
    
    init(with controller: MyController) {
        
        parent = controller
    }
}
