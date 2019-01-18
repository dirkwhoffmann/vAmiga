// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

/* This structure represents a physical keys on the Amiga keyboard.
 * The Amiga identified each key with a keycode which is a 7 bit identifier.
 *
 * Note:
 *   - There are multiple physical keyboard layouts:
 *      A1000 US, A1000 Europe, A500/2000 US, A500/2000 Europe
 *   - Some keys carry different labels in different countries.
 *     Hence, two keys with the same keycode can be visually different.
 *
 */
struct AmigaKey : Codable {
    
    // The unique identifier of this Amiga key
    var keyCode: Int = 0
    
    // A textual description of this key (country specific)
    var description: [Region: String] = [:]
    
    // The characters thar are printed on the key (country specific)
    var label: [Region: String] = [:]
    
    /*
    init(keyCode: Int, description: [Region: String], label: [Region: String]) {
        
        self.keyCode = keyCode
        self.description = description
        self.label = label
    }
    */
    
    init(_ description: [Region: String], _ label: [Region: String]) {
        
        self.description = description
        self.label = label
    }

    init(_ description: String, _ label: [Region: String]) {
        
        self.init([.generic: description], label)
    }
    
    init(_ description: String, _ label: String) {
        
        self.init([.generic: description], [.generic: label])
    }
    
    /*
    init(keyCode: Int, description: String, label: String) {
        
        self.init(keyCode: keyCode, description: [.generic: description], label: [.generic: label])
    }
    */
    
    
    init(keyCode: Int) {
        
        switch (keyCode) {
            
            /* 0x00 - 0x3F
             * "These are key codes assigned to specific positions on the main
             *  body of the keyboard. The letters on the tops of these keys are
             *  different for each country; not all countries use the QWERTY key
             *  layout. These keycodes are best described positionally as shown
             *  in Figure 8-9 and Figure 8-10 at the end of the keyboard
             *  section. The international keyboards have two more keys that are
             *  'cut out' of larger keys on the USA version. These are $30, cut
             *  out from the the left shift, and $2B, cut out from the return
             *  key." [Amiga Hardware Reference, 3rd]
             */
            
        case 0x00: self.init("TILDE", "~`")
        case 0x01: self.init("1", "!1")
        case 0x02: self.init("2", "\u{0022}2")
        case 0x03: self.init("3", "\u{00A3}3")
        case 0x04: self.init("4", "$4")
        case 0x05: self.init("5", "%5")
        case 0x06: self.init("6", [.us: "^6", .german: "&6"])
        case 0x07: self.init("7", [.us: "&7", .german: "/7"])
        case 0x08: self.init("8", [.us: "*8", .german: "(8"])
        case 0x09: self.init("9", [.us: "(9", .german: ")9"])
        case 0x0A: self.init("0", [.us: ")0", .german: "=0"])
        
        //
        case 0x45: self.init("ESC", "ESC")
        case 0x50: self.init("F1", "F1")
        default:
            self.init("???", "?")
        }
        
        self.keyCode = keyCode
    }
}

