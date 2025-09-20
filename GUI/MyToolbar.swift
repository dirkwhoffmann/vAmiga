// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class MyToolbar: NSToolbar {
    
    var amiga: EmulatorProxy { return controller.emu }
    
    @IBOutlet weak var controller: MyController!

    // References to toolbar items
    @IBOutlet weak var controlPort1Item: NSToolbarItem!
    @IBOutlet weak var controlPort2Item: NSToolbarItem!
    @IBOutlet weak var keyboardItem: NSToolbarItem!
    @IBOutlet weak var preferencesItem: NSToolbarItem!
    @IBOutlet weak var controlsItem: NSToolbarItem!

    // Reference to toolbar item objects
    @IBOutlet weak var controlPort1: NSPopUpButton!
    @IBOutlet weak var controlPort2: NSPopUpButton!
    @IBOutlet weak var snapshotSegCtrl: NSSegmentedControl!
    @IBOutlet weak var screenshotSegCtrl: NSSegmentedControl!
    @IBOutlet weak var controlsSegCtrl: NSSegmentedControl!

    override func validateVisibleItems() {

        // Disable the keyboard button if the virtual keyboard is open
        let visible = controller.virtualKeyboard?.window?.isVisible ?? false
        let view = keyboardItem.view as? NSButton
        view?.isEnabled = !visible

        // Disable the snapshot revert button if no snapshots have been taken
        snapshotSegCtrl.setEnabled(controller.snapshotCount > 0, forSegment: 1)

        // Update input devices
        controller.gamePadManager.refresh(popup: controlPort1)
        controller.gamePadManager.refresh(popup: controlPort2)
        controlPort1.selectItem(withTag: controller.config.gameDevice1)
        controlPort2.selectItem(withTag: controller.config.gameDevice2)

        controlPort1Item.menuFormRepresentation = nil
        controlPort2Item.menuFormRepresentation = nil
        keyboardItem.menuFormRepresentation = nil
        preferencesItem.menuFormRepresentation = nil
        controlsItem.menuFormRepresentation = nil
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
            controlsSegCtrl.setImage(NSImage(named: "pauseTemplate"), forSegment: 0)
        } else {
            controlsSegCtrl.setToolTip("Run", forSegment: 0)
            controlsSegCtrl.setImage(NSImage(named: "runTemplate"), forSegment: 0)
        }
    }
    
    //
    // Action methods
    //
    
    @IBAction
    func inspectAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: controller.inspectorAction(sender)
        case 1: controller.dashboardAction(sender)
        case 2: controller.consoleAction(sender)

        default:
            fatalError()
        }
    }
    
    @IBAction
    func snapshotAction(_ sender: NSSegmentedControl) {
        
        switch sender.selectedSegment {
            
        case 0: controller.takeSnapshotAction(self)
        case 1: controller.restoreSnapshotAction(self)
        case 2: controller.browseSnapshotsAction(self)
            
        default:
            fatalError()
        }
    }
    
    @IBAction
    func screenshotAction(_ sender: NSSegmentedControl) {
                
        switch sender.selectedSegment {
            
        case 0: controller.takeScreenshotAction(self)
        case 1: controller.browseScreenshotsAction(self)
            
        default:
            fatalError()
        }
    }

    @IBAction
    func port1Action(_ sender: Any) {
        
        if let popup = sender as? NSPopUpButton {
            controller.config.gameDevice1 = popup.selectedTag()
        }
    }
 
    @IBAction
    func port2Action(_ sender: Any) {
        
        if let popup = sender as? NSPopUpButton {
            controller.config.gameDevice2 = popup.selectedTag()
        }
    }
            
    @IBAction
    func keyboardAction(_ sender: Any!) {
        
        if controller.virtualKeyboard == nil {
            controller.virtualKeyboard = VirtualKeyboardController.make(parent: controller)
        }
        if controller.virtualKeyboard?.window?.isVisible == false {
            controller.virtualKeyboard?.showAsSheet()
        }
    }
    
    @IBAction
    func preferencesAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: controller.settingsAction(sender)
        case 1: controller.openConfiguratorAsSheet()

        default:
            fatalError()
        }
    }

    @IBAction
    func controlsAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: controller.stopAndGoAction(self)
        case 1: controller.resetAction(self)
        case 2: controller.powerAction(self)

        default:
            fatalError()
        }
    }
}
