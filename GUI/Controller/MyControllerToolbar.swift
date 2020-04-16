// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

struct InputDevice {
    static let none = -1
    static let keyset1 = 0
    static let keyset2 = 1
    static let mouse = 2
    static let joystick1 = 3
    static let joystick2 = 4
}

extension MyController {

    func connectPort1(device: Int) {

        let cpd: ControlPortDevice =
            device == InputDevice.none ? CPD_NONE :
                device == InputDevice.mouse ? CPD_MOUSE : CPD_JOYSTICK

        amiga.controlPort1.connect(cpd)
        prefs.inputDevice1 = device
        UserDefaults.standard.set(device, forKey: Keys.inputDevice1)
    }

    func connectPort2(device: Int) {

        let cpd: ControlPortDevice =
            device == InputDevice.none ? CPD_NONE :
                device == InputDevice.mouse ? CPD_MOUSE : CPD_JOYSTICK

        amiga.controlPort2.connect(cpd)
        prefs.inputDevice2 = device
        UserDefaults.standard.set(device, forKey: Keys.inputDevice2)
    }

    @IBAction func port1Action(_ sender: NSPopUpButton) {
        
        setPort1(sender.selectedTag())
    }
 
    func setPort1(_ value: Int) {

        let newDevice1 = value
        var newDevice2 = prefs.inputDevice2

        // Avoid double mappings
        if newDevice1 == newDevice2 { newDevice2 = InputDevice.none }

        // Update ports
        if newDevice1 != prefs.inputDevice1 { connectPort1(device: newDevice1) }
        if newDevice2 != prefs.inputDevice2 { connectPort2(device: newDevice2) }

        toolbar.validateVisibleItems()
    }
    
    @IBAction func port2Action(_ sender: NSPopUpButton) {
        
        setPort2(sender.selectedTag())
    }
    
    func setPort2(_ value: Int) {
        
        var newDevice1 = prefs.inputDevice1
        let newDevice2 = value

        // Avoid double mappings
        if newDevice1 == newDevice2 { newDevice1 = InputDevice.none }

        // Update ports
        if newDevice1 != prefs.inputDevice1 { connectPort1(device: newDevice1) }
        if newDevice2 != prefs.inputDevice2 { connectPort2(device: newDevice2) }

        toolbar.validateVisibleItems()
    }
    
    @IBAction func inspectAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: inspectorAction(sender)
        case 1: monitorAction(sender)

        default: assert(false)
        }
    }
    
    @IBAction func snapshotAction(_ sender: NSSegmentedControl) {
        
        switch sender.selectedSegment {
            
        case 0: takeSnapshotAction(self)
        case 1: restoreSnapshotAction(self)
        case 2: browseSnapshotsAction(self)
            
        default:
            assert(false)
        }
    }
    
    @IBAction func screenshotAction(_ sender: NSSegmentedControl) {
        
        track()
        
        switch sender.selectedSegment {
            
        case 0: takeScreenshotAction(self)
        case 1: browseScreenshotsAction(self)
            
        default:
            assert(false)
        }
    }
        
    @IBAction func keyboardAction(_ sender: Any!) {
        
        // Open the virtual keyboard as a sheet
        if virtualKeyboardSheet == nil {
            virtualKeyboardSheet = VKBController.make(parent: self)
        }
        virtualKeyboardSheet?.showSheet(autoClose: true)
    }
}
