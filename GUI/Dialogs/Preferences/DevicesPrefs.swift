// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {
    
    var selectedDev: GamePad? {
        if devSelector.indexOfSelectedItem == 0 {
            return parent.gamePadManager.gamePads[3]
        } else {
            return parent.gamePadManager.gamePads[4]
        }
    }

    func refreshDevicesTab() {
                
        let pad = selectedDev

        func property(_ key: String) -> String {
            return pad?.property(key: key) ?? "-"
        }

        // Let us notify when the device is pulled
        pad?.notify = true

        devManufacturer.stringValue = property(kIOHIDManufacturerKey)
        devProduct.stringValue = property(kIOHIDProductKey)
        devVersion.stringValue = property(kIOHIDVersionNumberKey)
        devVendorID.stringValue = property(kIOHIDVendorIDKey)
        devProductID.stringValue = property(kIOHIDProductIDKey)
        devTransport.stringValue = property(kIOHIDTransportKey)
        devUsagePage.stringValue = property(kIOHIDPrimaryUsagePageKey)
        devLocationID.stringValue = property(kIOHIDLocationIDKey)
        devUniqueID.stringValue = property(kIOHIDUniqueIDKey)

        if let usageDescription = pad?.device?.usageDescription {
            devUsage.stringValue = usageDescription
        } else {
            devUsage.stringValue = property(kIOHIDPrimaryUsageKey)
        }

        /*
        devLeftScheme.selectItem(withTag: db.left(vendorID: vend, productID: prod))
        devRightScheme.selectItem(withTag: db.right(vendorID: vend, productID: prod))
        devHatScheme.selectItem(withTag: db.hatSwitch(vendorID: vend, productID: prod))
        */

        if pad?.isKnown == true {
            devInfoBoxTitle.stringValue = ""
            devInfoBoxTitle.textColor = .secondaryLabelColor
        } else if pad?.isKnown == false {
            devInfoBoxTitle.stringValue = "This device is not known to the emulator. It may or may not work."
            devInfoBoxTitle.textColor = .warning
        } else {
            devInfoBoxTitle.stringValue = "Not connected"
            devInfoBoxTitle.textColor = .secondaryLabelColor
        }
        
        let hide = pad == nil || pad?.isMouse == true
        devHidEvent.isHidden = hide
        devAction.isHidden = hide
        devAction2.isHidden = hide
    }

    func refreshDeviceEvent(event: HIDEvent, nr: Int, value: Int) {

        var text = ""

        switch event {

        case .BUTTON: text = "b\(nr) = \(value)"
        case .AXIS: text = "a\(nr) = \(value)"
        case .DPAD_UP: text = "DPad = Up"
        case .DPAD_DOWN: text = "DPad = Down"
        case .DPAD_RIGHT: text = "DPad = Right"
        case .DPAD_LEFT: text = "DPad = Left"
        default: text = ""
        }

        devHidEvent.stringValue = text
    }

    func refreshDeviceActions(actions: [GamePadAction]) {
            
        var activity = "", activity2 = ""
        
        func add(_ str: String) {
            if activity == "" { activity += str } else { activity2 += str }
        }
        
        if actions.contains(.PULL_UP) { add(" Pull Up ") }
        if actions.contains(.PULL_DOWN) { add(" Pull Down ") }
        if actions.contains(.PULL_RIGHT) { add(" Pull Right ") }
        if actions.contains(.PULL_LEFT) { add(" Pull Left ") }
        if actions.contains(.PRESS_FIRE) { add(" Press Fire ") }
        if actions.contains(.RELEASE_X) { add(" Release X Axis ") }
        if actions.contains(.RELEASE_Y) { add(" Release Y Axis ") }
        if actions.contains(.RELEASE_XY) { add(" Release Axis ") }
        if actions.contains(.RELEASE_FIRE) { add(" Release Fire ") }
     
        devAction.stringValue = activity
        devAction2.stringValue = activity2
    }
    
    func selectDevicesTab() {
        
        devAction.stringValue = ""
        devAction2.stringValue = ""
        refreshDevicesTab()
    }

    //
    // Action methods (Misc)
    //

    /*
    @IBAction func selectDeviceAction(_ sender: Any!) {

        refresh()
    }

    @IBAction func devLeftAction(_ sender: NSPopUpButton!) {
        
        let selectedTag = "\(sender.selectedTag())"
                
        if let device = selectedDev {
            myAppDelegate.database.setLeft(vendorID: device.vendorID,
                                           productID: device.productID,
                                           selectedTag)
            device.updateMappingScheme()
        }
        refresh()
    }

    @IBAction func devRightAction(_ sender: NSPopUpButton!) {
        
        let selectedTag = "\(sender.selectedTag())"
                
        if let device = selectedDev {
            myAppDelegate.database.setRight(vendorID: device.vendorID,
                                            productID: device.productID,
                                            selectedTag)
            device.updateMappingScheme()
        }
        refresh()
    }

    @IBAction func devHatSwitchAction(_ sender: NSPopUpButton!) {
        
        let selectedTag = "\(sender.selectedTag())"
        
        if let device = selectedDev {
            myAppDelegate.database.setHatSwitch(vendorID: device.vendorID,
                                                productID: device.productID,
                                                selectedTag)
            device.updateMappingScheme()
        }
        refresh()
    }

    @IBAction func devPresetAction(_ sender: NSPopUpButton!) {
        
        assert(sender.selectedTag() == 0)
        
        if let device = selectedDev {
            
            let db = myAppDelegate.database
            db.setLeft(vendorID: device.vendorID, productID: device.productID, nil)
            db.setRight(vendorID: device.vendorID, productID: device.productID, nil)
            db.setHatSwitch(vendorID: device.vendorID, productID: device.productID, nil)
            device.updateMappingScheme()
        }
        
        refresh()
    }
    */
}
