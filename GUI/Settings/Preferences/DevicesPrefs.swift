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
                
        func property(_ pad: GamePad?, _ key: String) -> String {
            return pad?.property(key: key) ?? "-"
        }

        track()

        let pad = selectedDev
        
        // Let us notify when the device is pulled
        pad?.notify = true
        
        let db = myAppDelegate.database
        let vend = property(pad, kIOHIDVendorIDKey)
        let prod = property(pad, kIOHIDProductIDKey)

        devManufacturer.stringValue = property(pad, kIOHIDManufacturerKey)
        devProduct.stringValue = property(pad, kIOHIDProductKey)
        devVersion.stringValue = property(pad, kIOHIDVersionNumberKey)
        devVendorID.stringValue = vend
        devProductID.stringValue = prod
        devTransport.stringValue = property(pad, kIOHIDTransportKey)
        devUsage.stringValue = property(pad, kIOHIDPrimaryUsageKey)
        devUsagePage.stringValue = property(pad, kIOHIDPrimaryUsagePageKey)
        devLocationID.stringValue = property(pad, kIOHIDLocationIDKey)
        devUniqueID.stringValue = property(pad, kIOHIDUniqueIDKey)

        devLeftStickScheme.selectItem(withTag: db.left(vendorID: vend, productID: prod))
        devRightStickScheme.selectItem(withTag: db.right(vendorID: vend, productID: prod))
        devHatSwitchScheme.selectItem(withTag: db.hatSwitch(vendorID: vend, productID: prod))

        if pad != nil {
            
            devInfoBox.title = db.name(vendorID: vend, productID: prod)!
            devLeftStickScheme.isEnabled = true
            devRightStickScheme.isEnabled = true
            devHatSwitchScheme.isEnabled = true
            
        } else {

            devInfoBox.title = "Unrecognized Device"
            devLeftStickScheme.isEnabled = false
            devRightStickScheme.isEnabled = false
            devHatSwitchScheme.isEnabled = false
            devActivity.stringValue = ""
        }
    }

    func refreshDeviceEvents(events: [GamePadAction]) {
    
        var activity = ""
        if events.contains(.PULL_UP) { activity += " Pull Up " }
        if events.contains(.PULL_DOWN) { activity += " Pull Down " }
        if events.contains(.PULL_RIGHT) { activity += " Pull Right " }
        if events.contains(.PULL_LEFT) { activity += " Pull Left " }
        if events.contains(.PRESS_FIRE) { activity += " Press Fire " }
        if events.contains(.RELEASE_X) { activity += " Release X Axis " }
        if events.contains(.RELEASE_Y) { activity += " Release Y Axis " }
        if events.contains(.RELEASE_XY) { activity += " Release Axis " }
        if events.contains(.RELEASE_FIRE) { activity += " Release Fire " }
        devActivity.stringValue = activity
    }
    
    //
    // Action methods (Misc)
    //
      
    @IBAction func selectDeviceAction(_ sender: Any!) {

        refresh()
    }
    
    @IBAction func devLeftAction(_ sender: NSPopUpButton!) {
        
        let selectedTag = "\(sender.selectedTag())"
        
        track("tag = \(sender.tag)")
        track("selectedTag = \(selectedTag)")
        
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
        
        track("tag = \(sender.tag)")
        track("selectedTag = \(selectedTag)")
        
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

        track("tag = \(sender.tag)")
        track("selectedTag = \(selectedTag)")
        
        if let device = selectedDev {
            myAppDelegate.database.setHatSwitch(vendorID: device.vendorID,
                                                productID: device.productID,
                                                selectedTag)
            device.updateMappingScheme()
        }
        refresh()
    }
        
    @IBAction func devPresetAction(_ sender: NSPopUpButton!) {
        
        track()
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
}
