// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* This class stores all items that are specific to an individual emulator
 * instance. Each instance keeps its own object of this class inside the
 * document controller.
 *
 * See class "Preferences" for shared settings.
 */

class Configuration {

    var parent: MyController!
    var amiga: AmigaProxy { return parent.amiga }
    var renderer: Renderer { return parent.renderer }
    var gamePadManager: GamePadManager { return parent.gamePadManager }
    var ressourceManager: RessourceManager { return renderer.ressourceManager }
    
    //
    // Roms
    //
        
    var extStart: Int {
        get { return amiga.getConfig(.EXT_START) }
        set { amiga.configure(.EXT_START, value: newValue) }
    }

    //
    // Hardware
    //

    var machineType: Int {
        get { return amiga.getConfig(.VIDEO_FORMAT) }
        set { amiga.configure(.VIDEO_FORMAT, value: newValue) }
    }
    var cpuRev: Int {
        get { return amiga.getConfig(.CPU_REVISION) }
        set { amiga.configure(.CPU_REVISION, value: newValue) }
    }
    var cpuDasmRev: Int {
        get { return amiga.getConfig(.CPU_DASM_REVISION) }
        set { amiga.configure(.CPU_DASM_REVISION, value: newValue) }
    }
    var cpuSpeed: Int {
        get { return amiga.getConfig(.CPU_OVERCLOCKING) }
        set { amiga.configure(.CPU_OVERCLOCKING, value: newValue) }
    }
    var warpMode: Int {
        get { return amiga.getConfig(.WARP_MODE) }
        set { amiga.configure(.WARP_MODE, value: newValue) }
    }
    var agnusRev: Int {
        get { return amiga.getConfig(.AGNUS_REVISION) }
        set { amiga.configure(.AGNUS_REVISION, value: newValue) }
    }
    var deniseRev: Int {
        get { return amiga.getConfig(.DENISE_REVISION) }
        set { amiga.configure(.DENISE_REVISION, value: newValue) }
    }
    var ciaRev: Int {
        get { return amiga.getConfig(.CIA_REVISION) }
        set { amiga.configure(.CIA_REVISION, value: newValue) }
    }
    var rtClock: Int {
        get { return amiga.getConfig(.RTC_MODEL) }
        set { amiga.configure(.RTC_MODEL, value: newValue) }
    }
    var filterType: Int {
        get { return amiga.getConfig(.FILTER_TYPE) }
        set { amiga.configure(.FILTER_TYPE, value: newValue) }
    }
    var chipRam: Int {
        get { return amiga.getConfig(.CHIP_RAM) }
        set { amiga.configure(.CHIP_RAM, value: newValue) }
    }
    var slowRam: Int {
        get { return amiga.getConfig(.SLOW_RAM) }
        set { amiga.configure(.SLOW_RAM, value: newValue) }
    }
    var fastRam: Int {
        get { return amiga.getConfig(.FAST_RAM) }
        set { amiga.configure(.FAST_RAM, value: newValue) }
    }
    var bankMap: Int {
        get { return amiga.getConfig(.BANKMAP) }
        set { amiga.configure(.BANKMAP, value: newValue) }
    }
    var ramInitPattern: Int {
        get { return amiga.getConfig(.RAM_INIT_PATTERN) }
        set { amiga.configure(.RAM_INIT_PATTERN, value: newValue) }
    }
    var unmappingType: Int {
        get { return amiga.getConfig(.UNMAPPING_TYPE) }
        set { amiga.configure(.UNMAPPING_TYPE, value: newValue) }
    }

    //
    // Peripherals
    //

