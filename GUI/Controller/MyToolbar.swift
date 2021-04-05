// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class MyToolbar: NSToolbar {
    
    var amiga: AmigaProxy { return parent.amiga }
    
    @IBOutlet weak var parent: MyController!
    
    // Toolbar items
    @IBOutlet weak var controlPort1: NSPopUpButton!
    @IBOutlet weak var controlPort2: NSPopUpButton!
    @IBOutlet weak var keyboardButton: NSToolbarItem!
    @IBOutlet weak var snapshotSegCtrl: NSSegmentedControl!
    @IBOutlet weak var controlsSegCtrl: NSSegmentedControl!

    override func validateVisibleItems() {

        // Disable the keyboard button of the virtual keyboard is open
        let visible = parent.virtualKeyboard?.window?.isVisible ?? false
        let view = keyboardButton.view as? NSButton
        view?.isEnabled = !visible

        // Update input devices
        parent.gamePadManager.refresh(popup: controlPort1)
        parent.gamePadManager.refresh(popup: controlPort2)
        controlPort1.selectItem(withTag: parent.config.gameDevice1)
        controlPort2.selectItem(withTag: parent.config.gameDevice2)
    }
    
    func updateToolbar() {
        
        if amiga.poweredOn {
            controlsSegCtrl.setEnabled(true, forSegment: 0) // Pause
            controlsSegCtrl.setEnabled(true, forSegment: 1) // Reset
            controlsSegCtrl.setToolTip("Power off", forSegment: 2) // Power
        } else {
            controlsSegCtrl.setEnabled(false, forSegment: 0) // Pause
            controlsSegCtrl.setEnabled(false, forSegment: 1) // Reset
            controlsSegCtrl.setToolTip("Power on", forSegment: 2) // Power
        }
        if amiga.running {
            controlsSegCtrl.setToolTip("Pause", forSegment: 0)
            controlsSegCtrl.setImage(NSImage.init(named: "pauseTemplate"), forSegment: 0)
        } else {
            controlsSegCtrl.setToolTip("Run", forSegment: 0)
            controlsSegCtrl.setImage(NSImage.init(named: "runTemplate"), forSegment: 0)
        }
    }
    
    //
    // Action methods
    //
    
    @IBAction func port1Action(_ sender: NSPopUpButton) {
        
        parent.config.gameDevice1 = sender.selectedTag()
    }
 
    @IBAction func port2Action(_ sender: NSPopUpButton) {
        
        parent.config.gameDevice2 = sender.selectedTag()
    }
    
    @IBAction func inspectAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: parent.inspectorAction(sender)
        case 1: parent.monitorAction(sender)
        case 2: parent.consoleAction(sender)

        default: assert(false)
        }
    }
    
    @IBAction func snapshotAction(_ sender: NSSegmentedControl) {
        
        switch sender.selectedSegment {
            
        case 0: parent.takeSnapshotAction(self)
        case 1: parent.restoreSnapshotAction(self)
        case 2: parent.browseSnapshotsAction(self)
            
        default: assert(false)
        }
    }
    
    @IBAction func screenshotAction(_ sender: NSSegmentedControl) {
        
        track()
        
        switch sender.selectedSegment {
            
        case 0: parent.takeScreenshotAction(self)
        case 1: parent.browseScreenshotsAction(self)
            
        default: assert(false)
        }
    }
    
    @IBAction func keyboardAction(_ sender: Any!) {
        
        // Open the virtual keyboard as a sheet
        if parent.virtualKeyboard == nil {
            parent.virtualKeyboard = VirtualKeyboardController.make(parent: parent)
        }
        parent.virtualKeyboard?.showSheet()
    }
    
    @IBAction func controlsAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: parent.stopAndGoAction(self)
        case 1: parent.resetAction(self)
        case 2: parent.powerAction(self)

        default: assert(false)
        }
    }
    
    @IBAction func toolbarPrefAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: parent.preferencesAction(sender)
        case 1: parent.configureAction(sender)

        default: assert(false)
        }
    }
}
