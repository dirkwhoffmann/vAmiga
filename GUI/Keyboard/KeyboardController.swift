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

/* Layout of the emulated keyboard
 * The keyboard layout defines the number of keys on the keyboard, the visual
 * appearance of their key caps and their physical shape.
 */
enum Layout : Int, Codable {

    case generic // Used as a fallback if no matching layout is found
    case us
    case german
    case italian
}

// Keyboard event handler
class KeyboardController: NSObject {
    
    // Indicates if the joystick emulation keys should trigger key events.
    var disconnectJoyKeys = Defaults.disconnectJoyKeys
    
    // Indicates if the pressing the ESC key should exit fullscreen mode.
    var exitOnEsc = Defaults.exitOnEsc
    
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
    /*
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
    }
    */
    
    func keyDown(with event: NSEvent) {
        
        guard let controller = myController else { return }
        
        // Ignore repeating keys
        if event.isARepeat {
            return
        }
        
        // Exit fullscreen mode if escape key is pressed
        if event.keyCode == kVK_Escape && controller.metal.fullscreen && exitOnEsc {
            myController?.window!.toggleFullScreen(nil)
        }
        
        // Ignore keys that are pressed in combination with the Command key
        if event.modifierFlags.contains(NSEvent.ModifierFlags.command) {
            return
        }
        
        keyDown(with: MacKey.init(event: event))
    }
    
    func keyUp(with event: NSEvent)
    {
        keyUp(with: MacKey.init(event: event))
    }
    
    func flagsChanged(with event: NSEvent) {
        
        let mod = event.modifierFlags
        let keyCode = event.keyCode

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
            
        case kVK_Command where mapCommandKeys:
            leftCommand = event.modifierFlags.contains(.command)
            leftCommand ? keyDown(with: MacKey.command) : keyUp(with: MacKey.command)
            
        case kVK_RightCommand where mapCommandKeys:
            rightCommand = event.modifierFlags.contains(.command)
            rightCommand ? keyDown(with: MacKey.rightCommand) : keyUp(with: MacKey.rightCommand)
            
        default:
            break
        }
    }
    
    func keyDown(with macKey: MacKey) {
        
        guard let controller = myController else { return }
        
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
