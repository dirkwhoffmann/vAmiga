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
    // Rom settings
    //
    
    var romURL: URL = Defaults.rom
    var extURL: URL = Defaults.ext
    
    var extStart: Int {
        get { return amiga.getConfig(VA_EXT_START) }
        set { amiga.configure(VA_EXT_START, value: newValue) }
    }

    //
    // Hardware settings
    //
    
    var agnusRev: Int {
        get { return amiga.getConfig(VA_AGNUS_REVISION) }
        set { amiga.configure(VA_AGNUS_REVISION, value: newValue) }
    }
    var deniseRev: Int {
        get { return amiga.getConfig(VA_DENISE_REVISION) }
        set { amiga.configure(VA_DENISE_REVISION, value: newValue) }
    }
    var rtClock: Int {
        get { return amiga.getConfig(VA_RT_CLOCK) }
        set { amiga.configure(VA_RT_CLOCK, value: newValue) }
    }
    var chipRam: Int {
        get { return amiga.getConfig(VA_CHIP_RAM) }
        set { amiga.configure(VA_CHIP_RAM, value: newValue) }
    }
    var slowRam: Int {
        get { return amiga.getConfig(VA_SLOW_RAM) }
        set { amiga.configure(VA_SLOW_RAM, value: newValue) }
    }
    var fastRam: Int {
        get { return amiga.getConfig(VA_FAST_RAM) }
        set { amiga.configure(VA_FAST_RAM, value: newValue) }
    }
    var serialDevice: Int {
        get { return amiga.getConfig(VA_SERIAL_DEVICE) }
        set { amiga.configure(VA_SERIAL_DEVICE, value: newValue) }
    }
    var df0Connected: Bool {
        get { return amiga.getConfig(VA_DRIVE_CONNECT, drive: 0) != 0 }
        set { amiga.configure(VA_DRIVE_CONNECT, drive: 0, enable: newValue) }
    }
    var df0Type: Int {
        get { return amiga.getConfig(VA_DRIVE_TYPE, drive: 0) }
        set { amiga.configure(VA_DRIVE_TYPE, drive: 0, value: newValue) }
    }
    var df1Connected: Bool {
        get { return amiga.getConfig(VA_DRIVE_CONNECT, drive: 1) != 0 }
        set { amiga.configure(VA_DRIVE_CONNECT, drive: 1, enable: newValue) }
    }
    var df1Type: Int {
        get { return amiga.getConfig(VA_DRIVE_TYPE, drive: 1) }
        set { amiga.configure(VA_DRIVE_TYPE, drive: 1, value: newValue) }
    }
    var df2Connected: Bool {
        get { return amiga.getConfig(VA_DRIVE_CONNECT, drive: 2) != 0 }
        set { amiga.configure(VA_DRIVE_CONNECT, drive: 2, enable: newValue) }
    }
    var df2Type: Int {
        get { return amiga.getConfig(VA_DRIVE_TYPE, drive: 2) }
        set { amiga.configure(VA_DRIVE_TYPE, drive: 2, value: newValue) }
    }
    var df3Connected: Bool {
        get { return amiga.getConfig(VA_DRIVE_CONNECT, drive: 3) != 0 }
        set { amiga.configure(VA_DRIVE_CONNECT, drive: 3, enable: newValue) }
    }
    var df3Type: Int {
        get { return amiga.getConfig(VA_DRIVE_TYPE, drive: 3) }
        set { amiga.configure(VA_DRIVE_TYPE, drive: 3, value: newValue) }
    }

    //
    // Compatibility settings
    //

    var clxSprSpr: Bool {
        get { return amiga.getConfig(VA_CLX_SPR_SPR) != 0 }
        set { amiga.configure(VA_CLX_SPR_SPR, enable: newValue) }
    }
    var clxSprPlf: Bool {
        get { return amiga.getConfig(VA_CLX_SPR_PLF) != 0 }
        set { amiga.configure(VA_CLX_SPR_PLF, enable: newValue) }
    }
    var clxPlfPlf: Bool {
        get { return amiga.getConfig(VA_CLX_PLF_PLF) != 0 }
        set { amiga.configure(VA_CLX_PLF_PLF, enable: newValue) }
    }
    var samplingMethod: Int {
        get { return amiga.getConfig(VA_SAMPLING_METHOD) }
        set { amiga.configure(VA_SAMPLING_METHOD, value: newValue) }
    }
    var filterActivation: Int {
        get { return amiga.getConfig(VA_FILTER_ACTIVATION) }
        set { amiga.configure(VA_FILTER_ACTIVATION, value: newValue) }
    }
    var filterType: Int {
        get { return amiga.getConfig(VA_FILTER_TYPE) }
        set { amiga.configure(VA_FILTER_TYPE, value: newValue) }
    }
    var blitterAccuracy: Int {
        get { return amiga.getConfig(VA_BLITTER_ACCURACY) }
        set { amiga.configure(VA_BLITTER_ACCURACY, value: newValue) }
    }
    var driveSpeed: Int {
        get { return amiga.getConfig(VA_DRIVE_SPEED) }
        set { amiga.configure(VA_DRIVE_SPEED, value: newValue) }
    }
    var fifoBuffering: Bool {
        get { return amiga.getConfig(VA_FIFO_BUFFERING) != 0 }
        set { amiga.configure(VA_FIFO_BUFFERING, enable: newValue) }
    }
    var todBug: Bool {
        get { return amiga.getConfig(VA_TODBUG) != 0 }
        set { amiga.configure(VA_TODBUG, enable: newValue) }
    }
    
    //
    // Monitor settings
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
