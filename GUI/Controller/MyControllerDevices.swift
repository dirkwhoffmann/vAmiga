// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyController {
    
    //
    // Keyboard events
    //
    
    // Keyboard events are handled by the emulator window.
    // If they are handled here, some keys such as 'TAB' don't trigger an event.
    
    //
    //  Game port events
    //
    
    // DEPRECATED
    func connect(device: Int, port: Int) {
        
        let cpd: ControlPortDevice =
            device == InputDevice.none ? CPD_NONE :
                device == InputDevice.mouse ? CPD_MOUSE : CPD_JOYSTICK
        
        amiga.suspend()
        if port == 1 { amiga.controlPort1.connect(cpd) }
        if port == 2 { amiga.controlPort2.connect(cpd) }
        amiga.resume()
    }
    
    // Feeds game pad actions into the port associated with a certain game pad
    @discardableResult
    func emulateEventsOnGamePort(slot: Int, events: [GamePadAction]) -> Bool {
        
        if slot == config.gameDevice1 {
            return emulateEventsOnGamePort1(events)
        }
        if slot == config.gameDevice2 {
            return emulateEventsOnGamePort2(events)
        }
        return false
    }
    
    // Feeds game pad actions into the Amiga's first control port
    func emulateEventsOnGamePort1(_ events: [GamePadAction]) -> Bool {
        
        for event in events {
            amiga.joystick1.trigger(event)
            amiga.mouse.trigger(event)
        }
        return events != []
    }
    
    // Feeds game pad actions into the Amiga's second control port
    func emulateEventsOnGamePort2(_ events: [GamePadAction]) -> Bool {
        
        for event in events {
            amiga.joystick2.trigger(event)
            amiga.mouse.trigger(event)
        }
        return events != []
    }
}
