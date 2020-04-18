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
            for (key, direction) in prefs.keyMaps[map] where direction == dir.rawValue {
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

        // First joystick keyset
        refreshKey(map: 0, dir: PULL_UP, button: devUp1button, txt: devUp1)
        refreshKey(map: 0, dir: PULL_DOWN, button: devDown1button, txt: devDown1)
        refreshKey(map: 0, dir: PULL_LEFT, button: devLeft1button, txt: devLeft1)
        refreshKey(map: 0, dir: PULL_RIGHT, button: devRight1button, txt: devRight1)
        refreshKey(map: 0, dir: PRESS_FIRE, button: devFire1button, txt: devFire1)

        // Second joystick keyset
        refreshKey(map: 1, dir: PULL_UP, button: devUp2button, txt: devUp2)
        refreshKey(map: 1, dir: PULL_DOWN, button: devDown2button, txt: devDown2)
        refreshKey(map: 1, dir: PULL_LEFT, button: devLeft2button, txt: devLeft2)
        refreshKey(map: 1, dir: PULL_RIGHT, button: devRight2button, txt: devRight2)
        refreshKey(map: 1, dir: PRESS_FIRE, button: devFire2button, txt: devFire2)

        // Mouse button keyset
        refreshKey(map: 2, dir: PRESS_LEFT, button: devMouseLeftButton, txt: devMouseLeft)
        refreshKey(map: 2, dir: PRESS_RIGHT, button: devMouseRightButton, txt: devMouseRight)

        devDisconnectKeys.state = prefs.disconnectJoyKeys ? .on : .off

        // Joystick buttons
        assert(prefs.autofire == joystick2.autofire())
        assert(prefs.autofireBullets == joystick2.autofireBullets())
        assert(prefs.autofireFrequency == joystick2.autofireFrequency())
        assert(joystick1.autofire() == joystick2.autofire())
        assert(joystick1.autofireBullets() == joystick2.autofireBullets())
        assert(joystick1.autofireFrequency() == joystick2.autofireFrequency())
        devAutofire.state = prefs.autofire ? .on : .off
        devAutofireCease.state = prefs.autofireBullets > 0 ? .on : .off
        devAutofireBullets.integerValue = Int(prefs.autofireBullets.magnitude)
        devAutofireFrequency.floatValue = prefs.autofireFrequency
        devAutofireCease.isEnabled = devAutofire.state == .on
        devAutofireCeaseText.textColor = devAutofire.state == .on ? .controlTextColor : .disabledControlTextColor
        devAutofireBullets.isEnabled = devAutofire.state == .on
        devAutofireFrequency.isEnabled = devAutofire.state == .on
        
        // Mouse
        devRetainMouseKeyComb.selectItem(withTag: prefs.retainMouseKeyComb)
        devRetainMouseKeyComb.isEnabled = prefs.retainMouseWithKeys
        devRetainMouseWithKeys.state = prefs.retainMouseWithKeys ? .on : .off
        devRetainMouseByClick.state = prefs.retainMouseByClick ? .on : .off
        devRetainMouseByEntering.state = prefs.retainMouseByEntering ? .on : .off
        devReleaseMouseKeyComb.selectItem(withTag: prefs.releaseMouseKeyComb)
        devReleaseMouseKeyComb.isEnabled = prefs.releaseMouseWithKeys
        devReleaseMouseWithKeys.state = prefs.releaseMouseWithKeys ? .on : .off
        devReleaseMouseByShaking.state = prefs.releaseMouseByShaking ? .on : .off

        // OK Button
        devOKButton.title = buttonLabel
    }

    // Translates a button tag back to the related slot and gamepad action
    func gamePadAction(for tag: Int) -> (Int, GamePadAction) {

        switch tag {
        case 0...4:   return (0, GamePadAction(UInt32(tag)))
        case 10...14: return (1, GamePadAction(UInt32(tag - 10)))
        case 5...6:   return (2, GamePadAction(UInt32(tag)))
        default: fatalError()
        }
    }

    //
    // Keyboard events
    //
    
    func devKeyDown(with macKey: MacKey) {
        
        let manager = parent.gamePadManager!

        // Check for the ESC key
        if macKey == MacKey.escape {

            // Close sheet if no key is being recorded at the moment
            if devRecordedKey == nil { cancelAction(self); return }

            // Abort the recording sessing
            devRecordedKey = nil
        }

        if let rec = devRecordedKey {
            let (slot, action) = gamePadAction(for: rec)
            manager.gamePads[slot]?.bind(key: macKey, action: action)
            devRecordedKey = nil
        }

        refresh()
    }
    
    //
    // Action methods
    //
    
    @IBAction func devRecordKeyAction(_ sender: NSButton!) {

        devRecordedKey = sender.tag
        refresh()
    }
    
    @IBAction func devDisconnectKeysAction(_ sender: NSButton!) {
        
        prefs.disconnectJoyKeys = (sender.state == .on)
        
        refresh()
    }

    @IBAction func devDeleteKeysetAction(_ sender: NSButton!) {

        assert(sender.tag >= 0 && sender.tag <= 2)
        
        prefs.keyMaps[sender.tag] = [:]
        refresh()
    }

    @IBAction func devAutofireAction(_ sender: NSButton!) {
        
        for amiga in myAppDelegate.proxies {
            amiga.joystick1.setAutofire(sender.state == .on)
            amiga.joystick2.setAutofire(sender.state == .on)
        }
        refresh()
    }
    
    @IBAction func devAutofireCeaseAction(_ sender: NSButton!) {
        
        let sign = sender.state == .on ? 1 : -1
        let bullets = prefs.autofireBullets.magnitude
        
        for amiga in myAppDelegate.proxies {
            amiga.joystick1.setAutofireBullets(Int(bullets) * sign)
            amiga.joystick2.setAutofireBullets(Int(bullets) * sign)
        }
        refresh()
    }
    
    @IBAction func devAutofireBulletsAction(_ sender: NSTextField!) {
        
        let value = sender.integerValue
        
        for amiga in myAppDelegate.proxies {
            amiga.joystick1.setAutofireBullets(value)
            amiga.joystick2.setAutofireBullets(value)
        }
        refresh()
    }
    
    @IBAction func devAutofireFrequencyAction(_ sender: NSSlider!) {
        
        let value = sender.floatValue
        
        for amiga in myAppDelegate.proxies {
            amiga.joystick1.setAutofireFrequency(value)
            amiga.joystick2.setAutofireFrequency(value)
        }
        refresh()
    }
        
    @IBAction func devFactorySettingsAction(_ sender: Any!) {
        
        UserDefaults.resetDevicesUserDefaults()
        prefs.loadDevicesUserDefaults()
        refresh()
    }

    @IBAction func devRetainMouseKeyCombAction(_ sender: NSPopUpButton!) {
        
        prefs.retainMouseKeyComb = sender.selectedTag()
        refresh()
    }
    
    @IBAction func devRetainMouseAction(_ sender: NSButton!) {
        
        switch sender.tag {
            
        case 0: prefs.retainMouseWithKeys   = (sender.state == .on)
        case 1: prefs.retainMouseByClick    = (sender.state == .on)
        case 2: prefs.retainMouseByEntering = (sender.state == .on)
        default: fatalError()
        }
        
        refresh()
    }
    
    @IBAction func devReleaseMouseKeyCombAction(_ sender: NSPopUpButton!) {
        
        prefs.releaseMouseKeyComb = sender.selectedTag()
        refresh()
    }
    
    @IBAction func devReleaseMouseAction(_ sender: NSButton!) {
        
        switch sender.tag {
            
        case 0: prefs.releaseMouseWithKeys  = (sender.state == .on)
        case 1: prefs.releaseMouseByShaking = (sender.state == .on)
        default: fatalError()
        }
        
        refresh()
    }
}
