// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import AppKit

class ConfigurationController: DialogController {

    var config: Configuration { return parent.config }
    
    @IBOutlet weak var prefTabView: NSTabView!
        
    //
    // Roms
    //
        
    // Rom info
    @IBOutlet weak var romDropView: RomDropView!
    @IBOutlet weak var romTitle: NSTextField!
    @IBOutlet weak var romSubtitle: NSTextField!
    @IBOutlet weak var romSubsubtitle: NSTextField!
    @IBOutlet weak var romModel: NSTextField!
    @IBOutlet weak var romDeleteButton: NSButton!

    @IBOutlet weak var extDropView: ExtRomDropView!
    @IBOutlet weak var extTitle: NSTextField!
    @IBOutlet weak var extSubtitle: NSTextField!
    @IBOutlet weak var extSubsubtitle: NSTextField!
    @IBOutlet weak var extModel: NSTextField!
    @IBOutlet weak var extDeleteButton: NSButton!
    @IBOutlet weak var extMapText: NSTextField!
    @IBOutlet weak var extMapAddr: NSPopUpButton!

    // Explanation
    @IBOutlet weak var romExpImage: NSButton!
    @IBOutlet weak var romExpInfo1: NSTextField!
    @IBOutlet weak var romExpInfo2: NSTextField!

    // Lock
    @IBOutlet weak var romLockImage: NSButton!
    @IBOutlet weak var romLockInfo1: NSTextField!
    @IBOutlet weak var romLockInfo2: NSTextField!
   
    // Buttons
    @IBOutlet weak var romArosPopup: NSPopUpButton!
    @IBOutlet weak var romOKButton: NSButton!
    @IBOutlet weak var romPowerButton: NSButton!

    //
    // Hardware
    //
    
    // Lock
    @IBOutlet weak var hwLockImage: NSButton!
    @IBOutlet weak var hwLockInfo1: NSTextField!
    @IBOutlet weak var hwLockInfo2: NSTextField!

    // Buttons
    @IBOutlet weak var hwFactorySettingsPopup: NSPopUpButton!
    @IBOutlet weak var hwOKButton: NSButton!
    @IBOutlet weak var hwPowerButton: NSButton!

    //
    // Hardware::Chipset
    //

    // CPU
    @IBOutlet weak var csCpuRevision: NSPopUpButton!
    @IBOutlet weak var csCpuSpeed: NSPopUpButton!
    @IBOutlet weak var csCpuInfo1: NSTextField!
    @IBOutlet weak var csCpuInfo2: NSTextField!

    // Agnus
    @IBOutlet weak var csAgnusRevision: NSPopUpButton!
    @IBOutlet weak var csMachineType: NSPopUpButton!
    @IBOutlet weak var csAgnusInfo1: NSTextField!
    @IBOutlet weak var csAgnusInfo2: NSTextField!

    // Denise
    @IBOutlet weak var csDeniseRevision: NSPopUpButton!
    @IBOutlet weak var csDeniseInfo1: NSTextField!
    @IBOutlet weak var csDeniseInfo2: NSTextField!

    // CIAs
    @IBOutlet weak var csCiaRevision: NSPopUpButton!
    @IBOutlet weak var csCiaInfo1: NSTextField!
    @IBOutlet weak var csCiaInfo2: NSTextField!

    // RT clock
    @IBOutlet weak var csRtcRevision: NSPopUpButton!
    @IBOutlet weak var csRtcInfo1: NSTextField!
    @IBOutlet weak var csRtcInfo2: NSTextField!
    @IBOutlet weak var csRtcIcon: NSButton!

    //
    // Hardware::Memory
    //
    
    // RAM
    @IBOutlet weak var memChipRamPopup: NSPopUpButton!
    @IBOutlet weak var memSlowRamPopup: NSPopUpButton!
    @IBOutlet weak var memFastRamPopup: NSPopUpButton!

    // Memory properties
    @IBOutlet weak var memRamInitPattern: NSPopUpButton!
    @IBOutlet weak var memBankMap: NSPopUpButton!
    @IBOutlet weak var memUnmappingType: NSPopUpButton!

    // Warning
    @IBOutlet weak var memWarnImage: NSButton!
    @IBOutlet weak var memWarnInfo1: NSTextField!
    @IBOutlet weak var memWarnInfo2: NSTextField!

    //
    // Peripherals
    //

    // Flopp drives
    @IBOutlet weak var perDf0Type: NSPopUpButton!
    @IBOutlet weak var perDf1Connect: NSButton!
    @IBOutlet weak var perDf1Type: NSPopUpButton!
    @IBOutlet weak var perDf2Connect: NSButton!
    @IBOutlet weak var perDf2Type: NSPopUpButton!
    @IBOutlet weak var perDf3Connect: NSButton!
    @IBOutlet weak var perDf3Type: NSPopUpButton!