    func dfnConnected(_ n: Int) -> Bool {
        precondition(0 <= n && n <= 3)
        return amiga.getConfig(.DRIVE_CONNECT, drive: n) != 0
    }
    func setDfnConnected(_ n: Int, connect: Bool) {
        precondition(0 <= n && n <= 3)
        amiga.configure(.DRIVE_CONNECT, drive: n, enable: connect)
    }
    func dfnType(_ n: Int) -> Int {
        precondition(0 <= n && n <= 3)
        return amiga.getConfig(.DRIVE_TYPE, drive: n)
    }
    func setDfnType(_ n: Int, type: Int) {
        precondition(0 <= n && n <= 3)
        amiga.configure(.DRIVE_TYPE, drive: n, value: type)
    }
    func dfnRpm(_ n: Int) -> Int {
        precondition(0 <= n && n <= 3)
        return amiga.getConfig(.DRIVE_RPM, drive: n)
    }
    func setDfnRpm(_ n: Int, type: Int) {
        precondition(0 <= n && n <= 3)
        amiga.configure(.DRIVE_RPM, drive: n, value: type)
    }
    func hdnConnected(_ n: Int) -> Bool {
        precondition(0 <= n && n <= 3)
        return amiga.getConfig(.HDC_CONNECT, drive: n) != 0
    }
    func setHdnConnected(_ n: Int, connect: Bool) {
        precondition(0 <= n && n <= 3)
        amiga.configure(.HDC_CONNECT, drive: n, enable: connect)
    }
    func hdnType(_ n: Int) -> Int {
        precondition(0 <= n && n <= 3)
        return amiga.getConfig(.HDR_TYPE, drive: n)
    }
    func setHdnType(_ n: Int, type: Int) {
        precondition(0 <= n && n <= 3)
        amiga.configure(.HDR_TYPE, drive: n, value: type)
    }

    var df0Connected: Bool {
        get { return dfnConnected(0) }
        set { setDfnConnected(0, connect: newValue) }
    }
    var df0Type: Int {
        get { return dfnType(0) }
        set { setDfnType(0, type: newValue) }
    }
    var df0Rpm: Int {
        get { return dfnRpm(0) }
        set { setDfnRpm(0, type: newValue) }
    }
    var df1Connected: Bool {
        get { return dfnConnected(1) }
        set { setDfnConnected(1, connect: newValue) }
    }
    var df1Type: Int {
        get { return dfnType(1) }
        set { setDfnType(1, type: newValue) }
    }
    var df1Rpm: Int {
        get { return dfnRpm(1) }
        set { setDfnRpm(1, type: newValue) }
    }
    var df2Connected: Bool {
        get { return dfnConnected(2) }
        set { setDfnConnected(2, connect: newValue) }
    }
    var df2Type: Int {
        get { return dfnType(2) }
        set { setDfnType(2, type: newValue) }
    }
    var df2Rpm: Int {
        get { return dfnRpm(2) }
        set { setDfnRpm(2, type: newValue) }
    }
    var df3Connected: Bool {
        get { return dfnConnected(3) }
        set { setDfnConnected(3, connect: newValue) }
    }
    var df3Type: Int {
        get { return dfnType(3) }
        set { setDfnType(3, type: newValue) }
    }
    var df3Rpm: Int {
        get { return dfnRpm(3) }
        set { setDfnRpm(3, type: newValue) }
    }
    var hd0Connected: Bool {
        get { return hdnConnected(0) }
        set { setHdnConnected(0, connect: newValue) }
    }
    var hd0Type: Int {
        get { return hdnType(0) }
        set { setHdnType(0, type: newValue) }
    }
    var hd1Connected: Bool {
        get { return hdnConnected(1) }
        set { setHdnConnected(1, connect: newValue) }
    }
    var hd1Type: Int {
        get { return hdnType(1) }
        set { setHdnType(1, type: newValue) }
    }
    var hd2Connected: Bool {
        get { return hdnConnected(2) }
        set { setHdnConnected(2, connect: newValue) }
    }
    var hd2Type: Int {
        get { return hdnType(2) }
        set { setHdnType(2, type: newValue) }
    }
    var hd3Connected: Bool {
        get { return hdnConnected(3) }
        set { setHdnConnected(3, connect: newValue) }
    }
    var hd3Type: Int {
        get { return hdnType(3) }
        set { setHdnType(3, type: newValue) }
    }
    // var hdPersist = [ false, false, false, false ]

