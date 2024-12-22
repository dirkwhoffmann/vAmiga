// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {

    var db: DeviceDatabase { return myAppDelegate.database }

    var pad: GamePad? {
        if devSelector.indexOfSelectedItem == 0 {
            return parent.gamePadManager.gamePads[3]
        } else {
            return parent.gamePadManager.gamePads[4]
        }
    }

    var guid: GUID {return pad?.guid ?? GUID() }
    func property(_ key: String) -> String { return pad?.property(key: key) ?? "-" }

    func refreshDevicesTab() {

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

        // HID mapping
        devHidMapping.focusRingType = .none
        if let descriptor = pad?.db.seek(guid: guid) {

            let trimmed = descriptor.trimmingCharacters(in: CharacterSet(charactersIn: ","))
            devHidMapping.string = trimmed.replacingOccurrences(of: ",", with: ",\n")
        }

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
        devImage.isHidden = hide
        devHidMappingScrollView.isHidden = hide
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
        
        devHidEvent.stringValue = ""
        devAction.stringValue = ""
        devAction2.stringValue = ""
        refreshDevicesTab()
    }

    //
    // Action methods (Misc)
    //

    @IBAction func selectDeviceAction(_ sender: Any!) {

        refresh()
    }

    @IBAction func devPresetAction(_ sender: NSPopUpButton!) {
        
        assert(sender.selectedTag() == 0)

        // Reset the database
        myAppDelegate.database.reset()

        // Make the change effective
        gamePadManager.updateHidMapping()

        refresh()
    }
}

extension PreferencesController : NSTextViewDelegate {

    func textDidChange(_ notification: Notification) {

        if let textView = notification.object as? NSTextView {

            print("Text changed: \(textView.string)")

            // Add the update device description to the 'custom' database
            db.update(line: textView.string)

            // Make the change effective
            gamePadManager.updateHidMapping()
        }
    }

}
