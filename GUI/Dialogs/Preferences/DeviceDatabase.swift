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

    var isGeneric: Bool { return vendorID == 0 && productID == 0 && version == 0 }
}

class DeviceDatabase {

    // Parsed mapping file
    var database: [String: String] = [:]


    // Mapping scheme ( VendorID -> (ProductID -> Dictionary) )
    // typealias DeviceDescription = [ String: [ String: [ String: String ] ] ]

    // Known devices
    // var known: DeviceDescription = [:]

    // Devices configured by the user
    // var custom: DeviceDescription = [:]

    init() {

        reset()
    }

    func reset() {

        database = [:]
        parse(file: "gamecontrollerdb")
    }

    func parse(file: String) {

        if let url = Bundle.main.url(forResource: file, withExtension: "txt") {

            do {

                let fileContents = try String(contentsOf: url, encoding: .utf8)
                fileContents.enumerateLines { line, _ in self.parse(line: line) }

            } catch { print("Error reading file: \(error)") }
        }

        for (key, value) in database.prefix(10) {
            print("\(key): \(value)")
        }
    }

    func parse(line: String) {

        // Eliminate newline characters (if any)
        var descriptor = line.replacingOccurrences(of: "\n", with: "")

        // Eliminate unneeded commas
        descriptor = descriptor.trimmingCharacters(in: CharacterSet(charactersIn: ","))

        // Split off the GUID and fill the database
        if let range = descriptor.range(of: ",") {

            let prefix = String(descriptor[..<range.lowerBound])
            if prefix.hasPrefix("0") && prefix.count > 28 {

                self.database[prefix] = descriptor
            }
        }
    }

    //
    // Querying the database
    //

    func seekDevice(vendorID: String, productID: String, version: String) -> String? {

        func parse(_ value: String?) -> Int? {
            return value == nil ? nil : value == "" ? 0 : Int(value!)
        }

        let vend = parse(vendorID)
        let prod = parse(productID)
        let vers = parse(version)

        return seekDevice(vendorID: vend, productID: prod, version: vers) ??
        seekDevice(vendorID: vend, productID: prod) ?? nil
    }

    func seekDevice(vendorID: Int?, productID: Int?, version: Int? = nil) -> String? {

        print("seekDevice(\(vendorID), \(productID), \(version))")

        // var result: String?

        for (key, value) in database {

            func hex(_ i: Int) -> Int {

                let start = key.index(key.startIndex, offsetBy: i)
                let end = key.index(start, offsetBy: 2)
                return Int(key[start..<end], radix: 16) ?? 0
            }

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

                return value
            }
        }

        return "Generic,a:b0,b:b1,leftx:a0,lefty:a1"
    }

    func query(vendorID: String, productID: String, version: String) -> MyData {

        var result = MyData()

        if let descr = seekDevice(vendorID: vendorID, productID: productID, version: version) { // ??
            // seekDevice(vendorID: vendorID, productID: productID) ??
            // seekDevice(vendorID: 0, productID: 0) {

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
                    case "b0": result.b[1] = mapButton(key: key)
                    case "b1": result.b[2] = mapButton(key: key)
                    case "b2": result.b[3] = mapButton(key: key)
                    case "b3": result.b[4] = mapButton(key: key)
                    case "b4": result.b[5] = mapButton(key: key)
                    case "b5": result.b[6] = mapButton(key: key)
                    case "b6": result.b[7] = mapButton(key: key)
                    case "b7": result.b[8] = mapButton(key: key)
                    case "b8": result.b[9] = mapButton(key: key)
                    case "b9": result.b[10] = mapButton(key: key)
                    case "b10": result.b[11] = mapButton(key: key)
                    case "b11": result.b[12] = mapButton(key: key)
                    case "b12": result.b[13] = mapButton(key: key)
                    case "b13": result.b[14] = mapButton(key: key)
                    case "b14": result.b[15] = mapButton(key: key)
                    case "b15": result.b[16] = mapButton(key: key)
                    case "b16": result.b[17] = mapButton(key: key)
                    default:
                        break
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
        case "rightx": return ([.PULL_RIGHT], [.RELEASE_X], [.PULL_LEFT])
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
        case "leftshoulder": return ([.PRESS_FIRE], [.RELEASE_FIRE])
        case "rightshoulder": return ([.PRESS_FIRE], [.RELEASE_FIRE])
        case "lefttrigger": return ([.PRESS_FIRE], [.RELEASE_FIRE])
        case "righttrigger": return ([.PRESS_FIRE], [.RELEASE_FIRE])
        case "dpdown": return ([.PULL_DOWN], [.RELEASE_Y])
        case "dpup": return ([.PULL_UP], [.RELEASE_Y])
        case "dpleft": return ([.PULL_LEFT], [.RELEASE_X])
        case "dpright": return ([.PULL_RIGHT], [.RELEASE_X])
        default:
            return ([],[])
        }
    }
}
