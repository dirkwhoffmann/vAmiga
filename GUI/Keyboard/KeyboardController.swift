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
//
//

class KeyCapButton : NSButton {

    override func rightMouseDown(with event: NSEvent) {
        
        let tag = self.tag
        track("\(event) \(tag)")
        
        if let controller = window?.delegate as? VirtualKeyboardController {

            track("************************************")
            controller.pressVirtualKey(self)
        }
        
    }
}

//
// CGEvents
//


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

extension KeyboardController {
    
    @discardableResult
    func disableCmdShortcuts() -> Bool {
        
        if myController?.eventTap != nil { return true }
        
        track("Trying to disable keyboard shortcuts...")
        
        /* To disable shortcuts, we are going to filter out the Command flag
         * from all keyUp and keyDown CGEvents. We do this by installing a
         * CGEvent callback handler which requires accessability priviledges.
         * We first check if we have the priviledges and prompt the user if
         * we don't. In this case, the operation will fail and we won't be
         * able to disable the shortcuts. In the meantime however, the user
         * has the ability to grant us access in the system preferences. If
         * he does this, we'll pass this chechpoint on the next function call.
         */
        let trusted = kAXTrustedCheckOptionPrompt.takeUnretainedValue()
        let privOptions = [trusted: true] as CFDictionary
        
        if !AXIsProcessTrustedWithOptions(privOptions) {
            
            track("Aborting. Access denied")
            return false
        }
  
        // Set up an event mask. We want to intercept keyDown and keyUp events
        let mask = CGEventMask(
            (1 << CGEventType.keyDown.rawValue) | (1 << CGEventType.keyUp.rawValue))
        
        // Try to create an event tap
        let eventTap = CGEvent.tapCreate(tap: .cgSessionEventTap,
                                                   place: .headInsertEventTap,
                                                   options: .defaultTap,
                                                   eventsOfInterest: mask,
                                                   callback: cgEventCallback,
                                                   userInfo: nil)
        if eventTap == nil {
            
            track("Aborting. Failed to create an event tap.")
            return false
        }
        
        // Add end enable the event tap
        myController?.eventTap = eventTap
        let runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0)
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
        CGEvent.tapEnable(tap: eventTap!, enable: true)
        
        track("Success")
        return true
    }
    
    func enableCmdShortcuts() {
        
        if let eventTap = myController?.eventTap {
            
            track("Disabling keyboard shortcuts.")
            CGEvent.tapEnable(tap: eventTap, enable: false)
            myController?.eventTap = nil
        }
    }
}

//
// Capturing CGEvents
//

/* To establish a direct mapping of the Command keys to the Amiga keys, this
 * callback is registered. It intercepts keyDown and keyUp events and filters
 * out the Command key modifier flag. As a result, all keyboard shortcuts are
 * disabled and all keys that are pressed in combination with the Command key
 * trigger a standard Cocoa key event.
 */
func cgEventCallback(proxy: CGEventTapProxy,
                     type: CGEventType,
                     event: CGEvent,
                     refcon: UnsafeMutableRawPointer?) -> Unmanaged<CGEvent>? {
    
    let keyCode = event.getIntegerValueField(.keyboardEventKeycode)
    
    if type == .keyDown {
        
        track("CGEvent keyDown \(keyCode)")
        event.flags.remove(.maskCommand)
    }
    
    if type == .keyUp {
        
        track("CGEvent keyUp \(keyCode)")
        event.flags.remove(.maskCommand)
    }
 
    return Unmanaged.passRetained(event)
}
