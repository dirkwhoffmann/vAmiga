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

        var map: [MacKey: UInt32]

        let manager    = parent.gamePadManager!
        let keyboard   = parent.kbController!
        let metal      = parent.metal!
        let joystick1  = amiga.joystick1!
        let joystick2  = amiga.joystick2!

        func refreshKey(dir: GamePadAction, button: NSButton, txt: NSTextField) {

            var macKeyCode: NSAttributedString = NSAttributedString.init()
            var macKeyDesc: String = ""

            // Which MacKey is assigned to this joystick action?
            for (key, direction) in map where direction == dir.rawValue {
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
        map = manager.gamePads[0]?.keyMap ?? [:]
        refreshKey(dir: PULL_UP, button: devUp1button, txt: devUp1)
        refreshKey(dir: PULL_DOWN, button: devDown1button, txt: devDown1)
        refreshKey(dir: PULL_LEFT, button: devLeft1button, txt: devLeft1)
        refreshKey(dir: PULL_RIGHT, button: devRight1button, txt: devRight1)
        refreshKey(dir: PRESS_FIRE, button: devFire1button, txt: devFire1)

        // Second joystick keyset
        map =  manager.gamePads[1]?.keyMap ?? [:]
        refreshKey(dir: PULL_UP, button: devUp2button, txt: devUp2)
        refreshKey(dir: PULL_DOWN, button: devDown2button, txt: devDown2)
        refreshKey(dir: PULL_LEFT, button: devLeft2button, txt: devLeft2)
        refreshKey(dir: PULL_RIGHT, button: devRight2button, txt: devRight2)
        refreshKey(dir: PRESS_FIRE, button: devFire2button, txt: devFire2)

        // Mouse button keyset
        map = manager.gamePads[2]?.keyMap ?? [:]
        refreshKey(dir: PRESS_LEFT, button: devMouseLeftButton, txt: devMouseLeft)
        refreshKey(dir: PRESS_RIGHT, button: devMouseRightButton, txt: devMouseRight)

        devDisconnectKeys.state = keyboard.disconnectJoyKeys ? .on : .off

        // Joystick buttons
        assert(joystick1.autofire() == joystick2.autofire())
        assert(joystick1.autofireBullets() == joystick2.autofireBullets())
        assert(joystick1.autofireFrequency() == joystick2.autofireFrequency())
        devAutofire.state = joystick1.autofire() ? .on : .off
        devAutofireCease.state = joystick1.autofireBullets() > 0 ? .on : .off
        devAutofireBullets.integerValue = Int(joystick1.autofireBullets().magnitude)
        devAutofireFrequency.floatValue = joystick1.autofireFrequency()
        devAutofireCease.isEnabled = devAutofire.state == .on
        devAutofireCeaseText.textColor = devAutofire.state == .on ? .controlTextColor : .disabledControlTextColor
        devAutofireBullets.isEnabled = devAutofire.state == .on
        devAutofireFrequency.isEnabled = devAutofire.state == .on
        
        // Mouse
        devRetainMouseKeyComb.selectItem(withTag: metal.retainMouseKeyComb)
        devRetainMouseKeyComb.isEnabled = metal.retainMouseWithKeys
        devRetainMouseWithKeys.state = metal.retainMouseWithKeys ? .on : .off
        devRetainMouseByClick.state = metal.retainMouseByClick ? .on : .off
        devRetainMouseByEntering.state = metal.retainMouseByEntering ? .on : .off
        devReleaseMouseKeyComb.selectItem(withTag: metal.releaseMouseKeyComb)
        devReleaseMouseKeyComb.isEnabled = metal.releaseMouseWithKeys
        devReleaseMouseWithKeys.state = metal.releaseMouseWithKeys ? .on : .off
        devReleaseMouseByShaking.state = metal.releaseMouseByShaking ? .on : .off

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
        
        parent.kbController.disconnectJoyKeys = (sender.state == .on)
        
        refresh()
    }

    @IBAction func devDeleteKeysetAction(_ sender: NSButton!) {

        assert(sender.tag >= 0 && sender.tag <= 2)

        let manager = parent.gamePadManager!
        manager.gamePads[sender.tag]?.keyMap = [:]
        refresh()
    }

    @IBAction func devAutofireAction(_ sender: NSButton!) {
        
        amiga.joystick1.setAutofire(sender.state == .on)
        amiga.joystick2.setAutofire(sender.state == .on)
        
        refresh()
    }
    
    @IBAction func devAutofireCeaseAction(_ sender: NSButton!) {
        
        let bullets = amiga.joystick1.autofireBullets().magnitude
        
        let sign = sender.state == .on ? 1 : -1
        amiga.joystick1.setAutofireBullets(Int(bullets) * sign)
        amiga.joystick2.setAutofireBullets(Int(bullets) * sign)

        refresh()
    }
    
    @IBAction func devAutofireBulletsAction(_ sender: NSTextField!) {
        
        let value = sender.integerValue
        
        amiga.joystick1.setAutofireBullets(value)
        amiga.joystick2.setAutofireBullets(value)
        
        refresh()
    }
    
    @IBAction func devAutofireFrequencyAction(_ sender: NSSlider!) {
        
        let value = sender.floatValue
        
        amiga.joystick1.setAutofireFrequency(value)
        amiga.joystick2.setAutofireFrequency(value)
        
        refresh()
    }
        
    @IBAction func devFactorySettingsAction(_ sender: Any!) {
        
        parent.resetDevicesUserDefaults()
        refresh()
    }

    @IBAction func devRetainMouseKeyCombAction(_ sender: NSPopUpButton!) {
        
        parent.metal.retainMouseKeyComb = sender.selectedTag()
        refresh()
    }
    
    @IBAction func devRetainMouseAction(_ sender: NSButton!) {
        
        switch sender.tag {
            
        case 0: parent.metal.retainMouseWithKeys   = (sender.state == .on)
        case 1: parent.metal.retainMouseByClick    = (sender.state == .on)
        case 2: parent.metal.retainMouseByEntering = (sender.state == .on)
        default: fatalError()
        }
        
        refresh()
    }
    
    @IBAction func devReleaseMouseKeyCombAction(_ sender: NSPopUpButton!) {
        
        parent.metal.releaseMouseKeyComb = sender.selectedTag()
        refresh()
    }
    
    @IBAction func devReleaseMouseAction(_ sender: NSButton!) {
        
        switch sender.tag {
            
        case 0: parent.metal.releaseMouseWithKeys  = (sender.state == .on)
        case 1: parent.metal.releaseMouseByShaking = (sender.state == .on)
        default: fatalError()
        }
        
        refresh()
    }
}
