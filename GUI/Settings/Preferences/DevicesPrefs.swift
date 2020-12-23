// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {
    
    func refreshDevicesTab() {
                
        func property(_ pad: GamePad?, _ key: String) -> String {
            return pad?.property(key: key) ?? "-"
        }

        track()

        let pad1 = parent.gamePadManager.gamePads[3]
        let pad2 = parent.gamePadManager.gamePads[4]
        let db = myAppDelegate.database
        let v1 = property(pad1, kIOHIDVendorIDKey)
        let p1 = property(pad1, kIOHIDProductIDKey)
        // let v2 = property(pad2, kIOHIDVendorIDKey)
        // let p2 = property(pad2, kIOHIDProductIDKey)

        devTransport1.stringValue = property(pad1, kIOHIDTransportKey)
        devManufacturer1.stringValue = property(pad1, kIOHIDManufacturerKey)
        devProduct1.stringValue = property(pad1, kIOHIDProductKey)
        devVersion1.stringValue = property(pad1, kIOHIDVersionNumberKey)
        devProductID1.stringValue = p1
        devVendorID1.stringValue = v1
        devLocationID1.stringValue = property(pad1, kIOHIDLocationIDKey)

        devName1.stringValue = db.name(vendorID: v1, productID: p1) ?? "Device 1"
        devImage1.image = db.image(vendorID: v1, productID: p1)
        if pad1 == nil {
            //
        }
        if pad2 == nil {
            //
        }
    }

    //
    // Action methods (Misc)
    //
    
    @IBAction func devPresetAction(_ sender: NSPopUpButton!) {
        
        track()
        assert(sender.selectedTag() == 0)
        
        refresh()
    }
    
    @IBAction func capLeftStickAction(_ sender: NSPopUpButton!) {
        
        track("tag = \(sender.tag)")
        track("selectedTag = \(sender.selectedTag())")

        refresh()
    }

    @IBAction func capRightStickAction(_ sender: NSPopUpButton!) {
        
        track("tag = \(sender.tag)")
        track("selectedTag = \(sender.selectedTag())")

        refresh()
    }

    @IBAction func capHeadSwitchAction(_ sender: NSPopUpButton!) {
        
        track("tag = \(sender.tag)")
        track("selectedTag = \(sender.selectedTag())")

        refresh()
    }
}
