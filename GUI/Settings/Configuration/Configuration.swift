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
        return amiga.getConfig(.HDR_CONNECT, drive: n) != 0
    }
    func setHdnConnected(_ n: Int, connect: Bool) {
        precondition(0 <= n && n <= 3)
        amiga.configure(.HDR_CONNECT, drive: n, enable: connect)
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
    var hdPersist = [ false, false, false, false ]

    var gameDevice1 = PeripheralsDefaults.std.gameDevice1 {
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
    var gameDevice2 = PeripheralsDefaults.std.gameDevice2 {
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
    
    func saveRomUserDefaults() {
                
        log(level: 2)
        
        let fm = FileManager.default
        let defaults = UserDefaults.standard
        var url: URL?

        amiga.suspend()
                
        defaults.set(extStart, forKey: Keys.Rom.extStart)
        
        do {
            url = UserDefaults.womUrl
            if url == nil { throw VAError(.FILE_CANT_WRITE) }
            try? fm.removeItem(at: url!)
            try amiga.mem.saveWom(url!)
            
            url = UserDefaults.romUrl
            if url == nil { throw VAError(.FILE_CANT_WRITE) }
            try? fm.removeItem(at: url!)
            try amiga.mem.saveRom(url!)

            url = UserDefaults.extUrl
            if url == nil { throw VAError(.FILE_CANT_WRITE) }
            try? fm.removeItem(at: url!)
            try amiga.mem.saveExt(url!)
            
        } catch {
            
            if error is VAError && url != nil {
                
                VAError.warning("Failed to save Roms",
                                "Can't write to file \(url!.path)")
            }
            if error is VAError && url == nil {
                
                VAError.warning("Failed to save Roms",
                                "Unable to access the application defaults folder")
            }
        }
        
        amiga.resume()
    }

    //
    // Chipset
    //
    
    func loadChipsetDefaults(_ defaults: HardwareDefaults) {
        
        log(level: 2)
        amiga.suspend()
        
        agnusRev = defaults.agnusRev.rawValue
        deniseRev = defaults.deniseRev.rawValue
        ciaRev = defaults.ciaRev.rawValue
        rtClock = defaults.realTimeClock.rawValue
        
        amiga.resume()
    }
    
    func loadChipsetUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        agnusRev = defaults.integer(forKey: Keys.Hrw.agnusRev)
        deniseRev = defaults.integer(forKey: Keys.Hrw.deniseRev)
        ciaRev = defaults.integer(forKey: Keys.Hrw.ciaRev)
        rtClock = defaults.integer(forKey: Keys.Hrw.realTimeClock)

        amiga.resume()
    }
    
    func saveChipsetUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard

        defaults.set(agnusRev, forKey: Keys.Hrw.agnusRev)
        defaults.set(deniseRev, forKey: Keys.Hrw.deniseRev)
        defaults.set(ciaRev, forKey: Keys.Hrw.ciaRev)
        defaults.set(rtClock, forKey: Keys.Hrw.realTimeClock)
    }

    //
    // Memory
    //
    
    func loadMemoryDefaults(_ defaults: MemoryDefaults) {
        
        log(level: 2)
        amiga.suspend()
        
        chipRam = defaults.chipRam
        slowRam = defaults.slowRam
        fastRam = defaults.fastRam
        ramInitPattern = defaults.ramInitPattern.rawValue

        bankMap = defaults.bankMap.rawValue
        unmappingType = defaults.unmappingType.rawValue
        
        slowRamDelay = defaults.slowRamDelay
        slowRamMirror = defaults.slowRamMirror

        amiga.resume()
    }
    
    func loadMemoryUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        chipRam = defaults.integer(forKey: Keys.Mem.chipRam)
        slowRam = defaults.integer(forKey: Keys.Mem.slowRam)
        fastRam = defaults.integer(forKey: Keys.Mem.fastRam)
        ramInitPattern = defaults.integer(forKey: Keys.Mem.ramInitPattern)

        bankMap = defaults.integer(forKey: Keys.Mem.bankMap)
        unmappingType = defaults.integer(forKey: Keys.Mem.unmappingType)

        slowRamDelay = defaults.bool(forKey: Keys.Mem.slowRamDelay)
        slowRamMirror = defaults.bool(forKey: Keys.Mem.slowRamMirror)
        
        amiga.resume()
    }
    
    func saveMemoryUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard

        defaults.set(chipRam, forKey: Keys.Mem.chipRam)
        defaults.set(slowRam, forKey: Keys.Mem.slowRam)
        defaults.set(fastRam, forKey: Keys.Mem.fastRam)
        defaults.set(ramInitPattern, forKey: Keys.Mem.ramInitPattern)

        defaults.set(bankMap, forKey: Keys.Mem.bankMap)
        defaults.set(unmappingType, forKey: Keys.Mem.unmappingType)
        
        defaults.set(slowRamDelay, forKey: Keys.Mem.slowRamDelay)
        defaults.set(slowRamMirror, forKey: Keys.Mem.slowRamMirror)
    }
    
    //
    // Peripherals
    //
    
    func loadPeripheralsDefaults(_ defaults: PeripheralsDefaults) {
        
        log(level: 2)
        
        amiga.suspend()
                
        df0Connected = defaults.driveConnect[0]
        df1Connected = defaults.driveConnect[1]
        df2Connected = defaults.driveConnect[2]
        df3Connected = defaults.driveConnect[3]
        df0Type = defaults.driveType[0].rawValue
        df1Type = defaults.driveType[1].rawValue
        df2Type = defaults.driveType[2].rawValue
        df3Type = defaults.driveType[3].rawValue
 
        hd0Connected = defaults.hardDriveConnect[0]
        hd1Connected = defaults.hardDriveConnect[1]
        hd2Connected = defaults.hardDriveConnect[2]
        hd3Connected = defaults.hardDriveConnect[3]
        hd0Type = defaults.hardDriveType[0].rawValue
        hd1Type = defaults.hardDriveType[1].rawValue
        hd2Type = defaults.hardDriveType[2].rawValue
        hd3Type = defaults.hardDriveType[3].rawValue
        
        gameDevice1 = defaults.gameDevice1
        gameDevice2 = defaults.gameDevice2
        serialDevice = defaults.serialDevice.rawValue
        serialDevicePort = defaults.serialDevicePort
                
        amiga.resume()
    }
    
    func loadPeripheralsUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        amiga.hd0.backupPath = UserDefaults.hdnUrl(0)!.path
        amiga.hd1.backupPath = UserDefaults.hdnUrl(1)!.path
        amiga.hd2.backupPath = UserDefaults.hdnUrl(2)!.path
        amiga.hd3.backupPath = UserDefaults.hdnUrl(3)!.path

        df0Connected = defaults.bool(forKey: Keys.Per.df0Connect)
        df1Connected = defaults.bool(forKey: Keys.Per.df1Connect)
        df2Connected = defaults.bool(forKey: Keys.Per.df2Connect)
        df3Connected = defaults.bool(forKey: Keys.Per.df3Connect)
        df0Type = defaults.integer(forKey: Keys.Per.df0Type)
        df1Type = defaults.integer(forKey: Keys.Per.df1Type)
        df2Type = defaults.integer(forKey: Keys.Per.df2Type)
        df3Type = defaults.integer(forKey: Keys.Per.df3Type)

        hd0Connected = defaults.bool(forKey: Keys.Per.hd0Connect)
        hd1Connected = defaults.bool(forKey: Keys.Per.hd1Connect)
        hd2Connected = defaults.bool(forKey: Keys.Per.hd2Connect)
        hd3Connected = defaults.bool(forKey: Keys.Per.hd3Connect)
        hd0Type = defaults.integer(forKey: Keys.Per.hd0Type)
        hd1Type = defaults.integer(forKey: Keys.Per.hd1Type)
        hd2Type = defaults.integer(forKey: Keys.Per.hd2Type)
        hd3Type = defaults.integer(forKey: Keys.Per.hd3Type)

        gameDevice1 = defaults.integer(forKey: Keys.Per.gameDevice1)
        gameDevice2 = defaults.integer(forKey: Keys.Per.gameDevice2)
        serialDevice = defaults.integer(forKey: Keys.Per.serialDevice)
        serialDevicePort = defaults.integer(forKey: Keys.Per.serialDevicePort)
        
        for n in 0...3 {
            
            if let url = UserDefaults.hdnUrl(n) {
                
                if FileManager.default.fileExists(atPath: url.path) {
                    log("File \(url) found")
                    hdPersist[n] = true
                } else {
                    log("File \(url) not found")
                    hdPersist[n] = false
                }
            }
        }
        
        amiga.resume()
    }
    
    func savePeripheralsUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        defaults.set(df0Connected, forKey: Keys.Per.df0Connect)
        defaults.set(df1Connected, forKey: Keys.Per.df1Connect)
        defaults.set(df2Connected, forKey: Keys.Per.df2Connect)
        defaults.set(df3Connected, forKey: Keys.Per.df3Connect)
        defaults.set(df0Type, forKey: Keys.Per.df0Type)
        defaults.set(df1Type, forKey: Keys.Per.df1Type)
        defaults.set(df2Type, forKey: Keys.Per.df2Type)
        defaults.set(df3Type, forKey: Keys.Per.df3Type)

        defaults.set(hd0Connected, forKey: Keys.Per.hd0Connect)
        defaults.set(hd1Connected, forKey: Keys.Per.hd1Connect)
        defaults.set(hd2Connected, forKey: Keys.Per.hd2Connect)
        defaults.set(hd3Connected, forKey: Keys.Per.hd3Connect)
        defaults.set(hd0Type, forKey: Keys.Per.hd0Type)
        defaults.set(hd1Type, forKey: Keys.Per.hd1Type)
        defaults.set(hd2Type, forKey: Keys.Per.hd2Type)
        defaults.set(hd3Type, forKey: Keys.Per.hd3Type)

        defaults.set(gameDevice1, forKey: Keys.Per.gameDevice1)
        defaults.set(gameDevice2, forKey: Keys.Per.gameDevice2)
        defaults.set(serialDevice, forKey: Keys.Per.serialDevice)
        defaults.set(serialDevicePort, forKey: Keys.Per.serialDevicePort)
    }

    func persistHardDrives() throws {

        for n in 0...3 { try persistHd(n) }
    }

    func persistHd(_ n: Int) throws {

        var url: URL?
        
        do {
            // Get URL for the backup file
            url = UserDefaults.hdnUrl(n)
            if url == nil { throw VAError(.FILE_CANT_WRITE) }
            
            // Remove the old file (if any)
            let fm = FileManager.default
            try? fm.removeItem(at: url!)

            // Save hard drive image
            if hdPersist[n] { try amiga.hd(n)?.writeToFile(url!) }
            
        } catch {
            
            if error is VAError && url != nil {
                
                VAError.warning("Failed to save hard drive HD\(n)",
                                "Can't write to file \(url!.path)")
            }
            if error is VAError && url == nil {
                
                VAError.warning("Failed to save hard drive HD\(n)",
                                "Unable to access the application defaults folder")
            }
        }
    }
    
    //
    // Compatibility
    //
    
    func loadCompatibilityDefaults(_ defaults: CompatibilityDefaults) {
         
        log(level: 2)
        amiga.suspend()
        
        blitterAccuracy = defaults.blitterAccuracy

        todBug = defaults.todBug

        eClockSyncing = defaults.eClockSyncing

        clxSprSpr = defaults.clxSprSpr
        clxSprPlf = defaults.clxSprPlf
        clxPlfPlf = defaults.clxPlfPlf
                        
        driveSpeed = defaults.driveSpeed
        mechanicalDelays = defaults.mechanicalDelays
        lockDskSync = defaults.lockDskSync
        autoDskSync = defaults.autoDskSync
        
        accurateKeyboard = defaults.accurateKeyboard

        amiga.resume()
     }
    
    func loadCompatibilityUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        blitterAccuracy = defaults.integer(forKey: Keys.Com.blitterAccuracy)

        todBug = defaults.bool(forKey: Keys.Com.todBug)

        eClockSyncing = defaults.bool(forKey: Keys.Com.eClockSyncing)

        clxSprSpr = defaults.bool(forKey: Keys.Com.clxSprSpr)
        clxSprPlf = defaults.bool(forKey: Keys.Com.clxSprPlf)
        clxPlfPlf = defaults.bool(forKey: Keys.Com.clxPlfPlf)
                
        driveSpeed = defaults.integer(forKey: Keys.Com.driveSpeed)
        mechanicalDelays = defaults.bool(forKey: Keys.Com.mechanicalDelays)
        lockDskSync = defaults.bool(forKey: Keys.Com.lockDskSync)
        autoDskSync = defaults.bool(forKey: Keys.Com.autoDskSync)

        accurateKeyboard = defaults.bool(forKey: Keys.Com.accurateKeyboard)

        amiga.resume()
     }
    
    func saveCompatibilityUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        defaults.set(blitterAccuracy, forKey: Keys.Com.blitterAccuracy)

        defaults.set(todBug, forKey: Keys.Com.todBug)

        defaults.set(eClockSyncing, forKey: Keys.Com.eClockSyncing)

        defaults.set(clxSprSpr, forKey: Keys.Com.clxSprSpr)
        defaults.set(clxSprPlf, forKey: Keys.Com.clxSprPlf)
        defaults.set(clxPlfPlf, forKey: Keys.Com.clxPlfPlf)
                
        defaults.set(driveSpeed, forKey: Keys.Com.driveSpeed)
        defaults.set(mechanicalDelays, forKey: Keys.Com.mechanicalDelays)
        defaults.set(lockDskSync, forKey: Keys.Com.lockDskSync)
        defaults.set(autoDskSync, forKey: Keys.Com.autoDskSync)
        
        defaults.set(accurateKeyboard, forKey: Keys.Com.accurateKeyboard)
    }
    
    //
    // Audio
    //
    
    func loadAudioDefaults(_ defaults: AudioDefaults) {
        
        log(level: 2)
        amiga.suspend()
        
        vol0 = defaults.vol0
        vol1 = defaults.vol1
        vol2 = defaults.vol2
        vol3 = defaults.vol3
        pan0 = defaults.pan0
        pan1 = defaults.pan1
        pan2 = defaults.pan2
        pan3 = defaults.pan3
        
        volL = defaults.volL
        volR = defaults.volR
        samplingMethod = defaults.samplingMethod.rawValue
        
        df0Pan = defaults.drivePan[0]
        df1Pan = defaults.drivePan[1]
        df2Pan = defaults.drivePan[2]
        df3Pan = defaults.drivePan[3]
        hd0Pan = defaults.hdPan[0]
        hd1Pan = defaults.hdPan[1]
        hd2Pan = defaults.hdPan[2]
        hd3Pan = defaults.hdPan[3]
        stepVolume = defaults.stepVolume
        pollVolume = defaults.pollVolume
        insertVolume = defaults.insertVolume
        ejectVolume = defaults.ejectVolume
        
        filterType = defaults.filterType.rawValue
        filterAlwaysOn = defaults.filterAlwaysOn

        amiga.resume()
    }
    
    func loadAudioUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        vol0 = defaults.integer(forKey: Keys.Aud.vol0)
        vol1 = defaults.integer(forKey: Keys.Aud.vol1)
        vol2 = defaults.integer(forKey: Keys.Aud.vol2)
        vol3 = defaults.integer(forKey: Keys.Aud.vol3)
        pan0 = defaults.integer(forKey: Keys.Aud.pan0)
        pan1 = defaults.integer(forKey: Keys.Aud.pan1)
        pan2 = defaults.integer(forKey: Keys.Aud.pan2)
        pan3 = defaults.integer(forKey: Keys.Aud.pan3)
        
        volL = defaults.integer(forKey: Keys.Aud.volL)
        volR = defaults.integer(forKey: Keys.Aud.volR)
        samplingMethod = defaults.integer(forKey: Keys.Aud.samplingMethod)
        
        df0Pan = defaults.integer(forKey: Keys.Aud.df0Pan)
        df1Pan = defaults.integer(forKey: Keys.Aud.df1Pan)
        df2Pan = defaults.integer(forKey: Keys.Aud.df2Pan)
        df3Pan = defaults.integer(forKey: Keys.Aud.df3Pan)
        hd0Pan = defaults.integer(forKey: Keys.Aud.hd0Pan)
        hd1Pan = defaults.integer(forKey: Keys.Aud.hd1Pan)
        hd2Pan = defaults.integer(forKey: Keys.Aud.hd2Pan)
        hd3Pan = defaults.integer(forKey: Keys.Aud.hd3Pan)
        stepVolume = defaults.integer(forKey: Keys.Aud.stepVolume)
        pollVolume = defaults.integer(forKey: Keys.Aud.pollVolume)
        insertVolume = defaults.integer(forKey: Keys.Aud.insertVolume)
        ejectVolume = defaults.integer(forKey: Keys.Aud.ejectVolume)

        filterType = defaults.integer(forKey: Keys.Aud.filterType)
        filterAlwaysOn = defaults.bool(forKey: Keys.Aud.filterAlwaysOn)

        amiga.resume()
    }
    
    func saveAudioUserDefaults() {
        
        log(level: 2)
        let defaults = UserDefaults.standard
        
        defaults.set(vol0, forKey: Keys.Aud.vol0)
        defaults.set(vol1, forKey: Keys.Aud.vol1)
        defaults.set(vol2, forKey: Keys.Aud.vol2)
        defaults.set(vol3, forKey: Keys.Aud.vol3)
        defaults.set(pan0, forKey: Keys.Aud.pan0)
        defaults.set(pan1, forKey: Keys.Aud.pan1)
        defaults.set(pan2, forKey: Keys.Aud.pan2)
        defaults.set(pan3, forKey: Keys.Aud.pan3)
        
        defaults.set(volL, forKey: Keys.Aud.volL)
        defaults.set(volR, forKey: Keys.Aud.volR)
        defaults.set(samplingMethod, forKey: Keys.Aud.samplingMethod)
        
        defaults.set(df0Pan, forKey: Keys.Aud.df0Pan)
        defaults.set(df1Pan, forKey: Keys.Aud.df1Pan)
        defaults.set(df2Pan, forKey: Keys.Aud.df2Pan)
        defaults.set(df3Pan, forKey: Keys.Aud.df3Pan)
        defaults.set(hd0Pan, forKey: Keys.Aud.hd0Pan)
        defaults.set(hd1Pan, forKey: Keys.Aud.hd1Pan)
        defaults.set(hd2Pan, forKey: Keys.Aud.hd2Pan)
        defaults.set(hd3Pan, forKey: Keys.Aud.hd3Pan)
        defaults.set(stepVolume, forKey: Keys.Aud.stepVolume)
        defaults.set(pollVolume, forKey: Keys.Aud.pollVolume)
        defaults.set(insertVolume, forKey: Keys.Aud.insertVolume)
        defaults.set(ejectVolume, forKey: Keys.Aud.ejectVolume)

        defaults.set(filterType, forKey: Keys.Aud.filterType)
        defaults.set(filterAlwaysOn, forKey: Keys.Aud.filterAlwaysOn)
    }
    
    //
    // Video
    //

    func loadColorDefaults(_ defaults: VideoDefaults) {
        
        log(level: 2)
        amiga.suspend()
        
        palette = defaults.palette.rawValue
        brightness = defaults.brightness
        contrast = defaults.contrast
        saturation = defaults.saturation
        
        amiga.resume()
    }
    
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
        
        palette = defaults.integer(forKey: Keys.Vid.palette)
        brightness = defaults.integer(forKey: Keys.Vid.brightness)
        contrast = defaults.integer(forKey: Keys.Vid.contrast)
        saturation = defaults.integer(forKey: Keys.Vid.saturation)

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
        let defaults = UserDefaults.standard
        
        defaults.set(palette, forKey: Keys.Vid.palette)
        defaults.set(brightness, forKey: Keys.Vid.brightness)
        defaults.set(contrast, forKey: Keys.Vid.contrast)
        defaults.set(saturation, forKey: Keys.Vid.saturation)

        defaults.set(enhancer, forKey: Keys.Vid.enhancer)
        defaults.set(upscaler, forKey: Keys.Vid.upscaler)
        
        defaults.set(bloom, forKey: Keys.Vid.bloom)
        defaults.set(bloomRadius, forKey: Keys.Vid.bloomRadius)
        defaults.set(bloomBrightness, forKey: Keys.Vid.bloomBrightness)
        defaults.set(bloomWeight, forKey: Keys.Vid.bloomWeight)
        defaults.set(flicker, forKey: Keys.Vid.flicker)
        defaults.set(flickerWeight, forKey: Keys.Vid.flickerWeight)
        defaults.set(dotMask, forKey: Keys.Vid.dotMask)
        defaults.set(dotMaskBrightness, forKey: Keys.Vid.dotMaskBrightness)
        defaults.set(scanlines, forKey: Keys.Vid.scanlines)
        defaults.set(scanlineBrightness, forKey: Keys.Vid.scanlineBrightness)
        defaults.set(scanlineWeight, forKey: Keys.Vid.scanlineWeight)
        defaults.set(disalignment, forKey: Keys.Vid.disalignment)
        defaults.set(disalignmentH, forKey: Keys.Vid.disalignmentH)
        defaults.set(disalignmentV, forKey: Keys.Vid.disalignmentV)
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
