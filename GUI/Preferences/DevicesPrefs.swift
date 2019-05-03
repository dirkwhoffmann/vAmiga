// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension PreferencesController {
    
    func refreshDevicesTab() {

        guard
            let controller = myController,
            let amiga      = amigaProxy,
            let port1      = amiga.controlPort1,
            let port2      = amiga.controlPort2,
            let metal      = controller.metal
            else { return }
       
        // Joystick emulation keys
        updateJoyKeyMap(0, dir: JOYSTICK_UP, button: devUp1button, txt: devUp1)
        updateJoyKeyMap(0, dir: JOYSTICK_DOWN, button: devDown1button, txt: devDown1)
        updateJoyKeyMap(0, dir: JOYSTICK_LEFT, button: devLeft1button, txt: devLeft1)
        updateJoyKeyMap(0, dir: JOYSTICK_RIGHT, button: devRight1button, txt: devRight1)
        updateJoyKeyMap(0, dir: JOYSTICK_FIRE, button: devFire1button, txt: devFire1)
        updateJoyKeyMap(1, dir: JOYSTICK_UP, button: devUp2button, txt: devUp2)
        updateJoyKeyMap(1, dir: JOYSTICK_DOWN, button: devDown2button, txt: devDown2)
        updateJoyKeyMap(1, dir: JOYSTICK_LEFT, button: devLeft2button, txt: devLeft2)
        updateJoyKeyMap(1, dir: JOYSTICK_RIGHT, button: devRight2button, txt: devRight2)
        updateJoyKeyMap(1, dir: JOYSTICK_FIRE, button: devFire2button, txt: devFire2)
        devDisconnectKeys.state = controller.keyboardcontroller.disconnectJoyKeys ? .on : .off
        
        assert(port1.autofire() == port2.autofire())
        assert(port1.autofireBullets() == port2.autofireBullets())
        assert(port1.autofireFrequency() == port2.autofireFrequency())
        
        // Joystick buttons
        devAutofire.state = port1.autofire() ? .on : .off
        devAutofireCease.state = port1.autofireBullets() > 0 ? .on : .off
        devAutofireBullets.integerValue = Int(port1.autofireBullets().magnitude)
        devAutofireFrequency.floatValue = port1.autofireFrequency()
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
    }
    
    func updateJoyKeyMap(_ nr: Int, dir: JoystickDirection, button: NSButton, txt: NSTextField) {
        
        assert(nr == 0 || nr == 1)
        
        guard let manager = myController?.gamePadManager else { return }
        guard let keyMap = manager.gamePads[nr]?.keyMap else { return }
        
        // Which MacKey is assigned to this joystick action?
        var macKey: MacKey?
        var macKeyCode: NSAttributedString = NSAttributedString.init()
        var macKeyDesc: String = ""
        for (key, direction) in keyMap {
            if direction == dir.rawValue {
                let attr = [NSAttributedString.Key.foregroundColor: NSColor.black]
                macKey = key
                let myStr = NSString(format: "%02X", macKey!.keyCode) as String
                macKeyCode = NSAttributedString(string: myStr, attributes: attr)
                macKeyDesc = macKey?.stringValue ?? ""
                break
            }
        }
        
        // Update text and button image
        let recordKey = (nr == 0) ? devRecordKey1 : devRecordKey2
        if (recordKey == dir) {
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
    
    //
    // Keyboard events
    //
    
    func devKeyDown(with macKey: MacKey) {
        
        guard let manager = myController?.gamePadManager else { return }
        track()
        
        // Check for ESC key
        if macKey == MacKey.escape {
            cancelAction(self)
            return
        }
        
        if devRecordKey1 != nil {
            
            manager.gamePads[0]?.assign(key: macKey, direction: devRecordKey1!)
            devRecordKey1 = nil
        }
        if devRecordKey2 != nil {
            
            manager.gamePads[1]?.assign(key: macKey, direction: devRecordKey2!)
            devRecordKey2 = nil
        }
        
        refresh()
    }
    
    //
    // Action methods
    //
    
    @IBAction func devRecordKeyAction(_ sender: NSButton!) {
        
        let tag = UInt32(sender.tag)
        
        if tag >= 0 && tag <= 5 {
            devRecordKey1 = JoystickDirection(rawValue: tag)
            devRecordKey2 = nil
        } else if tag >= 10 && tag <= 15 {
            devRecordKey1 = nil
            devRecordKey2 = JoystickDirection(rawValue: (tag - 10))
        } else {
            assert(false);
        }
        
        refresh()
    }
    
    @IBAction func devDisconnectKeysAction(_ sender: NSButton!) {
        
        myController?.keyboardcontroller.disconnectJoyKeys = (sender.state == .on)
        
        refresh()
    }
    
    @IBAction func devAutofireAction(_ sender: NSButton!) {
        
        amigaProxy?.controlPort1.setAutofire(sender.state == .on)
        amigaProxy?.controlPort2.setAutofire(sender.state == .on)
        
        refresh()
    }
    
    @IBAction func devAutofireCeaseAction(_ sender: NSButton!) {
        
        if let bullets = amigaProxy?.controlPort1.autofireBullets().magnitude {
        
            let sign = sender.state == .on ? 1 : -1
            amigaProxy?.controlPort1.setAutofireBullets(Int(bullets) * sign)
            amigaProxy?.controlPort2.setAutofireBullets(Int(bullets) * sign)
            
            refresh()
        }
    }
    
    @IBAction func devAutofireBulletsAction(_ sender: NSTextField!) {
        
        let value = sender.integerValue
        
        amigaProxy?.controlPort1.setAutofireBullets(value)
        amigaProxy?.controlPort2.setAutofireBullets(value)
        
        refresh()
    }
    
    @IBAction func devAutofireFrequencyAction(_ sender: NSSlider!) {
        
        let value = sender.floatValue
        
        amigaProxy?.controlPort1.setAutofireFrequency(value)
        amigaProxy?.controlPort2.setAutofireFrequency(value)
        
        refresh()
    }
        
    @IBAction func devFactorySettingsAction(_ sender: Any!) {
        
        myController?.resetDevicesUserDefaults()
        refresh()
    }

    @IBAction func devRetainMouseKeyCombAction(_ sender: NSPopUpButton!) {
        
        myController?.metal.retainMouseKeyComb = sender.selectedTag()
        refresh()
    }
    
    @IBAction func devRetainMouseAction(_ sender: NSButton!) {
        
        switch (sender.tag) {
            
        case 0: myController?.metal.retainMouseWithKeys   = (sender.state == .on)
        case 1: myController?.metal.retainMouseByClick    = (sender.state == .on)
        case 2: myController?.metal.retainMouseByEntering = (sender.state == .on)
        default: fatalError()
        }
        
        refresh()
    }
    
    @IBAction func devReleaseMouseKeyCombAction(_ sender: NSPopUpButton!) {
        
        myController?.metal.releaseMouseKeyComb = sender.selectedTag()
        refresh()
    }
    
    @IBAction func devReleaseMouseAction(_ sender: NSButton!) {
        
        switch (sender.tag) {
            
        case 0: myController?.metal.releaseMouseWithKeys  = (sender.state == .on)
        case 1: myController?.metal.releaseMouseByShaking = (sender.state == .on)
        default: fatalError()
        }
        
        refresh()
    }
}
