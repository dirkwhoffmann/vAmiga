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

@MainActor
class Configuration {

    var parent: MyController!
    var emu: EmulatorProxy? { return parent.emu }
    var renderer: Renderer { return parent.renderer }
    var gamePadManager: GamePadManager { return parent.gamePadManager }
    var ressourceManager: RessourceManager { return renderer.ressourceManager }

    //
    // Roms
    //

    var extStart: Int {
        get { return emu?.get(.MEM_EXT_START) ?? 0 }
        set { emu?.set(.MEM_EXT_START, value: newValue) }
    }

    //
    // Hardware
    //

    var machineType: Int {
        get { return emu?.get(.AMIGA_VIDEO_FORMAT) ?? 0 }
        set { emu?.set(.AMIGA_VIDEO_FORMAT, value: newValue) }
    }
    var cpuRev: Int {
        get { return emu?.get(.CPU_REVISION) ?? 0 }
        set { emu?.set(.CPU_REVISION, value: newValue) }
    }
    var cpuDasmRev: Int {
        get { return emu?.get(.CPU_DASM_REVISION) ?? 0 }
        set { emu?.set(.CPU_DASM_REVISION, value: newValue) }
    }
    var cpuSpeed: Int {
        get { return emu?.get(.CPU_OVERCLOCKING) ?? 0 }
        set { emu?.set(.CPU_OVERCLOCKING, value: newValue) }
    }
    var agnusRev: Int {
        get { return emu?.get(.AGNUS_REVISION) ?? 0 }
        set { emu?.set(.AGNUS_REVISION, value: newValue) }
    }
    var deniseRev: Int {
        get { return emu?.get(.DENISE_REVISION) ?? 0 }
        set { emu?.set(.DENISE_REVISION, value: newValue) }
    }
    var ciaRev: Int {
        get { return emu?.get(.CIA_REVISION) ?? 0 }
        set { emu?.set(.CIA_REVISION, value: newValue) }
    }
    var rtClock: Int {
        get { return emu?.get(.RTC_MODEL) ?? 0 }
        set { emu?.set(.RTC_MODEL, value: newValue) }
    }
    var filterType: Int {
        get { return emu?.get(.AUD_FILTER_TYPE) ?? 0 }
        set { emu?.set(.AUD_FILTER_TYPE, value: newValue) }
    }
    var chipRam: Int {
        get { return emu?.get(.MEM_CHIP_RAM) ?? 0 }
        set { emu?.set(.MEM_CHIP_RAM, value: newValue) }
    }
    var slowRam: Int {
        get { return emu?.get(.MEM_SLOW_RAM) ?? 0 }
        set { emu?.set(.MEM_SLOW_RAM, value: newValue) }
    }
    var fastRam: Int {
        get { return emu?.get(.MEM_FAST_RAM) ?? 0 }
        set { emu?.set(.MEM_FAST_RAM, value: newValue) }
    }
    var bankMap: Int {
        get { return emu?.get(.MEM_BANKMAP) ?? 0 }
        set { emu?.set(.MEM_BANKMAP, value: newValue) }
    }
    var ramInitPattern: Int {
        get { return emu?.get(.MEM_RAM_INIT_PATTERN) ?? 0 }
        set { emu?.set(.MEM_RAM_INIT_PATTERN, value: newValue) }
    }
    var unmappingType: Int {
        get { return emu?.get(.MEM_UNMAPPING_TYPE) ?? 0 }
        set { emu?.set(.MEM_UNMAPPING_TYPE, value: newValue) }
    }

    //
    // Peripherals
    //

    func dfnConnected(_ n: Int) -> Bool {
        precondition(0 <= n && n <= 3)
        return emu?.get(.DRIVE_CONNECT, drive: n) != 0
    }

    func setDfnConnected(_ n: Int, connect: Bool) {
        precondition(0 <= n && n <= 3)
        emu?.set(.DRIVE_CONNECT, drive: n, enable: connect)
    }

    func dfnType(_ n: Int) -> Int {
        precondition(0 <= n && n <= 3)
        return emu?.get(.DRIVE_TYPE, drive: n) ?? 0
    }

    func setDfnType(_ n: Int, type: Int) {
        precondition(0 <= n && n <= 3)
        emu?.set(.DRIVE_TYPE, drive: n, value: type)
    }

