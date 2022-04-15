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
    var filterAlwaysOn: Bool {
        get { return amiga.getConfig(.FILTER_ALWAYS_ON) != 0}
        set { amiga.configure(.FILTER_ALWAYS_ON, enable: newValue) }
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
    var df1Connected: Bool {
        get { return dfnConnected(1) }
        set { setDfnConnected(1, connect: newValue) }
    }
    var df1Type: Int {
        get { return dfnType(1) }
        set { setDfnType(1, type: newValue) }
    }
    var df2Connected: Bool {
        get { return dfnConnected(2) }
        set { setDfnConnected(2, connect: newValue) }
    }
    var df2Type: Int {
        get { return dfnType(2) }
        set { setDfnType(2, type: newValue) }
    }
    var df3Connected: Bool {
        get { return dfnConnected(3) }
        set { setDfnConnected(3, connect: newValue) }
    }
    var df3Type: Int {
        get { return dfnType(3) }
        set { setDfnType(3, type: newValue) }
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

    var gameDevice1 = 0 {
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
        get { return amiga.getConfig(.SERIAL_DEVICE) }
        set { amiga.configure(.SERIAL_DEVICE, value: newValue) }
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
    var eClockSyncing: Bool {
        get { return amiga.getConfig(.ECLOCK_SYNCING) != 0 }
        set { amiga.configure(.ECLOCK_SYNCING, enable: newValue) }
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
    var mechanicalDelays: Bool {
        get { return amiga.getConfig(.EMULATE_MECHANICS, drive: 0) != 0 }
        set {
            amiga.configure(.EMULATE_MECHANICS, drive: 0, enable: newValue)
            amiga.configure(.EMULATE_MECHANICS, drive: 1, enable: newValue)
            amiga.configure(.EMULATE_MECHANICS, drive: 2, enable: newValue)
            amiga.configure(.EMULATE_MECHANICS, drive: 3, enable: newValue)
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
    var hAutoCenter = false
    var hCenter = GeometryDefaults.wide.hCenter {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var vAutoCenter = false
    var vCenter = GeometryDefaults.wide.vCenter {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var hZoom = GeometryDefaults.wide.hZoom {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var vZoom = GeometryDefaults.wide.vZoom {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var enhancer = VideoDefaults.tft.enhancer {
        didSet {
            if !ressourceManager.selectEnhancer(enhancer) { enhancer = oldValue }
        }
    }
    var upscaler = VideoDefaults.tft.upscaler {
        didSet {
            if !ressourceManager.selectUpscaler(upscaler) { upscaler = oldValue }
        }
    }
    var blur = VideoDefaults.tft.blur {
        didSet { renderer.shaderOptions.blur = blur }
    }
    var blurRadius = VideoDefaults.tft.blurRadius {
        didSet { renderer.shaderOptions.blurRadius = blurRadius }
    }
    var bloom = VideoDefaults.tft.bloom {
        didSet {
            renderer.shaderOptions.bloom = Int32(bloom)
            if !ressourceManager.selectBloomFilter(bloom) { bloom = oldValue }
        }
    }
    var bloomRadius = VideoDefaults.tft.bloomRadius {
        didSet { renderer.shaderOptions.bloomRadius = bloomRadius }
    }
    var bloomBrightness = VideoDefaults.tft.bloomBrightness {
        didSet { renderer.shaderOptions.bloomBrightness = bloomBrightness }
    }
    var bloomWeight = VideoDefaults.tft.bloomWeight {
        didSet { renderer.shaderOptions.bloomWeight = bloomWeight }
    }
    var flicker = VideoDefaults.tft.flicker {
        didSet { renderer.shaderOptions.flicker = flicker }
    }
    var flickerWeight = VideoDefaults.tft.flickerWeight {
        didSet { renderer.shaderOptions.flickerWeight = flickerWeight }
    }
    var dotMask = VideoDefaults.tft.dotMask {
        didSet {
            renderer.shaderOptions.dotMask = Int32(dotMask)
            ressourceManager.buildDotMasks()
            if !ressourceManager.selectDotMask(dotMask) { dotMask = oldValue }
        }
    }
    var dotMaskBrightness = VideoDefaults.tft.dotMaskBrightness {
        didSet {
            renderer.shaderOptions.dotMaskBrightness = dotMaskBrightness
            ressourceManager.buildDotMasks()
        }
    }
    var scanlines = VideoDefaults.tft.scanlines {
        didSet {
            renderer.shaderOptions.scanlines = Int32(scanlines)
            if !ressourceManager.selectScanlineFilter(scanlines) { scanlines = oldValue }
        }
    }
    var scanlineBrightness = VideoDefaults.tft.scanlineBrightness {
        didSet { renderer.shaderOptions.scanlineBrightness = scanlineBrightness }
    }
    var scanlineWeight = VideoDefaults.tft.scanlineWeight {
        didSet { renderer.shaderOptions.scanlineWeight = scanlineWeight }
    }
    var disalignment = VideoDefaults.tft.disalignment {
        didSet { renderer.shaderOptions.disalignment = disalignment }
    }
    var disalignmentH = VideoDefaults.tft.disalignmentH {
        didSet { renderer.shaderOptions.disalignmentH = disalignmentH }
    }
    var disalignmentV = VideoDefaults.tft.disalignmentV {
        didSet { renderer.shaderOptions.disalignmentV = disalignmentV }
    }
    
    init(with controller: MyController) { parent = controller }

    //
    // Roms
    //
    
    /*
    func loadRomUserDefaults() {

        log(level: 2)
        let defaults = UserDefaults.standard

        amiga.suspend()

        extStart = defaults.integer(forKey: Keys.Rom.extStart)
        
        if let url = UserDefaults.womUrl {
            try? amiga.mem.loadRom(url)
        }
        if let url = UserDefaults.romUrl {
            try? amiga.mem.loadRom(url)
        }
        if let url = UserDefaults.extUrl {
            try? amiga.mem.loadExt(url)
        }
        
        amiga.resume()
    }
    */

    func saveRomUserDefaults() throws {
                
        log(level: 2)
        
        let defaults = AmigaProxy.defaults!
        let fm = FileManager.default
        var url: URL?

        amiga.suspend()
        
        defaults.set(.EXT_START, value: extStart)
        defaults.save()
        
        do {
                        
            url = UserDefaults.romUrl
            if url == nil { throw VAError(.FILE_CANT_WRITE) }
            try? fm.removeItem(at: url!)
            try amiga.mem.saveRom(url!)

            url = UserDefaults.extUrl
            if url == nil { throw VAError(.FILE_CANT_WRITE) }
            try? fm.removeItem(at: url!)
            try amiga.mem.saveExt(url!)
            
        } catch {

            amiga.resume()
            throw error
        }
        
        amiga.resume()
    }

    //
    // Chipset
    //

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

    func removeChipsetUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        defaults.remove(.AGNUS_REVISION)
        defaults.remove(.DENISE_REVISION)
        defaults.remove(.CIA_REVISION)
        defaults.remove(.RTC_MODEL)

        amiga.resume()
    }

    func updateChipsetUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        agnusRev = defaults.get(.AGNUS_REVISION)
        deniseRev = defaults.get(.DENISE_REVISION)
        ciaRev = defaults.get(.CIA_REVISION)
        rtClock = defaults.get(.RTC_MODEL)
        
        amiga.resume()
    }
            
    //
    // Memory
    //
    
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
    
    func removeMemoryUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        defaults.remove(.CHIP_RAM)
        defaults.remove(.SLOW_RAM)
        defaults.remove(.FAST_RAM)
        defaults.remove(.RAM_INIT_PATTERN)
        defaults.remove(.BANKMAP)
        defaults.remove(.UNMAPPING_TYPE)
        defaults.remove(.SLOW_RAM_DELAY)
        defaults.remove(.SLOW_RAM_MIRROR)

        amiga.resume()
    }

    func updateMemoryUserDefaults() {
        
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
    
    //
    // Peripherals
    //

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

        defaults.save()
                
        // DEPRECATED
        UserDefaults.standard.set(gameDevice1, forKey: Keys.Per.gameDevice1)
        UserDefaults.standard.set(gameDevice2, forKey: Keys.Per.gameDevice2)

        amiga.resume()
    }

    func removePeripheralsUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        for i in 0 ... 3 {
            
            defaults.remove(.DRIVE_CONNECT, nr: i)
            defaults.remove(.DRIVE_TYPE, nr: i)
            defaults.remove(.HDC_CONNECT, nr: i)
            defaults.remove(.HDR_TYPE, nr: i)
        }
        
        defaults.remove(.SERIAL_DEVICE)
        defaults.remove(.SRV_PORT)

        amiga.resume()
    }

    func updatePeripheralsUserDefaults() {
        
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

        amiga.resume()
    }
    
    //
    // Compatibility
    //
    
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
    
    func removeCompatibilityUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

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
        
        amiga.resume()
    }

    func updateCompatibilityUserDefaults() {
        
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
        mechanicalDelays = defaults.get(.EMULATE_MECHANICS) != 0
        lockDskSync = defaults.get(.LOCK_DSKSYNC) != 0
        autoDskSync = defaults.get(.AUTO_DSKSYNC) != 0
        accurateKeyboard = defaults.get(.ACCURATE_KEYBOARD) != 0
        
        amiga.resume()
    }
    
    //
    // Audio
    //
    
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
    
    func removeAudioUserDefaults() {
        
        log(level: 2)
        let defaults = AmigaProxy.defaults!

        amiga.suspend()

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

        amiga.resume()
    }

    func updateAudioUserDefaults() {
        
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
        stepVolume = defaults.get(.STEP_VOLUME)
        pollVolume = defaults.get(.POLL_VOLUME)
        insertVolume = defaults.get(.INSERT_VOLUME)
        ejectVolume = defaults.get(.EJECT_VOLUME)
        filterType = defaults.get(.FILTER_TYPE)
        filterAlwaysOn = defaults.get(.FILTER_ALWAYS_ON) != 0

        amiga.resume()
    }
    
    //
    // Video
    //

    /*
    func loadColorDefaults(_ defaults: VideoDefaults) {
        
        log(level: 2)
        amiga.suspend()
        
        palette = defaults.palette.rawValue
        brightness = defaults.brightness
        contrast = defaults.contrast
        saturation = defaults.saturation
        
        amiga.resume()
    }
    */
    
    func loadGeometryDefaults(_ defaults: GeometryDefaults) {
        
        hAutoCenter = defaults.hAutoCenter
        vAutoCenter = defaults.vAutoCenter
        hCenter = defaults.hCenter
        vCenter = defaults.vCenter
        hZoom = defaults.hZoom
        vZoom = defaults.vZoom
        
        renderer.canvas.updateTextureRect()
    }
    
    func loadShaderDefaults(_ defaults: VideoDefaults) {
        
        log(level: 2)
        
        enhancer = defaults.enhancer
        upscaler = defaults.upscaler
        
        blur = defaults.blur
        blurRadius = defaults.blurRadius
        
        bloom = defaults.bloom
        bloomRadius = defaults.bloomRadius
        bloomBrightness = defaults.bloomBrightness
        bloomWeight = defaults.bloomWeight
        flicker = defaults.flicker
        flickerWeight = defaults.flickerWeight
        dotMask = defaults.dotMask
        dotMaskBrightness = defaults.dotMaskBrightness
        scanlines = defaults.scanlines
        scanlineBrightness = defaults.scanlineBrightness
        scanlineWeight = defaults.scanlineWeight
        disalignment = defaults.disalignment
        disalignmentH = defaults.disalignmentH
        disalignment = defaults.disalignment
    }
    
    func loadVideoUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        /*
        palette = defaults.integer(forKey: Keys.Vid.palette)
        brightness = defaults.integer(forKey: Keys.Vid.brightness)
        contrast = defaults.integer(forKey: Keys.Vid.contrast)
        saturation = defaults.integer(forKey: Keys.Vid.saturation)
        */
        
        enhancer = defaults.integer(forKey: Keys.Vid.enhancer)
        upscaler = defaults.integer(forKey: Keys.Vid.upscaler)
        
        bloom = defaults.integer(forKey: Keys.Vid.bloom)
        bloomRadius = defaults.float(forKey: Keys.Vid.bloomRadius)
        bloomBrightness = defaults.float(forKey: Keys.Vid.bloomBrightness)
        bloomWeight = defaults.float(forKey: Keys.Vid.bloomWeight)
        flicker = Int32(defaults.integer(forKey: Keys.Vid.flicker))
        flickerWeight = defaults.float(forKey: Keys.Vid.flickerWeight)
        dotMask = defaults.integer(forKey: Keys.Vid.dotMask)
        dotMaskBrightness = defaults.float(forKey: Keys.Vid.dotMaskBrightness)
        scanlines = defaults.integer(forKey: Keys.Vid.scanlines)
        scanlineBrightness = defaults.float(forKey: Keys.Vid.scanlineBrightness)
        scanlineWeight = defaults.float(forKey: Keys.Vid.scanlineWeight)
        disalignment = Int32(defaults.integer(forKey: Keys.Vid.disalignment))
        disalignmentH = defaults.float(forKey: Keys.Vid.disalignmentH)
        disalignmentV = defaults.float(forKey: Keys.Vid.disalignmentV)
                
        amiga.resume()
    }

    func loadGeometryUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        hAutoCenter = defaults.bool(forKey: Keys.Vid.hAutoCenter)
        vAutoCenter = defaults.bool(forKey: Keys.Vid.vAutoCenter)
        hCenter = defaults.float(forKey: Keys.Vid.hCenter)
        vCenter = defaults.float(forKey: Keys.Vid.vCenter)
        hZoom = defaults.float(forKey: Keys.Vid.hZoom)
        vZoom = defaults.float(forKey: Keys.Vid.vZoom)
        
        renderer.canvas.updateTextureRect()
        
        amiga.resume()
    }
    
    func saveVideoUserDefaults() {
        
        log(level: 2)
                
        amiga.suspend()
        
        let defaults = AmigaProxy.defaults!

        defaults.set(.PALETTE, value: palette)
        defaults.set(.BRIGHTNESS, value: brightness)
        defaults.set(.CONTRAST, value: contrast)
        defaults.set(.SATURATION, value: saturation)
        
        /*
        std.set(palette, forKey: Keys.Vid.palette)
        std.set(brightness, forKey: Keys.Vid.brightness)
        std.set(contrast, forKey: Keys.Vid.contrast)
        std.set(saturation, forKey: Keys.Vid.saturation)
        */

        let std = UserDefaults.standard

        // DEPRECATED
        std.set(enhancer, forKey: Keys.Vid.enhancer)
        std.set(upscaler, forKey: Keys.Vid.upscaler)
        
        std.set(bloom, forKey: Keys.Vid.bloom)
        std.set(bloomRadius, forKey: Keys.Vid.bloomRadius)
        std.set(bloomBrightness, forKey: Keys.Vid.bloomBrightness)
        std.set(bloomWeight, forKey: Keys.Vid.bloomWeight)
        std.set(flicker, forKey: Keys.Vid.flicker)
        std.set(flickerWeight, forKey: Keys.Vid.flickerWeight)
        std.set(dotMask, forKey: Keys.Vid.dotMask)
        std.set(dotMaskBrightness, forKey: Keys.Vid.dotMaskBrightness)
        std.set(scanlines, forKey: Keys.Vid.scanlines)
        std.set(scanlineBrightness, forKey: Keys.Vid.scanlineBrightness)
        std.set(scanlineWeight, forKey: Keys.Vid.scanlineWeight)
        std.set(disalignment, forKey: Keys.Vid.disalignment)
        std.set(disalignmentH, forKey: Keys.Vid.disalignmentH)
        std.set(disalignmentV, forKey: Keys.Vid.disalignmentV)
    }
    
    func saveGeometryUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        defaults.set(hAutoCenter, forKey: Keys.Vid.hAutoCenter)
        defaults.set(vAutoCenter, forKey: Keys.Vid.vAutoCenter)
        defaults.set(hCenter, forKey: Keys.Vid.hCenter)
        defaults.set(vCenter, forKey: Keys.Vid.vCenter)
        defaults.set(hZoom, forKey: Keys.Vid.hZoom)
        defaults.set(vZoom, forKey: Keys.Vid.vZoom)
    }
}
