// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import IOKit.hid

/* An object of this class represents an input device connected to the Game
 * Port. The object can either represent a connected HID device or a keyboard
 * emulated device. In the first case, the object serves as a callback handler
 * for HID events. In the latter case, it translates keyboard events to
 * GamePadAction events by utilizing a key map.
 */
class GamePad {

    // References
    var manager: GamePadManager
    var amiga: EmulatorProxy { return manager.controller.emu }
    var prefs: Preferences { return manager.controller.pref }
    var config: Configuration { return manager.controller.config }
    var db: DeviceDatabase { return myAppDelegate.database }

    // The Amiga port this device is connected to (1, 2, or nil)
    var port: Int?

    // GamePad properties (derived from database)
    var traits: MyData = MyData()

    // Reference to the HID device
    var device: IOHIDDevice?
    var vendorID: String { return device?.vendorID ?? "" }
    var productID: String { return device?.productID ?? "" }
    var locationID: String { return device?.locationID ?? "" }
    var version: String { return device?.versionNumberKey ?? "" }

    // Type of the managed device (joystick or mouse)
    var type: ControlPortDevice
    var isMouse: Bool { return type == .MOUSE }
    var isJoystick: Bool { return type == .JOYSTICK }

    // Name of the managed device
    var name = ""

    // Icon of this device
    var icon: NSImage?
            
    // Indicates if this device is officially supported
    var isKnown: Bool { return !traits.isGeneric }

    // Keymap of the managed device (only set for keyboard emulated devices)
    var keyMap: Int?
    
    // Indicates if a joystick emulation key is currently pressed
    var keyUp = false, keyDown = false, keyLeft = false, keyRight = false
    
    // Indicates if other components should be notified when the device is used
    var notify = false

    /* Rescued information from the latest invocation of the action function.
     * This information is utilized to determine whether a joystick event has
     * to be triggered.
     */
    var oldEvents: [Int: [GamePadAction]] = [:]
    
    // Receivers for HID events
    let inputValueCallback: IOHIDValueCallback = {
        inContext, inResult, inSender, value in
        let this: GamePad = unsafeBitCast(inContext, to: GamePad.self)
        this.hidInputValueAction(context: inContext,
                                 result: inResult,
                                 sender: inSender,
                                 value: value)
    }
    
    init(manager: GamePadManager, device: IOHIDDevice? = nil, type: ControlPortDevice) {
                
        self.manager = manager
        self.device = device
        self.type = type

        traits = db.query(vendorID: vendorID, productID: productID, version: version)

        name = traits.name
        icon = NSImage(named: isMouse ? "devMouseTemplate" : "devGamepad1Template")!
    }

    func property(key: String) -> String? {
            
        if device != nil {
            if let prop = IOHIDDeviceGetProperty(device!, key as CFString) {
                return "\(prop)"
            }
        }
        return nil
    }
    
    func dump() {
        
        print(name != "" ? "\(name) " : "Placeholder device ", terminator: "")
        print(isMouse ? "(Mouse) " : "", terminator: "")
        print(port != nil ? "[\(port!)]" : "[-] ", terminator: "")
        print(vendorID == "" ? "" : "v: \(vendorID) ", terminator: "")
        print(productID == "" ? "" : "v: \(productID) ", terminator: "")
        print(locationID == "" ? "" : "v: \(locationID) ")
        print("")
    }
    
    //
    // Responding to keyboard events
    //

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
                    
