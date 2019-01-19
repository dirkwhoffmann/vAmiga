  // -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation
import Carbon.HIToolbox // Mac keycode identifiers

// Mapping from Mac key codes to Amiga key codes.
// Mac keycodes are based on the Apple Extended Keyboard II layout (ISO).

let isomac2amiga : [Int : Int] = [
    
    kVK_ISO_Section:         AmigaKeycode.ansi.grave,
    kVK_ANSI_1:              AmigaKeycode.ansi.digit1,
    kVK_ANSI_2:              AmigaKeycode.ansi.digit2,
    kVK_ANSI_3:              AmigaKeycode.ansi.digit3,
    kVK_ANSI_4:              AmigaKeycode.ansi.digit4,
    kVK_ANSI_5:              AmigaKeycode.ansi.digit5,
    kVK_ANSI_6:              AmigaKeycode.ansi.digit6,
    kVK_ANSI_7:              AmigaKeycode.ansi.digit7,
    kVK_ANSI_8:              AmigaKeycode.ansi.digit8,
    kVK_ANSI_9:              AmigaKeycode.ansi.digit9,
    kVK_ANSI_0:              AmigaKeycode.ansi.digit0,
    kVK_ANSI_Minus:          AmigaKeycode.ansi.minus,
    kVK_ANSI_Equal:          AmigaKeycode.ansi.equal,
    // MISSING ON MAC KEYBOARD AmigaKeycode.ansi.backslash:    [.us: "| \\"],
    
    kVK_ANSI_Keypad0:        AmigaKeycode.ansi.keypad0,
    
    kVK_ANSI_Q:              AmigaKeycode.ansi.q,
    kVK_ANSI_W:              AmigaKeycode.ansi.w,
    kVK_ANSI_E:              AmigaKeycode.ansi.e,
    kVK_ANSI_R:              AmigaKeycode.ansi.r,
    kVK_ANSI_T:              AmigaKeycode.ansi.t,
    kVK_ANSI_Y:              AmigaKeycode.ansi.y,
    kVK_ANSI_U:              AmigaKeycode.ansi.u,
    kVK_ANSI_I:              AmigaKeycode.ansi.i,
    kVK_ANSI_O:              AmigaKeycode.ansi.o,
    kVK_ANSI_P:              AmigaKeycode.ansi.p,
    kVK_ANSI_LeftBracket:    AmigaKeycode.ansi.lBracket,
    kVK_ANSI_RightBracket:   AmigaKeycode.ansi.rBracket,
    
    kVK_ANSI_Keypad1:        AmigaKeycode.ansi.keypad1,
    kVK_ANSI_Keypad2:        AmigaKeycode.ansi.keypad2,
    kVK_ANSI_Keypad3:        AmigaKeycode.ansi.keypad3,
    
    kVK_ANSI_A:              AmigaKeycode.ansi.a,
    kVK_ANSI_S:              AmigaKeycode.ansi.s,
    kVK_ANSI_D:              AmigaKeycode.ansi.d,
    kVK_ANSI_F:              AmigaKeycode.ansi.f,
    kVK_ANSI_G:              AmigaKeycode.ansi.g,
    kVK_ANSI_H:              AmigaKeycode.ansi.h,
    kVK_ANSI_J:              AmigaKeycode.ansi.j,
    kVK_ANSI_K:              AmigaKeycode.ansi.k,
    kVK_ANSI_L:              AmigaKeycode.ansi.l,
    kVK_ANSI_Semicolon:      AmigaKeycode.ansi.semicolon,
    kVK_ANSI_Quote:          AmigaKeycode.ansi.quote,
    
    kVK_ANSI_Keypad4:        AmigaKeycode.ansi.keypad4,
    kVK_ANSI_Keypad5:        AmigaKeycode.ansi.keypad5,
    kVK_ANSI_Keypad6:        AmigaKeycode.ansi.keypad6,
    
    kVK_ANSI_Z:              AmigaKeycode.ansi.z,
    kVK_ANSI_X:              AmigaKeycode.ansi.x,
    kVK_ANSI_C:              AmigaKeycode.ansi.c,
    kVK_ANSI_V:              AmigaKeycode.ansi.v,
    kVK_ANSI_B:              AmigaKeycode.ansi.b,
    kVK_ANSI_N:              AmigaKeycode.ansi.n,
    kVK_ANSI_M:              AmigaKeycode.ansi.m,
    kVK_ANSI_Comma:          AmigaKeycode.ansi.comma,
    kVK_ANSI_Period:         AmigaKeycode.ansi.period,
    kVK_ANSI_Slash:          AmigaKeycode.ansi.slash,
    
    kVK_ANSI_KeypadDecimal:  AmigaKeycode.ansi.keypadDecimal,
    kVK_ANSI_Keypad7:        AmigaKeycode.ansi.keypad7,
    kVK_ANSI_Keypad8:        AmigaKeycode.ansi.keypad8,
    kVK_ANSI_Keypad9:        AmigaKeycode.ansi.keypad9,
    
    kVK_ANSI_Backslash:      AmigaKeycode.iso.hashtag,
    kVK_ANSI_Grave:          AmigaKeycode.iso.laceBrace,
    
    kVK_Space:               AmigaKeycode.space,
    kVK_Delete:              AmigaKeycode.backspace,
    kVK_Tab:                 AmigaKeycode.tab,
    kVK_ANSI_KeypadEnter:    AmigaKeycode.keypadEnter,
    kVK_Return:              AmigaKeycode.enter,
    kVK_Escape:              AmigaKeycode.escape,
    kVK_ForwardDelete:       AmigaKeycode.delete,
    kVK_ANSI_KeypadMinus:    AmigaKeycode.keypadMinus,
    kVK_UpArrow:             AmigaKeycode.cursorUp,
    kVK_DownArrow:           AmigaKeycode.cursorDown,
    kVK_RightArrow:          AmigaKeycode.cursorRight,
    kVK_LeftArrow:           AmigaKeycode.cursorLeft,
    kVK_F1:                  AmigaKeycode.f1,
    kVK_F2:                  AmigaKeycode.f2,
    kVK_F3:                  AmigaKeycode.f3,
    kVK_F4:                  AmigaKeycode.f4,
    kVK_F5:                  AmigaKeycode.f6,
    kVK_F7:                  AmigaKeycode.f7,
    kVK_F8:                  AmigaKeycode.f8,
    kVK_F9:                  AmigaKeycode.f9,
    kVK_F10:                 AmigaKeycode.f10,
    kVK_ANSI_KeypadClear:    AmigaKeycode.keypadLBracket,
    kVK_ANSI_KeypadEquals:   AmigaKeycode.keypadRBracket,
    kVK_ANSI_KeypadDivide:   AmigaKeycode.keypadDivide,
    kVK_ANSI_KeypadMultiply: AmigaKeycode.keypadMultiply,
    kVK_ANSI_KeypadPlus:     AmigaKeycode.keypadPlus,
    kVK_F12:                 AmigaKeycode.help,
    
    kVK_Shift:               AmigaKeycode.leftShift,
    kVK_RightShift:          AmigaKeycode.rightShift,

    kVK_Control:             AmigaKeycode.control,
    kVK_Option:              AmigaKeycode.leftAlt,
    kVK_RightOption:         AmigaKeycode.rightAlt,
    /// ????? kVK_Command: AmigaKeycode.leftAmiga:         [.us: ""],
    /// AmigaKeycode.rightAmiga:        [.us: ""],
]


