// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

/* List of Amiga key codes that are common to all keyboards
 */
struct AmigaKeycodes {

    static let numPadNull     = 0x0F
    
    // 0x40 - 0x5F
    static let space          = 0x40
    static let backspace      = 0x41
    static let tab            = 0x42
    static let numPadEnter    = 0x43
    static let enter          = 0x44
    static let escape         = 0x45
    static let delete         = 0x46
    static let numPadMinus    = 0x4A
    static let numPadUp       = 0x4C
    static let cursorDown     = 0x4D
    static let cursorRight    = 0x4E
    static let cursorLeft     = 0x4F
    static let f1             = 0x50
    static let f2             = 0x51
    static let f3             = 0x52
    static let f4             = 0x53
    static let f5             = 0x54
    static let f6             = 0x55
    static let f7             = 0x56
    static let f8             = 0x57
    static let f9             = 0x58
    static let f10            = 0x59
    static let numPadL        = 0x5A
    static let numPadR        = 0x5B
    static let numPadSlash    = 0x5C
    static let numPadAsterisk = 0x5D
    static let numPadPlus     = 0x5E
    static let help           = 0x5F

    // 0x60 - 0x67 (Qualifier keys)
    static let leftShift      = 0x60
    static let rightShift     = 0x61
    static let capsLock       = 0x62
    static let control        = 0x63
    static let leftAlt        = 0x64
    static let rightAlt       = 0x65
    static let leftAmiga      = 0x66
    static let rightAmiga     = 0x67
}
 
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
    var description: [Country: String] = [:]
    
    // The characters thar are printed on the key (country specific)
    var label: [Country: String] = [:]
    
    /*
    init(keyCode: Int, description: [Country: String], label: [Country: String]) {
        
        self.keyCode = keyCode
        self.description = description
        self.label = label
    }
    */
    
    init(_ description: [Country: String], _ label: [Country: String]) {
        
        self.description = description
        self.label = label
    }

    init(_ description: String, _ label: [Country: String]) {
        
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
        
        switch keyCode {
            
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
        case 0x06: self.init("6", [.us: "^6", .germany: "&6"])
        case 0x07: self.init("7", [.us: "&7", .germany: "/7"])
        case 0x08: self.init("8", [.us: "*8", .germany: "(8"])
        case 0x09: self.init("9", [.us: "(9", .germany: ")9"])
        case 0x0A: self.init("0", [.us: ")0", .germany: "=0"])
        case 0x0B: self.init("",  [.us: "_-", .germany: "?ß"])
        case 0x0C: self.init("",  [.us: "+=", .germany: "`´"])
        case 0x0D: self.init("",  "|\\")
            
        //
        // case 0x45: self.init("ESC", "ESC")
        // case 0x50: self.init("F1", "F1")
        default:
            self.init("???", "?")
        }
        
        self.keyCode = keyCode
    }
}

extension AmigaKey : Equatable {
    static func ==(lhs: AmigaKey, rhs: AmigaKey) -> Bool {
        return lhs.keyCode == rhs.keyCode
    }
}

extension AmigaKey : Hashable {
    var hashValue: Int {
        return keyCode
    }
}

//
// Image factory
//

extension NSImage {
    
    func imprint(text: String, x: CGFloat, y: CGFloat, fontSize: CGFloat) {
        
        let font = NSFont.systemFont(ofSize: fontSize)
        
        let w = size.width
        let h = size.height
        track("\(w) \(h)")
        
        let textRect = CGRect(x: x, y: -y, width: w - x, height: h - y)
        // let paragraphStyle = NSMutableParagraphStyle()
        let attributes : [NSAttributedString.Key : Any] = [
            .font: font,
            .foregroundColor: NSColor.textColor,
            // .paragraphStyle: paragraphStyle,
            ]
        lockFocus()
        text.draw(in: textRect, withAttributes: attributes)
        unlockFocus()
    }
    
    func imprint(character c: Character, x: CGFloat, y: CGFloat, fontSize: CGFloat) {

        return imprint(text: String(c), x: x, y: y, fontSize: fontSize)
    }
}
            
extension AmigaKey {
    