    var gameDevice1 = -1 {
        didSet {
                         
            // Try to connect the device (may disconnect the other device)
            gamePadManager.connect(slot: gameDevice1, port: 1)

            // Avoid double mappings
            if gameDevice1 != -1 && gameDevice1 == gameDevice2 {
                gameDevice2 = -1
            }
            
            parent.toolbar.validateVisibleItems()
        }
    }
    var gameDevice2 = -1 {
        didSet {
 
            // Try to connect the device (may disconnect the other device)
            gamePadManager.connect(slot: gameDevice2, port: 2)

            // Avoid double mappings
            if gameDevice2 != -1 && gameDevice2 == gameDevice1 {
                gameDevice1 = -1
            }
            
            parent.toolbar.validateVisibleItems()
        }
    }
    var serialDevice: Int {
        get { return amiga.getConfig(.SER_DEVICE) }
        set { amiga.configure(.SER_DEVICE, value: newValue) }
    }
    var serialDevicePort: Int {
        get { return amiga.getConfig(.SRV_PORT, id: ServerType.SER.rawValue) }
        set { amiga.configure(.SRV_PORT, id: ServerType.SER.rawValue, value: newValue) }
    }

    //
    // Compatibility
    //

    var blitterAccuracy: Int {
        get { return amiga.getConfig(.BLITTER_ACCURACY) }
        set { amiga.configure(.BLITTER_ACCURACY, value: newValue) }
    }
    var slowRamMirror: Bool {
        get { return amiga.getConfig(.SLOW_RAM_MIRROR) != 0 }
        set { amiga.configure(.SLOW_RAM_MIRROR, enable: newValue) }
    }
    var todBug: Bool {
        get { return amiga.getConfig(.TODBUG) != 0 }
        set { amiga.configure(.TODBUG, enable: newValue) }
    }
    var ptrDrops: Bool {
        get { return amiga.getConfig(.PTR_DROPS) != 0 }
        set { amiga.configure(.PTR_DROPS, enable: newValue) }
    }
    var eClockSyncing: Bool {
        get { return amiga.getConfig(.ECLOCK_SYNCING) != 0 }
        set { amiga.configure(.ECLOCK_SYNCING, enable: newValue) }
    }
    var ciaIdleSleep: Bool {
        get { return amiga.getConfig(.CIA_IDLE_SLEEP) != 0 }
        set { amiga.configure(.CIA_IDLE_SLEEP, enable: newValue) }
    }
    var frameSkipping: Int {
        get { return amiga.getConfig(.FRAME_SKIPPING) }
        set { amiga.configure(.FRAME_SKIPPING, value: newValue) }
    }
    var slowRamDelay: Bool {
        get { return amiga.getConfig(.SLOW_RAM_DELAY) != 0 }
        set { amiga.configure(.SLOW_RAM_DELAY, enable: newValue) }
    }
    var clxSprSpr: Bool {
        get { return amiga.getConfig(.CLX_SPR_SPR) != 0 }
        set { amiga.configure(.CLX_SPR_SPR, enable: newValue) }
    }
    var clxSprPlf: Bool {
        get { return amiga.getConfig(.CLX_SPR_PLF) != 0 }
        set { amiga.configure(.CLX_SPR_PLF, enable: newValue) }
    }
    var clxPlfPlf: Bool {
        get { return amiga.getConfig(.CLX_PLF_PLF) != 0 }
        set { amiga.configure(.CLX_PLF_PLF, enable: newValue) }
    }
    var driveSpeed: Int {
        get { return amiga.getConfig(.DRIVE_SPEED) }
        set { amiga.configure(.DRIVE_SPEED, value: newValue) }
    }
    var driveMechanics: Int {
        get { return amiga.getConfig(.DRIVE_MECHANICS, drive: 0) }
        set {
            amiga.configure(.DRIVE_MECHANICS, drive: 0, value: newValue)
            amiga.configure(.DRIVE_MECHANICS, drive: 1, value: newValue)
            amiga.configure(.DRIVE_MECHANICS, drive: 2, value: newValue)
            amiga.configure(.DRIVE_MECHANICS, drive: 3, value: newValue)
        }
    }
    var lockDskSync: Bool {
        get { return amiga.getConfig(.LOCK_DSKSYNC) != 0 }
        set { amiga.configure(.LOCK_DSKSYNC, enable: newValue) }
    }
    var autoDskSync: Bool {
        get { return amiga.getConfig(.AUTO_DSKSYNC) != 0 }
        set { amiga.configure(.AUTO_DSKSYNC, enable: newValue) }
    }
    var accurateKeyboard: Bool {
        get { return amiga.getConfig(.ACCURATE_KEYBOARD) != 0 }
        set { amiga.configure(.ACCURATE_KEYBOARD, enable: newValue) }
    }
    