/* This structure represents a physical keys on the Mac keyboard.
 */
struct MacKey : Codable {
    
    // The unique identifier of this Mac key
    var keyCode: Int = 0
    
    /// Keycode in hex format as a string
    // var keyCodeStr: String { return String.init(format: "%02X", keyCode) }
    
    /// Textual description of this key
    var description: String?

    init(keyCode: Int, characters: String? = nil) {
        
        self.keyCode = keyCode
        self.description = characters
    }
    
    init(keyCode: UInt16, characters: String? = nil) {
        
        self.init(keyCode: Int(keyCode), characters: characters)
    }
    
    init(with event: NSEvent) {
        
        keyCode = Int(event.keyCode)
        
        let stdSymbols: [Int:String] = [
            kVK_Return: "\u{21a9}",
            kVK_Tab: "\u{21e5}",
            kVK_Space: "\u{23b5}",
            kVK_Delete: "\u{232b}",
            kVK_Escape: "\u{238b}",
            kVK_Shift: "\u{21e7}",
            kVK_Option: "\u{2325}",
            kVK_Control: "\u{2303}",
            kVK_F1: "F1",
            kVK_F2: "F2",
            kVK_F3: "F3",
            kVK_F4: "F4",
            kVK_F5: "F5",
            kVK_F6: "F6",
            kVK_F7: "F7",
            kVK_F8: "F8",
            kVK_LeftArrow: "\u{2190}",
            kVK_RightArrow: "\u{2192}",
            kVK_DownArrow: "\u{2193}",
            kVK_UpArrow: "\u{2191}"
        ]
        
        description = stdSymbols[Int(keyCode)] ?? event.charactersIgnoringModifiers
    }
    