    // Special keys (commons)
    static let specialKeys : [Int : (String,String)] = [
        
        AmigaKeycodes.escape:         ("100x100", "dark"),
        AmigaKeycodes.f1:             ("125x100", "dark"),
        AmigaKeycodes.f2:             ("125x100", "dark"),
        AmigaKeycodes.f3:             ("125x100", "dark"),
        AmigaKeycodes.f4:             ("125x100", "dark"),
        AmigaKeycodes.f5:             ("125x100", "dark"),
        AmigaKeycodes.f6:             ("125x100", "dark"),
        AmigaKeycodes.f7:             ("125x100", "dark"),
        AmigaKeycodes.f8:             ("125x100", "dark"),
        AmigaKeycodes.f9:             ("125x100", "dark"),
        AmigaKeycodes.f10:            ("125x100", "dark"),
        AmigaKeycodes.numPadL:        ("100x100", "dark"),
        AmigaKeycodes.numPadR:        ("100x100", "dark"),
        AmigaKeycodes.numPadSlash:    ("100x100", "dark"),
        AmigaKeycodes.numPadAsterisk: ("100x100", "dark"),
        AmigaKeycodes.numPadMinus:    ("100x100", "dark"),
        AmigaKeycodes.numPadPlus:     ("100x100", "dark"),
        AmigaKeycodes.numPadNull:     ("100x100", "white")
    ]
    
    // Special keys (A1000 commons)
    static let specialKeysA1000 : [Int : (String,String)] = [
        
        AmigaKeycodes.leftAlt:        ("125x100", "white"),
        AmigaKeycodes.leftAmiga:      ("125x100", "white"),
        AmigaKeycodes.rightAlt:       ("125x100", "white"),
        AmigaKeycodes.rightAmiga:     ("125x100", "white"),
        AmigaKeycodes.numPadEnter:    ("200x100", "white")
    ]
    
    // Special keys (A1000 US)
    static let specialKeysA1000US : [Int : (String,String)] = [
        
        AmigaKeycodes.enter:          ("200x200", "white"),
        AmigaKeycodes.leftShift:      ("250x100", "white")
    ]
    
    // Special keys (A1000 International)
    static let specialKeysA1000Int : [Int : (String,String)] = [
        
        AmigaKeycodes.enter:          ("125x200", "white"),
        AmigaKeycodes.leftShift:      ("155x100", "white")
    ]
    
    // Special keys (A500 commons)
    static let specialKeysA500 : [Int : (String,String)] = [
        
        AmigaKeycodes.leftAlt:        ("125x100", "dark"),
        AmigaKeycodes.leftAmiga:      ("125x100", "dark"),
        AmigaKeycodes.rightAlt:       ("125x100", "dark"),
        AmigaKeycodes.rightAmiga:     ("125x100", "dark"),
        AmigaKeycodes.numPadEnter:    ("200x100", "dark")
    ]
    
    // Special keys (A500 US)
    static let specialKeysA500US : [Int : (String,String)] = [
        
        AmigaKeycodes.enter:          ("225x200", "white"),
        AmigaKeycodes.leftShift:      ("275x100", "white")
    ]
    
    // Special keys (A500 International)
    static let specialKeysA500Int : [Int : (String,String)] = [
        
        AmigaKeycodes.enter:          ("150x200", "white"),
        AmigaKeycodes.leftShift:      ("175x100", "white")
    ]
    
    // Returns an unlabeled background image of the right shape
    func backgroundImage(model: AmigaModel, country: Country) -> NSImage? {
        
        // Crawl through the key description arrays
        if let info = AmigaKey.specialKeys[keyCode] {
            return NSImage(named: info.1 + info.0)
        } else if let info = AmigaKey.specialKeysA1000[keyCode], model == A1000 {
            return NSImage(named: info.1 + info.0)
        } else if let info = AmigaKey.specialKeysA1000US[keyCode], model == A1000, country == .us {
            return NSImage(named: info.1 + info.0)
        } else if let info = AmigaKey.specialKeysA1000Int[keyCode], model == A1000, country != .us {
            return NSImage(named: info.1 + info.0)
        } else if let info = AmigaKey.specialKeysA500[keyCode], model == A500 {
            return NSImage(named: info.1 + info.0)
        } else if let info = AmigaKey.specialKeysA500US[keyCode], model != A1000, country == .us {
            return NSImage(named: info.1 + info.0)
        } else if let info = AmigaKey.specialKeysA500Int[keyCode], model != A1000, country != .us {
            return NSImage(named: info.1 + info.0)
        } else {
            return NSImage(named: "white100x100")
        }
    }
    
    func image(model: AmigaModel, country: Country) -> NSImage? {
        
        // Get a background image
        guard let image = backgroundImage(model: model, country: country)?.copy() as? NSImage else {
            return nil
        }

        // Get the key label
        guard let text = label[country] else {
            return image
        }
        
        track("text = \(text)")
        
        // Write label
        switch text.count {
            
        case 0:
            break
            
        case 1:
            image.imprint(text: text, x: 8, y: 2, fontSize: 18)
            
        case 2:
             image.imprint(character: Array(text)[0], x: 6, y: 2, fontSize: 13)
             image.imprint(character: Array(text)[1], x: 6, y: 8, fontSize: 13)
        default:
            break
        }
        
        return image
    }
}
