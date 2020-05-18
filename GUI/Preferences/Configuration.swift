// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Configuration {

    var parent: MyController!
    var amiga: AmigaProxy { return parent.amiga }
    var renderer: Renderer { return parent.renderer }
    var gamePadManager: GamePadManager { return parent.gamePadManager }
    var kbController: KBController { return parent.kbController }

    //
    // Rom settings
    //
    
    // var romURL: URL = RomDefaults.std.rom
    // var extURL: URL = RomDefaults.std.ext
    
    var extStart: Int {
        get { return amiga.getConfig(VA_EXT_START) }
        set { amiga.configure(VA_EXT_START, value: newValue) }
    }

    //
    // Hardware settings
    //
    
    // Chips
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
    
    // Memory
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
 
    // Floppy drives
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

    // Ports
    var gameDevice1 = HardwareDefaults.A500.gameDevice1 {
        didSet {
            parent.connect(device: gameDevice1, port: 1)
            if gameDevice1 == gameDevice2 && gameDevice2 != InputDevice.none {
                gameDevice2 = InputDevice.none
            }
            parent.toolbar.validateVisibleItems()
        }
    }
    var gameDevice2 = HardwareDefaults.A500.gameDevice2 {
        didSet {
            parent.connect(device: gameDevice2, port: 2)
            if gameDevice2 == gameDevice1 && gameDevice1 != InputDevice.none {
                gameDevice1 = InputDevice.none
            }
            parent.toolbar.validateVisibleItems()
        }
    }
    var serialDevice: Int {
         get { return amiga.getConfig(VA_SERIAL_DEVICE) }
         set { amiga.configure(VA_SERIAL_DEVICE, value: newValue) }
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
    var blitterAccuracy: Int {
        get { return amiga.getConfig(VA_BLITTER_ACCURACY) }
        set { amiga.configure(VA_BLITTER_ACCURACY, value: newValue) }
    }
    var driveSpeed: Int {
        get { return amiga.getConfig(VA_DRIVE_SPEED) }
        set { amiga.configure(VA_DRIVE_SPEED, value: newValue) }
    }
    var asyncFifo: Bool {
        get { return amiga.getConfig(VA_ASYNC_FIFO) != 0 }
        set { amiga.configure(VA_ASYNC_FIFO, enable: newValue) }
    }
    var lockDskSync: Bool {
        get { return amiga.getConfig(VA_LOCK_DSKSYNC) != 0 }
        set { amiga.configure(VA_LOCK_DSKSYNC, enable: newValue) }
    }
    var autoDskSync: Bool {
        get { return amiga.getConfig(VA_AUTO_DSKSYNC) != 0 }
        set { amiga.configure(VA_AUTO_DSKSYNC, enable: newValue) }
    }
    var todBug: Bool {
        get { return amiga.getConfig(VA_TODBUG) != 0 }
        set { amiga.configure(VA_TODBUG, enable: newValue) }
    }
    var accurateKeyboard: Bool {
        get { return amiga.getConfig(VA_ACCURATE_KEYBOARD) != 0 }
        set { amiga.configure(VA_ACCURATE_KEYBOARD, enable: newValue) }
    }
    
    //
    // Audio settings
    //

    var vol0: Double {
        get { return amiga.paula.vol(0) }
        set { amiga.paula.setVol(0, value: newValue) }
    }
    var vol1: Double {
        get { return amiga.paula.vol(1) }
        set { amiga.paula.setVol(1, value: newValue) }
    }
    var vol2: Double {
        get { return amiga.paula.vol(2) }
        set { amiga.paula.setVol(2, value: newValue) }
    }
    var vol3: Double {
        get { return amiga.paula.vol(3) }
        set { amiga.paula.setVol(3, value: newValue) }
    }
    var pan0: Double {
        get { return amiga.paula.pan(0) }
        set { amiga.paula.setPan(0, value: newValue) }
    }
    var pan1: Double {
        get { return amiga.paula.pan(1) }
        set { amiga.paula.setPan(1, value: newValue) }
    }
    var pan2: Double {
        get { return amiga.paula.pan(2) }
        set { amiga.paula.setPan(2, value: newValue) }
    }
    var pan3: Double {
        get { return amiga.paula.pan(3) }
        set { amiga.paula.setPan(3, value: newValue) }
    }
    var volL: Double {
        get { return amiga.paula.volL() }
        set { amiga.paula.setVolL(newValue) }
    }
    var volR: Double {
        get { return amiga.paula.volR() }
        set { amiga.paula.setVolR(newValue) }
    }
    var samplingMethod: Int {
        get { return amiga.getConfig(VA_SAMPLING_METHOD) }
        set { amiga.configure(VA_SAMPLING_METHOD, value: newValue) }
    }
    var filterType: Int {
        get { return amiga.getConfig(VA_FILTER_TYPE) }
        set { amiga.configure(VA_FILTER_TYPE, value: newValue) }
    }
    var filterAlwaysOn: Bool {
        get { return amiga.getConfig(VA_FILTER_ALWAYS_ON) != 0}
        set { amiga.configure(VA_FILTER_ALWAYS_ON, enable: newValue) }
    }
    
    //
    // Video settings
    //
    
    var palette: Int {
        get { return Int(amiga.denise.palette()) }
        set { amiga.denise.setPalette(Palette(newValue)) }
    }
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
    var hCenter = VideoDefaults.tft.hCenter {
        didSet { renderer.updateTextureRect() }
    }
    var vCenter = VideoDefaults.tft.vCenter {
        didSet { renderer.updateTextureRect() }
    }
    var hZoom = VideoDefaults.tft.hZoom {
        didSet { renderer.updateTextureRect() }
    }
    var vZoom = VideoDefaults.tft.vZoom {
        didSet { renderer.updateTextureRect() }
    }
    var enhancer = VideoDefaults.tft.enhancer {
        didSet { if !renderer.selectEnhancer(enhancer) { enhancer = oldValue } }
    }
    var upscaler = VideoDefaults.tft.upscaler {
        didSet { if !renderer.selectUpscaler(upscaler) { upscaler = oldValue } }
    }
    var blur = VideoDefaults.tft.blur {
        didSet { renderer.shaderOptions.blur = blur }
    }
    var blurRadius = VideoDefaults.tft.blurRadius {
        didSet { renderer.shaderOptions.blurRadius = blurRadius }
    }
    var bloom = VideoDefaults.tft.bloom {
        didSet { renderer.shaderOptions.bloom = bloom }
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
            renderer.shaderOptions.dotMask = dotMask
            renderer.buildDotMasks()
        }
    }
    var dotMaskBrightness = VideoDefaults.tft.dotMaskBrightness {
        didSet {
            renderer.shaderOptions.dotMaskBrightness = dotMaskBrightness
            renderer.buildDotMasks()
        }
    }
    var scanlines = VideoDefaults.tft.scanlines {
        didSet { renderer.shaderOptions.scanlines = scanlines }
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

        amiga.suspend()
        
        if let wom = UserDefaults.romUrl(name: "wom.bin") {
            track("Seeking Wom")
            amiga.mem.loadRom(fromFile: wom)
        }
        
        if let rom = UserDefaults.romUrl(name: "rom.bin") {
            track("Seeking Rom")
            amiga.mem.loadRom(fromFile: rom)
        }
  
        if let ext = UserDefaults.romUrl(name: "ext.bin") {
            track("Seeking Ext")
            amiga.mem.loadExt(fromFile: ext)
        }
        
        let defaults = UserDefaults.standard
        extStart = defaults.integer(forKey: Keys.extStart)

        amiga.resume()
    }
    
    func saveRomUserDefaults() {
        
        let fm = FileManager.default
        
        amiga.suspend()
                
        if let wom = UserDefaults.romUrl(name: "wom.bin") {
            track("Saving Wom")
            try? fm.removeItem(at: wom)
            amiga.mem.saveWom(wom)
        }
        
        if let rom = UserDefaults.romUrl(name: "rom.bin") {
            track("Saving Rom")
            try? fm.removeItem(at: rom)
            amiga.mem.saveRom(rom)
        }
        
        if let ext = UserDefaults.romUrl(name: "ext.bin") {
            track("Saving Ext")
            try? fm.removeItem(at: ext)
            amiga.mem.saveExt(ext)
        }
        
        let defaults = UserDefaults.standard
        defaults.set(extStart, forKey: Keys.extStart)
    }

    //
    // Hardware
    //
    
    func loadHardwareDefaults(_ defaults: HardwareDefaults) {
        
        amiga.suspend()
        
        agnusRev = defaults.agnusRevision.rawValue
        deniseRev = defaults.deniseRevision.rawValue
        rtClock = defaults.realTimeClock.rawValue
        
        chipRam = defaults.chipRam
        slowRam = defaults.slowRam
        fastRam = defaults.fastRam
        
        df0Connected = defaults.driveConnect[0]
        df1Connected = defaults.driveConnect[1]
        df2Connected = defaults.driveConnect[2]
        df3Connected = defaults.driveConnect[3]
        df0Type = defaults.driveType[0].rawValue
        df1Type = defaults.driveType[1].rawValue
        df2Type = defaults.driveType[2].rawValue
        df3Type = defaults.driveType[3].rawValue
        
        gameDevice1 = defaults.gameDevice1
        gameDevice2 = defaults.gameDevice2
        serialDevice = defaults.serialDevice.rawValue
        
        amiga.resume()
    }
    
    func loadHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        agnusRev = defaults.integer(forKey: Keys.agnusRev)
        deniseRev = defaults.integer(forKey: Keys.deniseRev)
        rtClock = defaults.integer(forKey: Keys.realTimeClock)

        chipRam = defaults.integer(forKey: Keys.chipRam)
        slowRam = defaults.integer(forKey: Keys.slowRam)
        fastRam = defaults.integer(forKey: Keys.fastRam)
        
        driveSpeed = defaults.integer(forKey: Keys.driveSpeed)
        df0Connected = defaults.bool(forKey: Keys.df0Connect)
        df1Connected = defaults.bool(forKey: Keys.df1Connect)
        df2Connected = defaults.bool(forKey: Keys.df2Connect)
        df3Connected = defaults.bool(forKey: Keys.df3Connect)
        df0Type = defaults.integer(forKey: Keys.df0Type)
        df1Type = defaults.integer(forKey: Keys.df1Type)
        df2Type = defaults.integer(forKey: Keys.df2Type)
        df3Type = defaults.integer(forKey: Keys.df3Type)

        gameDevice1 = defaults.integer(forKey: Keys.gameDevice1)
        gameDevice2 = defaults.integer(forKey: Keys.gameDevice2)
        serialDevice = defaults.integer(forKey: Keys.serialDevice)

        amiga.resume()
    }
    
    func saveHardwareUserDefaults() {
        
        track()
        
        let defaults = UserDefaults.standard

        defaults.set(agnusRev, forKey: Keys.agnusRev)
        defaults.set(deniseRev, forKey: Keys.deniseRev)
        defaults.set(rtClock, forKey: Keys.realTimeClock)

        defaults.set(chipRam, forKey: Keys.chipRam)
        defaults.set(slowRam, forKey: Keys.slowRam)
        defaults.set(fastRam, forKey: Keys.fastRam)

        defaults.set(driveSpeed, forKey: Keys.driveSpeed)
        defaults.set(df0Connected, forKey: Keys.df0Connect)
        defaults.set(df1Connected, forKey: Keys.df1Connect)
        defaults.set(df2Connected, forKey: Keys.df2Connect)
        defaults.set(df3Connected, forKey: Keys.df3Connect)
        defaults.set(df0Type, forKey: Keys.df0Type)
        defaults.set(df1Type, forKey: Keys.df1Type)
        defaults.set(df2Type, forKey: Keys.df2Type)
        defaults.set(df3Type, forKey: Keys.df3Type)

        defaults.set(gameDevice1, forKey: Keys.gameDevice1)
        defaults.set(gameDevice2, forKey: Keys.gameDevice2)
        defaults.set(serialDevice, forKey: Keys.serialDevice)
    }
    
    //
    // Compatibility
    //
    
    func loadCompatibilityDefaults(_ defaults: CompatibilityDefaults) {
         
        amiga.suspend()
        
        clxSprSpr = defaults.clxSprSpr
        clxSprPlf = defaults.clxSprPlf
        clxPlfPlf = defaults.clxPlfPlf
                
        blitterAccuracy = defaults.blitterAccuracy

        todBug = defaults.todBug

        driveSpeed = defaults.driveSpeed
        asyncFifo = defaults.asyncFifo
        lockDskSync = defaults.lockDskSync
        autoDskSync = defaults.autoDskSync
        
        accurateKeyboard = defaults.accurateKeyboard

        amiga.resume()
     }
    
    func loadCompatibilityUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        clxSprSpr = defaults.bool(forKey: Keys.clxSprSpr)
        clxSprPlf = defaults.bool(forKey: Keys.clxSprPlf)
        clxPlfPlf = defaults.bool(forKey: Keys.clxPlfPlf)
        blitterAccuracy = defaults.integer(forKey: Keys.blitterAccuracy)
        todBug = defaults.bool(forKey: Keys.todBug)
        driveSpeed = defaults.integer(forKey: Keys.driveSpeed)
        asyncFifo = defaults.bool(forKey: Keys.asyncFifo)
        lockDskSync = defaults.bool(forKey: Keys.lockDskSync)
        autoDskSync = defaults.bool(forKey: Keys.autoDskSync)
        accurateKeyboard = defaults.bool(forKey: Keys.accurateKeyboard)

        amiga.resume()
     }

     func saveCompatibilityUserDefaults() {

         track()
         
         let defaults = UserDefaults.standard

         defaults.set(clxSprSpr, forKey: Keys.clxSprSpr)
         defaults.set(clxSprPlf, forKey: Keys.clxSprPlf)
         defaults.set(clxPlfPlf, forKey: Keys.clxPlfPlf)
         defaults.set(blitterAccuracy, forKey: Keys.blitterAccuracy)
         defaults.set(todBug, forKey: Keys.todBug)
         defaults.set(driveSpeed, forKey: Keys.driveSpeed)
         defaults.set(asyncFifo, forKey: Keys.asyncFifo)
         defaults.set(lockDskSync, forKey: Keys.lockDskSync)
         defaults.set(autoDskSync, forKey: Keys.autoDskSync)
         defaults.set(accurateKeyboard, forKey: Keys.accurateKeyboard)
     }
    
    //
    // Audio
    //
    
    func loadAudioDefaults(_ defaults: AudioDefaults) {
        
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
        filterType = defaults.filterType.rawValue
        filterAlwaysOn = defaults.filterAlwaysOn
        
        amiga.resume()
    }
    
    func loadAudioUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        vol0 = defaults.double(forKey: Keys.vol0)
        vol1 = defaults.double(forKey: Keys.vol1)
        vol2 = defaults.double(forKey: Keys.vol2)
        vol3 = defaults.double(forKey: Keys.vol3)
        pan0 = defaults.double(forKey: Keys.pan0)
        pan1 = defaults.double(forKey: Keys.pan1)
        pan2 = defaults.double(forKey: Keys.pan2)
        pan3 = defaults.double(forKey: Keys.pan3)
        
        volL = defaults.double(forKey: Keys.volL)
        volR = defaults.double(forKey: Keys.volR)
        samplingMethod = defaults.integer(forKey: Keys.samplingMethod)
        filterType = defaults.integer(forKey: Keys.filterType)
        filterAlwaysOn = defaults.bool(forKey: Keys.filterAlwaysOn)
        
        amiga.resume()
    }
    
    func saveAudioUserDefaults() {
        
        track()
        
        let defaults = UserDefaults.standard
        
        defaults.set(vol0, forKey: Keys.vol0)
        defaults.set(vol1, forKey: Keys.vol1)
        defaults.set(vol2, forKey: Keys.vol2)
        defaults.set(vol3, forKey: Keys.vol3)
        defaults.set(pan0, forKey: Keys.pan0)
        defaults.set(pan1, forKey: Keys.pan1)
        defaults.set(pan2, forKey: Keys.pan2)
        defaults.set(pan3, forKey: Keys.pan3)
        
        defaults.set(volL, forKey: Keys.volL)
        defaults.set(volR, forKey: Keys.volR)
        defaults.set(samplingMethod, forKey: Keys.samplingMethod)
        defaults.set(filterType, forKey: Keys.filterType)
        defaults.set(filterAlwaysOn, forKey: Keys.filterAlwaysOn)
    }
    
    //
    // Video
    //

    func loadColorDefaults(_ defaults: VideoDefaults) {
        
        amiga.suspend()
        
        palette = defaults.palette.rawValue
        brightness = defaults.brightness
        contrast = defaults.contrast
        saturation = defaults.saturation
        
        amiga.resume()
    }
    
    func loadGeometryDefaults(_ defaults: VideoDefaults) {
        
        hCenter = defaults.hCenter
        vCenter = defaults.vCenter
        hZoom = defaults.hZoom
        vZoom = defaults.vZoom
        
        renderer.updateTextureRect()
    }
    
    func loadShaderDefaults(_ defaults: VideoDefaults) {
        
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
        
        renderer.buildDotMasks()
    }
    
    func loadVideoDefaults(_ defaults: VideoDefaults) {
        
        loadColorDefaults(defaults)
        loadGeometryDefaults(defaults)
        loadColorDefaults(defaults)
    }

    func loadVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        palette = defaults.integer(forKey: Keys.palette)
        brightness = defaults.double(forKey: Keys.brightness)
        contrast = defaults.double(forKey: Keys.contrast)
        saturation = defaults.double(forKey: Keys.saturation)

        hCenter = defaults.float(forKey: Keys.hCenter)
        vCenter = defaults.float(forKey: Keys.vCenter)
        hZoom = defaults.float(forKey: Keys.hZoom)
        vZoom = defaults.float(forKey: Keys.vZoom)

        enhancer = defaults.integer(forKey: Keys.enhancer)
        upscaler = defaults.integer(forKey: Keys.upscaler)
        
        bloom = Int32(defaults.integer(forKey: Keys.bloom))
        bloomRadius = defaults.float(forKey: Keys.bloomRadius)
        bloomBrightness = defaults.float(forKey: Keys.bloomBrightness)
        bloomWeight = defaults.float(forKey: Keys.bloomWeight)
        flicker = Int32(defaults.integer(forKey: Keys.flicker))
        flickerWeight = defaults.float(forKey: Keys.flickerWeight)
        dotMask = Int32(defaults.integer(forKey: Keys.dotMask))
        dotMaskBrightness = defaults.float(forKey: Keys.dotMaskBrightness)
        scanlines = Int32(defaults.integer(forKey: Keys.scanlines))
        scanlineBrightness = defaults.float(forKey: Keys.scanlineBrightness)
        scanlineWeight = defaults.float(forKey: Keys.scanlineWeight)
        disalignment = Int32(defaults.integer(forKey: Keys.disalignment))
        disalignmentH = defaults.float(forKey: Keys.disalignmentH)
        disalignmentV = defaults.float(forKey: Keys.disalignmentV)
        
        renderer.updateTextureRect()
        renderer.buildDotMasks()
        
        amiga.resume()
    }
    
    func saveVideoUserDefaults() {
        
        track()
        
        let defaults = UserDefaults.standard
        
        defaults.set(palette, forKey: Keys.palette)
        defaults.set(brightness, forKey: Keys.brightness)
        defaults.set(contrast, forKey: Keys.contrast)
        defaults.set(saturation, forKey: Keys.saturation)

        defaults.set(hCenter, forKey: Keys.hCenter)
        defaults.set(vCenter, forKey: Keys.vCenter)
        defaults.set(hZoom, forKey: Keys.hZoom)
        defaults.set(vZoom, forKey: Keys.vZoom)

        defaults.set(enhancer, forKey: Keys.enhancer)
        defaults.set(upscaler, forKey: Keys.upscaler)
        
        defaults.set(bloom, forKey: Keys.bloom)
        defaults.set(bloomRadius, forKey: Keys.bloomRadius)
        defaults.set(bloomBrightness, forKey: Keys.bloomBrightness)
        defaults.set(bloomWeight, forKey: Keys.bloomWeight)
        defaults.set(flicker, forKey: Keys.flicker)
        defaults.set(flickerWeight, forKey: Keys.flickerWeight)
        defaults.set(dotMask, forKey: Keys.dotMask)
        defaults.set(dotMaskBrightness, forKey: Keys.dotMaskBrightness)
        defaults.set(scanlines, forKey: Keys.scanlines)
        defaults.set(scanlineBrightness, forKey: Keys.scanlineBrightness)
        defaults.set(scanlineWeight, forKey: Keys.scanlineWeight)
        defaults.set(disalignment, forKey: Keys.disalignment)
        defaults.set(disalignmentH, forKey: Keys.disalignmentH)
        defaults.set(disalignmentV, forKey: Keys.disalignmentV)
    }
}