    // Hard drives
    @IBOutlet weak var perHd0Connect: NSButton!
    @IBOutlet weak var perHd0Type: NSPopUpButton!
    @IBOutlet weak var perHd1Connect: NSButton!
    @IBOutlet weak var perHd1Type: NSPopUpButton!
    @IBOutlet weak var perHd2Connect: NSButton!
    @IBOutlet weak var perHd2Type: NSPopUpButton!
    @IBOutlet weak var perHd3Connect: NSButton!
    @IBOutlet weak var perHd3Type: NSPopUpButton!

    // Ports
    @IBOutlet weak var perGameDevice1: NSPopUpButton!
    @IBOutlet weak var perGameDevice2: NSPopUpButton!
    @IBOutlet weak var perSerialDevice: NSPopUpButton!
    @IBOutlet weak var perSerialPort: NSTextField!
    @IBOutlet weak var perSerialPortText: NSTextField!

    // Lock
    @IBOutlet weak var perLockImage: NSButton!
    @IBOutlet weak var perLockInfo1: NSTextField!
    @IBOutlet weak var perLockInfo2: NSTextField!
    
    // Buttons
    @IBOutlet weak var perFactorySettingsPopup: NSPopUpButton!
    @IBOutlet weak var perOKButton: NSButton!
    @IBOutlet weak var perPowerButton: NSButton!

    //
    // Performance
    //

    // Warp
    @IBOutlet weak var prfWarpMode: NSPopUpButton!
    @IBOutlet weak var prfWarpBoot: NSTextField!

    // Threading
    @IBOutlet weak var prfVSync: NSButton!
    @IBOutlet weak var prfTimeLapse: NSSlider!
    @IBOutlet weak var prfTimeLapseInfo: NSTextField!

    // Collision detection
    @IBOutlet weak var prfClxSprSpr: NSButton!
    @IBOutlet weak var prfClxSprPlf: NSButton!
    @IBOutlet weak var prfClxPlfPlf: NSButton!

    // Boosters
    @IBOutlet weak var prfCiaIdleSleep: NSButton!
    @IBOutlet weak var prfFrameSkipping: NSButton!
    @IBOutlet weak var prfAudioFastPath: NSButton!

    // Lock
    @IBOutlet weak var prfLockImage: NSButton!
    @IBOutlet weak var prfLockInfo1: NSTextField!
    @IBOutlet weak var prfLockInfo2: NSTextField!

    // Buttons
    @IBOutlet weak var prfFactorySettingsPopup: NSPopUpButton!
    @IBOutlet weak var prfOKButton: NSButton!
    @IBOutlet weak var prfPowerButton: NSButton!

    //
    // Compatibility
    //

    // Blitter
    @IBOutlet weak var compBltAccuracy: NSSlider!
    @IBOutlet weak var compBltLevel2: NSTextField!
    @IBOutlet weak var compBltLevel1: NSTextField!

    // Chipset features
    @IBOutlet weak var compSlowRamMirror: NSButton!
    @IBOutlet weak var compSlowRamDelay: NSButton!
    @IBOutlet weak var compTodBug: NSButton!
    @IBOutlet weak var compPtrDrops: NSButton!

    // Timing
    @IBOutlet weak var compEClockSyncing: NSButton!
    
    // Disk controller
    @IBOutlet weak var compDriveSpeed: NSPopUpButton!
    @IBOutlet weak var compMechanics: NSButton!
    @IBOutlet weak var compLockDskSync: NSButton!
    @IBOutlet weak var compAutoDskSync: NSButton!

    // Keyboard
    @IBOutlet weak var compAccurateKeyboard: NSButton!

    // Buttons
    @IBOutlet weak var compOKButton: NSButton!
    @IBOutlet weak var compPowerButton: NSButton!
    
    //
    // Audio
    //
    
    // In
    @IBOutlet weak var audVol0: NSSlider!
    @IBOutlet weak var audVol1: NSSlider!
    @IBOutlet weak var audVol2: NSSlider!
    @IBOutlet weak var audVol3: NSSlider!
    @IBOutlet weak var audPan0: NSSlider!
    @IBOutlet weak var audPan1: NSSlider!
    @IBOutlet weak var audPan2: NSSlider!
    @IBOutlet weak var audPan3: NSSlider!
    
    // Out
    @IBOutlet weak var audVolL: NSSlider!
    @IBOutlet weak var audVolR: NSSlider!
    @IBOutlet weak var audSamplingMethod: NSPopUpButton!

    // Drive volumes
    @IBOutlet weak var audStepVolume: NSSlider!
    @IBOutlet weak var audPollVolume: NSSlider!
    @IBOutlet weak var audEjectVolume: NSSlider!
    @IBOutlet weak var audInsertVolume: NSSlider!
    @IBOutlet weak var audDf0Pan: NSSlider!
    @IBOutlet weak var audDf1Pan: NSSlider!
    @IBOutlet weak var audDf2Pan: NSSlider!
    @IBOutlet weak var audDf3Pan: NSSlider!
    @IBOutlet weak var audHd0Pan: NSSlider!
    @IBOutlet weak var audHd1Pan: NSSlider!
    @IBOutlet weak var audHd2Pan: NSSlider!
    @IBOutlet weak var audHd3Pan: NSSlider!