    // Returns the Amiga key code for this Mac key
    var amigaKeyCode: Int {
       
        get {
            // Catch key 0x32 manually, because it has a different physical
            // position on an ANSI Mac keyboard.
            if keyCode == 0x32 {
                if KBGetLayoutType(Int16(LMGetKbdType())) == kKeyboardANSI {
                    return AmigaKeycode.ansi.grave
                }
            }
            
            return isomac2amiga[keyCode]!
        }
    }
}

extension MacKey: Equatable {
    static func ==(lhs: MacKey, rhs: MacKey) -> Bool {
        return lhs.keyCode == rhs.keyCode
    }
}

extension MacKey: Hashable {
    var hashValue: Int {
        return Int(self.keyCode)
    }
}

extension MacKey {
    
    // Layout independend keys. Keycodes are the same on all Mac keyboards
    static let ret = MacKey.init(keyCode: kVK_Return, characters: "\u{21a9}")
    static let tab = MacKey.init(keyCode: kVK_Tab, characters: "\u{21e5}")
    static let space = MacKey.init(keyCode: kVK_Space, characters: "\u{23b5}")
    static let delete = MacKey.init(keyCode: kVK_Delete, characters: "\u{232b}")
    static let escape = MacKey.init(keyCode: kVK_Escape, characters: "\u{238b}")
    static let leftShift = MacKey.init(keyCode: kVK_Shift, characters: "\u{21e7}")
    static let rightShift = MacKey.init(keyCode: kVK_RightShift, characters: "\u{21e7}")
    static let option = MacKey.init(keyCode: kVK_Option, characters: "\u{2325}")
    static let control = MacKey.init(keyCode: kVK_Control, characters: "\u{2303}")
    
    static let F1 = MacKey.init(keyCode: kVK_F1, characters: "F1")
    static let F2 = MacKey.init(keyCode: kVK_F2, characters: "F2")
    static let F3 = MacKey.init(keyCode: kVK_F3, characters: "F3")
    static let F4 = MacKey.init(keyCode: kVK_F4, characters: "F4")
    static let F5 = MacKey.init(keyCode: kVK_F5, characters: "F5")
    static let F6 = MacKey.init(keyCode: kVK_F6, characters: "F6")
    static let F7 = MacKey.init(keyCode: kVK_F7, characters: "F7")
    static let F8 = MacKey.init(keyCode: kVK_F8, characters: "F8")
    
    static let curLeft = MacKey.init(keyCode: kVK_LeftArrow, characters: "\u{2190}")
    static let curRight = MacKey.init(keyCode: kVK_RightArrow, characters: "\u{2192}")
    static let curDown = MacKey.init(keyCode: kVK_DownArrow, characters: "\u{2193}")
    static let curUp = MacKey.init(keyCode: kVK_UpArrow, characters: "\u{2191}")
    
    // Layout dependend keys. Keycodes refer to the keys on a standard ANSI US keyboard
    struct ansi {
        static let grave = MacKey.init(keyCode: kVK_ANSI_Grave, characters: "")
        static let digit0 = MacKey.init(keyCode: kVK_ANSI_0, characters: "0")
        static let digit1 = MacKey.init(keyCode: kVK_ANSI_1, characters: "1")
        static let digit2 = MacKey.init(keyCode: kVK_ANSI_2, characters: "2")
        static let digit3 = MacKey.init(keyCode: kVK_ANSI_3, characters: "3")
        static let digit4 = MacKey.init(keyCode: kVK_ANSI_4, characters: "4")
        static let digit5 = MacKey.init(keyCode: kVK_ANSI_5, characters: "5")
        static let digit6 = MacKey.init(keyCode: kVK_ANSI_6, characters: "6")
        static let digit7 = MacKey.init(keyCode: kVK_ANSI_7, characters: "7")
        static let digit8 = MacKey.init(keyCode: kVK_ANSI_8, characters: "8")
        static let digit9 = MacKey.init(keyCode: kVK_ANSI_9, characters: "9")
        static let minus = MacKey.init(keyCode: kVK_ANSI_Minus, characters: "")
        static let equal = MacKey.init(keyCode: kVK_ANSI_Equal, characters: "")
        
