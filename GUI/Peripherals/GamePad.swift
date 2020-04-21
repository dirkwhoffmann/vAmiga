// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import IOKit.hid

/* An object that represents a single input device connected to the Game Port.
 * The object can either represent a connected HID device or a keyboard emulated
 * device. In the first case, the object serves as a callback handler for HID
 * events. In the latter case, it translates keyboard events to GamePadAction
 * events by using a key map.
 */
class GamePad {

    // Reference to the game pad manager
    var manager: GamePadManager

    // Quick references
    var prefs: Preferences { return manager.parent.prefs }

    // Name of the managed device
    var name: String?

    // Position of this game pad in the manager's game pad list
    var nr = 0
    
    // Keymap of the managed device (set for keyboard emulated devices only)
    var keyMap: Int?
    
    // Indicates if a joystick emulation key is currently pressed
    var keyUp = false, keyDown = false, keyLeft = false, keyRight = false
    
    // Vendor ID of the managed device (used for HID devices only)
    var vendorID: Int

    // Product ID of the managed device (used for HID devices only)
    var productID: Int

    // Location ID of the managed device (used for HID devices only)
    var locationID: Int

    // Minimum and maximum value of analog axis event
    var min: Int?, max: Int?
    
    // Cotroller specific usage IDs for left and right gamepad joysticks
    var lThumbXUsageID = kHIDUsage_GD_X
    var lThumbYUsageID = kHIDUsage_GD_Y
    var rThumbXUsageID = kHIDUsage_GD_Rz
    var rThumbYUsageID = kHIDUsage_GD_Z

    /* Rescued information from the latest invocation of the action function.
     * It is needed to determine whether a joystick event has to be triggered.
     */
    var oldEvents: [Int: [GamePadAction]] = [:]
    
    // Receiver for HID events
    let actionCallback: IOHIDValueCallback = {
        inContext, inResult, inSender, value in
        let this: GamePad = unsafeBitCast(inContext, to: GamePad.self)
        this.hidDeviceAction(context: inContext,
                             result: inResult,
                             sender: inSender,
                             value: value)
    }
    
    init(_ nr: Int, manager: GamePadManager,
         vendorID: Int = 0, productID: Int = 0, locationID: Int = 0) {
        
        // track("\(nr): \(vendorID) \(productID) \(locationID)")
        
        self.nr = nr
        self.manager = manager
        self.vendorID = vendorID
        self.productID = productID
        self.locationID = locationID
        
        // Check for known devices
        switch vendorID {
            
        case 0x40B where productID == 0x6533:
            name = "Competition Pro SL-6602"

        case 0x738 where productID == 0x2217:
            name = "Competition Pro SL-650212"

        case 0x1C59 where productID == 0x24:
            name = "The C64 Joystick"
            
        case 0x79 where productID == 0x11:
            name = "iNNEXT Retro (SNES)"
            
        case 0x54C where productID == 0x268:
            name = "Sony DualShock 3"
            rThumbXUsageID = kHIDUsage_GD_Z
            rThumbYUsageID = kHIDUsage_GD_Rz
            
        case 0x54C where productID == 0x5C4:
            name = "Sony DualShock 4"
            rThumbXUsageID = kHIDUsage_GD_Z
            rThumbYUsageID = kHIDUsage_GD_Rz
            
        case 0x54C where productID == 0x9CC:
            name = "Sony Dualshock 4 (2nd Gen)"
            rThumbXUsageID = kHIDUsage_GD_Z
            rThumbYUsageID = kHIDUsage_GD_Rz
            
        case 0x483 where productID == 0x9005:
            name = "RetroFun! Joystick Adapter"
            
        case 0x004 where productID == 0x0001:
            name = "aJoy Retro Adapter"
            
        default:
            break  // name = "Generic Gamepad"
        }
    }
}

//
// Keyboard emulation
//

extension GamePad {

    // Binds a key to a gamepad action
    func bind(key: MacKey, action: GamePadAction) {

        guard let n = keyMap else { return }
        
        // Avoid double mappings
        unbind(action: action)

        prefs.keyMaps[n][key] = action.rawValue
    }

    // Removes a key binding to the specified gampad action (if any)
    func unbind(action: GamePadAction) {
        
        guard let n = keyMap else { return }
        
        for (k, dir) in prefs.keyMaps[n] where dir == action.rawValue {
            prefs.keyMaps[n][k] = nil
        }
     }

    // Translates a key press event to a list of gamepad actions
    func keyDownEvents(_ macKey: MacKey) -> [GamePadAction] {
        
        guard let n = keyMap, let direction = prefs.keyMaps[n][macKey] else { return [] }
                    
        switch GamePadAction(direction) {
            
        case PULL_UP:
            keyUp = true
            return [PULL_UP]
            
        case PULL_DOWN:
            keyDown = true
            return [PULL_DOWN]
            
        case PULL_LEFT:
            keyLeft = true
            return [PULL_LEFT]
            
        case PULL_RIGHT:
            keyRight = true
            return [PULL_RIGHT]
            
        case PRESS_FIRE:
            return [PRESS_FIRE]
            
        case PRESS_LEFT:
            return [PRESS_LEFT]
            
        case PRESS_RIGHT:
            return [PRESS_RIGHT]
            
        default:
            fatalError()
        }
    }
        
