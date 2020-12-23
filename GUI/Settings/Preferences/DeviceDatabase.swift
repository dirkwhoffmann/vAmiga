// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* This class represents a small data bases storing information about external
 * devices such as Game pads or Joysticks (in general: every device that can
 * be connected via USB or Blutooth).
 *
 * An object of this class is used inside the PreferencesController
 */

class DeviceDatabase {
 
    // Mapping scheme for database entries
    typealias DeviceDescription = [ String: [ String: [ String: String ] ] ]
    
    // Known devices ( VendorID -> (ProductID -> Dictionary) )
    var known: DeviceDescription = [:]
    
    // Devices configured by the user ( VendorID -> (ProductID -> Dictionary) )
    var custom: DeviceDescription = [:]
        
    init() {
    
        track()
        
        // Register known devices
        known =
            ["4":
                ["1": ["Name": "aJoy Retro Adapter"]],
             "121":
                ["17": ["Name": "iNNEXT Retro (SNES)"]],
             "1035":
                ["25907": ["Name": "Competition Pro SL-6602"]],
             "1118":
                ["2835": ["Name": "XBox Carbon Black", "HatSwitch": "1"]],
             "1133":
                ["0": ["Name": "Logitech Mouse"]],
             "1155":
                ["36869": ["Name": "RetroFun! Joystick Adapter"]],
             "1356":
                ["616": ["Name": "Sony DualShock 3"],
                 "1476": ["Name": "Sony DualShock 4"],
                 "2508": ["Name": "Sony Dualshock 4 (2nd Gen)"]],
             "1848":
                ["8727": ["Name": "Competition Pro SL-650212"]],
             "3853":
                ["193": ["Name": "HORIPAD for Nintendo Switch"]],
             "7257":
                ["36": ["Name": "The C64 Joystick"]]
            ]
        
        // Load custom device information
        let defaults = UserDefaults.standard
        if let obj = defaults.object(forKey: Keys.Dev.schemes) as? DeviceDescription {
            custom = obj
        }
        
        track("\(custom)")
    }
    
    func save() {
        
        track()
        
        // Save mapping schemes
        let defaults = UserDefaults.standard
        defaults.set(custom, forKey: Keys.Dev.schemes)
        
        track("\(custom)")
    }
    
    //
    // Querying the database
    //
    
    func query(_ vendorID: Int, _ productID: Int, _ key: String) -> String? {
        
        return query("\(vendorID)", "\(productID)", key)
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

    func image(vendorID: String, productID: String) -> NSImage? {
    
        if let value = query(vendorID, productID, "Image") {
            return NSImage(named: value)
        }
        return nil
    }

    func left(vendorID: String, productID: String) -> Int {
    
        if let value = query(vendorID, productID, "Left") {
            return Int.init(value) ?? 0
        }
        return 0
    }
    
    func right(vendorID: String, productID: String) -> Int {
    
        if let value = query(vendorID, productID, "Right") {
            return Int.init(value) ?? 0
        }
        return 0
    }

    func hatSwitch(vendorID: String, productID: String) -> Int {
    
        if let value = query(vendorID, productID, "HatSwitch") {
            return Int.init(value) ?? 0
        }
        return 0
    }

    //
    // Updating the database
    //
    
    func replace(_ vendorID: Int, _ productID: Int, _ key: String, _ value: String?) {

        replace("\(vendorID)", "\(productID)", key, value)
    }

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

    func setLeft(vendorID: Int, productID: Int, _ value: String?) {
        
        replace(vendorID, productID, "Left", value)
        track("\(custom)")
    }

    func setRight(vendorID: Int, productID: Int, _ value: String?) {
        
        replace(vendorID, productID, "Right", value)
        track("\(custom)")
    }

    func setHatSwitch(vendorID: Int, productID: Int, _ value: String?) {
        
        replace(vendorID, productID, "HatSwitch", value)
        track("\(custom)")
    }
}
