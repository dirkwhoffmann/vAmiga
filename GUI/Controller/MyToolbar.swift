// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class MyToolbar : NSToolbar {
    
    @IBOutlet weak var controller: MyController!
    
    // Toolbar items
    @IBOutlet weak var controlPort1: NSPopUpButton!
    @IBOutlet weak var controlPort2: NSPopUpButton!
    @IBOutlet weak var pauseTbItem: NSToolbarItem!
    @IBOutlet weak var snapshotSegCtrl: NSSegmentedControl!
    
    override func validateVisibleItems() {
        
        guard let amiga = amigaProxy else { return }
        let button = pauseTbItem.view as! NSButton
        
        button.isEnabled = amiga.isPoweredOn()

        if amiga.isRunning() {
            button.image = NSImage.init(named: "pauseTemplate")
            pauseTbItem.label = "Pause"
        } else {
            button.image = NSImage.init(named: "continueTemplate")
            pauseTbItem.label = "Run"
        }
        
        validateJoystickToolbarItems()
    }
    
    func validateJoystickToolbarItem(_ popup: NSPopUpButton, selectedSlot: Int, port: AmigaControlPortProxy!) {
        
        let gpm = controller.gamePadManager!
        let menu =  popup.menu
        
        let item3 = menu?.item(withTag: InputDevice.joystick1)
        let item4 = menu?.item(withTag: InputDevice.joystick2)
        
        // USB joysticks
        item3?.title = gpm.gamePads[3]?.name ?? "USB Device 1"
        item4?.title = gpm.gamePads[4]?.name ?? "USB Device 2"
        item3?.isEnabled = !gpm.slotIsEmpty(InputDevice.joystick1)
        item4?.isEnabled = !gpm.slotIsEmpty(InputDevice.joystick2)
        
        // Mark game pad connected to port
        popup.selectItem(withTag: selectedSlot)
    }
    
    func validateJoystickToolbarItems() {
        
        guard let amiga = amigaProxy else { return }
        let device1 = controller.inputDevice1
        let device2 = controller.inputDevice2
        let port1 = amiga.controlPort1
        let port2 = amiga.controlPort2
            
        validateJoystickToolbarItem(controlPort1, selectedSlot: device1, port: port1)
        validateJoystickToolbarItem(controlPort2, selectedSlot: device2, port: port2)
    }
}
