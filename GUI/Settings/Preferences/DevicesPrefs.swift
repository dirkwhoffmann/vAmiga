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
        
        track()
        
        func property(_ pad: GamePad?, _ key: String) -> String {
            return pad?.property(key: key) ?? "-"
        }
        
        let pad1 = parent.gamePadManager.gamePads[3]
        devTransport1.stringValue = property(pad1, kIOHIDTransportKey)
        devManufacturer1.stringValue = property(pad1, kIOHIDManufacturerKey)
        devProduct1.stringValue = property(pad1, kIOHIDProductKey)
        devVersion1.stringValue = property(pad1, kIOHIDVersionNumberKey)
        devProductID1.stringValue = property(pad1, kIOHIDProductIDKey)
        devVendorID1.stringValue = property(pad1, kIOHIDVendorIDKey)
        devLocationID1.stringValue = property(pad1, kIOHIDLocationIDKey)

        let pad2 = parent.gamePadManager.gamePads[4]
        
        if pad1 == nil {
            devTransport1.stringValue = "Not connected"
        }
        if pad2 == nil {
            // devTransport2 = "Not connected"
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
