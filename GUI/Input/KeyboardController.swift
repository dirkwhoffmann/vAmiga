// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Carbon.HIToolbox

@MainActor
class KeyboardController: NSObject {

    // var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
    var parent: MyController!

    var keyboard: KeyboardProxy? { return parent.emu?.keyboard }
    var renderer: Renderer { return parent.renderer }
    var pref: Preferences { return parent.pref }
        
    // Remembers the state of some keys (true = currently pressed)
    var leftShift   = false, rightShift   = false
    var leftControl = false, rightControl = false
    var leftOption  = false, rightOption  = false
    var leftCommand = false, rightCommand = false
    var capsLock    = false

    // Remembers the state of the two virtual Amiga keys
    var leftAmiga   = false, rightAmiga   = false

    // Remembers the warp mode when caps lock is pressed
    var oldWarpMode: WarpMode?

    // Mapping from Unicode scalars to keycodes (used for auto-typing)
    var symKeyMap: [UnicodeScalar: UInt16] = [:]
    var symKeyMapShifted: [UnicodeScalar: UInt16] = [:]

    init(parent: MyController) {
        
        self.parent = parent

        // Setup symbolic key maps
        for keyCode: UInt16 in 0 ... 255 {
            
            if let s = String(keyCode: keyCode, carbonFlags: 0), s.count == 1 {
                if let scalar = s.unicodeScalars.first {
                    symKeyMap[scalar] = keyCode
                }
            }
            if let s = String(keyCode: keyCode, carbonFlags: shiftKey), s.count == 1 {
                if let scalar = s.unicodeScalars.first {
                    symKeyMapShifted[scalar] = keyCode
                }
            }
        }
    }
    
    func keyDown(with event: NSEvent) {

        print("keyDown: \(event)")

        // Intercept if the console is open
        if renderer.console.isVisible { renderer.console.keyDown(with: event); return }
                
        // Ignore repeating keys
        if event.isARepeat { return }
        
        // Exit fullscreen mode if escape key is pressed
        if event.keyCode == kVK_Escape && renderer.fullscreen && pref.exitOnEsc {
            parent.window!.toggleFullScreen(nil)
            return
        }
        
        // Ignore keys that are pressed in combination with the Command key
        if event.modifierFlags.contains(NSEvent.ModifierFlags.command) {
            return
        }
        
        keyDown(with: MacKey(event: event))
    }
    
    func keyUp(with event: NSEvent) {

        print("keyUp: \(event)")

        // Intercept if the console is open
        if renderer.console.isVisible { renderer.console.keyUp(with: event); return }

        keyUp(with: MacKey(event: event))
    }
    
