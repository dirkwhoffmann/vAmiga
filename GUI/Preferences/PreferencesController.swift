// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class PreferencesController : UserDialogController {

    @IBOutlet weak var prefTabView: NSTabView!
    
    //
    // Rom preferences
    //
    
    @IBOutlet weak var romBootDropView: RomDropView!
    @IBOutlet weak var romBootTitle: NSTextField!
    @IBOutlet weak var romBootHash: NSTextField!
    @IBOutlet weak var romBootPath: NSTextField!
    @IBOutlet weak var romBootText: NSTextField!
    @IBOutlet weak var romBootSubText: NSTextField!
    @IBOutlet weak var romBootDeleteButton: NSButton!

    @IBOutlet weak var romKickDropView: RomDropView!
    @IBOutlet weak var romKickTitle: NSTextField!
    @IBOutlet weak var romKickHash: NSTextField!
    @IBOutlet weak var romKickPath: NSTextField!
    @IBOutlet weak var romKickText: NSTextField!
    @IBOutlet weak var romKickSubText: NSTextField!
    @IBOutlet weak var romKickDeleteButton: NSButton!
    
    // Lock
    @IBOutlet weak var romLockImage: NSButton!
    @IBOutlet weak var romLockText: NSTextField!
    @IBOutlet weak var romLockSubText: NSTextField!
    

    //
    // Hardware preferences
    //
    
    // Machine
    @IBOutlet weak var hwAmigaModelPopup: NSPopUpButton!
    @IBOutlet weak var hwRealTimeClock: NSButton!

    // Memory
    @IBOutlet weak var hwChipRamPopup: NSPopUpButton!
    @IBOutlet weak var hwSlowRamPopup: NSPopUpButton!
    @IBOutlet weak var hwFastRamPopup: NSPopUpButton!

    // Drive
    @IBOutlet weak var hwDf0Connect: NSButton!
    @IBOutlet weak var hwDf0Type: NSPopUpButton!
    @IBOutlet weak var hwDf1Connect: NSButton!
    @IBOutlet weak var hwDf1Type: NSPopUpButton!

    // Lock
    @IBOutlet weak var hwLockImage: NSImageView!
    @IBOutlet weak var hwLockText: NSTextField!
    @IBOutlet weak var hwLockSubText: NSTextField!
    
    //
    // Video preferences
    //
    
    // Palette
    @IBOutlet weak var vidPalettePopup: NSPopUpButton!
    @IBOutlet weak var vidBrightnessSlider: NSSlider!
    @IBOutlet weak var vidContrastSlider: NSSlider!
    @IBOutlet weak var vidSaturationSlider: NSSlider!
    
    // Effects
    @IBOutlet weak var vidUpscalerPopup: NSPopUpButton!
    @IBOutlet weak var vidBlurPopUp: NSPopUpButton!
    @IBOutlet weak var vidBlurRadiusSlider: NSSlider!
    
    @IBOutlet weak var vidBloomPopup:NSPopUpButton!
    @IBOutlet weak var vidBloomRadiusRSlider: NSSlider!
    @IBOutlet weak var vidBloomRadiusGSlider: NSSlider!
    @IBOutlet weak var vidBloomRadiusBSlider: NSSlider!
    @IBOutlet weak var vidBloomBrightnessSlider: NSSlider!
    @IBOutlet weak var vidBloomWeightSlider: NSSlider!
    
    @IBOutlet weak var vidDotMaskPopUp: NSPopUpButton!
    @IBOutlet weak var vidDotMaskBrightnessSlider: NSSlider!
    
    @IBOutlet weak var vidScanlinesPopUp: NSPopUpButton!
    @IBOutlet weak var vidScanlineBrightnessSlider: NSSlider!
    @IBOutlet weak var vidScanlineWeightSlider: NSSlider!
    
    @IBOutlet weak var vidMisalignmentPopUp: NSPopUpButton!
    @IBOutlet weak var vidMisalignmentXSlider: NSSlider!
    @IBOutlet weak var vidMisalignmentYSlider: NSSlider!
    
    // Geometry
    @IBOutlet weak var vidAspectRatioButton: NSButton!
    @IBOutlet weak var vidEyeXSlider: NSSlider!
    @IBOutlet weak var vidEyeYSlider: NSSlider!
    @IBOutlet weak var vidEyeZSlider: NSSlider!

    
    //
    // Emulator preferences
    //
    
    // VC1541
    @IBOutlet weak var emuWarpLoad: NSButton!
    @IBOutlet weak var emuDriveSounds: NSButton!
    
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
    

    //
    // Devices preferences
    //
    
    /// Indicates if a keycode should be recorded for keyset 1
    var devRecordKey1: JoystickDirection?
    
    /// Indicates if a keycode should be recorded for keyset 1
    var devRecordKey2: JoystickDirection?
    
    /// Joystick emulation keys
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
    @IBOutlet weak var devDisconnectKeys: NSButton!
    
    // Joystick buttons
    @IBOutlet weak var devAutofire: NSButton!
    @IBOutlet weak var devAutofireCease: NSButton!
    @IBOutlet weak var devAutofireCeaseText: NSTextField!
    @IBOutlet weak var devAutofireBullets: NSTextField!
    @IBOutlet weak var devAutofireFrequency: NSSlider!
    
    // Mouse
    @IBOutlet weak var devMouseModel: NSPopUpButton!
    @IBOutlet weak var devMouseInfo: NSTextField!

    
    //
    // Keymap preferences
    //
    
    @IBOutlet weak var info: NSTextField!
    @IBOutlet weak var keyMappingPopup: NSPopUpButton!
    @IBOutlet weak var keyMatrixScrollView: NSScrollView!
    @IBOutlet weak var keyMatrixCollectionView: NSCollectionView!

    // Double array of key images, indexed by their row and column number
    var keyImage = Array(repeating: Array(repeating: nil as NSImage?, count: 8), count: 8)
    
    // Selected C64 key
    var selectedKey: C64Key? = nil
        
    override func awakeFromNib() {
    
        awakeVideoPrefsFromNib()
        refresh()
    }
    
    override func refresh() {
        
        if let id = prefTabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Roms": refreshRomTab()
            case "Hardware": refreshHardwareTab()
            case "Video": refreshVideoTab()
            case "Emulator": refreshEmulatorTab()
            case "Devices": refreshDevicesTab()
            case "Keyboard": refreshKeyboardTab()
            default: break
            }
        }
    }
    
    override func keyDown(with key: MacKey) {
        
        if let id = prefTabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Devices": devKeyDown(with: key)
            case "Keyboard": mapKeyDown(with: key)
            default: break
            }
        }
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
        
        if proxy == nil || !proxy!.isRunnable() {
            NSApp.terminate(self)
        }
    }
}

extension PreferencesController : NSTabViewDelegate {

    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {

        refresh()
    }
}

extension PreferencesController : NSTextFieldDelegate {
    
    func controlTextDidChange(_ obj: Notification) {
        
        track()
        
        if let view = obj.object as? NSTextField {
            
            let formatter = view.formatter as? NumberFormatter
            
            switch view {
                
            case emuSnapshotInterval:
                
                if let _ = formatter?.number(from: view.stringValue) {
                    emuSnapshotIntervalAction(view)
                }
                
            case devAutofireBullets:
                
                if let _ = formatter?.number(from: view.stringValue) {
                    devAutofireBulletsAction(view)
                }
                
            default:
                break
            }
        }
    }
}
