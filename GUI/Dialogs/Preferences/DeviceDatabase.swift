// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* This class implements a data provider for Game pads connected via USB or
 * Bluetooth. It is build around the gamecontrollerdb.txt file hosted at:
 *
 *   https://github.com/mdqinc/SDL_GameControllerDB
 *
 * The file contains SDL compatible controller mappings such as the following:
 *
 *   030000000b0400003365000000010000,Competitio Pro,
 *     a:b0,b:b1,back:b2,leftx:a0,lefty:a1,start:b3,platform:Linux,
 *
 * Each entry starts with a GUID of the following format (from SDL_joystick.c):
 *
 *   16-bit bus
 *   16-bit CRC16 of the joystick name (can be zero)
 *   16-bit vendor ID
 *   16-bit zero
 *   16-bit product ID
 *   16-bit zero
 *   16-bit version
 *   8-bit driver identifier ('h' for HIDAPI, 'x' for XInput, etc.)
 *   8-bit driver-dependent type info
 *
 * When a game pad is attached to the emulator, the database is searched for
 * a matching vendor ID, product ID, and version. All other fields are ignored.
 * If no perfect match is found, the search is repeated by only comparing the
 * vendor ID and product ID.
 */

// Mapping scheme: HIDEvent -> Item -> Value -> [Actions]
typealias HIDMapping = [ HIDEvent: [ Int: [ Int: [GamePadAction] ] ] ]

class DeviceDatabase {

    // Known devices
    var known: [GUID: String] = [:]

    // Devices configured by the user
    // var custom: [String: String] = [:]

    //
    // Initializing
    //

    init() {

        reset()
    }

    func reset() {

        // Start from scratch
        known = [:]
        // custom = [:]

        // Register all known devices
        parse(file: "gamecontrollerdb", withExtension: "txt")
    }

    //
    // Creating the database
    //

    func parse(file: String, withExtension ext: String) {

        if let url = Bundle.main.url(forResource: file, withExtension: ext) {

            do {

                let fileContents = try String(contentsOf: url, encoding: .utf8)
                for line in fileContents.split(separator: "\n") {
                    self.parse(line: String(line))
                }

            } catch { print("Error reading file: \(error)") }
        }
    }

    func parse(line: String) {

        // Eliminate newline characters (if any)
        var descriptor = line.replacingOccurrences(of: "\n", with: "")

        // Eliminate unneeded commas at both ends
        descriptor = descriptor.trimmingCharacters(in: CharacterSet(charactersIn: ","))

        // Extract the GUID and create a mapping
        if let guid = GUID(string: descriptor) { known[guid] = descriptor }
    }

    func update(line: String) {

        parse(line: line)
    }

    //
    // Querying the database
    //

    func hasMatch(guid: GUID) -> Bool {

        return hasMatch(vendorID: guid.vendorID, productID: guid.productID)
    }

    func hasPerfectMatch(guid: GUID) -> Bool {

        return hasMatch(vendorID: guid.vendorID, productID: guid.productID, version: guid.version)
    }

    func hasMatch(vendorID: Int, productID: Int, version: Int? = nil) -> Bool {

        for (guid, _) in known {

            if guid.vendorID == vendorID && guid.productID == productID {
                if version == nil || guid.version == version { return true }
            }
        }

        return false
    }

    func seek(guid: GUID) -> String? {

        for (otherguid, result) in known {

            // Compare vendorID, productID, and version
            if !guid.match(guid: otherguid, offset: 8, length: 4) { continue }
            if !guid.match(guid: otherguid, offset: 16, length: 4) { continue }
            if !guid.match(guid: otherguid, offset: 24, length: 4) { continue }

            return result;
        }

        for (otherguid, result) in known {

            // Only compare the vendorID and productID
            if !guid.match(guid: otherguid, offset: 8, length: 4) { continue }
            if !guid.match(guid: otherguid, offset: 16, length: 4) { continue }

            return result;
        }

        return nil
    }

    /*
    func seekDevice(vendorID: String?, productID: String?, version: String? = nil) -> String? {

        func parse(_ value: String?) -> Int? {
            return value == nil ? nil : value == "" ? 0 : Int(value!)
        }

        return seekDevice(vendorID: parse(vendorID),
                          productID: parse(productID),
                          version: parse(version),
                          mapping: mapping)

    }
    */

    func seek(vendorID: Int?, productID: Int?, version: Int? = nil) -> String? {

        print("seek(\(vendorID), \(productID), \(version))")

        for (guid, value) in known {

            if vendorID != nil && vendorID != guid.vendorID { continue }
            if productID != nil && productID != guid.productID { continue }
            if version != nil && version != guid.version { continue }

            return value
        }
        return nil
    }

