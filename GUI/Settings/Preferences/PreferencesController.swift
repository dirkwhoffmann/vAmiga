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
    // General preferences
    //
    
    // Drive
    @IBOutlet weak var genDriveBlankDiskFormat: NSPopUpButton!
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
    
    //
    // Capture preferences
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

    func refresh() {
        
        if let id = tabView?.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "General": refreshGeneralTab()
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
        case "Devices": tabView.selectTabViewItem(at: 1)
        case "Captures": tabView.selectTabViewItem(at: 2)
        default: fatalError()
        }
    }

    @discardableResult
    func keyDown(with key: MacKey) -> Bool {
        
        track()
        
        if let id = tabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "Devices": return devKeyDown(with: key)
            default: break
            }
        }
        return false
    }
    
    @IBAction override func okAction(_ sender: Any!) {
                        
        pref.saveGeneralUserDefaults()
        pref.saveDevicesUserDefaults()
        pref.saveCaptureUserDefaults()
        
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
