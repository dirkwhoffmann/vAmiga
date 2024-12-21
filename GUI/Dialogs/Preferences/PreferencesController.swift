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
    var gamePadManager: GamePadManager { return parent.gamePadManager }
    var firstTab: String? // The tab to open first

    @IBOutlet weak var tabView: NSTabView!
        
    //
    // General
    //
    
    // Snapshots
    @IBOutlet weak var genAutoSnapshots: NSButton!
    @IBOutlet weak var genSnapshotInterval: NSTextField!
    @IBOutlet weak var genSnapshotStorage: NSTextField!

    // Screenshots
    @IBOutlet weak var genScreenshotSourcePopup: NSPopUpButton!
    @IBOutlet weak var genScreenshotTargetPopup: NSPopUpButton!

    // Screen captures
    @IBOutlet weak var genFFmpegPath: NSComboBox!
    @IBOutlet weak var genSource: NSPopUpButton!
    @IBOutlet weak var genAspectX: NSTextField!
    @IBOutlet weak var genAspectY: NSTextField!
    @IBOutlet weak var genBitRate: NSComboBox!
    
    // Fullscreen
    @IBOutlet weak var genAspectRatioButton: NSButton!
    @IBOutlet weak var genExitOnEscButton: NSButton!
        
    // Misc
    @IBOutlet weak var genEjectWithoutAskingButton: NSButton!
    @IBOutlet weak var genDetachWithoutAskingButton: NSButton!
    @IBOutlet weak var genCloseWithoutAskingButton: NSButton!
    @IBOutlet weak var genPauseInBackground: NSButton!

    //
    // Controls
    //

    // Tag of the button that is currently being recorded
    var conRecordedKey: Int?

    // Emulation keys
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

    @IBOutlet weak var conMouseLeft: NSTextField!
    @IBOutlet weak var conMouseLeftButton: NSButton!
    @IBOutlet weak var conMouseMiddle: NSTextField!
    @IBOutlet weak var conMouseMiddleButton: NSButton!
    @IBOutlet weak var conMouseRight: NSTextField!
    @IBOutlet weak var conMouseRightButton: NSButton!

    @IBOutlet weak var conDisconnectKeys: NSButton!

    // Mouse
    @IBOutlet weak var conRetainMouseKeyComb: NSPopUpButton!
    @IBOutlet weak var conRetainMouseWithKeys: NSButton!
    @IBOutlet weak var conRetainMouseByClick: NSButton!
    @IBOutlet weak var conRetainMouseByEntering: NSButton!
    @IBOutlet weak var conReleaseMouseKeyComb: NSPopUpButton!
    @IBOutlet weak var conReleaseMouseWithKeys: NSButton!
    @IBOutlet weak var conReleaseMouseByShaking: NSButton!

    //
    // Devices
    //
    
    @IBOutlet weak var devSelector: NSSegmentedControl!
    
    // Property Box
    @IBOutlet weak var devInfoBox: NSBox!
    @IBOutlet weak var devInfoBoxTitle: NSTextField!
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

    // Controller
    @IBOutlet weak var devImage: NSImageView!
    @IBOutlet weak var devAction: NSTextField!
    @IBOutlet weak var devAction2: NSTextField!
    @IBOutlet weak var devHidEvent: NSTextField!
    @IBOutlet weak var devHidMapping: NSTextView!
    @IBOutlet weak var devHidMappingScrollView: NSScrollView!

    //
    // Methods
    //
    
    func showSheet(tab: String) {
        
        firstTab = tab
        showAsSheet()
    }

    override func awakeFromNib() {

        super.awakeFromNib()
        
        if let id = firstTab { tabView?.selectTabViewItem(withIdentifier: id) }
        refresh()
    }

    override func dialogDidShow() {

        super.dialogDidShow()
        if let id = firstTab { tabView?.selectTabViewItem(withIdentifier: id) }
    }

    override func cleanup() {
     
        parent.gamePadManager.gamePads[3]?.notify = false
        parent.gamePadManager.gamePads[4]?.notify = false
    }
        
    func refresh() {
        
        if let id = tabView?.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "General": refreshGeneralTab()
            case "Controls": refreshControlsTab()
            case "Devices": refreshDevicesTab()
            default: fatalError()
            }
        }
    }
    
    func select() {
        
        if let id = tabView.selectedTabViewItem?.identifier as? String {
            
            switch id {
            case "General": selectGeneralTab()
            case "Controls": selectControlsTab()
            case "Devices": selectDevicesTab()
            default: fatalError()
            }
        }
    }

    @discardableResult
    func keyDown(with key: MacKey) -> Bool {
                
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
        close()
    }
}

extension PreferencesController: NSTabViewDelegate {

    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {

        select()
    }
}

extension PreferencesController {
    
    override func windowWillClose(_ notification: Notification) {

        super.windowWillClose(notification)
        cleanup()
    }
    
    func windowDidBecomeKey(_ notification: Notification) {
        
        select()
    }
}
    
extension PreferencesController: NSTextFieldDelegate {
    
    func controlTextDidChange(_ obj: Notification) {
                
        if let view = obj.object as? NSTextField {
            
            let formatter = view.formatter as? NumberFormatter
            
            switch view {
                
            case genSnapshotInterval:
                
                if formatter?.number(from: view.stringValue) != nil {
                    genSnapshotIntervalAction(view)
                }

            default:
                break
            }
        }
    }
}
