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
    
    var romURL: URL = Defaults.rom.rom
    var extURL: URL = Defaults.rom.ext
    
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
    var gameDevice1 = Defaults.A500.gameDevice1 {
        didSet {
            if oldValue != gameDevice1 {
                parent.connect(device: gameDevice1, port: 1)
                if gameDevice1 == gameDevice2 { gameDevice2 = InputDevice.none }
                parent.toolbar.validateVisibleItems()
            }
        }
    }
    var gameDevice2 = Defaults.A500.gameDevice2 {
        didSet {
            if oldValue != gameDevice2 {
                parent.connect(device: gameDevice2, port: 2)
                if gameDevice2 == gameDevice1 { gameDevice1 = InputDevice.none }
                parent.toolbar.validateVisibleItems()
            }
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
    
    var hCenter = Defaults.hCenter {
        didSet { renderer.updateTextureRect() }
    }
    var vCenter = Defaults.vCenter {
        didSet { renderer.updateTextureRect() }
    }
    var hZoom = Defaults.hZoom {
        didSet { renderer.updateTextureRect() }
    }
    var vZoom = Defaults.vZoom {
        didSet { renderer.updateTextureRect() }
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
    
    init(with controller: MyController) { parent = controller }

    func loadRomUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        amiga.suspend()
        
        if let url = defaults.url(forKey: Keys.rom) {
            romURL = url
            amiga.mem.loadRom(fromFile: romURL)
        }
        if let url = defaults.url(forKey: Keys.ext) {
            extURL = url
            amiga.mem.loadExt(fromFile: extURL)
        }
        extStart = defaults.integer(forKey: Keys.extStart)

        amiga.resume()
    }
    
    func saveRomUserDefaults() {
        
        track()
        
        let hwconfig = amiga.config()
        let defaults = UserDefaults.standard

        defaults.set(romURL, forKey: Keys.rom)
        defaults.set(extURL, forKey: Keys.ext)
        defaults.set(hwconfig.mem.extStart, forKey: Keys.extStart)
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
    
    func loadCompatibilityUserDefaults() {

         let defaults = UserDefaults.standard

         amiga.suspend()

         clxSprSpr = defaults.bool(forKey: Keys.clxSprSpr)
         clxSprPlf = defaults.bool(forKey: Keys.clxSprPlf)
         clxPlfPlf = defaults.bool(forKey: Keys.clxPlfPlf)
         samplingMethod = defaults.integer(forKey: Keys.samplingMethod)
         filterActivation = defaults.integer(forKey: Keys.filterActivation)
         filterType = defaults.integer(forKey: Keys.filterType)
         blitterAccuracy = defaults.integer(forKey: Keys.blitterAccuracy)
         driveSpeed = defaults.integer(forKey: Keys.driveSpeed)
         fifoBuffering = defaults.bool(forKey: Keys.fifoBuffering)
         todBug = defaults.bool(forKey: Keys.todBug)

         amiga.resume()
     }

     func saveCompatibilityUserDefaults() {

         track()
         
         let defaults = UserDefaults.standard

         defaults.set(clxSprSpr, forKey: Keys.clxSprSpr)
         defaults.set(clxSprPlf, forKey: Keys.clxSprPlf)
         defaults.set(clxPlfPlf, forKey: Keys.clxPlfPlf)
         defaults.set(samplingMethod, forKey: Keys.samplingMethod)
         defaults.set(filterActivation, forKey: Keys.filterActivation)
         defaults.set(filterType, forKey: Keys.filterType)
         defaults.set(blitterAccuracy, forKey: Keys.blitterAccuracy)
         defaults.set(fifoBuffering, forKey: Keys.fifoBuffering)
         defaults.set(todBug, forKey: Keys.todBug)
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

        defaults.decode(&renderer.shaderOptions, forKey: Keys.shaderOptions)

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

        defaults.encode(renderer.shaderOptions, forKey: Keys.shaderOptions)
    }
}
