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
    
    // Mapping from Unicode scalars to keycodes (used for auto-typing)
    var symKeyMap : [UnicodeScalar : UInt16] = [:]
    var symKeyMapShifted : [UnicodeScalar : UInt16] = [:]
    
    override init() {
        
        track()
        
        // Setup symbolic key maps
        for keyCode : UInt16 in 0 ... 255 {
            
            if let s = String.init(keyCode: keyCode, carbonFlags: 0), s.count == 1 {
                if let scalar = s.unicodeScalars.first {
                    symKeyMap[scalar] = keyCode
                }
            }
            if let s = String.init(keyCode: keyCode, carbonFlags: shiftKey), s.count == 1 {
                if let scalar = s.unicodeScalars.first {
                    symKeyMapShifted[scalar] = keyCode
                }
            }
        }
    }
    
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
    
    func keyDown(with keyCode: UInt16) {
        
        let macKey = MacKey.init(keyCode: keyCode)
        myController?.amiga.keyboard.pressKey(macKey.amigaKeyCode)
    }
    
    func keyUp(with keyCode: UInt16) {
        
        let macKey = MacKey.init(keyCode: keyCode)
        myController?.amiga.keyboard.releaseKey(macKey.amigaKeyCode)
    }
    
    func autoType(_ string: String) {
        
        var shift = false
        for scalar in string.unicodeScalars {
            
            if let keyCode = symKeyMap[scalar] {
                
                if shift { keyUp(with: MacKey.shift); shift = false }
                keyDown(with: keyCode)
                keyUp(with: keyCode)
                continue
            }
            
            if let keyCode = symKeyMapShifted[scalar] {
                
                if !shift { keyDown(with: MacKey.shift); shift = true }
                keyDown(with: keyCode)
                keyUp(with: keyCode)
                continue
            }
        }
        
        if shift { keyUp(with: MacKey.shift) }
    }
}
