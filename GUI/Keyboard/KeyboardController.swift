// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation
import Carbon.HIToolbox

/* The region code of the emulated keyboard
 * The region code influences the visual appearance of the emulated keyboard by
 * defining the printed labels on the keycaps. Each key on the Amiga keyboard
 * is uniquely identifies by it's keycode, but keys with the same code can be
 * labeled differently in different countries.
 */
enum Language : Int, Codable {

    case generic // Used as a fallback if no matching language is found
    case us
    case german
    case italian
}

//
// CGEvents
//

func myCGEventCallback(proxy: CGEventTapProxy,
                       type: CGEventType, event: CGEvent,
                       refcon: UnsafeMutableRawPointer?) -> Unmanaged<CGEvent>? {
    
    track("Catching CGEvent")
    let keyCode = event.getIntegerValueField(.keyboardEventKeycode)
    let flags = event.flags
    
    let shift = flags.contains(.maskShift)
    let control = flags.contains(.maskControl)
    let option = flags.contains(.maskAlternate)
    let command = flags.contains(.maskCommand)
    
    
    if type == .keyDown {
        track("INTERCEPTED: keyDown \(keyCode) S: \(shift) C: \(control) O: \(option) Cmd: \(command)")
    }
    
    if type == .keyUp {
        track("INTERCEPTED keyUp \(keyCode) S: \(shift) C: \(control) O: \(option) Cmd: \(command)")
    }
    
    if type == .flagsChanged {
        
        track("INTERCEPTED flagsChanged \(keyCode) S: \(shift) C: \(control) O: \(option) Cmd: \(command)")
    }
    
    event.flags.remove(.maskCommand)
    
    return Unmanaged.passRetained(event)
}

extension MyController {
    
    @discardableResult
    func disableCmdShortcuts() -> Bool {
        
        track("Trying to install a CGEvent interceptor...")
        
        func acquirePrivileges() {
            let trusted = kAXTrustedCheckOptionPrompt.takeUnretainedValue()
            let privOptions = [trusted: true] as CFDictionary
            let accessEnabled = AXIsProcessTrustedWithOptions(privOptions)
            
            if accessEnabled == true {
                track("Access granted")
            } else {
                track("Access DISABLED")
            }
        }
        
        acquirePrivileges()
        
        let eventMask =
            (1 << CGEventType.keyDown.rawValue) |
                (1 << CGEventType.keyUp.rawValue) |
                (1 << CGEventType.flagsChanged.rawValue)
        
        eventTap = CGEvent.tapCreate(tap: .cgSessionEventTap,
                                     place: .headInsertEventTap,
                                     options: .defaultTap,
                                     eventsOfInterest: CGEventMask(eventMask),
                                     callback: myCGEventCallback,
                                     userInfo: nil)
        
        if eventTap == nil {
            track("Failed to create event tap. Won't be able to catch CGEvents.")
            return false
        }
        
        track("Success")
        
        let runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0)
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
        CGEvent.tapEnable(tap: eventTap!, enable: true)
        
        return true
    }

    func enableCmdShortcuts() {
        
        track("Stopping CGEvent interception")
        if eventTap != nil {
            CGEvent.tapEnable(tap: eventTap!, enable: false)
            eventTap = nil
        }
    }
}

// Keyboard event handler
class KeyboardController: NSObject {
    
    /* Indicates if the joystick emulation keys should also trigger key events.
     * Set to true to prevent key events.
     */
    var disconnectJoyKeys = Defaults.disconnectJoyKeys
    
    // Remembers the currently pressed key modifiers
    var leftShift   = false, rightShift   = false
    var leftControl = false, rightControl = false
    var leftOption  = false, rightOption  = false
    var leftCommand = false, rightCommand = false
    
    /** DEPRECATED
     Remembers the currently pressed keys and their assigned C64 key list

     This variable is only used when keys are mapped symbolically. It's written
     in keyDown and picked up in keyUp.
     */
    var pressedKeys: [MacKey:[C64Key]] = [:]
    
    /**
     Checks if the internal values are consistent with the provides flags.
     
     There should never be an insonsistency. But if there is, we release the
     suspicous key. Otherwise, we risk to block the C64's keyboard matrix
     for good.
     */
    func checkConsistency(withEvent event: NSEvent) {
        
        let flags   = event.modifierFlags
        let shift   = leftShift   || rightShift
        let control = leftControl || rightControl
        let option  = leftOption  || rightOption
        let command = leftCommand || rightCommand
        
        if shift != flags.contains(NSEvent.ModifierFlags.shift) {
            keyUp(with: MacKey.shift)
            keyUp(with: MacKey.rightShift)
            track("** SHIFT inconsistency ** \(leftShift) \(rightShift)")
        }
        
        if control != flags.contains(NSEvent.ModifierFlags.control) {
            keyUp(with: MacKey.control)
            keyUp(with: MacKey.rightControl)
            track("** CONTROL inconsistency ** \(leftControl) \(rightControl)")
        }
        if option != flags.contains(NSEvent.ModifierFlags.option) {
            keyUp(with: MacKey.option)
            keyUp(with: MacKey.rightOption)
            track("*** ALT inconsistency *** \(leftOption) \(rightOption)")
        }
        if command != flags.contains(NSEvent.ModifierFlags.command) {
            keyUp(with: MacKey.command)
            keyUp(with: MacKey.rightCommand)
            track("*** COMMAND inconsistency *** \(leftCommand) \(rightCommand)")
        }
    }
    
