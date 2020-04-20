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
        gamePads = [:]
    }

    deinit {

        track()
        IOHIDManagerClose(hidManager, IOOptionBits(kIOHIDOptionsTypeNone))
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
    
    // Looks up a game pad. Returns -1 if no game pad was found
    /*
    func lookupGamePad(_ gamePad: GamePad) -> Int {
        
        for (slotNr, device) in gamePads where device === gamePad {
            return slotNr
        }
        return -1
    }
    */
    
    // Looks up game pad by locationID. Returns -1 if no game pad was found
    /*
    func lookupGamePad(locationID: Int) -> Int {
        
        for (slotNr, device) in gamePads where device.locationID == locationID {
            return slotNr
        }
        return -1
    }
    */
 
    //
    // HID stuff
    //
    
    // Device matching callback
    // This method is invoked when a matching HID device is plugged in.
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
        let hidContext = unsafeBitCast(gamePads[slotNr],
                                       to: UnsafeMutableRawPointer.self)
        IOHIDDeviceRegisterInputValueCallback(device,
                                              gamePads[slotNr]!.actionCallback,
                                              hidContext)

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
        
        // Closing the HID device always fails.
        // Think, we don't have to close it, because it's disconnected anyway. Am I right?
        /* 
        let optionBits = kIOHIDOptionsTypeNone // kIOHIDOptionsTypeSeizeDevice
        let status = IOHIDDeviceClose(device, IOOptionBits(optionBits))
        if (status != kIOReturnSuccess) {
            track("WARNING: Cannot close HID device")
        }
        */
        
        // Inform the controller about the new device
        parent.toolbar.validateVisibleItems()
        
        listDevices()
    }
    
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