    //
    // Audio settings
    //

    var vol0: Int {
        get { return amiga.getConfig(.AUDVOL, id: 0) }
        set { amiga.configure(.AUDVOL, id: 0, value: newValue) }
    }
    var vol1: Int {
        get { return amiga.getConfig(.AUDVOL, id: 1) }
        set { amiga.configure(.AUDVOL, id: 1, value: newValue) }
    }
    var vol2: Int {
        get { return amiga.getConfig(.AUDVOL, id: 2) }
        set { amiga.configure(.AUDVOL, id: 2, value: newValue) }
    }
    var vol3: Int {
        get { return amiga.getConfig(.AUDVOL, id: 3) }
        set { amiga.configure(.AUDVOL, id: 3, value: newValue) }
    }
    var pan0: Int {
        get { return amiga.getConfig(.AUDPAN, id: 0) }
        set { amiga.configure(.AUDPAN, id: 0, value: newValue) }
    }
    var pan1: Int {
        get { return amiga.getConfig(.AUDPAN, id: 1) }
        set { amiga.configure(.AUDPAN, id: 1, value: newValue) }
    }
    var pan2: Int {
        get { return amiga.getConfig(.AUDPAN, id: 2) }
        set { amiga.configure(.AUDPAN, id: 2, value: newValue) }
    }
    var pan3: Int {
        get { return amiga.getConfig(.AUDPAN, id: 3) }
        set { amiga.configure(.AUDPAN, id: 3, value: newValue) }
    }
    var volL: Int {
        get { return amiga.getConfig(.AUDVOLL) }
        set { amiga.configure(.AUDVOLL, value: newValue) }
    }
    var volR: Int {
        get { return amiga.getConfig(.AUDVOLR) }
        set { amiga.configure(.AUDVOLR, value: newValue) }
    }
    var samplingMethod: Int {
        get { return amiga.getConfig(.SAMPLING_METHOD) }
        set { amiga.configure(.SAMPLING_METHOD, value: newValue) }
    }
    var df0Pan: Int {
        get { return amiga.getConfig(.DRIVE_PAN, drive: 0) }
        set { amiga.configure(.DRIVE_PAN, id: 0, value: newValue) }
    }
    var df1Pan: Int {
        get { return amiga.getConfig(.DRIVE_PAN, drive: 1) }
        set { amiga.configure(.DRIVE_PAN, id: 1, value: newValue) }
    }
    var df2Pan: Int {
        get { return amiga.getConfig(.DRIVE_PAN, drive: 2) }
        set { amiga.configure(.DRIVE_PAN, id: 2, value: newValue) }
    }
    var df3Pan: Int {
        get { return amiga.getConfig(.DRIVE_PAN, drive: 3) }
        set { amiga.configure(.DRIVE_PAN, id: 3, value: newValue) }
    }
    var hd0Pan: Int {
        get { return amiga.getConfig(.HDR_PAN, drive: 0) }
        set { amiga.configure(.HDR_PAN, id: 0, value: newValue) }
    }
    var hd1Pan: Int {
        get { return amiga.getConfig(.HDR_PAN, drive: 1) }
        set { amiga.configure(.HDR_PAN, id: 1, value: newValue) }
    }
    var hd2Pan: Int {
        get { return amiga.getConfig(.HDR_PAN, drive: 2) }
        set { amiga.configure(.HDR_PAN, id: 2, value: newValue) }
    }
    var hd3Pan: Int {
        get { return amiga.getConfig(.HDR_PAN, drive: 3) }
        set { amiga.configure(.HDR_PAN, id: 3, value: newValue) }
    }
    var stepVolume: Int {
        get { return amiga.getConfig(.STEP_VOLUME, drive: 0) }
        set { amiga.configure(.STEP_VOLUME, value: newValue)
            amiga.configure(.HDR_STEP_VOLUME, value: newValue)
        }
    }
    var pollVolume: Int {
        get { return amiga.getConfig(.POLL_VOLUME, drive: 0) }
        set { amiga.configure(.POLL_VOLUME, value: newValue) }
    }
    var insertVolume: Int {
        get { return amiga.getConfig(.INSERT_VOLUME, drive: 0) }
        set { amiga.configure(.INSERT_VOLUME, value: newValue) }
    }
    var ejectVolume: Int {
        get { return amiga.getConfig(.EJECT_VOLUME, drive: 0) }
        set { amiga.configure(.EJECT_VOLUME, value: newValue) }
    }