    func dfnRpm(_ n: Int) -> Int {
        precondition(0 <= n && n <= 3)
        return emu?.get(.DRIVE_RPM, drive: n) ?? 0
    }

    func setDfnRpm(_ n: Int, type: Int) {
        precondition(0 <= n && n <= 3)
        emu?.set(.DRIVE_RPM, drive: n, value: type)
    }

    func hdnConnected(_ n: Int) -> Bool {
        precondition(0 <= n && n <= 3)
        return emu?.get(.HDC_CONNECT, drive: n) != 0
    }

    func setHdnConnected(_ n: Int, connect: Bool) {
        precondition(0 <= n && n <= 3)
        emu?.set(.HDC_CONNECT, drive: n, enable: connect)
    }

    func hdnType(_ n: Int) -> Int {
        precondition(0 <= n && n <= 3)
        return emu?.get(.HDR_TYPE, drive: n) ?? 0
    }

    func setHdnType(_ n: Int, type: Int) {
        precondition(0 <= n && n <= 3)
        emu?.set(.HDR_TYPE, drive: n, value: type)
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

    var gameDevice1 = -1 {
        didSet {

            // Try to connect the device (may disconnect the other device)
            gamePadManager.connect(slot: gameDevice1, port: 0)

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
            gamePadManager.connect(slot: gameDevice2, port: 1)

            // Avoid double mappings
            if gameDevice2 != -1 && gameDevice2 == gameDevice1 {
                gameDevice1 = -1
            }

            parent.toolbar.validateVisibleItems()
        }
    }

    var autofire: Bool {
        get { return emu?.get(.JOY_AUTOFIRE, id: 1) != 0 }
        set { emu?.set(.JOY_AUTOFIRE, enable: newValue) }
    }

    var autofireBursts: Bool {
        get { return emu?.get(.JOY_AUTOFIRE_BURSTS, id: 1) != 0 }
        set { emu?.set(.JOY_AUTOFIRE_BURSTS, enable: newValue) }
    }

    var autofireBullets: Int {
        get { return emu?.get(.JOY_AUTOFIRE_BULLETS, id: 1) ?? 0 }
        set { emu?.set(.JOY_AUTOFIRE_BULLETS, value: newValue) }
    }

    var autofireDelay: Int {
        get { return emu?.get(.JOY_AUTOFIRE_DELAY, id: 1) ?? 0 }
        set { emu?.set(.JOY_AUTOFIRE_DELAY, value: newValue) }
    }

    var serialDevice: Int {
        get { return emu?.get(.SER_DEVICE) ?? 0 }
        set { emu?.set(.SER_DEVICE, value: newValue) }
    }

    var serialDevicePort: Int {
        get { return emu?.get(.SRV_PORT, id: ServerType.SER.rawValue) ?? 0 }
        set { emu?.set(.SRV_PORT, id: ServerType.SER.rawValue, value: newValue) }
    }

    //
    // Performance
    //

    var warpMode: Int {
        get { return emu?.get(.AMIGA_WARP_MODE) ?? 0 }
        set { emu?.set(.AMIGA_WARP_MODE, value: newValue) }
    }
    var warpBoot: Int {
        get { return emu?.get(.AMIGA_WARP_BOOT) ?? 0 }
        set { emu?.set(.AMIGA_WARP_BOOT, value: newValue) }
    }
    var clxSprSpr: Bool {
        get { return emu?.get(.DENISE_CLX_SPR_SPR) != 0 }
        set { emu?.set(.DENISE_CLX_SPR_SPR, enable: newValue) }
    }
    var clxSprPlf: Bool {
        get { return emu?.get(.DENISE_CLX_SPR_PLF) != 0 }
        set { emu?.set(.DENISE_CLX_SPR_PLF, enable: newValue) }
    }
    var clxPlfPlf: Bool {
        get { return emu?.get(.DENISE_CLX_PLF_PLF) != 0 }
        set { emu?.set(.DENISE_CLX_PLF_PLF, enable: newValue) }
    }
    var ciaIdleSleep: Bool {
        get { return emu?.get(.CIA_IDLE_SLEEP) != 0 }
        set { emu?.set(.CIA_IDLE_SLEEP, enable: newValue) }
    }
    var frameSkipping: Int {
        get { return emu?.get(.DENISE_FRAME_SKIPPING) ?? 0 }
        set { emu?.set(.DENISE_FRAME_SKIPPING, value: newValue) }
    }
    var audioFastPath: Bool {
        get { return emu?.get(.AUD_FASTPATH) != 0 }
        set { emu?.set(.AUD_FASTPATH, enable: newValue) }
    }
    var vsync: Bool {
        get { return emu?.get(.AMIGA_VSYNC) != 0 }
        set { emu?.set(.AMIGA_VSYNC, enable: newValue) }
    }
    var speedBoost: Int {
        get { return emu?.get(.AMIGA_SPEED_BOOST) ?? 0 }
        set { emu?.set(.AMIGA_SPEED_BOOST, value: newValue) }
    }
    var runAhead: Int {
        get { return emu?.get(.AMIGA_RUN_AHEAD) ?? 0 }
        set { emu?.set(.AMIGA_RUN_AHEAD, value: newValue) }
    }
    var wsCompressor: Int {
        get { return emu?.get(.AMIGA_WS_COMPRESSION) ?? 0 }
        set { emu?.set(.AMIGA_WS_COMPRESSION, value: newValue) }
    }

    //
    // Compatibility
    //

    var blitterAccuracy: Int {
        get { return emu?.get(.BLITTER_ACCURACY) ?? 0 }
        set { emu?.set(.BLITTER_ACCURACY, value: newValue) }
    }
    var todBug: Bool {
        get { return emu?.get(.CIA_TODBUG) != 0 }
        set { emu?.set(.CIA_TODBUG, enable: newValue) }
    }
    var ptrDrops: Bool {
        get { return emu?.get(.AGNUS_PTR_DROPS) != 0 }
        set { emu?.set(.AGNUS_PTR_DROPS, enable: newValue) }
    }
    var eClockSyncing: Bool {
        get { return emu?.get(.CIA_ECLOCK_SYNCING) != 0 }
        set { emu?.set(.CIA_ECLOCK_SYNCING, enable: newValue) }
    }
    var slowRamDelay: Bool {
        get { return emu?.get(.MEM_SLOW_RAM_DELAY) != 0 }
        set { emu?.set(.MEM_SLOW_RAM_DELAY, enable: newValue) }
    }
    var slowRamMirror: Bool {
        get { return emu?.get(.MEM_SLOW_RAM_MIRROR) != 0 }
        set { emu?.set(.MEM_SLOW_RAM_MIRROR, enable: newValue) }
    }
    var driveSpeed: Int {
        get { return emu?.get(.DC_SPEED) ?? 0 }
        set { emu?.set(.DC_SPEED, value: newValue) }
    }
    var driveMechanics: Int {
        get { return emu?.get(.DRIVE_MECHANICS, drive: 0) ?? 0 }
        set {
            emu?.set(.DRIVE_MECHANICS, drive: 0, value: newValue)
            emu?.set(.DRIVE_MECHANICS, drive: 1, value: newValue)
            emu?.set(.DRIVE_MECHANICS, drive: 2, value: newValue)
            emu?.set(.DRIVE_MECHANICS, drive: 3, value: newValue)
        }
    }
    var lockDskSync: Bool {
        get { return emu?.get(.DC_LOCK_DSKSYNC) != 0 }
        set { emu?.set(.DC_LOCK_DSKSYNC, enable: newValue) }
    }
    var autoDskSync: Bool {
        get { return emu?.get(.DC_AUTO_DSKSYNC) != 0 }
        set { emu?.set(.DC_AUTO_DSKSYNC, enable: newValue) }
    }
    var accurateKeyboard: Bool {
        get { return emu?.get(.KBD_ACCURACY) != 0 }
        set { emu?.set(.KBD_ACCURACY, enable: newValue) }
    }

    //
    // Audio settings
    //

    var vol0: Int {
        get { return emu?.get(.AUD_VOL0)  ?? 0}
        set { emu?.set(.AUD_VOL0, value: newValue) }
    }
    var vol1: Int {
        get { return emu?.get(.AUD_VOL1) ?? 0 }
        set { emu?.set(.AUD_VOL1, value: newValue) }
    }
    var vol2: Int {
        get { return emu?.get(.AUD_VOL2) ?? 0 }
        set { emu?.set(.AUD_VOL2, value: newValue) }
    }
    var vol3: Int {
        get { return emu?.get(.AUD_VOL3) ?? 0 }
        set { emu?.set(.AUD_VOL3, value: newValue) }
    }
    var pan0: Int {
        get { return emu?.get(.AUD_PAN0) ?? 0 }
        set { emu?.set(.AUD_PAN0, value: newValue) }
    }
    var pan1: Int {
        get { return emu?.get(.AUD_PAN1) ?? 0 }
        set { emu?.set(.AUD_PAN1, value: newValue) }
    }
    var pan2: Int {
        get { return emu?.get(.AUD_PAN2) ?? 0 }
        set { emu?.set(.AUD_PAN2, value: newValue) }
    }
    var pan3: Int {
        get { return emu?.get(.AUD_PAN3) ?? 0 }
        set { emu?.set(.AUD_PAN3, value: newValue) }
    }
    var volL: Int {
        get { return emu?.get(.AUD_VOLL) ?? 0 }
        set { emu?.set(.AUD_VOLL, value: newValue) }
    }
    var volR: Int {
        get { return emu?.get(.AUD_VOLR) ?? 0 }
        set { emu?.set(.AUD_VOLR, value: newValue) }
    }
    var df0Pan: Int {
        get { return emu?.get(.DRIVE_PAN, drive: 0) ?? 0 }
        set { emu?.set(.DRIVE_PAN, id: 0, value: newValue) }
    }
    var df1Pan: Int {
        get { return emu?.get(.DRIVE_PAN, drive: 1) ?? 0 }
        set { emu?.set(.DRIVE_PAN, id: 1, value: newValue) }
    }
    var df2Pan: Int {
        get { return emu?.get(.DRIVE_PAN, drive: 2) ?? 0 }
        set { emu?.set(.DRIVE_PAN, id: 2, value: newValue) }
    }
    var df3Pan: Int {
        get { return emu?.get(.DRIVE_PAN, drive: 3) ?? 0 }
        set { emu?.set(.DRIVE_PAN, id: 3, value: newValue) }
    }
    var hd0Pan: Int {
        get { return emu?.get(.HDR_PAN, drive: 0) ?? 0 }
        set { emu?.set(.HDR_PAN, id: 0, value: newValue) }
    }
    var hd1Pan: Int {
        get { return emu?.get(.HDR_PAN, drive: 1) ?? 0 }
        set { emu?.set(.HDR_PAN, id: 1, value: newValue) }
    }
    var hd2Pan: Int {
        get { return emu?.get(.HDR_PAN, drive: 2) ?? 0 }
        set { emu?.set(.HDR_PAN, id: 2, value: newValue) }
    }
    var hd3Pan: Int {
        get { return emu?.get(.HDR_PAN, drive: 3) ?? 0 }
        set { emu?.set(.HDR_PAN, id: 3, value: newValue) }
    }
    var stepVolume: Int {
        get { return emu?.get(.DRIVE_STEP_VOLUME, drive: 0)  ?? 0}
        set { emu?.set(.DRIVE_STEP_VOLUME, value: newValue)
            emu?.set(.HDR_STEP_VOLUME, value: newValue)
        }
    }
    var pollVolume: Int {
        get { return emu?.get(.DRIVE_POLL_VOLUME, drive: 0) ?? 0 }
        set { emu?.set(.DRIVE_POLL_VOLUME, value: newValue) }
    }
    var insertVolume: Int {
        get { return emu?.get(.DRIVE_INSERT_VOLUME, drive: 0) ?? 0 }
        set { emu?.set(.DRIVE_INSERT_VOLUME, value: newValue) }
    }
    var ejectVolume: Int {
        get { return emu?.get(.DRIVE_EJECT_VOLUME, drive: 0) ?? 0 }
        set { emu?.set(.DRIVE_EJECT_VOLUME, value: newValue) }
    }
    var samplingMethod: Int {
        get { return emu?.get(.AUD_SAMPLING_METHOD) ?? 0 }
        set { emu?.set(.AUD_SAMPLING_METHOD, value: newValue) }
    }
    var asr: Int {
        get { return emu?.get(.AUD_ASR) ?? 0 }
        set { emu?.set(.AUD_ASR, value: newValue) }
    }
    var audioBufferSize: Int {
        get { return emu?.get(.AUD_BUFFER_SIZE) ?? 0 }
        set { emu?.set(.AUD_BUFFER_SIZE, value: newValue) }
    }

    //
    // Video settings
    //

    var palette: Int {
        get { return emu?.get(.MON_PALETTE) ?? 0 }
        set { emu?.set(.MON_PALETTE, value: newValue) }
    }
    var brightness: Int {
        get { return emu?.get(.MON_BRIGHTNESS) ?? 0 }
        set { emu?.set(.MON_BRIGHTNESS, value: newValue) }
    }
    var contrast: Int {
        get { return emu?.get(.MON_CONTRAST) ?? 0 }
        set { emu?.set(.MON_CONTRAST, value: newValue) }
    }
    var saturation: Int {
        get { return emu?.get(.MON_SATURATION) ?? 0 }
        set { emu?.set(.MON_SATURATION, value: newValue) }
    }
    var zoom: Int {
        get { return emu?.get(.MON_ZOOM) ?? 0 }
        set { emu?.set(.MON_ZOOM, value: newValue) }
    }
    var hZoom: Int {
        get { return emu?.get(.MON_HZOOM) ?? 0 }
        set { emu?.set(.MON_HZOOM, value: newValue) }
    }
    var vZoom: Int {
        get { return emu?.get(.MON_VZOOM) ?? 0 }
        set { emu?.set(.MON_VZOOM, value: newValue) }
    }
    var center: Int {
        get { return emu?.get(.MON_CENTER) ?? 0 }
        set { emu?.set(.MON_CENTER, value: newValue) }
    }
    var hCenter: Int {
        get { return emu?.get(.MON_HCENTER) ?? 0 }
        set { emu?.set(.MON_HCENTER, value: newValue) }
    }
    var vCenter: Int {
        get { return emu?.get(.MON_VCENTER) ?? 0 }
        set { emu?.set(.MON_VCENTER, value: newValue) }
    }
    var enhancer: Int {
        get { return emu?.get(.MON_ENHANCER) ?? 0 }
        set { emu?.set(.MON_ENHANCER, value: newValue) }
    }
    var upscaler: Int {
        get { return emu?.get(.MON_UPSCALER) ?? 0 }
        set { emu?.set(.MON_UPSCALER, value: newValue) }
    }
    var blur: Int {
        get { return emu?.get(.MON_BLUR) ?? 0 }
        set { emu?.set(.MON_BLUR, value: newValue) }
    }
    var blurRadius: Int {
        get { return emu?.get(.MON_BLUR_RADIUS) ?? 0 }
        set { emu?.set(.MON_BLUR_RADIUS, value: newValue) }
    }
    var bloom: Int {
        get { return emu?.get(.MON_BLOOM) ?? 0 }
        set { emu?.set(.MON_BLOOM, value: newValue) }
    }
    var bloomRadius: Int {
        get { return emu?.get(.MON_BLOOM_RADIUS) ?? 0 }
        set { emu?.set(.MON_BLOOM_RADIUS, value: newValue) }
    }
    var bloomBrightness: Int {
        get { return emu?.get(.MON_BLOOM_BRIGHTNESS) ?? 0 }
        set { emu?.set(.MON_BLOOM_BRIGHTNESS, value: newValue) }
    }
    var bloomWeight: Int {
        get { return emu?.get(.MON_BLOOM_WEIGHT) ?? 0 }
        set { emu?.set(.MON_BLOOM_WEIGHT, value: newValue) }
    }
    var flicker: Int {
        get { return emu?.get(.MON_FLICKER) ?? 0 }
        set { emu?.set(.MON_FLICKER, value: newValue) }
    }
    var flickerWeight: Int {
        get { return emu?.get(.MON_FLICKER_WEIGHT) ?? 0 }
        set { emu?.set(.MON_FLICKER_WEIGHT, value: newValue) }
    }
    var dotMask: Int {
        get { return emu?.get(.MON_DOTMASK) ?? 0 }
        set { emu?.set(.MON_DOTMASK, value: newValue) }
    }
    var dotMaskBrightness: Int {
        get { return emu?.get(.MON_DOTMASK_BRIGHTNESS) ?? 0 }
        set { emu?.set(.MON_DOTMASK_BRIGHTNESS, value: newValue) }
    }
    var scanlines: Int {
        get { return emu?.get(.MON_SCANLINES) ?? 0 }
        set { emu?.set(.MON_SCANLINES, value: newValue) }
    }
    var scanlineBrightness: Int {
        get { return emu?.get(.MON_SCANLINE_BRIGHTNESS) ?? 0 }
        set { emu?.set(.MON_SCANLINE_BRIGHTNESS, value: newValue) }
    }
    var scanlineWeight: Int {
        get { return emu?.get(.MON_SCANLINE_WEIGHT) ?? 0 }
        set { emu?.set(.MON_SCANLINE_WEIGHT, value: newValue) }
    }
    var disalignment: Int {
        get { return emu?.get(.MON_DISALIGNMENT) ?? 0 }
        set { emu?.set(.MON_DISALIGNMENT, value: newValue) }
    }
    var disalignmentH: Int {
        get { return emu?.get(.MON_DISALIGNMENT_H) ?? 0 }
        set { emu?.set(.MON_DISALIGNMENT_H, value: newValue) }
    }
    var disalignmentV: Int {
        get { return emu?.get(.MON_DISALIGNMENT_V) ?? 0 }
        set { emu?.set(.MON_DISALIGNMENT_V, value: newValue) }
    }

    //
    // Servers
    //

    var rshServerEnable: Bool {
        get { return emu?.get(.SRV_ENABLE, id: 0) != 0 }
        set { emu?.set(.SRV_ENABLE, id: 0, enable: newValue ) }
    }
    var rshServerPort: Int {
        get { return emu?.get(.SRV_PORT, id: 0) ?? 0 }
        set { emu?.set(.SRV_PORT, id: 0, value: newValue ) }
    }
    var rpcServerEnable: Bool {
        get { return emu?.get(.SRV_ENABLE, id: 1) != 0 }
        set { emu?.set(.SRV_ENABLE, id: 1, enable: newValue ) }
    }
    var rpcServerPort: Int {
        get { return emu?.get(.SRV_PORT, id: 1) ?? 0 }
        set { emu?.set(.SRV_PORT, id: 1, value: newValue ) }
    }
    var gdbServerEnable: Bool {
        get { return emu?.get(.SRV_ENABLE, id: 2) != 0 }
        set { emu?.set(.SRV_ENABLE, id: 2, enable: newValue ) }
    }
    var gdbServerPort: Int {
        get { return emu?.get(.SRV_PORT, id: 2) ?? 0 }
        set { emu?.set(.SRV_PORT, id: 2, value: newValue ) }
    }
    var promServerEnable: Bool {
        get { return emu?.get(.SRV_ENABLE, id: 3) != 0 }
        set { emu?.set(.SRV_ENABLE, id: 3, enable: newValue ) }
    }
    var promServerPort: Int {
        get { return emu?.get(.SRV_PORT, id: 3) ?? 0 }
        set { emu?.set(.SRV_PORT, id: 3, value: newValue ) }
    }
    var serServerEnable: Bool {
        get { return emu?.get(.SRV_ENABLE, id: 4) != 0 }
        set { emu?.set(.SRV_ENABLE, id: 4, enable: newValue ) }
    }
    var serServerPort: Int {
        get { return emu?.get(.SRV_PORT, id: 4) ?? 0 }
        set { emu?.set(.SRV_PORT, id: 4, value: newValue ) }
    }

    init(with controller: MyController) { parent = controller }

    func revertTo(model: Int) {

        print("revertTo \(model)")
        
        switch model {
            
        case 0:

            // Amiga 500
            agnusRev = AgnusRevision.ECS_1MB.rawValue
            deniseRev = DeniseRevision.OCS.rawValue
            rtClock = RTCRevision.NONE.rawValue
            bankMap = BankMap.A500.rawValue
            hd0Connected = false

        case 1:

            // Amiga 1000
            agnusRev = AgnusRevision.OCS_OLD.rawValue
            deniseRev = DeniseRevision.OCS.rawValue
            rtClock = RTCRevision.NONE.rawValue
            bankMap = BankMap.A1000.rawValue
            hd0Connected = false
            
        case 2:

            // Amiga 2000
            agnusRev = AgnusRevision.ECS_2MB.rawValue
            deniseRev = DeniseRevision.OCS.rawValue
            rtClock = RTCRevision.OKI.rawValue
            bankMap = BankMap.A2000B.rawValue
            hd0Connected = true
            
        case 3:

            // Amiga 500+
            agnusRev = AgnusRevision.ECS_2MB.rawValue
            deniseRev = DeniseRevision.ECS.rawValue
            rtClock = RTCRevision.OKI.rawValue
            bankMap = BankMap.A500.rawValue
            hd0Connected = false

        default:
            fatalError()
        }
    }
}
