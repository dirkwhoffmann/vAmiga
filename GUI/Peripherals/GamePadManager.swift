// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* Holds and manages an array of GamePad objects.
 * Up to five gamepads are managed. The first three gamepads are initialized
 * by default and represent a mouse and two keyboard emulated joysticks.
 * All remaining gamepads are added dynamically when a HID device is connected.
 */
class GamePadManager {

    // Reference to the the controller
    var parent: MyController!
    
    // Reference to the HID manager
    var hidManager: IOHIDManager

    // private let inputLock = NSLock()
    // Such a thing is used here: TODO: Check if we need this
    // https://github.com/joekarl/swift_handmade_hero/blob/master/Handmade%20Hero%20OSX/Handmade%20Hero%20OSX/InputManager.swift
    
    // Gamepad storage
    var gamePads: [Int: GamePad] = [:]
    
    //
    // Initialization
    //
    
    init(parent: MyController) {
    
        self.parent = parent
      
        hidManager = IOHIDManagerCreate(kCFAllocatorDefault,
                                        IOOptionBits(kIOHIDOptionsTypeNone))

        // Add default devices
        gamePads[0] = GamePad(0, manager: self)     // Mouse
        gamePads[0]!.keyMap = 0                     // Mouse keyset
        gamePads[0]!.isMouse = true

        gamePads[1] = GamePad(1, manager: self)     // Joystick
        gamePads[1]!.keyMap = 1                     // Joystick keyset 1

        gamePads[2] = GamePad(2, manager: self)     // Joystick
        gamePads[2]!.keyMap = 2                     // Joystick keyset 2

        // Prepare to accept HID devices
        let deviceCriteria = [
            [
                kIOHIDDeviceUsagePageKey: kHIDPage_GenericDesktop,
                kIOHIDDeviceUsageKey: kHIDUsage_GD_Joystick
            ],
            [
                kIOHIDDeviceUsagePageKey: kHIDPage_GenericDesktop,
                kIOHIDDeviceUsageKey: kHIDUsage_GD_GamePad
            ],
            [
                kIOHIDDeviceUsagePageKey: kHIDPage_GenericDesktop,
                kIOHIDDeviceUsageKey: kHIDUsage_GD_MultiAxisController
            ],
            [
                kIOHIDDeviceUsagePageKey: kHIDPage_GenericDesktop,
                kIOHIDDeviceUsageKey: kHIDUsage_GD_Mouse
            ]
        ]
                
        // Declare bridging closures (needed to bridge between Swift methods and C callbacks)
        let matchingCallback: IOHIDDeviceCallback = { inContext, inResult, inSender, device in
            let this: GamePadManager = unsafeBitCast(inContext, to: GamePadManager.self)
            this.hidDeviceAdded(context: inContext, result: inResult, sender: inSender, device: device)
        }
        
        let removalCallback: IOHIDDeviceCallback = { inContext, inResult, inSender, device in
            let this: GamePadManager = unsafeBitCast(inContext, to: GamePadManager.self)
            this.hidDeviceRemoved(context: inContext, result: inResult, sender: inSender, device: device)
        }
        
        // Configure the HID manager
        let hidContext = unsafeBitCast(self, to: UnsafeMutableRawPointer.self)
        IOHIDManagerSetDeviceMatchingMultiple(hidManager, deviceCriteria as CFArray)
        IOHIDManagerRegisterDeviceMatchingCallback(hidManager, matchingCallback, hidContext)
        IOHIDManagerRegisterDeviceRemovalCallback(hidManager, removalCallback, hidContext)
        IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), CFRunLoopMode.defaultMode.rawValue)
        IOHIDManagerOpen(hidManager, IOOptionBits(kIOHIDOptionsTypeNone))
    }
    
    func shutDown() {

        track()
        
        // Terminate communication with all connected HID devices
        for (_, pad) in gamePads { pad.close() }

        // Close the HID manager
        IOHIDManagerClose(hidManager, IOOptionBits(kIOHIDOptionsTypeNone))

        // Free all slots
        gamePads = [:]
    }

    deinit {

        track()
    }
        
    //
    // Managing slots
    //
    
    // Returns true iff the specified game pad slot is free
    public func slotIsEmpty(_ nr: Int) -> Bool {
        
        return gamePads[nr] == nil
    }
    
    // Returns the lowest free slot number or nil if all slots are occupied
    func findFreeSlot() -> Int? {
        
        var nr = 0
        while !slotIsEmpty(nr) { nr += 1 }
        
        // We support up to 5 devices
        return (nr < 5) ? nr : nil
    }
    
    func connect(slot: Int, port: Int) {
        
        // Remove any existing binding to this port
        for (_, pad) in gamePads where pad.port == port { pad.port = 0 }
        
        // Bind the new device
        gamePads[slot]?.port = port
        
        // Connect the proper device on the Amiga side
        let cpd: ControlPortDevice =
            slot == InputDevice.none ? CPD_NONE :
                slot == InputDevice.mouse ? CPD_MOUSE : CPD_JOYSTICK
        
        parent.amiga.suspend()
        if port == 1 { parent.amiga.controlPort1.connect(cpd) }
        if port == 2 { parent.amiga.controlPort2.connect(cpd) }
        parent.amiga.resume()
    }
    
    func slotConnectedTo(port: Int) -> Int {
        
        var result = InputDevice.none
        
        for (slot, pad) in gamePads where pad.port == port {
            assert(result == InputDevice.none)
            result = slot
        }
        
        return result
    }
        
    //
    // HID stuff
    //
    
    func isBuiltIn(device: IOHIDDevice) -> Bool {
        
        let key = kIOHIDBuiltInKey as CFString
        
        if let value = IOHIDDeviceGetProperty(device, key) as? Int {
            return value != 0
        } else {
            return false
        }
    }
    
    /*
    func isMouse(device: IOHIDDevice) -> Bool {
        
        let key = kIOHIDPrimaryUsageKey as CFString
        
        if let value = IOHIDDeviceGetProperty(device, key) as? Int {
            return value == kHIDUsage_GD_Mouse
        } else {
            return false
        }
    }
    */
    
    func listProperties(device: IOHIDDevice) {
        
        let keys = [kIOHIDTransportKey, kIOHIDVendorIDKey, kIOHIDVendorIDSourceKey, kIOHIDProductIDKey, kIOHIDVersionNumberKey, kIOHIDManufacturerKey, kIOHIDProductKey, kIOHIDSerialNumberKey, kIOHIDCountryCodeKey, kIOHIDStandardTypeKey, kIOHIDLocationIDKey, kIOHIDDeviceUsageKey, kIOHIDDeviceUsagePageKey, kIOHIDDeviceUsagePairsKey, kIOHIDPrimaryUsageKey, kIOHIDPrimaryUsagePageKey, kIOHIDMaxInputReportSizeKey, kIOHIDMaxOutputReportSizeKey, kIOHIDMaxFeatureReportSizeKey, kIOHIDReportIntervalKey, kIOHIDSampleIntervalKey, kIOHIDBatchIntervalKey, kIOHIDRequestTimeoutKey, kIOHIDReportDescriptorKey, kIOHIDResetKey, kIOHIDKeyboardLanguageKey, kIOHIDAltHandlerIdKey, kIOHIDBuiltInKey, kIOHIDDisplayIntegratedKey, kIOHIDProductIDMaskKey, kIOHIDProductIDArrayKey, kIOHIDPowerOnDelayNSKey, kIOHIDCategoryKey, kIOHIDMaxResponseLatencyKey, kIOHIDUniqueIDKey, kIOHIDPhysicalDeviceUniqueIDKey]
        
        for key in keys {
            if let prop = IOHIDDeviceGetProperty(device, key as CFString) {
                print("\t" + key + ": \(prop)")
            }
        }
    }
    
    // Device matching callback
    // This method is invoked when a matching HID device is plugged in.
    func hidDeviceAdded(context: UnsafeMutableRawPointer?,
                        result: IOReturn,
                        sender: UnsafeMutableRawPointer?,
                        device: IOHIDDevice) {
    
        track()

        // listProperties(device: device)
        
        // Ignore internal devices
        if isBuiltIn(device: device) {
            // track("Ignoring built-in device")
            return
        }
        
        // Find a free slot for the new device
        guard let slotNr = findFreeSlot() else {
            track("Maximum number of devices reached. Ignoring device")
            return
        }
        
        // Create GamePad object
        let vendorIDKey = kIOHIDVendorIDKey as CFString
        let productIDKey = kIOHIDProductIDKey as CFString
        let locationIDKey = kIOHIDLocationIDKey as CFString
        var vendorID = 0
        var productID = 0
        var locationID = 0
        
        if let value = IOHIDDeviceGetProperty(device, vendorIDKey) as? Int {
            vendorID = value
        }
        if let value = IOHIDDeviceGetProperty(device, productIDKey) as? Int {
            productID = value
        }
        if let value = IOHIDDeviceGetProperty(device, locationIDKey) as? Int {
            locationID = value
        }
        
        track("    slotNr = \(slotNr)")
        track("  vendorID = \(vendorID)")
        track(" productID = \(productID)")
        track("locationID = \(locationID)")

        // Open device
        let optionBits = kIOHIDOptionsTypeNone // kIOHIDOptionsTypeSeizeDevice
        let status = IOHIDDeviceOpen(device, IOOptionBits(optionBits))
        if status != kIOReturnSuccess {
            track("WARNING: Cannot open HID device")
            return
        }

        // Create a GamePad object for this device
        gamePads[slotNr] = GamePad(slotNr,
                                   manager: self,
                                   device: device,
                                   vendorID: vendorID,
                                   productID: productID,
                                   locationID: locationID)
    
        // Register input value callback
        let hidContext = unsafeBitCast(gamePads[slotNr], to: UnsafeMutableRawPointer.self)
        if device.isMouse() {
            track("Registering as Mouse")
            IOHIDDeviceRegisterInputValueCallback(device,
                                                  gamePads[slotNr]!.mouseActionCallback,
                                                  hidContext)
        } else {
            track("Registering as Joystick")
            IOHIDDeviceRegisterInputValueCallback(device,
                                                  gamePads[slotNr]!.joystickActionCallback,
                                                  hidContext)
        }
        
        // Inform the controller about the new device
        parent.toolbar.validateVisibleItems()
        
        listDevices()
    }
    
    func hidDeviceRemoved(context: UnsafeMutableRawPointer?,
                          result: IOReturn,
                          sender: UnsafeMutableRawPointer?,
                          device: IOHIDDevice) {
        
        track()
    
        let locationIDKey = kIOHIDLocationIDKey as CFString
        var locationID = 0
        if let value = IOHIDDeviceGetProperty(device, locationIDKey) as? Int {
            locationID = value
        }
                
        // Search for a matching locationID and remove device
        for (slotNr, device) in gamePads where device.locationID == locationID {
            gamePads[slotNr] = nil
            track("Clearing slot \(slotNr)")
        }
                
        // Inform the controller about the new device
        parent.toolbar.validateVisibleItems()
        
        listDevices()
    }
    
    func listDevices() {
        
        print("Input devices:")
        for i in 0 ... Int.max {
            
            guard let dev = gamePads[i] else { break }
            
            print("Slot \(i) [\(dev.port)]: ", terminator: "")
            if let name = dev.name {
                print("\(name) (\(dev.vendorID), \(dev.productID), \(dev.locationID))", terminator: "")
            } else {
                print("Placeholder device", terminator: "")
            }
            print(dev.isMouse ? " (Mouse)" : "")
        }
    }
}

extension IOHIDDevice {
    
    func isMouse() -> Bool {
        
        let key = kIOHIDPrimaryUsageKey as CFString
        
        if let value = IOHIDDeviceGetProperty(self, key) as? Int {
            return value == kHIDUsage_GD_Mouse
        } else {
            return false
        }
    }
}
