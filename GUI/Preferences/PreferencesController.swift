// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class PreferencesController: DialogController {

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

    // Lock
    @IBOutlet weak var romLockImage: NSButton!
    @IBOutlet weak var romLockText: NSTextField!
    @IBOutlet weak var romLockSubText: NSTextField!
   
    @IBOutlet weak var romFactoryButton: NSButton!

    // Button
    @IBOutlet weak var romOKButton: NSButton!

    //
    // Hardware preferences
    //
    
    // Chipset
    @IBOutlet weak var hwAgnusRevisionPopup: NSPopUpButton!
    @IBOutlet weak var hwDeniseRevisionPopup: NSPopUpButton!
    @IBOutlet weak var hwRealTimeClock: NSPopUpButton!

    // Ports
    @IBOutlet weak var hwSerialDevice: NSPopUpButton!

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

    // Lock
    @IBOutlet weak var hwLockImage: NSButton!
    @IBOutlet weak var hwLockText: NSTextField!
    @IBOutlet weak var hwLockSubText: NSTextField!

    // Buttons
    @IBOutlet weak var hwFactorySettingsPopup: NSPopUpButton!
    @IBOutlet weak var hwOKButton: NSButton!

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
    @IBOutlet weak var compFifoBuffering: NSButton!

    // CIAs
    @IBOutlet weak var compTodBug: NSButton!

    // Lock
    @IBOutlet weak var compLockText: NSTextField!
    @IBOutlet weak var compLockSubText: NSTextField!

    // Button
    @IBOutlet weak var compOKButton: NSButton!

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
    @IBOutlet weak var vidEyeXSlider: NSSlider!
    @IBOutlet weak var vidEyeYSlider: NSSlider!
    @IBOutlet weak var vidEyeZSlider: NSSlider!

    // Button
    @IBOutlet weak var vidOKButton: NSButton!

    //
    // Emulator preferences
    //
    
    // Drive
    @IBOutlet weak var emuWarpLoad: NSButton!
    @IBOutlet weak var emuDriveSounds: NSButton!
    @IBOutlet weak var emuDriveSoundsNoPoll: NSButton!
    @IBOutlet weak var emuDriveBlankDiskFormat: NSPopUpButton!

    // Fullscreen
    @IBOutlet weak var emuAspectRatioButton: NSButton!
    @IBOutlet weak var emuExitOnEscButton: NSButton!

    // Screenshots
    @IBOutlet weak var emuScreenshotSourcePopup: NSPopUpButton!
    @IBOutlet weak var emuScreenshotTargetPopup: NSPopUpButton!
    
    // User Dialogs
    @IBOutlet weak var emuAutoMountButton: NSButton!
    @IBOutlet weak var emuCloseWithoutAskingButton: NSButton!
    @IBOutlet weak var emuEjectWithoutAskingButton: NSButton!
    
    // Misc
    @IBOutlet weak var emuPauseInBackground: NSButton!
    @IBOutlet weak var emuAutoSnapshots: NSButton!
    @IBOutlet weak var emuSnapshotInterval: NSTextField!

    // Button
    @IBOutlet weak var emuOKButton: NSButton!

    //
    // Devices preferences
    //

    // Tag of the button that is currently being recorded
    var devRecordedKey: Int?

    // Joystick emulation keys
    @IBOutlet weak var devLeft1: NSTextField!
    @IBOutlet weak var devLeft1button: NSButton!
    @IBOutlet weak var devRight1: NSTextField!
    @IBOutlet weak var devRight1button: NSButton!
    @IBOutlet weak var devUp1: NSTextField!
    @IBOutlet weak var devUp1button: NSButton!
    @IBOutlet weak var devDown1: NSTextField!
    @IBOutlet weak var devDown1button: NSButton!
    @IBOutlet weak var devFire1: NSTextField!
    @IBOutlet weak var devFire1button: NSButton!
    @IBOutlet weak var devLeft2: NSTextField!
    @IBOutlet weak var devLeft2button: NSButton!
    @IBOutlet weak var devRight2: NSTextField!
    @IBOutlet weak var devRight2button: NSButton!
    @IBOutlet weak var devUp2: NSTextField!
    @IBOutlet weak var devUp2button: NSButton!
    @IBOutlet weak var devDown2: NSTextField!
    @IBOutlet weak var devDown2button: NSButton!
    @IBOutlet weak var devFire2: NSTextField!
    @IBOutlet weak var devFire2button: NSButton!

    // Mouse emulation keys
    @IBOutlet weak var devMouseLeft: NSTextField!
    @IBOutlet weak var devMouseLeftButton: NSButton!
    @IBOutlet weak var devMouseRight: NSTextField!
    @IBOutlet weak var devMouseRightButton: NSButton!
    @IBOutlet weak var devDisconnectKeys: NSButton!

    @IBOutlet weak var devJoy1KeysDelButton: NSButton!
    @IBOutlet weak var devJoy2KeysDelButton: NSButton!
    @IBOutlet weak var devMouseKeysDelButton: NSButton!

    // Mouse control
    @IBOutlet weak var devRetainMouseKeyComb: NSPopUpButton!
    @IBOutlet weak var devRetainMouseWithKeys: NSButton!
    @IBOutlet weak var devRetainMouseByClick: NSButton!
    @IBOutlet weak var devRetainMouseByEntering: NSButton!
    @IBOutlet weak var devReleaseMouseKeyComb: NSPopUpButton!
    @IBOutlet weak var devReleaseMouseWithKeys: NSButton!
    @IBOutlet weak var devReleaseMouseByShaking: NSButton!

    // Joystick buttons
    @IBOutlet weak var devAutofire: NSButton!
    @IBOutlet weak var devAutofireCease: NSButton!
    @IBOutlet weak var devAutofireCeaseText: NSTextField!
    @IBOutlet weak var devAutofireBullets: NSTextField!
    @IBOutlet weak var devAutofireFrequency: NSSlider!

    // Button
    @IBOutlet weak var devOKButton: NSButton!

    // The tab to open first
     var firstTab = ""

    func showSheet(tab: String) {

        firstTab = tab
        showSheet()
    }

    override func awakeFromNib() {

        if firstTab != "" { prefTabView?.selectTabViewItem(withIdentifier: firstTab) }
        awakeVideoPrefsFromNib()
        refresh()
    }

    override func sheetDidShow() {

        track("sheetDidShow")
        if firstTab != "" { prefTabView?.selectTabViewItem(withIdentifier: firstTab) }
    }

    override func refresh() {
        
        if let id = prefTabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Roms": refreshRomTab()
            case "Hardware": refreshHardwareTab()
            case "Compatibility": refreshCompatibilityTab()
            case "Video": refreshVideoTab()
            case "Emulator": refreshEmulatorTab()
            case "Devices": refreshDevicesTab()
            default: break
            }
        }
    }
    
    func keyDown(with key: MacKey) {
        
        if let id = prefTabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Devices": devKeyDown(with: key)
            default: break
            }
        }
    }

    @IBAction func unlockAction(_ sender: Any!) {

        amigaProxy?.powerOff()
        refresh()
    }

    @IBAction override func cancelAction(_ sender: Any!) {
        
        track()

        myController?.loadUserDefaults()
        refresh()
        hideSheet()
    }
    
    @IBAction override func okAction(_ sender: Any!) {
        
        track()

        hideSheet()
        myController?.saveUserDefaults()
    }
}

extension PreferencesController: NSTabViewDelegate {

    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {

        refresh()
    }
}

extension PreferencesController: NSTextFieldDelegate {
    
    func controlTextDidChange(_ obj: Notification) {
        
        track()
        
        if let view = obj.object as? NSTextField {
            
            let formatter = view.formatter as? NumberFormatter
            
            switch view {
                
            case emuSnapshotInterval:
                
                if formatter?.number(from: view.stringValue) != nil {
                    emuSnapshotIntervalAction(view)
                }
                
            case devAutofireBullets:
                
                if formatter?.number(from: view.stringValue) != nil {
                    devAutofireBulletsAction(view)
                }
                
            default:
                break
            }
        }
    }
}
