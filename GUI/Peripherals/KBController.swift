// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Carbon.HIToolbox

// Keyboard event handler
class KBController: NSObject {

    var parent: MyController!
    
    var keyboard: KeyboardProxy { return parent.amiga.keyboard }
    var renderer: Renderer { return parent.renderer }
    var prefs: Preferences { return parent.prefs }
        
    // Remembers the current state of some keys
    var leftShift   = false, rightShift   = false
    var leftControl = false, rightControl = false
    var leftOption  = false, rightOption  = false
    var leftCommand = false, rightCommand = false
    
    // Mapping from Unicode scalars to keycodes (used for auto-typing)
    var symKeyMap: [UnicodeScalar: UInt16] = [:]
    var symKeyMapShifted: [UnicodeScalar: UInt16] = [:]

    init(parent: MyController) {
        
        self.parent = parent

        // Setup symbolic key maps
        for keyCode: UInt16 in 0 ... 255 {
            
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
        
        // Ignore repeating keys
        if event.isARepeat { return }
        
        // Exit fullscreen mode if escape key is pressed
        if event.keyCode == kVK_Escape && renderer.fullscreen && prefs.exitOnEsc {
            parent.window!.toggleFullScreen(nil)
            return
        }
        
        // Ignore keys that are pressed in combination with the Command key
        if event.modifierFlags.contains(NSEvent.ModifierFlags.command) {
            return
        }
        
        keyDown(with: MacKey.init(event: event))
    }
    
    func keyUp(with event: NSEvent) {
                
        keyUp(with: MacKey.init(event: event))
    }
    
    func flagsChanged(with event: NSEvent) {
        
        // Check for a mouse controlling key combination
        parent.metal.checkForMouseKeys(with: event)
        
        // Determine the pressed or released key
        switch Int(event.keyCode) {
            
        case kVK_Shift:
            leftShift = event.modifierFlags.contains(.shift) ? !leftShift : false
            leftShift ? keyDown(with: MacKey.shift) : keyUp(with: MacKey.shift)

        case kVK_RightShift:
            rightShift = event.modifierFlags.contains(.shift) ? !rightShift : false
            rightShift ? keyDown(with: MacKey.rightShift) : keyUp(with: MacKey.rightShift)

        case kVK_Control:
            leftControl = event.modifierFlags.contains(.control) ? !leftControl : false
            leftControl ? keyDown(with: MacKey.control) : keyUp(with: MacKey.control)
            
        case kVK_RightControl:
            rightControl = event.modifierFlags.contains(.control) ? !rightControl : false
            rightControl ? keyDown(with: MacKey.rightControl) : keyUp(with: MacKey.rightControl)
            
        case kVK_Option:
            leftOption = event.modifierFlags.contains(.option) ? !leftOption : false
            leftOption ? keyDown(with: MacKey.option) : keyUp(with: MacKey.option)

        case kVK_RightOption:
            rightOption = event.modifierFlags.contains(.option) ? !rightOption : false
            rightOption ? keyDown(with: MacKey.rightOption) : keyUp(with: MacKey.rightOption)
            
        case kVK_Command where mapCommandKeys:
            leftCommand = event.modifierFlags.contains(.command) ? !leftCommand : false
            leftCommand ? keyDown(with: MacKey.command) : keyUp(with: MacKey.command)
            
        case kVK_RightCommand where mapCommandKeys:
            rightCommand = event.modifierFlags.contains(.command) ? !rightCommand : false
            rightCommand ? keyDown(with: MacKey.rightCommand) : keyUp(with: MacKey.rightCommand)

        default:
            break
        }
    }
    
    func keyDown(with macKey: MacKey) {
        
        // Check if this key is used for joystick emulation
        if parent.gamePad1?.processKeyDownEvent(macKey: macKey) == true {
            if prefs.disconnectJoyKeys { return }
        }
        if parent.gamePad2?.processKeyDownEvent(macKey: macKey) == true {
            if prefs.disconnectJoyKeys { return }
        }

        keyboard.pressKey(macKey.amigaKeyCode)
    }
    
    func keyUp(with macKey: MacKey) {
        
        // Check if this key is used for joystick emulation
        if parent.gamePad1?.processKeyUpEvent(macKey: macKey) == true {
            if prefs.disconnectJoyKeys { return }
        }
        if parent.gamePad2?.processKeyUpEvent(macKey: macKey) == true {
            if prefs.disconnectJoyKeys { return }
        }

        keyboard.releaseKey(macKey.amigaKeyCode)
    }
    
    func keyDown(with keyCode: UInt16) {
        
        let macKey = MacKey.init(keyCode: keyCode)
        keyboard.pressKey(macKey.amigaKeyCode)
    }
    
    func keyUp(with keyCode: UInt16) {
        
        let macKey = MacKey.init(keyCode: keyCode)
        keyboard.releaseKey(macKey.amigaKeyCode)
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