    //
    // Video settings
    //

    var syncMode: Int {
        get { return amiga.getConfig(.SYNC_MODE) }
        set { amiga.configure(.SYNC_MODE, value: newValue) }
    }
    var proposedFps: Int {
        get { return amiga.getConfig(.PROPOSED_FPS) }
        set { amiga.configure(.PROPOSED_FPS, value: newValue) }
    }
    var palette: Int {
        get { return amiga.getConfig(.PALETTE) }
        set { amiga.configure(.PALETTE, value: newValue) }
    }
    var brightness: Int {
        get { return amiga.getConfig(.BRIGHTNESS) }
        set { amiga.configure(.BRIGHTNESS, value: newValue) }
    }
    var contrast: Int {
        get { return amiga.getConfig(.CONTRAST) }
        set { amiga.configure(.CONTRAST, value: newValue) }
    }
    var saturation: Int {
        get { return amiga.getConfig(.SATURATION) }
        set { amiga.configure(.SATURATION, value: newValue) }
    }
    var zoom: Int = 0 {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var hZoom: Float = 0 {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var vZoom: Float = 0 {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var center: Int = 0 {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var hCenter: Float = 0 {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var vCenter: Float = 0 {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var enhancer: Int = 0 {
        didSet {
            if !ressourceManager.selectEnhancer(enhancer) { enhancer = oldValue }
        }
    }
    var upscaler: Int = 0 {
        didSet {
            if !ressourceManager.selectUpscaler(upscaler) { upscaler = oldValue }
        }
    }
    var blur: Int = 0 {
        didSet { renderer.shaderOptions.blur = Int32(blur) }
    }
    var blurRadius: Float = 0 {
        didSet { renderer.shaderOptions.blurRadius = blurRadius }
    }
    var bloom: Int = 0 {
        didSet {
            renderer.shaderOptions.bloom = Int32(bloom)
            if !ressourceManager.selectBloomFilter(bloom) { bloom = oldValue }
        }
    }
    var bloomRadius: Float = 0 {
        didSet { renderer.shaderOptions.bloomRadius = bloomRadius }
    }
    var bloomBrightness: Float = 0 {
        didSet { renderer.shaderOptions.bloomBrightness = bloomBrightness }
    }
    var bloomWeight: Float = 0 {
        didSet { renderer.shaderOptions.bloomWeight = bloomWeight }
    }
    var flicker: Int = 0 {
        didSet { renderer.shaderOptions.flicker = Int32(flicker) }
    }
    var flickerWeight: Float = 0 {
        didSet { renderer.shaderOptions.flickerWeight = flickerWeight }
    }
    var dotMask: Int = 0 {
        didSet {
            renderer.shaderOptions.dotMask = Int32(dotMask)
            ressourceManager.buildDotMasks()
            if !ressourceManager.selectDotMask(dotMask) { dotMask = oldValue }
        }
    }
    var dotMaskBrightness: Float = 0 {
        didSet {
            renderer.shaderOptions.dotMaskBrightness = dotMaskBrightness
            ressourceManager.buildDotMasks()
            ressourceManager.selectDotMask(dotMask)
        }
    }
    var scanlines: Int = 0 {
        didSet {
            renderer.shaderOptions.scanlines = Int32(scanlines)
            if !ressourceManager.selectScanlineFilter(scanlines) { scanlines = oldValue }
        }
    }
    var scanlineBrightness: Float = 0 {
        didSet { renderer.shaderOptions.scanlineBrightness = scanlineBrightness }
    }
    var scanlineWeight: Float = 0 {
        didSet { renderer.shaderOptions.scanlineWeight = scanlineWeight }
    }
    var disalignment: Int = 0 {
        didSet { renderer.shaderOptions.disalignment = Int32(disalignment) }
    }
    var disalignmentH: Float = 0 {
        didSet { renderer.shaderOptions.disalignmentH = disalignmentH }
    }
    var disalignmentV: Float = 0 {
        didSet { renderer.shaderOptions.disalignmentV = disalignmentV }
    }
    
    init(with controller: MyController) { parent = controller }
}