    func query(guid: GUID) -> HIDMapping {

        var result: HIDMapping = [

            .AXIS: [:],
            .BUTTON: [:],
            .DPAD_UP: [0:[0:[.PULL_UP], 1:[.RELEASE_Y]]],
            .DPAD_DOWN: [0:[0:[.PULL_DOWN], 1:[.RELEASE_Y]]],
            .DPAD_RIGHT: [0:[0:[.PULL_RIGHT], 1:[.RELEASE_X]]],
            .DPAD_LEFT: [0:[0:[.PULL_LEFT], 1:[.RELEASE_X]]],
            .HATSWITCH: [:]
        ]

        // Crawl through the database
        var descriptor = seek(guid: guid)

        // Assign a default descriptor if needed
        if descriptor == nil { descriptor = "Generic,a:b0,b:b1,leftx:a0,lefty:a1" }

        print("\(descriptor)")

        // Iterate through all key value pairs
        for assignment in descriptor!.split(separator: ",") {

            let pair = assignment.split(separator: ":").map { String($0) }
            if pair.count != 2 { continue }

            let key = pair[0]
            let value = pair[1]

            print("key: \(pair[0]) value: \(pair[1])")

            switch (value) {
            case "a0":  result[.AXIS]![0] = mapAxis(key: key)
            case "a1":  result[.AXIS]![1] = mapAxis(key: key)
            case "a2":  result[.AXIS]![2] = mapAxis(key: key)
            case "a3":  result[.AXIS]![3] = mapAxis(key: key)
            case "a4":  result[.AXIS]![4] = mapAxis(key: key)
            case "a5":  result[.AXIS]![5] = mapAxis(key: key)
            case "a0~": result[.AXIS]![0] = mapAxisRev(key: key)
            case "a1~": result[.AXIS]![1] = mapAxisRev(key: key)
            case "a2~": result[.AXIS]![2] = mapAxisRev(key: key)
            case "a3~": result[.AXIS]![3] = mapAxisRev(key: key)
            case "a4~": result[.AXIS]![4] = mapAxisRev(key: key)
            case "a5~": result[.AXIS]![5] = mapAxisRev(key: key)
            case "b0":  result[.BUTTON]![1] = mapButton(key: key)
            case "b1":  result[.BUTTON]![2] = mapButton(key: key)
            case "b2":  result[.BUTTON]![3] = mapButton(key: key)
            case "b3":  result[.BUTTON]![4] = mapButton(key: key)
            case "b4":  result[.BUTTON]![5] = mapButton(key: key)
            case "b5":  result[.BUTTON]![6] = mapButton(key: key)
            case "b6":  result[.BUTTON]![7] = mapButton(key: key)
            case "b7":  result[.BUTTON]![8] = mapButton(key: key)
            case "b8":  result[.BUTTON]![9] = mapButton(key: key)
            case "b9":  result[.BUTTON]![10] = mapButton(key: key)
            case "b10": result[.BUTTON]![11] = mapButton(key: key)
            case "b11": result[.BUTTON]![12] = mapButton(key: key)
            case "b12": result[.BUTTON]![13] = mapButton(key: key)
            case "b13": result[.BUTTON]![14] = mapButton(key: key)
            case "b14": result[.BUTTON]![15] = mapButton(key: key)
            case "b15": result[.BUTTON]![16] = mapButton(key: key)
            case "b16": result[.BUTTON]![17] = mapButton(key: key)

            default:
                break
            }
        }

        return result
    }

    private func mapAxis(key: String) -> [Int: [GamePadAction]] {

        switch (key) {

        case "leftx", "rightx": return [-1: [.PULL_LEFT], 0: [.RELEASE_X], 1: [.PULL_RIGHT]]
        case "lefty", "righty": return [-1: [.PULL_UP], 0: [.RELEASE_Y], 1: [.PULL_DOWN]]

        default:
            return [:]
        }
    }

    private func mapAxisRev(key: String) -> [Int: [GamePadAction]] {

        switch (key) {

        case "leftx", "rightx": return [-1: [.PULL_RIGHT], 0: [.RELEASE_X], 1: [.PULL_LEFT]]
        case "lefty", "righty": return [-1: [.PULL_DOWN], 0: [.RELEASE_Y], 1: [.PULL_UP]]

        default:
            return [:]
        }
    }

    private func mapButton(key: String) -> [Int: [GamePadAction]] {

        switch (key) {

        case "a", "b", "leftshoulder", "rightshoulder": return [0: [.RELEASE_FIRE], 1: [.PRESS_FIRE]]
        case "dpdown": return [0: [.RELEASE_Y], 1: [.PULL_DOWN]]
        case "dpup": return [0: [.RELEASE_Y], 1: [.PULL_UP]]
        case "dpleft": return [0: [.RELEASE_X], 1: [.PULL_LEFT]]
        case "dpright": return [0: [.RELEASE_X], 1: [.PULL_RIGHT]]

        default:
            return [:]
        }
    }
}