    func keyDown(with event: NSEvent) {
        
        guard let controller = myController else { return }
        
        // Ignore repeating keys
        if (event.isARepeat) {
            return
        }
        
        // Exit fullscreen mode if escape key is pressed
        if (event.keyCode == kVK_Escape && controller.metalScreen.fullscreen) {
            myController?.window!.toggleFullScreen(nil)
        }
        
        // Ignore keys that are pressed in combination with the command key
        if (event.modifierFlags.contains(NSEvent.ModifierFlags.command)) {
            return
        }
        
        checkConsistency(withEvent: event)
        keyDown(with: MacKey.init(event: event))
    }
    
    func keyUp(with event: NSEvent)
    {
        checkConsistency(withEvent: event)
        keyUp(with: MacKey.init(event: event))
    }
    
    func flagsChanged(with event: NSEvent) {
        
        let mod = event.modifierFlags
        let keyCode = event.keyCode

        track("\(mod)")
        track("\(keyCode)")

        switch Int(event.keyCode) {
            
        case kVK_Shift:
            leftShift = event.modifierFlags.contains(.shift)
            leftShift ? keyDown(with: MacKey.shift) : keyUp(with: MacKey.shift)
            
        case kVK_RightShift:
            rightShift = event.modifierFlags.contains(.shift)
            rightShift ? keyDown(with: MacKey.rightShift) : keyUp(with: MacKey.rightShift)
            
        case kVK_Control:
            leftControl = event.modifierFlags.contains(.control)
            leftControl ? keyDown(with: MacKey.control) : keyUp(with: MacKey.control)

        case kVK_RightControl:
            rightControl = event.modifierFlags.contains(.control)
            rightControl ? keyDown(with: MacKey.rightControl) : keyUp(with: MacKey.rightControl)

        case kVK_Option:
            leftOption = event.modifierFlags.contains(.option)
            leftOption ? keyDown(with: MacKey.option) : keyUp(with: MacKey.option)
            
        case kVK_RightOption:
            rightOption = event.modifierFlags.contains(.option)
            rightOption ? keyDown(with: MacKey.rightOption) : keyUp(with: MacKey.rightOption)
            
        case kVK_Command:
            leftCommand = event.modifierFlags.contains(.command)
            leftCommand ? keyDown(with: MacKey.command) : keyUp(with: MacKey.command)
            
        case kVK_RightCommand:
            rightCommand = event.modifierFlags.contains(.command)
            rightCommand ? keyDown(with: MacKey.rightCommand) : keyUp(with: MacKey.rightCommand)
            
        default:
            break
        }
        
        checkConsistency(withEvent: event)
    }
    
    func keyDown(with macKey: MacKey) {
        
        let p = macKey.stringValue
        track("**** \(p)")
        
        guard let controller = myController else { return }
        // track("\(macKey)")
        
        // Check if this key is used for joystick emulation
        if controller.gamePadManager.keyDown(with: macKey) && disconnectJoyKeys {
            return
        }
        
        myController?.amiga.keyboard.pressKey(macKey.amigaKeyCode)
    }
    
    func keyUp(with macKey: MacKey) {
        
        guard let controller = myController else { return }
        
        // Check if this key is used for joystick emulation
        if controller.gamePadManager.keyUp(with: macKey) && disconnectJoyKeys {
            return
        }
        
        myController?.amiga.keyboard.releaseKey(macKey.amigaKeyCode)
    }
    
    func _type(keyList: [C64Key]) {
        
        guard let controller = myController else { return }
        
        for key in keyList {
            if (key == .restore) {
                controller.c64.keyboard.pressRestoreKey()
            } else {
                controller.c64.keyboard.pressKey(atRow: key.row, col: key.col)
            }
        }
        usleep(useconds_t(50000))
        
        for key in keyList {
            if (key == .restore) {
                controller.c64.keyboard.releaseRestoreKey()
            } else {
            controller.c64.keyboard.releaseKey(atRow: key.row, col: key.col)
            }
        }
    }
    
    func _type(key: C64Key) {
        type(keyList: [key])
    }

    func type(keyList: [C64Key]) {
        
        DispatchQueue.global().async {
            self._type(keyList: keyList)
        }
    }
 
    func type(key: C64Key) {
        
        DispatchQueue.global().async {
            self._type(key: key)
        }
    }
    
    func type(string: String?,
              initialDelay: useconds_t = 0,
              completion: (() -> Void)? = nil) {

        if var truncated = string {
            
            // Shorten string if it is too large
            if (truncated.count > 255) {
                truncated = truncated.prefix(256) + "..."
            }
            
            // Type string ...
            DispatchQueue.global().async {
                
                usleep(initialDelay);
                for c in truncated.lowercased() {
                    let c64Keys = C64Key.translate(char: String(c))
                    self._type(keyList: c64Keys)
                    usleep(useconds_t(20000))
                }
                completion?()
            }
        }
    }
    
    func type(_ string: String?, initialDelay seconds: Double = 0.0) {
        let uSeconds = useconds_t(1000000 * seconds)
        type(string: string, initialDelay: uSeconds)
    }
}
