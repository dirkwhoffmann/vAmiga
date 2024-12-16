// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* This class implements a data provider for external devices such as GamePads
 * or Joysticks that can be connected via USB or Bluetooth.
 *
 * An object of this class is used inside the PreferencesController
 */

enum HIDEvent {

    case HID_A0
    case HID_A0_REV
    case HID_A1
    case HID_A1_REV
    case HID_A12
    case HID_A2
    case HID_A2_REV
    case HID_A20
    case HID_A3
    case HID_A3_REV
    case HID_A31
    case HID_A4
    case HID_A4_REV
    case HID_A5
    case HID_A5_REV
    case HID_A6
    case HID_A6_REV
    case HID_A7
    case HID_A8
    case HID_B0
    case HID_B1
    case HID_B10
    case HID_B11
    case HID_B12
    case HID_B13
    case HID_B14
    case HID_B15
    case HID_B16
    case HID_B17
    case HID_B19
    case HID_B2
    case HID_B20
    case HID_B21
    case HID_B22
    case HID_B23
    case HID_B24
    case HID_B25
    case HID_B26
    case HID_B27
    case HID_B3
    case HID_B4
    case HID_B5
    case HID_B6
    case HID_B7
    case HID_B8
    case HID_B9
}

struct MyData {

    var name = String()

    var vendorID: Int
    var productID: Int
    var version: Int

    var leftx = HIDEvent.HID_A0
    var rightx = HIDEvent.HID_A2
    var lefty = HIDEvent.HID_A1
    var righty = HIDEvent.HID_A3
    var button1 = HIDEvent.HID_B0
    var button2 = HIDEvent.HID_B1
}

let data = [
    MyData(name: "Generic", vendorID: 0, productID: 0, version: 0, leftx: .HID_A0, rightx: .HID_A2, lefty: .HID_A1, righty: .HID_A3, button1: .HID_B2, button2: .HID_B1),
    MyData(name: "Competition Pro", vendorID: 1035, productID: 25907, version: 256, leftx: .HID_A0, lefty: .HID_A1, button1: .HID_B0, button2: .HID_B1)
]


class DeviceDatabase {
 
    // Mapping scheme ( VendorID -> (ProductID -> Dictionary) )
    typealias DeviceDescription = [ String: [ String: [ String: String ] ] ]
    
    // Known devices
    var known: DeviceDescription = [:]
    
    // Devices configured by the user
    var custom: DeviceDescription = [:]
        
    init() {
    
        // Setup the lookup table for all known devices
        known =
            ["4":
                ["1": [:]],                     // aJoy Retro Adapter
             "13":
                ["0": ["L": "1", "R": "3", "H": "4"]], // Nimbus+
             "121":
                ["17": ["R": "1"],              // iNNEXT Retro (SNES)
                 "6354": [:]],                  // Mayflash Magic-NS 1.2
             "1035":
                ["25907": [:]],                 // Competition Pro SL-6602
             "1118":
                ["2835": ["H": "1"],            // XBox Carbon Black
                 "746": ["H": "3"]],            // XBox One Wired Controller
             "1133":
                ["49250": [:]],                 // Logitech Mouse
             "1155":
                ["36869": [:]],                 // RetroFun! Joystick Adapter
             "1356":
                ["616": [:],                    // Sony DualShock 3
                 "1476": [:],                   // Sony DualShock 4
                 "2508": [:],                   // Sony Dualshock 4 (2nd Gen)
                 "3302": [:]],                  // Sony DualSense
             "1848":
                ["8727": [:]],                  // Competition Pro SL-650212
             "3853":
                ["193": ["R": "1"]],            // HORIPAD for Nintendo Switch
             "7257":
                ["36": [:]]                     // The C64 Joystick
            ]
        
        // Load the lookup table for all custom devices
        let defaults = UserDefaults.standard
        if let obj = defaults.object(forKey: Keys.Dev.schemes) as? DeviceDescription {
            custom = obj
        }
    }

    //
    // Querying the database
    //

    func query(vendorID: String, productID: String, version: String) -> MyData {

        let vendor = Int(vendorID, radix: 10) ?? 0
        let product = Int(productID, radix: 10) ?? 0
        let version = Int(version, radix: 10) ?? 0

        return query(vendorID: vendor, productID: product, version: version)
    }

    func query(vendorID: Int, productID: Int, version: Int) -> MyData {

        for entry in data {

            if entry.vendorID != vendorID { continue }
            if entry.productID != productID { continue }
            if entry.version != version { continue }

            return entry
        }

        return data[0]
    }


    //
    // Old code
    //

    func save() {
        
        debug(.hid)
        
        let defaults = UserDefaults.standard
        defaults.set(custom, forKey: Keys.Dev.schemes)
        
        debug(.hid, "\(custom)")
    }

    //
    // Querying the database
    //
    
    func isKnown(vendorID: String, productID: String) -> Bool {
        
        return known[vendorID]?[productID] != nil
    }

    func query(_ v: String, _ p: String, _ key: String) -> String? {
        
        if let value = custom[v]?[p]?[key] {
            return value
        }
        if let value = known[v]?[p]?[key] {
            return value
        }
        return nil
    }

    func name(vendorID: String, productID: String) -> String? {
        
        if let value = query(vendorID, productID, "Name") {
            return value
        }
        return nil
    }

    func icon(vendorID: String, productID: String) -> NSImage? {
    
        if let value = query(vendorID, productID, "Image") {
            return NSImage(named: value)
        }
        return nil
    }

    func left(vendorID: String, productID: String) -> Int {
    
        if let value = query(vendorID, productID, "L") {
            return Int(value) ?? 0
        }
        return 0
    }
        
    func right(vendorID: String, productID: String) -> Int {
    
        if let value = query(vendorID, productID, "R") {
            return Int(value) ?? 0
        }
        return 0
    }

    func hatSwitch(vendorID: String, productID: String) -> Int {
    
        if let value = query(vendorID, productID, "H") {
            return Int(value) ?? 0
        }
        return 0
    }

    //
    // Updating the database
    //
    
    func replace(_ v: String, _ p: String, _ key: String, _ value: String?) {
        
        // Replace key / value pair if it already exists
        if custom[v]?[p] != nil { custom[v]![p]![key] = value; return }
        
        // Only proceed if there is something to set
        if value == nil { return }
        
        // Add a new key / value pair
        if custom[v] != nil {
            custom[v]![p] = [key: value!]
        } else {
            custom[v] = [p: [key: value!]]
        }
    }

    func setLeft(vendorID: String, productID: String, _ value: String?) {
        
        replace(vendorID, productID, "L", value)
    }

    func setRight(vendorID: String, productID: String, _ value: String?) {
        
        replace(vendorID, productID, "R", value)
    }

    func setHatSwitch(vendorID: String, productID: String, _ value: String?) {
        
        replace(vendorID, productID, "H", value)
    }
}
