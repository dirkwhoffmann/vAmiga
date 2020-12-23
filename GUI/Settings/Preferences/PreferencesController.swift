// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class PreferencesController: DialogController {

    var pref: Preferences { return parent.pref }
    var gamePadManager: GamePadManager { return parent.gamePadManager! }
    var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
    
    @IBOutlet weak var tabView: NSTabView!
        
    //
    // General
    //
    
    // Drive
    @IBOutlet weak var genDriveBlankDiskFormat: NSPopUpButton!
    @IBOutlet weak var genBootCode: NSPopUpButton!
    @IBOutlet weak var genBootCodeVirus: NSImageView!
    @IBOutlet weak var genEjectWithoutAskingButton: NSButton!
    @IBOutlet weak var genDriveSounds: NSButton!
    @IBOutlet weak var genDriveSoundPan: NSPopUpButton!
    @IBOutlet weak var genDriveInsertSound: NSButton!
    @IBOutlet weak var genDriveEjectSound: NSButton!
    @IBOutlet weak var genDriveHeadSound: NSButton!
    @IBOutlet weak var genDrivePollSound: NSButton!

    // Fullscreen
    @IBOutlet weak var genAspectRatioButton: NSButton!
    @IBOutlet weak var genExitOnEscButton: NSButton!
        
    // Warp mode
    @IBOutlet weak var genWarpMode: NSPopUpButton!

    // Misc
    @IBOutlet weak var genPauseInBackground: NSButton!
    @IBOutlet weak var genCloseWithoutAskingButton: NSButton!

    //
    // Controls
    //

    // Tag of the button that is currently being recorded
    var conRecordedKey: Int?

    // Joystick emulation keys
    @IBOutlet weak var conLeft1: NSTextField!
    @IBOutlet weak var conLeft1button: NSButton!
    @IBOutlet weak var conRight1: NSTextField!
    @IBOutlet weak var conRight1button: NSButton!
    @IBOutlet weak var conUp1: NSTextField!
    @IBOutlet weak var conUp1button: NSButton!
    @IBOutlet weak var conDown1: NSTextField!
    @IBOutlet weak var conDown1button: NSButton!
    @IBOutlet weak var conFire1: NSTextField!
    @IBOutlet weak var conFire1button: NSButton!
    @IBOutlet weak var conLeft2: NSTextField!
    @IBOutlet weak var conLeft2button: NSButton!
    @IBOutlet weak var conRight2: NSTextField!
    @IBOutlet weak var conRight2button: NSButton!
    @IBOutlet weak var conUp2: NSTextField!
    @IBOutlet weak var conUp2button: NSButton!
    @IBOutlet weak var conDown2: NSTextField!
    @IBOutlet weak var conDown2button: NSButton!
    @IBOutlet weak var conFire2: NSTextField!
    @IBOutlet weak var conFire2button: NSButton!

    // Mouse emulation keys
    @IBOutlet weak var conMouseLeft: NSTextField!
    @IBOutlet weak var conMouseLeftButton: NSButton!
    @IBOutlet weak var conMouseRight: NSTextField!
    @IBOutlet weak var conMouseRightButton: NSButton!
    @IBOutlet weak var conDisconnectKeys: NSButton!

    @IBOutlet weak var conJoy1KeysDelButton: NSButton!
    @IBOutlet weak var conJoy2KeysDelButton: NSButton!
    @IBOutlet weak var conMouseKeysDelButton: NSButton!

    // Mouse control
    @IBOutlet weak var conRetainMouseKeyComb: NSPopUpButton!
    @IBOutlet weak var conRetainMouseWithKeys: NSButton!
    @IBOutlet weak var conRetainMouseByClick: NSButton!
    @IBOutlet weak var conRetainMouseByEntering: NSButton!
    @IBOutlet weak var conReleaseMouseKeyComb: NSPopUpButton!
    @IBOutlet weak var conReleaseMouseWithKeys: NSButton!
    @IBOutlet weak var conReleaseMouseByShaking: NSButton!

    // Joystick buttons
    @IBOutlet weak var conAutofire: NSButton!
    @IBOutlet weak var conAutofireCease: NSButton!
    @IBOutlet weak var conAutofireCeaseText: NSTextField!
    @IBOutlet weak var conAutofireBullets: NSTextField!
    @IBOutlet weak var conAutofireFrequency: NSSlider!

    //
    // Devices
    //
    
    @IBOutlet weak var devSelector: NSSegmentedControl!
    @IBOutlet weak var devInfoBox: NSBox!
    @IBOutlet weak var devManufacturer: NSTextField!
    @IBOutlet weak var devProduct: NSTextField!
    @IBOutlet weak var devVersion: NSTextField!
    @IBOutlet weak var devVendorID: NSTextField!
    @IBOutlet weak var devProductID: NSTextField!
    @IBOutlet weak var devTransport: NSTextField!
    @IBOutlet weak var devUsage: NSTextField!
    @IBOutlet weak var devUsagePage: NSTextField!
    @IBOutlet weak var devLocationID: NSTextField!
    @IBOutlet weak var devUniqueID: NSTextField!
    @IBOutlet weak var devActivity1: NSTextField!
    @IBOutlet weak var devActivity2: NSTextField!
    @IBOutlet weak var devLeftStickScheme: NSPopUpButton!
    @IBOutlet weak var devRightStickScheme: NSPopUpButton!
    @IBOutlet weak var devHatSwitchScheme: NSPopUpButton!
    
    //
    // Captures
    //
    
    // Screenshots
    @IBOutlet weak var capAutoScreenshots: NSButton!
    @IBOutlet weak var capScreenshotInterval: NSTextField!
    @IBOutlet weak var capScreenshotSourcePopup: NSPopUpButton!
    @IBOutlet weak var capScreenshotTargetPopup: NSPopUpButton!

    // Snapshots
    @IBOutlet weak var capAutoSnapshots: NSButton!
    @IBOutlet weak var capSnapshotInterval: NSTextField!

    // Screen captures
    @IBOutlet weak var capFFmpegText: NSTextField!
    @IBOutlet weak var capFFmpegIcon1: NSButton!
    @IBOutlet weak var capFFmpegIcon2: NSImageView!
    @IBOutlet weak var capSource: NSPopUpButton!
    @IBOutlet weak var capAspectX: NSTextField!
    @IBOutlet weak var capAspectY: NSTextField!
    @IBOutlet weak var capBitRate: NSComboBox!
    
    // The tab to open first
    var firstTab: String?

    func showSheet(tab: String) {
        
        firstTab = tab
        showSheet()
    }

    override func awakeFromNib() {

        super.awakeFromNib()
        
        if let id = firstTab { tabView?.selectTabViewItem(withIdentifier: id) }
        refresh()
    }

    override func sheetDidShow() {

        if let id = firstTab { tabView?.selectTabViewItem(withIdentifier: id) }
    }

    override func cleanup() {
     
        track()
        parent.gamePadManager.gamePads[3]?.notify = false
        parent.gamePadManager.gamePads[4]?.notify = false
    }
        
    func refresh() {
        
        track()
        if let id = tabView?.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "General": refreshGeneralTab()
            case "Controls": refreshControlsTab()
            case "Devices": refreshDevicesTab()
            case "Captures": refreshCaptureTab()
            default: fatalError()
            }
        }
    }
    
    func selectTab(_ id: String) {
        
        track("selectTab(\(id))")
        
        switch id {
        case "General": tabView.selectTabViewItem(at: 0)
        case "Controls": tabView.selectTabViewItem(at: 1)
        case "Devices": tabView.selectTabViewItem(at: 2)
        case "Captures": tabView.selectTabViewItem(at: 3)
        default: fatalError()
        }
    }

    @discardableResult
    func keyDown(with key: MacKey) -> Bool {
        
        track()
        
        if let id = tabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Controls": return devKeyDown(with: key)
            default: break
            }
        }
        return false
    }
    
    @IBAction override func okAction(_ sender: Any!) {
                        
        pref.saveGeneralUserDefaults()
        pref.saveControlsUserDefaults()
        pref.saveDevicesUserDefaults()
        pref.saveCaptureUserDefaults()
        myAppDelegate.database.save()
        
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
                
            case capSnapshotInterval:
                
                if formatter?.number(from: view.stringValue) != nil {
                    capSnapshotIntervalAction(view)
                }
                
            case conAutofireBullets:
                
                if formatter?.number(from: view.stringValue) != nil {
                    conAutofireBulletsAction(view)
                }
                
            default:
                break
            }
        }
    }
}
