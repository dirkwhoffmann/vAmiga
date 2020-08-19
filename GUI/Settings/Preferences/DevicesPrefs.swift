// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {
    
    func refreshDevicesTab() {

        let joystick1  = amiga.joystick1!
        let joystick2  = amiga.joystick2!

        func refreshKey(map: Int, dir: GamePadAction, button: NSButton, txt: NSTextField) {

            var macKeyCode: NSAttributedString = NSAttributedString.init()
            var macKeyDesc: String = ""

            // Which MacKey is assigned to this joystick action?
            for (key, direction) in pref.keyMaps[map] where direction == dir.rawValue {
                let attr = [NSAttributedString.Key.foregroundColor: NSColor.black]
                let myStr = NSString(format: "%02X", key.keyCode) as String
                macKeyCode = NSAttributedString(string: myStr, attributes: attr)
                macKeyDesc = key.stringValue
                break
            }

            // Update text and button image
            if button.tag == devRecordedKey {
                button.title = ""
                button.image = NSImage(named: "key_red")
                button.imageScaling = .scaleAxesIndependently
            } else {
                button.image = NSImage(named: "key")
                button.imageScaling = .scaleAxesIndependently
            }
            button.attributedTitle = macKeyCode
            txt.stringValue = macKeyDesc
        }

        // Mouse button keyset
        refreshKey(map: 0, dir: PRESS_LEFT, button: devMouseLeftButton, txt: devMouseLeft)
        refreshKey(map: 0, dir: PRESS_RIGHT, button: devMouseRightButton, txt: devMouseRight)

        // First joystick keyset
        refreshKey(map: 1, dir: PULL_UP, button: devUp1button, txt: devUp1)
        refreshKey(map: 1, dir: PULL_DOWN, button: devDown1button, txt: devDown1)
        refreshKey(map: 1, dir: PULL_LEFT, button: devLeft1button, txt: devLeft1)
        refreshKey(map: 1, dir: PULL_RIGHT, button: devRight1button, txt: devRight1)
        refreshKey(map: 1, dir: PRESS_FIRE, button: devFire1button, txt: devFire1)

        // Second joystick keyset
        refreshKey(map: 2, dir: PULL_UP, button: devUp2button, txt: devUp2)
        refreshKey(map: 2, dir: PULL_DOWN, button: devDown2button, txt: devDown2)
        refreshKey(map: 2, dir: PULL_LEFT, button: devLeft2button, txt: devLeft2)
        refreshKey(map: 2, dir: PULL_RIGHT, button: devRight2button, txt: devRight2)
        refreshKey(map: 2, dir: PRESS_FIRE, button: devFire2button, txt: devFire2)

        devDisconnectKeys.state = pref.disconnectJoyKeys ? .on : .off

        // Joystick buttons
        assert(pref.autofire == joystick2.autofire)
        assert(pref.autofireBullets == joystick2.autofireBullets)
        assert(pref.autofireFrequency == joystick2.autofireFrequency)
        assert(joystick1.autofire == joystick2.autofire)
        assert(joystick1.autofireBullets == joystick2.autofireBullets)
        assert(joystick1.autofireFrequency == joystick2.autofireFrequency)
        devAutofire.state = pref.autofire ? .on : .off
        devAutofireCease.state = pref.autofireBullets > 0 ? .on : .off
        devAutofireBullets.integerValue = Int(pref.autofireBullets.magnitude)
        devAutofireFrequency.floatValue = pref.autofireFrequency
        devAutofireCease.isEnabled = devAutofire.state == .on
        devAutofireCeaseText.textColor = devAutofire.state == .on ? .controlTextColor : .disabledControlTextColor
        devAutofireBullets.isEnabled = devAutofire.state == .on
        devAutofireFrequency.isEnabled = devAutofire.state == .on
        
        // Mouse
        devRetainMouseKeyComb.selectItem(withTag: pref.retainMouseKeyComb)
        devRetainMouseKeyComb.isEnabled = pref.retainMouseWithKeys
        devRetainMouseWithKeys.state = pref.retainMouseWithKeys ? .on : .off
        devRetainMouseByClick.state = pref.retainMouseByClick ? .on : .off
        devRetainMouseByEntering.state = pref.retainMouseByEntering ? .on : .off
        devReleaseMouseKeyComb.selectItem(withTag: pref.releaseMouseKeyComb)
        devReleaseMouseKeyComb.isEnabled = pref.releaseMouseWithKeys
        devReleaseMouseWithKeys.state = pref.releaseMouseWithKeys ? .on : .off
        devReleaseMouseByShaking.state = pref.releaseMouseByShaking ? .on : .off
    }

    // Translates a button tag back to the related slot and gamepad action
    func gamePadAction(for tag: Int) -> (Int, GamePadAction) {

        switch tag {
        case 0...4:   return (1, GamePadAction(tag))      // Joy 1
        case 10...14: return (2, GamePadAction(tag - 10)) // Joy 2
        case 5...6:   return (0, GamePadAction(tag))      // Mouse
        default:      fatalError()
        }
    }

    //
    // Keyboard events
    //
    
    /* Handles a key press event.
     * Returns true if the controller has responded to this key.
     */
    func devKeyDown(with macKey: MacKey) -> Bool {

        // Only proceed if a recording sessing is in progress
        if devRecordedKey == nil { return false }
        
        track()
        
        // Record the key if it is not the ESC key
        if macKey != MacKey.escape {
            let (slot, action) = gamePadAction(for: devRecordedKey!)
            gamePadManager.gamePads[slot]?.bind(key: macKey, action: action)
        }

        devRecordedKey = nil
        refresh()
        return true
    }
    
    //
    // Action methods
    //
    
    @IBAction func devRecordKeyAction(_ sender: NSButton!) {

        devRecordedKey = sender.tag
        refresh()
    }
    
    @IBAction func devDisconnectKeysAction(_ sender: NSButton!) {
        
        pref.disconnectJoyKeys = (sender.state == .on)
        refresh()
    }

    @IBAction func devDeleteKeysetAction(_ sender: NSButton!) {

        assert(sender.tag >= 0 && sender.tag <= 2)
        
        pref.keyMaps[sender.tag] = [:]
        refresh()
    }

    @IBAction func devAutofireAction(_ sender: NSButton!) {
        
        pref.autofire = (sender.state == .on)
        refresh()
    }
    
    @IBAction func devAutofireCeaseAction(_ sender: NSButton!) {
        
        let sign = sender.state == .on ? 1 : -1
        let bullets = pref.autofireBullets.magnitude
        pref.autofireBullets = Int(bullets) * sign
        refresh()
    }
    
    @IBAction func devAutofireBulletsAction(_ sender: NSTextField!) {
        
        pref.autofireBullets = sender.integerValue
        refresh()
    }
    
    @IBAction func devAutofireFrequencyAction(_ sender: NSSlider!) {
        
        pref.autofireFrequency = sender.floatValue
        refresh()
    }
        
    @IBAction func devRetainMouseKeyCombAction(_ sender: NSPopUpButton!) {
        
        pref.retainMouseKeyComb = sender.selectedTag()
        refresh()
    }
    
    @IBAction func devRetainMouseAction(_ sender: NSButton!) {
        
        switch sender.tag {
            
        case 0: pref.retainMouseWithKeys   = (sender.state == .on)
        case 1: pref.retainMouseByClick    = (sender.state == .on)
        case 2: pref.retainMouseByEntering = (sender.state == .on)
        default: fatalError()
        }
        
        refresh()
    }
    
    @IBAction func devReleaseMouseKeyCombAction(_ sender: NSPopUpButton!) {
        
        pref.releaseMouseKeyComb = sender.selectedTag()
        refresh()
    }
    
    @IBAction func devReleaseMouseAction(_ sender: NSButton!) {
        
        switch sender.tag {
            
        case 0: pref.releaseMouseWithKeys  = (sender.state == .on)
        case 1: pref.releaseMouseByShaking = (sender.state == .on)
        default: fatalError()
        }
        
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    @IBAction func devPresetAction(_ sender: NSPopUpButton!) {
        
        track()
        assert(sender.selectedTag() == 0)
        
        UserDefaults.resetDevicesUserDefaults()
        pref.loadDevicesUserDefaults()
        refresh()
    }
}
