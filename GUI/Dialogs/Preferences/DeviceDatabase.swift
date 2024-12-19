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

/*
enum HIDEvent {

    case HID_VOID
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
*/

struct MyData {

    var name = "Generic"

    var vendorID = 0
    var productID = 0
    var version = 0

    // Button actions
    var b: [Int: ([GamePadAction],[GamePadAction])] = [:]

    // Axis actions
    var a0: ([GamePadAction],[GamePadAction],[GamePadAction]) = ([],[],[])
    var a1: ([GamePadAction],[GamePadAction],[GamePadAction]) = ([],[],[])
    var a2: ([GamePadAction],[GamePadAction],[GamePadAction]) = ([],[],[])
    var a3: ([GamePadAction],[GamePadAction],[GamePadAction]) = ([],[],[])
    var a4: ([GamePadAction],[GamePadAction],[GamePadAction]) = ([],[],[])
    var a5: ([GamePadAction],[GamePadAction],[GamePadAction]) = ([],[],[])
}

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

    func seekDevice(vendorID: String?, productID: String?, version: String? = nil) -> String? {

        func parse(_ value: String?) -> Int? {
            return value == nil ? nil : value == "" ? 0 : Int(value!)
        }

        return seekDevice(vendorID: parse(vendorID), productID: parse(productID), version: parse(version))
    }

    func seekDevice(vendorID: Int?, productID: Int?, version: Int? = nil) -> String? {

        print("seekDevice(\(vendorID), \(productID), \(version))")

        var result: String?

        if let url = Bundle.main.url(forResource: "gamecontrollerdb2", withExtension: "txt") {

            do {

                let fileContents = try String(contentsOf: url, encoding: .utf8)

                fileContents.enumerateLines { line, _ in

                    func hex(_ i: Int) -> Int {

                        let start = line.index(line.startIndex, offsetBy: i)
                        let end = line.index(start, offsetBy: 2)
                        return Int(line[start..<end], radix: 16) ?? 0
                    }

                    if line.hasPrefix("0") && line.count > 28 {

                        // print("line = \(line)")

                        /* From SDL_joystick.c
                         *
                         * This GUID fits the standard form:
                         * 16-bit bus
                         * 16-bit CRC16 of the joystick name (can be zero)
                         * 16-bit vendor ID
                         * 16-bit zero
                         * 16-bit product ID
                         * 16-bit zero
                         * 16-bit version
                         * 8-bit driver identifier ('h' for HIDAPI, 'x' for XInput, etc.)
                         * 8-bit driver-dependent type info
                         */
                        let _vendorID = hex(8) | hex(10) << 8;
                        let _productID = hex(16) | hex(18) << 8;
                        let _version = hex(24) | hex(26) << 8;

                        if (vendorID == nil || vendorID == _vendorID) &&
                            (productID == nil || productID == _productID) &&
                            (version == nil || version == _version) {

                            result = line
                        }
                    }
                }

            } catch { print("Error reading file: \(error)") }
        }

        print("result = \(result)")
        return result
    }

    func query(vendorID: String, productID: String, version: String) -> MyData {

        var result = MyData()

        if let descr =
            seekDevice(vendorID: vendorID, productID: productID, version: version) ??
            seekDevice(vendorID: vendorID, productID: productID) ??
            seekDevice(vendorID: 0, productID: 0) {

            print("\(descr)")

            let attributes = descr.split(separator: ",").map { String($0) }
            for attr in attributes {

                let pair = attr.split(separator: ":").map { String($0) }
                if pair.count == 2 {

                    let key = pair[0]
                    let value = pair[1]

                    print("key: \(pair[0]) value: \(pair[1])")

                    switch (value) {
                    case "a0": result.a0 = mapAxis(key: key)
                    case "a1": result.a1 = mapAxis(key: key)
                    case "a2": result.a2 = mapAxis(key: key)
                    case "a3": result.a3 = mapAxis(key: key)
                    case "a4": result.a4 = mapAxis(key: key)
                    case "a5": result.a5 = mapAxis(key: key)
                    case "a0~": result.a0 = mapAxisRev(key: key)
                    case "a1~": result.a1 = mapAxisRev(key: key)
                    case "a2~": result.a2 = mapAxisRev(key: key)
                    case "a3~": result.a3 = mapAxisRev(key: key)
                    case "a4~": result.a4 = mapAxisRev(key: key)
                    case "a5~": result.a5 = mapAxisRev(key: key)
                    default:
                        break
                    }

                    if value.hasPrefix("b") {

                        let secondCharacter = value[value.index(value.startIndex, offsetBy: 1)]
                        if let index = Int(String(secondCharacter)) {
                            result.b[index + 1] = mapButton(key: key) }
                    }
                }
            }
            // print("attributes = \(attributes)")
        }

        print("result = \(result)")
        return result
    }

    private func mapAxis(key: String) -> ([GamePadAction],[GamePadAction],[GamePadAction]) {

        switch (key) {

        case "leftx": return ([.PULL_LEFT], [.RELEASE_X], [.PULL_RIGHT])
        case "rightx": return ([.PULL_LEFT], [.RELEASE_X], [.PULL_RIGHT])
        case "lefty": return ([.PULL_UP], [.RELEASE_Y], [.PULL_DOWN])
        case "righty": return ([.PULL_UP], [.RELEASE_Y], [.PULL_DOWN])

        default:
            return ([],[],[])
        }
    }

    private func mapAxisRev(key: String) -> ([GamePadAction],[GamePadAction],[GamePadAction]) {

        switch (key) {

        case "leftx": return ([.PULL_RIGHT], [.RELEASE_X], [.PULL_LEFT])
        case "rightx": return ([.PULL_RIGHT], [.RELEASE_X], [.PULL_RIGHT])
        case "lefty": return ([.PULL_DOWN], [.RELEASE_Y], [.PULL_UP])
        case "righty": return ([.PULL_DOWN], [.RELEASE_Y], [.PULL_UP])

        default:
            return ([],[],[])
        }
    }

    private func mapButton(key: String) -> ([GamePadAction],[GamePadAction]) {

        switch (key) {

        case "a": return ([.PRESS_FIRE], [.RELEASE_FIRE])
        case "b": return ([.PRESS_FIRE], [.RELEASE_FIRE])

        default:
            return ([],[])
        }
    }


    /*
     func query(vendorID: Int, productID: Int, version: Int) -> MyData {

     var bestMatch = data[0]

     for entry in data {

     if entry.vendorID != vendorID { continue }
     if entry.productID != productID { continue }

     bestMatch = entry

     if entry.version != version { continue }

     bestMatch = entry

     }

     return bestMatch
     }
     */


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

    /*
    func isKnown(vendorID: String, productID: String) -> Bool {
        
        return known[vendorID]?[productID] != nil
    }
    */
    
    func query(_ v: String, _ p: String, _ key: String) -> String? {
        
        if let value = custom[v]?[p]?[key] {
            return value
        }
        if let value = known[v]?[p]?[key] {
            return value
        }
        return nil
    }

    /*
    func name(vendorID: String, productID: String) -> String? {
        
        if let value = query(vendorID, productID, "Name") {
            return value
        }
        return nil
    }
    */
    
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
