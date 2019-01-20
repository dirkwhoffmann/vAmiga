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
            keyUp(with: MacKey.leftShift)
            keyUp(with: MacKey.rightShift)
            track("** SHIFT inconsistency ** \(leftShift) \(rightShift)")
        }
        
        if control != flags.contains(NSEvent.ModifierFlags.control) {
            keyUp(with: MacKey.leftControl)
            keyUp(with: MacKey.rightControl)
            track("** CONTROL inconsistency ** \(leftControl) \(rightControl)")
        }
        if option != flags.contains(NSEvent.ModifierFlags.option) {
            keyUp(with: MacKey.leftOption)
            keyUp(with: MacKey.rightOption)
            track("*** ALT inconsistency *** \(leftOption) \(rightOption)")
        }
        if command != flags.contains(NSEvent.ModifierFlags.command) {
            keyUp(with: MacKey.leftCommand)
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
        if (event.keyCode == MacKey.escape.keyCode && controller.metalScreen.fullscreen) {
            myController?.window!.toggleFullScreen(nil)
        }
        
        let keyCode = event.keyCode
        let flags = event.modifierFlags
        let characters = event.charactersIgnoringModifiers
        
        // Ignore keys that are pressed in combination with the command key
        if (flags.contains(NSEvent.ModifierFlags.command)) {
            // track("Ignoring the command key")
            return
        }
        
        // Create a fitting MacKey and press it
        let macKey = MacKey.init(keyCode: keyCode, characters: characters)
        checkConsistency(withEvent: event)
        keyDown(with: macKey)
    }
    
    func keyUp(with event: NSEvent)
    {
        let keyCode = event.keyCode
        let characters = event.charactersIgnoringModifiers

        // Create a fitting MacKey and release it
        let macKey = MacKey.init(keyCode: keyCode, characters: characters)
        checkConsistency(withEvent: event)
        keyUp(with: macKey)
    }
    
    func flagsChanged(with event: NSEvent) {
        
        let mod = event.modifierFlags
        let keyCode = event.keyCode

        track("\(mod)")
        track("\(keyCode)")

        switch Int(event.keyCode) {
            
        case kVK_Shift:
            leftShift = event.modifierFlags.contains(.shift)
            leftShift ? keyDown(with: MacKey.leftShift) : keyUp(with: MacKey.leftShift)
            
        case kVK_RightShift:
            rightShift = event.modifierFlags.contains(.shift)
            rightShift ? keyDown(with: MacKey.rightShift) : keyUp(with: MacKey.rightShift)
            
        case kVK_Control:
            leftControl = event.modifierFlags.contains(.control)
            leftControl ? keyDown(with: MacKey.leftControl) : keyUp(with: MacKey.leftControl)

        case kVK_RightControl:
            rightControl = event.modifierFlags.contains(.control)
            rightControl ? keyDown(with: MacKey.rightControl) : keyUp(with: MacKey.rightControl)

        case kVK_Option:
            leftOption = event.modifierFlags.contains(.option)
            leftOption ? keyDown(with: MacKey.leftOption) : keyUp(with: MacKey.leftOption)
            
        case kVK_RightOption:
            rightOption = event.modifierFlags.contains(.option)
            rightOption ? keyDown(with: MacKey.rightOption) : keyUp(with: MacKey.rightOption)
            
        case kVK_Command:
            leftCommand = event.modifierFlags.contains(.command)
            leftCommand ? keyDown(with: MacKey.leftCommand) : keyUp(with: MacKey.leftCommand)
            
        case kVK_RightCommand:
            rightCommand = event.modifierFlags.contains(.command)
            rightCommand ? keyDown(with: MacKey.rightCommand) : keyUp(with: MacKey.rightCommand)
            
        default:
            break
        }
        
        checkConsistency(withEvent: event)
    }
    
    func keyDown(with macKey: MacKey) {
        
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
