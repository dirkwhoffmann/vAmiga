// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation
import Carbon.HIToolbox

/* Mapping from Mac key codes to Amiga key codes.
 * Mac keycodes are based on the Apple Extended Keyboard II layout (ISO).
 */
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
    // MISSING ON MAC(?!):   AmigaKeycode.ansi.backslash:
    
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
    kVK_Command:             AmigaKeycode.leftAmiga,
    kVK_RightCommand:        AmigaKeycode.rightAmiga,
]

/* Mapping from Mac key codes to textual representations.
 * The mapping only covers keys with an empty standard representation.
 */
let mac2string : [Int : String] = [
    
    kVK_ANSI_Keypad0:     "\u{2327}", // ⌧
    kVK_ANSI_KeypadEnter: "\u{2305}", // ⌅
    kVK_Return:           "\u{21A9}", // ↩
    kVK_Tab:              "\u{21E5}", // ⇥
    kVK_Space:            "\u{2423}", // ␣
    kVK_Delete:           "\u{232B}", // ⌫
    kVK_Escape:           "\u{238B}", // ⎋
    kVK_Command:          "\u{2318}", // ⌘
    kVK_RightCommand:     "\u{2318}", // ⌘
    kVK_Shift:            "\u{21E7}", // ⇧
    kVK_RightShift:       "\u{21e7}", // ⇧
    kVK_CapsLock:         "\u{21EA}", // ⇪
    kVK_Option:           "\u{2325}", // ⌥
    kVK_RightOption:      "\u{2325}", // ⌥
    kVK_Control:          "\u{2732}", // ✲
    kVK_RightControl:     "\u{2732}", // ✲
    
    kVK_F1:               "F1",
    kVK_F2:               "F2",
    kVK_F3:               "F3",
    kVK_F4:               "F4",
    kVK_F5:               "F5",
    kVK_F6:               "F6",
    kVK_F7:               "F7",
    kVK_F8:               "F8",
    kVK_F9:               "F9",
    kVK_F10:              "F10",
    kVK_F11:              "F11",
    kVK_F12:              "F12",
    kVK_F13:              "F13",
    kVK_F14:              "F14",
    kVK_F15:              "F15",
    kVK_F16:              "F16",
    kVK_F17:              "F17",
    kVK_F18:              "F18",
    kVK_F19:              "F19",
    kVK_F20:              "F20",
    kVK_Help:             "?\u{20DD}",// ?⃝
    kVK_Home:             "\u{2196}", // ↖
    kVK_PageUp:           "\u{21DE}", // ⇞
    kVK_PageDown:         "\u{21DF}", // ⇟
    kVK_ForwardDelete:    "\u{2326}", // ⌦
    kVK_End:              "\u{2198}", // ↘
    kVK_LeftArrow:        "\u{2190}", // ←
    kVK_RightArrow:       "\u{2192}", // →
    kVK_UpArrow:          "\u{2191}", // ↑
    kVK_DownArrow:        "\u{2193}", // ↓
]

/* This structure represents a physical key of the Mac keyboard.
 */
struct MacKey : Codable {
    
    // The unique identifier of this Mac key
    var keyCode: Int = 0
    
    // Modifier flags at the time the key was pressed
    var carbonFlags: Int = 0

    
    init(keyCode: Int, flags: NSEvent.ModifierFlags = []) {
    
        self.keyCode = keyCode
        
        if flags.contains(.shift)   { carbonFlags |= shiftKey }
        if flags.contains(.control) { carbonFlags |= controlKey }
        if flags.contains(.option)  { carbonFlags |= optionKey }
        if flags.contains(.command) { carbonFlags |= cmdKey }
    }
 
    init(keyCode: UInt16, flags: NSEvent.ModifierFlags = []) {
        
        self.init(keyCode: Int(keyCode), flags: flags)
    }
    
    init(event: NSEvent) {
        
        self.init(keyCode: event.keyCode, flags: event.modifierFlags)
    }
   
    // Returns the modifier flags of this key
    var modifierFlags: NSEvent.ModifierFlags {
        get {
            var cocoaFlags : NSEvent.ModifierFlags = []
            
            if (carbonFlags & shiftKey)   != 0 { cocoaFlags.insert(.shift) }
            if (carbonFlags & controlKey) != 0 { cocoaFlags.insert(.control) }
            if (carbonFlags & optionKey)  != 0 { cocoaFlags.insert(.option) }
            if (carbonFlags & cmdKey)     != 0 { cocoaFlags.insert(.command) }

            return cocoaFlags
        }
    }
    
    // Returns the Amiga key code for this key
    var amigaKeyCode: Int {
       
        get {
            // Catch key 0x32 manually, because it has a different physical
            // position on an ANSI Mac keyboard.
            if keyCode == 0x32, KBGetLayoutType(Int16(LMGetKbdType())) == kKeyboardANSI {
                return AmigaKeycode.ansi.grave
            }
            return isomac2amiga[keyCode]!
        }
    }
    
    // Returns a string representation for this key
    var stringValue : String {
        get {
            // Check if this key has a custom representation
            if let s = mac2string[keyCode] {
                return s
            }
            
            // Return (keyboard dependent) standard representation
            let source = TISCopyCurrentASCIICapableKeyboardLayoutInputSource().takeUnretainedValue()
            let layoutData = TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)
            let dataRef = unsafeBitCast(layoutData, to: CFData.self)
            let keyLayout = UnsafePointer<CoreServices.UCKeyboardLayout>.self
            let keyLayoutPtr = unsafeBitCast(CFDataGetBytePtr(dataRef), to: keyLayout)
            let keyTranslateOptions = OptionBits(CoreServices.kUCKeyTranslateNoDeadKeysBit)
            var deadKeyState: UInt32 = 0
            let maxChars = 256
            var length = 0
            var chars = [UniChar](repeating: 0, count: maxChars)
            
            let error = CoreServices.UCKeyTranslate(keyLayoutPtr,
                                                    UInt16(keyCode),
                                                    UInt16(CoreServices.kUCKeyActionDisplay),
                                                    UInt32(carbonFlags),
                                                    UInt32(LMGetKbdType()),
                                                    keyTranslateOptions,
                                                    &deadKeyState,
                                                    maxChars,
                                                    &length,
                                                    &chars)

            return error == noErr ? NSString(characters: &chars, length: length).uppercased : ""
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
    
    static let escape       = MacKey.init(keyCode: kVK_Escape)
    static let shift        = MacKey.init(keyCode: kVK_Shift)
    static let rightShift   = MacKey.init(keyCode: kVK_RightShift)
    static let option       = MacKey.init(keyCode: kVK_Option)
    static let rightOption  = MacKey.init(keyCode: kVK_RightOption)
    static let control      = MacKey.init(keyCode: kVK_Control)
    static let rightControl = MacKey.init(keyCode: kVK_RightControl)
    static let command      = MacKey.init(keyCode: kVK_Command)
    static let rightCommand = MacKey.init(keyCode: kVK_RightCommand)
}
  
