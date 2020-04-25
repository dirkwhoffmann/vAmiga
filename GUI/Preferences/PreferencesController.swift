// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class PreferencesController: DialogController {

    var prefs: Preferences { return parent.prefs }
    var gamePadManager: GamePadManager { return parent.gamePadManager! }
    
    @IBOutlet weak var prefTabView: NSTabView!
        
    //
    // General preferences
    //
    
    // Drive
    @IBOutlet weak var emuDriveBlankDiskFormat: NSPopUpButton!
    @IBOutlet weak var emuEjectWithoutAskingButton: NSButton!
    @IBOutlet weak var emuDriveSounds: NSButton!
    @IBOutlet weak var emuDriveSoundPan: NSPopUpButton!
    @IBOutlet weak var emuDriveInsertSound: NSButton!
    @IBOutlet weak var emuDriveEjectSound: NSButton!
    @IBOutlet weak var emuDriveHeadSound: NSButton!
    @IBOutlet weak var emuDrivePollSound: NSButton!

    // Fullscreen
    @IBOutlet weak var emuAspectRatioButton: NSButton!
    @IBOutlet weak var emuExitOnEscButton: NSButton!

    // Snapshots and Screenshots
    @IBOutlet weak var emuAutoSnapshots: NSButton!
    @IBOutlet weak var emuSnapshotInterval: NSTextField!
    @IBOutlet weak var emuAutoScreenshots: NSButton!
    @IBOutlet weak var emuScreenshotInterval: NSTextField!
    @IBOutlet weak var emuScreenshotSourcePopup: NSPopUpButton!
    @IBOutlet weak var emuScreenshotTargetPopup: NSPopUpButton!
        
    // Warp mode
    @IBOutlet weak var emuWarpMode: NSPopUpButton!

    // Misc
    @IBOutlet weak var emuPauseInBackground: NSButton!
    @IBOutlet weak var emuCloseWithoutAskingButton: NSButton!

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
    
    // The tab to open first
    var firstTab: String?

    func showSheet(tab: String) {

        firstTab = tab
        showSheet()
    }

    override func awakeFromNib() {

        super.awakeFromNib()
        
        if let id = firstTab { prefTabView?.selectTabViewItem(withIdentifier: id) }
        refresh()
    }

    override func sheetDidShow() {

        if let id = firstTab { prefTabView?.selectTabViewItem(withIdentifier: id) }
    }

    func refresh() {
        
        if let id = prefTabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "General": refreshGeneralTab()
            case "Devices": refreshDevicesTab()
            default: fatalError()
            }
        }
    }
    
    @discardableResult
    func keyDown(with key: MacKey) -> Bool {
        
        if let id = prefTabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Devices": return devKeyDown(with: key)
            default: break
            }
        }
        return false
    }
    
    @IBAction override func okAction(_ sender: Any!) {
                
        prefs.saveGeneralUserDefaults()
        prefs.saveDevicesUserDefaults()
        hideSheet()
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