        static let A = MacKey.init(keyCode: kVK_ANSI_A, characters: "A")
        static let a = MacKey.init(keyCode: kVK_ANSI_A, characters: "a")
        static let B = MacKey.init(keyCode: kVK_ANSI_B, characters: "B")
        static let C = MacKey.init(keyCode: kVK_ANSI_C, characters: "C")
        static let c = MacKey.init(keyCode: kVK_ANSI_C, characters: "c")
        static let D = MacKey.init(keyCode: kVK_ANSI_D, characters: "D")
        static let d = MacKey.init(keyCode: kVK_ANSI_D, characters: "d")
        static let E = MacKey.init(keyCode: kVK_ANSI_E, characters: "E")
        static let e = MacKey.init(keyCode: kVK_ANSI_E, characters: "e")
        static let F = MacKey.init(keyCode: kVK_ANSI_F, characters: "F")
        static let G = MacKey.init(keyCode: kVK_ANSI_G, characters: "G")
        static let H = MacKey.init(keyCode: kVK_ANSI_H, characters: "H")
        static let I = MacKey.init(keyCode: kVK_ANSI_I, characters: "I")
        static let J = MacKey.init(keyCode: kVK_ANSI_J, characters: "J")
        static let K = MacKey.init(keyCode: kVK_ANSI_K, characters: "K")
        static let L = MacKey.init(keyCode: kVK_ANSI_L, characters: "L")
        static let M = MacKey.init(keyCode: kVK_ANSI_M, characters: "M")
        static let N = MacKey.init(keyCode: kVK_ANSI_N, characters: "N")
        static let O = MacKey.init(keyCode: kVK_ANSI_O, characters: "O")
        static let P = MacKey.init(keyCode: kVK_ANSI_P, characters: "P")
        static let Q = MacKey.init(keyCode: kVK_ANSI_Q, characters: "Q")
        static let R = MacKey.init(keyCode: kVK_ANSI_R, characters: "R")
        static let S = MacKey.init(keyCode: kVK_ANSI_S, characters: "S")
        static let s = MacKey.init(keyCode: kVK_ANSI_S, characters: "s")
        static let T = MacKey.init(keyCode: kVK_ANSI_T, characters: "T")
        static let U = MacKey.init(keyCode: kVK_ANSI_U, characters: "U")
        static let V = MacKey.init(keyCode: kVK_ANSI_V, characters: "V")
        static let W = MacKey.init(keyCode: kVK_ANSI_W, characters: "W")
        static let w = MacKey.init(keyCode: kVK_ANSI_W, characters: "w")
        static let X = MacKey.init(keyCode: kVK_ANSI_X, characters: "X")
        static let x = MacKey.init(keyCode: kVK_ANSI_X, characters: "x")
        static let Y = MacKey.init(keyCode: kVK_ANSI_Y, characters: "Y")
        static let y = MacKey.init(keyCode: kVK_ANSI_Y, characters: "y")
        static let Z = MacKey.init(keyCode: kVK_ANSI_Z, characters: "Z")

        static let leftBracket = MacKey.init(keyCode: kVK_ANSI_LeftBracket, characters: "[")
        static let rightBracket = MacKey.init(keyCode: kVK_ANSI_RightBracket, characters: "]")
        
        static let comma = MacKey.init(keyCode: kVK_ANSI_Comma, characters: ",")
        static let period = MacKey.init(keyCode: kVK_ANSI_Period, characters: ".")
        static let slash = MacKey.init(keyCode: kVK_ANSI_Slash, characters: "/")
        static let backSlash = MacKey.init(keyCode: kVK_ANSI_Backslash, characters: "\\")
        static let semicolon = MacKey.init(keyCode: kVK_ANSI_Semicolon, characters: ";")
        static let quote = MacKey.init(keyCode: kVK_ANSI_Quote, characters: "'")
    }
    
    // Layout dependend keys. Keycodes refer to the keys on a standard ISO US keyboard
    struct iso {
        static let hat = MacKey.init(keyCode: 0x0A, characters: "")
    }
}

