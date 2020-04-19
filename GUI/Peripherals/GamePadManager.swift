// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* Holds and manages an array of GamePad objects
 * Up to five devices are managed. The first three are always present and
 * represent two keyset and an analog mouse. All other objects are dynamically
 * added when a USB joystick or game pad is plugged in.
 */
class GamePadManager {
    
    // private let inputLock = NSLock()
    // Such a thing is used here: TODO: Check if we need this
    // https://github.com/joekarl/swift_handmade_hero/blob/master/Handmade%20Hero%20OSX/Handmade%20Hero%20OSX/InputManager.swift
    
    // Reference to the the controller
    var controller: MyController!
    
    // Reference to the HID manager
    private var hidManager: IOHIDManager

    /* References to all registered game pads
     * Each device ist referenced by a slot number
     */
    var gamePads: [Int: GamePad] = [:]

    init() {

        hidManager = IOHIDManagerCreate(kCFAllocatorDefault, IOOptionBits(kIOHIDOptionsTypeNone))
        // super.init()
    }
    
    convenience init?(controller: MyController) {
        
        self.init()
        self.controller = controller
        
        // Add generic devices
        gamePads[0] = GamePad(0, manager: self)     // Mouse
        gamePads[0]!.keyMap = 0                     // Mouse keyset

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
        
        // Configure HID manager
        let hidContext = unsafeBitCast(self, to: UnsafeMutableRawPointer.self)
        IOHIDManagerSetDeviceMatchingMultiple(hidManager, deviceCriteria as CFArray)
        IOHIDManagerRegisterDeviceMatchingCallback(hidManager, matchingCallback, hidContext)
        IOHIDManagerRegisterDeviceRemovalCallback(hidManager, removalCallback, hidContext)
        IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), CFRunLoopMode.defaultMode.rawValue)
        IOHIDManagerOpen(hidManager, IOOptionBits(kIOHIDOptionsTypeNone))
    }
    
    deinit {
        track()
        IOHIDManagerClose(hidManager, IOOptionBits(kIOHIDOptionsTypeNone))
    }
    
    // Removes all registered devices
    func shutDown() {
        
        gamePads = [:]

    }
    
    //
    // Slot handling
    //
    
    // Returns true iff the specified game pad slot is free
    public func slotIsEmpty(_ nr: Int) -> Bool {
        
        return gamePads[nr] == nil
    }
    
    // Returns the lowest free slot number or nil if all slots are occupied
    func findFreeSlot() -> Int? {
        
        var nr = 0
        while !slotIsEmpty(nr) {
            nr += 1
        }
        
        // We support up to 5 devices
        return (nr < 5) ? nr : nil
    }
    
    // Looks up a game pad. Returns -1 if no game pad was found
    func lookupGamePad(_ gamePad: GamePad) -> Int {
        
        for (slotNr, device) in gamePads where device === gamePad {
            return slotNr
        }
        return -1
    }
    
    // Looks up game pad by locationID. Returns -1 if no game pad was found
    func lookupGamePad(locationID: Int) -> Int {
        
        for (slotNr, device) in gamePads where device.locationID == locationID {
            return slotNr
        }
        return -1
    }
        
    //
    // HID stuff
    //
    
    // Device matching callback
    // Method is invoked when a matching HID device is plugged in
    func hidDeviceAdded(context: UnsafeMutableRawPointer?,
                        result: IOReturn,
                        sender: UnsafeMutableRawPointer?,
                        device: IOHIDDevice) {
    
        track()
        
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
        
        gamePads[slotNr] = GamePad(slotNr,
                                   manager: self,
                                   vendorID: vendorID,
                                   productID: productID,
                                   locationID: locationID)

        track("    slotNr = \(slotNr)")
        track("  vendorID = \(vendorID)")
        track(" productID = \(productID)")
        track("locationID = \(locationID)")

        // Open HID device
        let optionBits = kIOHIDOptionsTypeNone // kIOHIDOptionsTypeSeizeDevice
        let status = IOHIDDeviceOpen(device, IOOptionBits(optionBits))
        if status != kIOReturnSuccess {
            track("WARNING: Cannot open HID device")
            return
        }
    
        // Register input value callback
        let hidContext = unsafeBitCast(gamePads[slotNr], to: UnsafeMutableRawPointer.self)
        IOHIDDeviceRegisterInputValueCallback(device, gamePads[slotNr]!.actionCallback, hidContext)

        // Inform controller
        controller.toolbar.validateVisibleItems()
        
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
        
        // let locationID = String(describing: IOHIDDeviceGetProperty(device, locationIDKey))
        
        // Search for a matching locationID and remove device
        for (slotNr, device) in gamePads where device.locationID == locationID {
            gamePads[slotNr] = nil
            track("Clearing slot \(slotNr)")
        }
        
        // Closing the HID device always fails.
        // Think, we don't have to close it, because it's disconnected anyway. Am I right?
        /* 
        let optionBits = kIOHIDOptionsTypeNone // kIOHIDOptionsTypeSeizeDevice
        let status = IOHIDDeviceClose(device, IOOptionBits(optionBits))
        if (status != kIOReturnSuccess) {
            track("WARNING: Cannot close HID device")
        }
        */
        
        // Inform controller
        controller.toolbar.validateVisibleItems()
        
        listDevices()
    }
    
    // Action method for events on a gamePad
    // Returns true, iff a joystick event has been triggered on port A or B
    /*
    @discardableResult
    func joystickAction(_ sender: GamePad!, events: [GamePadAction]) -> Bool {
    
        // Find slot of connected GamePad
        let slot = lookupGamePad(sender)
        
        // Pass joystick event to the main controller
        return controller.joystickAction(slot: slot, events: events)
    }
    */
    
    func listDevices() {
        
        for (slotNr, dev) in gamePads {
            
            print("Game pad slot \(slotNr): ", terminator: "")
            if let name = dev.name {
                print("\(name) (\(dev.vendorID), \(dev.productID), \(dev.locationID))")
            } else {
                print("Placeholder device")
            }
        }
    }
}