    // Filter
    @IBOutlet weak var audFilterType: NSPopUpButton!

    // Buttons
    @IBOutlet weak var audOKButton: NSButton!
    @IBOutlet weak var audPowerButton: NSButton!

    //
    // Video
    //
    
    // Palette
    @IBOutlet weak var vidPalettePopUp: NSPopUpButton!
    @IBOutlet weak var vidBrightnessSlider: NSSlider!
    @IBOutlet weak var vidContrastSlider: NSSlider!
    @IBOutlet weak var vidSaturationSlider: NSSlider!

    // Geometry
    @IBOutlet weak var vidZoom: NSPopUpButton!
    @IBOutlet weak var vidHZoom: NSSlider!
    @IBOutlet weak var vidVZoom: NSSlider!
    @IBOutlet weak var vidHZoomLabel: NSTextField!
    @IBOutlet weak var vidVZoomLabel: NSTextField!
    @IBOutlet weak var vidCenter: NSPopUpButton!
    @IBOutlet weak var vidHCenter: NSSlider!
    @IBOutlet weak var vidVCenter: NSSlider!
    @IBOutlet weak var vidHCenterLabel: NSTextField!
    @IBOutlet weak var vidVCenterLabel: NSTextField!

    // Frame rate
    @IBOutlet weak var vidSyncMode: NSPopUpButton!
    @IBOutlet weak var vidFpsSlider: NSSlider!
    @IBOutlet weak var vidFpsMin: NSTextField!
    @IBOutlet weak var vidFpsMax: NSTextField!

    // Effects
    @IBOutlet weak var vidEnhancerPopUp: NSPopUpButton!
    @IBOutlet weak var vidUpscalerPopUp: NSPopUpButton!
    @IBOutlet weak var vidBlurPopUp: NSPopUpButton!
    @IBOutlet weak var vidBlurRadiusSlider: NSSlider!
    
    @IBOutlet weak var vidBloomPopUp: NSPopUpButton!
    @IBOutlet weak var vidBloomRadiusSlider: NSSlider!
    @IBOutlet weak var vidBloomBrightnessSlider: NSSlider!
    @IBOutlet weak var vidBloomWeightSlider: NSSlider!

    @IBOutlet weak var vidFlickerPopUp: NSPopUpButton!
    @IBOutlet weak var vidFlickerWeightSlider: NSSlider!

    @IBOutlet weak var vidDotMaskPopUp: NSPopUpButton!
    @IBOutlet weak var vidDotMaskBrightnessSlider: NSSlider!
    
    @IBOutlet weak var vidScanlinesPopUp: NSPopUpButton!
    @IBOutlet weak var vidScanlineBrightnessSlider: NSSlider!
    @IBOutlet weak var vidScanlineWeightSlider: NSSlider!
    
    @IBOutlet weak var vidMisalignmentPopUp: NSPopUpButton!
    @IBOutlet weak var vidMisalignmentXSlider: NSSlider!
    @IBOutlet weak var vidMisalignmentYSlider: NSSlider!

    // Buttons
    @IBOutlet weak var vidOKButton: NSButton!
    @IBOutlet weak var vidPowerButton: NSButton!

    var bootable: Bool {
        
        do { try amiga.isReady() } catch { return false }
        return amiga.poweredOff
    }
    
    // The tab to open first
    var firstTab = ""
    
    func showSheet(tab: String) {

        firstTab = tab
        showAsSheet()
    }

    override func awakeFromNib() {

        super.awakeFromNib()
        awakeVideoPrefsFromNib()
        refreshRomSelector()
    }

    override func dialogWillShow() {

        super.dialogWillShow()
        if firstTab != "" { prefTabView?.selectTabViewItem(withIdentifier: firstTab) }
        refresh()
    }

    func refresh() {
        
        if let id = prefTabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Roms": refreshRomTab()
            case "Hardware": refreshHardwareTab()
            case "Peripherals": refreshPeripheralsTab()
            case "Performance": refreshPerformanceTab()
            case "Compatibility": refreshCompatibilityTab()
            case "Audio": refreshAudioTab()
            case "Video": refreshVideoTab()
            default: fatalError()
            }
        }
    }
    
    @IBAction func unlockAction(_ sender: Any!) {

        amiga.pause()
        amiga.powerOff()
        refresh()
    }
    
    @IBAction override func okAction(_ sender: Any!) {
        
        hide()
        // close()
    }

    @IBAction func powerAction(_ sender: Any!) {
        
        hide()
        try? amiga.run()
    }
}

extension ConfigurationController: NSTabViewDelegate {

    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {

        refresh()
    }
}