    // Handles a key release event
    func keyUpEvents(_ macKey: MacKey) -> [GamePadAction] {
        
        guard let n = keyMap, let direction = prefs.keyMaps[n][macKey] else { return [] }
                
        switch GamePadAction(direction) {
            
        case PULL_UP:
            keyUp = false
            return keyDown ? [PULL_DOWN] : [RELEASE_Y]
            
        case PULL_DOWN:
            keyDown = false
            return keyUp ? [PULL_UP] : [RELEASE_Y]
            
        case PULL_LEFT:
            keyLeft = false
            return keyRight ? [PULL_RIGHT] : [RELEASE_X]
            
        case PULL_RIGHT:
            keyRight = false
            return keyLeft ? [PULL_LEFT] : [RELEASE_X]
            
        case PRESS_FIRE:
            return [RELEASE_FIRE]
            
        case PRESS_LEFT:
            return [RELEASE_LEFT]
            
        case PRESS_RIGHT:
            return [RELEASE_RIGHT]
            
        default:
            fatalError()
        }
    }
}

//
// Event handling
//

extension GamePad {

    // Based on http://docs.ros.org/hydro/api/oculus_sdk/html/OSX__Gamepad_8cpp_source.html#l00170
    func mapAnalogAxis(value: IOHIDValue, element: IOHIDElement) -> Int? {
        
        if min == nil {
            min = IOHIDElementGetLogicalMin(element)
            // track("Minumum axis value = \(min!)")
        }
        if max == nil {
            max = IOHIDElementGetLogicalMax(element)
            // track("Maximum axis value = \(max!)")
        }
        let val = IOHIDValueGetIntegerValue(value)
        
        var v = (Double) (val - min!) / (Double) (max! - min!)
        v = v * 2.0 - 1.0
        if v < -0.45 { return -2 }
        if v < -0.1 { return nil }  // dead zone
        if v <= 0.1 { return 0 }
        if v <= 0.45 { return nil } // dead zone
        return 2
    }

    func hidDeviceAction(context: UnsafeMutableRawPointer?,
                         result: IOReturn,
                         sender: UnsafeMutableRawPointer?,
                         value: IOHIDValue) {
    
        let element   = IOHIDValueGetElement(value)
        let intValue  = Int(IOHIDValueGetIntegerValue(value))
        let usagePage = Int(IOHIDElementGetUsagePage(element))
        let usage     = Int(IOHIDElementGetUsage(element))
        
        // Buttons
        if usagePage == kHIDPage_Button {

            let events = (intValue != 0) ? [PRESS_FIRE] : [RELEASE_FIRE]
            manager.parent.emulateEventsOnGamePort(slot: nr, events: events)
            return
        }
        
        // Stick
        if usagePage == kHIDPage_GenericDesktop {
            
            var events: [GamePadAction]?
            
            switch usage {
                
            case lThumbXUsageID, rThumbXUsageID:
                
                // track("lThumbXUsageID, rThumbXUsageID: \(intValue)")
                if let v = mapAnalogAxis(value: value, element: element) {
                    events = (v == 2) ? [PULL_RIGHT] : (v == -2) ? [PULL_LEFT] : [RELEASE_X]
                }
                
            case lThumbYUsageID, rThumbYUsageID:
                
                // track("lThumbYUsageID, rThumbYUsageID: \(intValue)")
                if let v = mapAnalogAxis(value: value, element: element) {
                    events = (v == 2) ? [PULL_DOWN] : (v == -2) ? [PULL_UP] : [RELEASE_Y]
                }
                
            case kHIDUsage_GD_Hatswitch:
                
                // track("kHIDUsage_GD_Hatswitch \(intValue)")
                switch intValue {
                case 0: events = [PULL_UP, RELEASE_X]
                case 1: events = [PULL_UP, PULL_RIGHT]
                case 2: events = [PULL_RIGHT, RELEASE_Y]
                case 3: events = [PULL_RIGHT, PULL_DOWN]
                case 4: events = [PULL_DOWN, RELEASE_X]
                case 5: events = [PULL_DOWN, PULL_LEFT]
                case 6: events = [PULL_LEFT, RELEASE_Y]
                case 7: events = [PULL_LEFT, PULL_UP]
                default: events = [RELEASE_XY]
                }
                
            default:
                // track("Unknown HID usage: \(usage)")")
                break
            }
            
            // Only proceed if the event is different than the previous one
            if events == nil || oldEvents[usage] == events { return }
            oldEvents[usage] = events!
            
            // Trigger events
            manager.parent.emulateEventsOnGamePort(slot: nr, events: events!)
        }
    }
}