        switch GamePadAction(rawValue: direction) {
            
        case .PULL_UP:
            keyUp = true
            return [.PULL_UP]
            
        case .PULL_DOWN:
            keyDown = true
            return [.PULL_DOWN]
            
        case .PULL_LEFT:
            keyLeft = true
            return [.PULL_LEFT]
            
        case .PULL_RIGHT:
            keyRight = true
            return [.PULL_RIGHT]
            
        case .PRESS_FIRE:
            return [.PRESS_FIRE]

        case .PRESS_FIRE2:
            return [.PRESS_FIRE2]

        case .PRESS_FIRE3:
            return [.PRESS_FIRE3]

        case .PRESS_LEFT:
            return [.PRESS_LEFT]

        case .PRESS_MIDDLE:
            return [.PRESS_MIDDLE]

        case .PRESS_RIGHT:
            return [.PRESS_RIGHT]
            
        default:
            fatalError()
        }
    }
        
    // Handles a key release event
    func keyUpEvents(_ macKey: MacKey) -> [GamePadAction] {
        
        guard let n = keyMap, let direction = prefs.keyMaps[n][macKey] else { return [] }
                
        switch GamePadAction(rawValue: direction) {
            
        case .PULL_UP:
            keyUp = false
            return keyDown ? [.PULL_DOWN] : [.RELEASE_Y]
            
        case .PULL_DOWN:
            keyDown = false
            return keyUp ? [.PULL_UP] : [.RELEASE_Y]
            
        case .PULL_LEFT:
            keyLeft = false
            return keyRight ? [.PULL_RIGHT] : [.RELEASE_X]
            
        case .PULL_RIGHT:
            keyRight = false
            return keyLeft ? [.PULL_LEFT] : [.RELEASE_X]
            
        case .PRESS_FIRE:
            return [.RELEASE_FIRE]

        case .PRESS_FIRE2:
            return [.RELEASE_FIRE2]

        case .PRESS_FIRE3:
            return [.RELEASE_FIRE3]

        case .PRESS_LEFT:
            return [.RELEASE_LEFT]

        case .PRESS_MIDDLE:
            return [.RELEASE_MIDDLE]

        case .PRESS_RIGHT:
            return [.RELEASE_RIGHT]
            
        default:
            fatalError()
        }
    }
    
    //
    // Responding to HID events
    //

    // Based on
    // http://docs.ros.org/hydro/api/oculus_sdk/html/OSX__Gamepad_8cpp_source.html#l00170
    
    func mapAnalogAxis(value: IOHIDValue, element: IOHIDElement) -> Int? {
        
        let min = IOHIDElementGetLogicalMin(element)
        let max = IOHIDElementGetLogicalMax(element)
        let val = IOHIDValueGetIntegerValue(value)
        
        var v = (Double) (val - min) / (Double) (max - min)
        v = v * 2.0 - 1.0
        
        if v < 0 {
            if v < -0.45 { return -1 }
            if v > -0.35 { return 0 }
        } else {
            if v > 0.45 { return 1 }
            if v < 0.35 { return 0 }
        }
        
        return nil // Dead zone
    }

    /*
    func mapHAxis(value: IOHIDValue, element: IOHIDElement) -> [GamePadAction]? {
    
        if let v = mapAnalogAxis(value: value, element: element) {
            return v == 2 ? [.PULL_RIGHT] : v == -2 ? [.PULL_LEFT] : [.RELEASE_X]
        } else {
            return nil
        }
    }

    func mapHAxisRev(value: IOHIDValue, element: IOHIDElement) -> [GamePadAction]? {

        if let v = mapAnalogAxis(value: value, element: element) {
            return v == 2 ? [.PULL_LEFT] : v == -2 ? [.PULL_RIGHT] : [.RELEASE_X]
        } else {
            return nil
        }
    }

    func mapVAxis(value: IOHIDValue, element: IOHIDElement) -> [GamePadAction]? {
    
        if let v = mapAnalogAxis(value: value, element: element) {
            return v == 2 ? [.PULL_DOWN] : v == -2 ? [.PULL_UP] : [.RELEASE_Y]
        } else {
            return nil
        }
    }

    func mapVAxisRev(value: IOHIDValue, element: IOHIDElement) -> [GamePadAction]? {
    
        if let v = mapAnalogAxis(value: value, element: element) {
            return v == 2 ? [.PULL_UP] : v == -2 ? [.PULL_DOWN] : [.RELEASE_Y]
        } else {
            return nil
        }
    }
    */

    func hidInputValueAction(context: UnsafeMutableRawPointer?,
                             result: IOReturn,
                             sender: UnsafeMutableRawPointer?,
                             value: IOHIDValue) {
        
        let element   = IOHIDValueGetElement(value)
        let intValue  = Int(IOHIDValueGetIntegerValue(value))
        let usagePage = Int(IOHIDElementGetUsagePage(element))
        let usage     = Int(IOHIDElementGetUsage(element))

        var hidEvent: (HIDEvent?, Int, Int) = (nil, 0, 0)

        print("usagePage = \(usagePage) usage = \(usage) value = \(intValue)")

        var events: [GamePadAction]?
        
        if usagePage == kHIDPage_Button {

            // print("BUTTON: usagePage = \(usagePage) usage = \(usage) value = \(intValue)")
            hidEvent = (.BUTTON, usage, intValue)

            events = intValue != 0 ? (traits.b[usage]?.0 ?? []) : (traits.b[usage]?.1 ?? [])
            /*
            switch hScheme {
            
            case Schemes.B4B7:
                
                switch usage {
                case 5: events = intValue != 0 ? [.PULL_UP] : [.RELEASE_Y]
                case 6: events = intValue != 0 ? [.PULL_RIGHT] : [.RELEASE_X]
                case 7: events = intValue != 0 ? [.PULL_DOWN] : [.RELEASE_Y]
                case 8: events = intValue != 0 ? [.PULL_LEFT] : [.RELEASE_X]
                default: events = intValue != 0 ? [.PRESS_FIRE] : [.RELEASE_FIRE]
                }
            
            case Schemes.B11B14:
                
                switch usage {
                case 12: events = intValue != 0 ? [.PULL_UP] : [.RELEASE_Y]
                case 13: events = intValue != 0 ? [.PULL_DOWN] : [.RELEASE_Y]
                case 14: events = intValue != 0 ? [.PULL_LEFT] : [.RELEASE_X]
                case 15: events = intValue != 0 ? [.PULL_RIGHT] : [.RELEASE_X]
                default: events = intValue != 0 ? [.PRESS_FIRE] : [.RELEASE_FIRE]
                }
            
            default:
                events = intValue != 0 ? [.PRESS_FIRE] : [.RELEASE_FIRE]
            }
            */
        }
        
        if usagePage == kHIDPage_GenericDesktop {

            print("GENERIC DESKTOP: usagePage = \(usagePage) usage = \(usage) value = \(intValue)")

            switch usage {

            case kHIDUsage_GD_X: // A0

                if let value = mapAnalogAxis(value: value, element: element) {

                    if value == -1 { events = traits.a0.0 }
                    if value == 0  { events = traits.a0.1 }
                    if value == 1  { events = traits.a0.2 }

                    hidEvent = (.AXIS, 0, value)
                }

            case kHIDUsage_GD_Y: // A1

                if let value = mapAnalogAxis(value: value, element: element) {

                    if value == -1 { events = traits.a1.0 }
                    if value == 0  { events = traits.a1.1 }
                    if value == 1  { events = traits.a1.2 }

                    hidEvent = (.AXIS, 1, value)
                }

            case kHIDUsage_GD_Z: // A2

                if let value = mapAnalogAxis(value: value, element: element) {

                    if value == -1 { events = traits.a2.0 }
                    if value == 0  { events = traits.a2.1 }
                    if value == 1  { events = traits.a2.2 }

                    hidEvent = (.AXIS, 2, value)
                }

            case kHIDUsage_GD_Rx: // A3

                if let value = mapAnalogAxis(value: value, element: element) {

                    if value == -1 { events = traits.a3.0 }
                    if value == 0  { events = traits.a3.1 }
                    if value == 1  { events = traits.a3.2 }

                    hidEvent = (.AXIS, 3, value)
                }

            case kHIDUsage_GD_Ry: // A4

                if let value = mapAnalogAxis(value: value, element: element) {

                    if value == -1 { events = traits.a4.0 }
                    if value == 0  { events = traits.a4.1 }
                    if value == 1  { events = traits.a4.2 }

                    hidEvent = (.AXIS, 4, value)
                }

            case kHIDUsage_GD_Rz: // A5

                if let value = mapAnalogAxis(value: value, element: element) {

                    if value == -1 { events = traits.a5.0 }
                    if value == 0  { events = traits.a5.1 }
                    if value == 1  { events = traits.a5.2 }

                    hidEvent = (.AXIS, 5, value)
                }

            case kHIDUsage_GD_DPadUp:

                hidEvent = (.DPAD_UP, 0, intValue)
                events = intValue != 0 ? [.PULL_UP] : [.RELEASE_Y]

            case kHIDUsage_GD_DPadDown:

                hidEvent = (.DPAD_DOWN, 0, intValue)
                events = intValue != 0 ? [.PULL_DOWN] : [.RELEASE_Y]

            case kHIDUsage_GD_DPadRight:

                hidEvent = (.DPAD_RIGHT, 0, intValue)
                events = intValue != 0 ? [.PULL_RIGHT] : [.RELEASE_X]

            case kHIDUsage_GD_DPadLeft:

                hidEvent = (.DPAD_LEFT, 0, intValue)
                events = intValue != 0 ? [.PULL_LEFT] : [.RELEASE_X]

            /*
            case kHIDUsage_GD_X where lScheme == Schemes.A0A1:   // A0
                events = mapHAxis(value: value, element: element)

            case kHIDUsage_GD_X where lScheme == Schemes.A0A1r:  // A0
                events = mapHAxis(value: value, element: element)
                
            case kHIDUsage_GD_Y where lScheme == Schemes.A0A1:   // A1
                events = mapVAxis(value: value, element: element)
                
            case kHIDUsage_GD_Y where lScheme == Schemes.A0A1r:  // A1
               events = mapVAxisRev(value: value, element: element)

            case kHIDUsage_GD_Z where rScheme == Schemes.A2A5:   // A2
                events = mapHAxis(value: value, element: element)

            case kHIDUsage_GD_Z where rScheme == Schemes.A2A5r:  // A2
                events = mapHAxis(value: value, element: element)

            case kHIDUsage_GD_Z where rScheme == Schemes.A2A3:   // A2
                events = mapHAxis(value: value, element: element)
                    
            case kHIDUsage_GD_Rx where lScheme == Schemes.A3A4:  // A3
                events = mapHAxis(value: value, element: element)

            case kHIDUsage_GD_Rx where lScheme == Schemes.A2A3:  // A3
                events = mapVAxisRev(value: value, element: element)

            case kHIDUsage_GD_Ry where lScheme == Schemes.A3A4:  // A4
                events = mapVAxis(value: value, element: element)

            case kHIDUsage_GD_Rz where rScheme == Schemes.A2A5:  // A5
                events = mapVAxis(value: value, element: element)

            case kHIDUsage_GD_Rz where rScheme == Schemes.A2A5r: // A5
                events = mapVAxisRev(value: value, element: element)
                            
            case 0x90 where hScheme == Schemes.U90U93:
                events = intValue != 0 ? [.PULL_UP] : [.RELEASE_Y]

            case 0x91 where hScheme == Schemes.U90U93:
                events = intValue != 0 ? [.PULL_DOWN] : [.RELEASE_Y]

            case 0x92 where hScheme == Schemes.U90U93:
                events = intValue != 0 ? [.PULL_RIGHT] : [.RELEASE_X]

            case 0x93 where hScheme == Schemes.U90U93:
                events = intValue != 0 ? [.PULL_LEFT] : [.RELEASE_X]
            */

            case kHIDUsage_GD_Hatswitch:
                
                hidEvent = (.HATSWITCH, 0, intValue)

                // track("kHIDUsage_GD_Hatswitch: \(intValue)")
                let shift = 0 // hScheme == Schemes.H0H7 ? 0 : 1

                switch intValue - shift {
                case 0: events = [.PULL_UP, .RELEASE_X]
                case 1: events = [.PULL_UP, .PULL_RIGHT]
                case 2: events = [.PULL_RIGHT, .RELEASE_Y]
                case 3: events = [.PULL_RIGHT, .PULL_DOWN]
                case 4: events = [.PULL_DOWN, .RELEASE_X]
                case 5: events = [.PULL_DOWN, .PULL_LEFT]
                case 6: events = [.PULL_LEFT, .RELEASE_Y]
                case 7: events = [.PULL_LEFT, .PULL_UP]
                default: events = [.RELEASE_XY]
                }

            default:
                debug(.hid, "Unknown HID usage: \(usage)")
            }
        }

        if notify && hidEvent.0 != nil {
            myAppDelegate.hidEvent(event: hidEvent.0!, nr: hidEvent.1, value: hidEvent.2) }

        // Only proceed if the event is different than the previous one
        if events == nil || oldEvents[usage] == events { return }
        oldEvents[usage] = events!
        
        // Trigger events
        processJoystickEvents(events: events!)
    }
    
    //
    // Emulate events on the Amiga side
    //
    
    @discardableResult
    func processJoystickEvents(events: [GamePadAction]) -> Bool {
        
        let amiga = manager.controller.emu!
        
        if port == 1 { for e in events { amiga.controlPort1.joystick.trigger(e) } }
        if port == 2 { for e in events { amiga.controlPort2.joystick.trigger(e) } }
        
        // Notify other components (if requested)
        if notify { myAppDelegate.devicePulled(events: events) }

        return events != []
    }
    
    @discardableResult
    func processMouseEvents(events: [GamePadAction]) -> Bool {
        
        let amiga = manager.controller.emu!
        
        if port == 1 { for e in events { amiga.controlPort1.mouse.trigger(e) } }
        if port == 2 { for e in events { amiga.controlPort2.mouse.trigger(e) } }
        
        return events != []
    }
    
    func processMouseEvents(delta: NSPoint) {
        
        let amiga = manager.controller.emu!

        // Check for a shaking mouse
        amiga.controlPort1.mouse.detectShakeRel(delta)

        if port == 1 { amiga.controlPort1.mouse.setDxDy(delta) }
        if port == 2 { amiga.controlPort2.mouse.setDxDy(delta) }
    }
    
    func processKeyDownEvent(macKey: MacKey) -> Bool {

        // Only proceed if a keymap is present
         if keyMap == nil { return false }
         
         // Only proceed if this key is used for emulation
         let events = keyDownEvents(macKey)
         if events.isEmpty { return false }
         
         // Process the events
         processKeyboardEvent(events: events)
         return true
    }

    func processKeyUpEvent(macKey: MacKey) -> Bool {
        
        // Only proceed if a keymap is present
        if keyMap == nil { return false }
        
        // Only proceed if this key is used for emulation
        let events = keyUpEvents(macKey)
        if events.isEmpty { return false }
        
        // Process the events
        processKeyboardEvent(events: events)
        return true
    }

    func processKeyboardEvent(events: [GamePadAction]) {

        let amiga = manager.controller.emu!
        
        if isMouse {
            if port == 1 { for e in events { amiga.controlPort1.mouse.trigger(e) } }
            if port == 2 { for e in events { amiga.controlPort2.mouse.trigger(e) } }
        } else {
            if port == 1 { for e in events { amiga.controlPort1.joystick.trigger(e) } }
            if port == 2 { for e in events { amiga.controlPort2.joystick.trigger(e) } }
        }
    }
}
