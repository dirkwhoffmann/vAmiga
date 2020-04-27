// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class ConfigController: DialogController {

    var config: Configuration { return parent.config }
    
    @IBOutlet weak var prefTabView: NSTabView!
        
    //
    // Rom preferences
    //
        
    // Rom info
    @IBOutlet weak var romDropView: RomDropView!
    @IBOutlet weak var romTitle: NSTextField!
    @IBOutlet weak var romSubtitle: NSTextField!
    @IBOutlet weak var romSubsubtitle: NSTextField!
    @IBOutlet weak var romDeleteButton: NSButton!
    @IBOutlet weak var romMapText: NSTextField!
    @IBOutlet weak var romMapAddr: NSPopUpButton!

    @IBOutlet weak var extDropView: ExtRomDropView!
    @IBOutlet weak var extTitle: NSTextField!
    @IBOutlet weak var extSubtitle: NSTextField!
    @IBOutlet weak var extSubsubtitle: NSTextField!
    @IBOutlet weak var extDeleteButton: NSButton!
    @IBOutlet weak var extMapText: NSTextField!
    @IBOutlet weak var extMapAddr: NSPopUpButton!

    @IBOutlet weak var arosButton: NSButton!

    // Lock
    @IBOutlet weak var romLockImage: NSButton!
    @IBOutlet weak var romLockText: NSTextField!
    @IBOutlet weak var romLockSubText: NSTextField!
   
    @IBOutlet weak var romArosButton: NSButton!

    // Buttons
    @IBOutlet weak var romOKButton: NSButton!
    @IBOutlet weak var romBootButton: NSButton!

    //
    // Hardware preferences
    //
    
    // Chipset
    @IBOutlet weak var hwAgnusRevisionPopup: NSPopUpButton!
    @IBOutlet weak var hwDeniseRevisionPopup: NSPopUpButton!
    @IBOutlet weak var hwRealTimeClock: NSPopUpButton!

    // Memory
    @IBOutlet weak var hwChipRamPopup: NSPopUpButton!
    @IBOutlet weak var hwSlowRamPopup: NSPopUpButton!
    @IBOutlet weak var hwFastRamPopup: NSPopUpButton!

    // Drive
    @IBOutlet weak var hwDf0Type: NSPopUpButton!
    @IBOutlet weak var hwDf1Connect: NSButton!
    @IBOutlet weak var hwDf1Type: NSPopUpButton!
    @IBOutlet weak var hwDf2Connect: NSButton!
    @IBOutlet weak var hwDf2Type: NSPopUpButton!
    @IBOutlet weak var hwDf3Connect: NSButton!
    @IBOutlet weak var hwDf3Type: NSPopUpButton!

    // Ports
    @IBOutlet weak var hwGameDevice1: NSPopUpButton!
    @IBOutlet weak var hwGameDevice2: NSPopUpButton!
    @IBOutlet weak var hwSerialDevice: NSPopUpButton!

    // Lock
    @IBOutlet weak var hwLockImage: NSButton!
    @IBOutlet weak var hwLockText: NSTextField!
    @IBOutlet weak var hwLockSubText: NSTextField!

    // Buttons
    @IBOutlet weak var hwFactorySettingsPopup: NSPopUpButton!
    @IBOutlet weak var hwOKButton: NSButton!
    @IBOutlet weak var hwBootButton: NSButton!

    //
    // Compatibility preferences
    //

    // Graphics
    @IBOutlet weak var compClxSprSpr: NSButton!
    @IBOutlet weak var compClxSprPlf: NSButton!
    @IBOutlet weak var compClxPlfPlf: NSButton!

    // Blitter
    @IBOutlet weak var compBltAccuracy: NSSlider!
    @IBOutlet weak var compBltLevel2: NSTextField!
    @IBOutlet weak var compBltLevel1: NSTextField!

    // Audio
    @IBOutlet weak var compSamplingMethod: NSPopUpButton!
    @IBOutlet weak var compFilterActivation: NSPopUpButton!

    // Disk controller
    @IBOutlet weak var compDriveSpeed: NSPopUpButton!
    @IBOutlet weak var compAsyncFifo: NSButton!
    @IBOutlet weak var compLockDskSync: NSButton!
    @IBOutlet weak var compAutoDskSync: NSButton!
    
    // CIAs
    @IBOutlet weak var compTodBug: NSButton!

    // Lock
    @IBOutlet weak var compLockText: NSTextField!
    @IBOutlet weak var compLockSubText: NSTextField!

    // Buttons
    @IBOutlet weak var compOKButton: NSButton!
    @IBOutlet weak var compBootButton: NSButton!

    //
    // Video preferences
    //
    
    // Palette
    @IBOutlet weak var vidPalettePopUp: NSPopUpButton!
    @IBOutlet weak var vidBrightnessSlider: NSSlider!
    @IBOutlet weak var vidContrastSlider: NSSlider!
    @IBOutlet weak var vidSaturationSlider: NSSlider!
    
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
    
    // Geometry
    @IBOutlet weak var vidHCenter: NSSlider!
    @IBOutlet weak var vidVCenter: NSSlider!
    @IBOutlet weak var vidHZoom: NSSlider!
    @IBOutlet weak var vidVZoom: NSSlider!

    // Buttons
    @IBOutlet weak var vidOKButton: NSButton!
    @IBOutlet weak var vidBootButton: NSButton!

    var bootable: Bool {
        let off   = amiga.isPoweredOff()
        let ready = amiga.readyToPowerOn() == ERR_OK
        return off && ready
    }
    
    // The tab to open first
    var firstTab = ""
    
    func showSheet(tab: String) {

        firstTab = tab
        showSheet()
    }

    override func awakeFromNib() {

        track()
        super.awakeFromNib()
        awakeVideoPrefsFromNib()
    }

    override func sheetWillShow() {
        
        track()
        if firstTab != "" { prefTabView?.selectTabViewItem(withIdentifier: firstTab) }
        refresh()
    }

    /*
    override func sheetDidShow() {

        track("sheetDidShow")
        if firstTab != "" { prefTabView?.selectTabViewItem(withIdentifier: firstTab) }
    }
    */

    func refresh() {
        
        if let id = prefTabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Roms": refreshRomTab()
            case "Hardware": refreshHardwareTab()
            case "Compatibility": refreshCompatibilityTab()
            case "Audio": refreshAudioTab()
            case "Video": refreshVideoTab()
            default: fatalError()
            }
        }
    }
    
    @IBAction func unlockAction(_ sender: Any!) {

        amiga.powerOff()
        refresh()
    }

    @IBAction override func cancelAction(_ sender: Any!) {
        
        track()

        parent.loadUserDefaults()
        refresh()
        hideSheet()
    }
    
    @IBAction override func okAction(_ sender: Any!) {
        
        hideSheet()
    }

    @IBAction func bootAction(_ sender: Any!) {
        
        hideSheet()
        amiga.run()
    }
}

extension ConfigController: NSTabViewDelegate {

    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {

        track()
        refresh()
    }
}