    func flagsChanged(with event: NSEvent) {

        var cmd: Bool { event.modifierFlags.contains(.command) }

        // print("flagsChanged: \(event)")
        // Intercept if the console is open
        if renderer.console.isVisible { return }

        // Check for a mouse controlling key combination
        // if parent.metal.checkForMouseKeys(with: event) { return }

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
            if leftOption {
                leftAmiga = cmd && pref.amigaKeysCombEnable && pref.amigaKeysComb == 0
                leftAmiga ? keyDown(with: MacKey.command) : keyDown(with: MacKey.option)
            } else {
                leftAmiga ? keyUp(with: MacKey.command) : keyUp(with: MacKey.option)
                leftAmiga = false
            }

        case kVK_RightOption:
            rightOption = event.modifierFlags.contains(.option) ? !rightOption : false
            if rightOption {
                rightAmiga = cmd && pref.amigaKeysCombEnable && pref.amigaKeysComb == 0
                rightAmiga ? keyDown(with: MacKey.rightCommand) : keyDown(with: MacKey.rightOption)
            } else {
                rightAmiga ? keyUp(with: MacKey.rightCommand) : keyUp(with: MacKey.rightOption)
                rightAmiga = false
            }

        case kVK_CapsLock where myAppDelegate.mapCapsLockWarp:
            capsLock = event.modifierFlags.contains(.capsLock)
            capsLock ? capsLockDown() : capsLockUp()

        default:
            break
        }
    }

    func keyDown(with macKey: MacKey) {

        /*
        if macKey == .command { print("keyDown: command") }
        if macKey == .option { print("keyDown: option") }
        if macKey == .rightCommand { print("keyDown: rightCommand") }
        if macKey == .rightOption { print("keyDown: rightOption") }
        */

        // Check if this key is used to emulate a game device
        if parent.gamePad1?.processKeyDownEvent(macKey: macKey) == true {
            if pref.disconnectJoyKeys { return }
        }
        if parent.gamePad2?.processKeyDownEvent(macKey: macKey) == true {
            if pref.disconnectJoyKeys { return }
        }

        if let amigaKey = macKey.amigaKeyCode { keyboard?.press(amigaKey) }
        parent.virtualKeyboard?.refreshIfVisible()
    }
    
    func keyUp(with macKey: MacKey) {

        /*
        if macKey == .command { print("keyUp: command") }
        if macKey == .option { print("keyUp: option") }
        if macKey == .rightCommand { print("keyUp: rightCommand") }
        if macKey == .rightOption { print("keyUp: rightOption") }
        */

        // Check if this key is used to emulate a game device
        if parent.gamePad1?.processKeyUpEvent(macKey: macKey) == true {
            if pref.disconnectJoyKeys { return }
        }
        if parent.gamePad2?.processKeyUpEvent(macKey: macKey) == true {
            if pref.disconnectJoyKeys { return }
        }

        if let amigaKey = macKey.amigaKeyCode { keyboard?.release(amigaKey) }
        parent.virtualKeyboard?.refreshIfVisible()
    }

    func keyDown(with keyCode: UInt16) {

        let macKey = MacKey(keyCode: keyCode)
        if let amigaKey = macKey.amigaKeyCode { keyboard?.press(amigaKey) }
        parent.virtualKeyboard?.refreshIfVisible()
    }

    func keyUp(with keyCode: UInt16) {

        let macKey = MacKey(keyCode: keyCode)
        if let amigaKey = macKey.amigaKeyCode { keyboard?.release(amigaKey) }
        parent.virtualKeyboard?.refreshIfVisible()
    }

    func capsLockDown() {

        oldWarpMode = WarpMode(rawValue: parent.config.warpMode)
        parent.config.warpMode = WarpMode.ALWAYS.rawValue
    }

    func capsLockUp() {

        if let oldWarpMode = oldWarpMode {
            if parent.config.warpMode == WarpMode.ALWAYS.rawValue {
                parent.config.warpMode = oldWarpMode.rawValue
            }
        }
    }

    func autoType(_ string: String, max: Int) {

        var truncated = string
        
        // Shorten string if it is too large
        if string.count > max { truncated = truncated.prefix(max) + "..." }

        // Type string
        self.autoType(truncated)
    }
    
    func autoType(_ string: String) {

        var shift = false
        var delay = 0.0
        let delta = 0.02

        func pressShift() {
            if !shift {
                keyboard?.press(MacKey.shift.amigaKeyCode!, delay: delay)
                delay += delta
                shift = true
            }
        }
        func releaseShift() {
            if shift {
                keyboard?.release(MacKey.shift.amigaKeyCode!, delay: delay)
                delay += delta
                shift = false
            }
        }

        for scalar in string.unicodeScalars {

            if let keyCode = symKeyMap[scalar] {

                if let amigaKeyCode = MacKey(keyCode: keyCode).amigaKeyCode {

                    releaseShift()
                    keyboard?.press(amigaKeyCode, delay: delay)
                    delay += delta
                    keyboard?.release(amigaKeyCode, delay: delay)
                    delay += delta
                    continue
                }
            }
            if let keyCode = symKeyMapShifted[scalar] {

                if let amigaKeyCode = MacKey(keyCode: keyCode).amigaKeyCode {

                    pressShift()
                    keyboard?.press(amigaKeyCode, delay: delay)
                    delay += delta
                    keyboard?.release(amigaKeyCode)
                    delay += delta
                    continue
                }
            }
        }
        releaseShift()
    }
}
