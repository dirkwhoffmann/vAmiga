// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// swiftlint:disable colon

/* List of Amiga key codes that are common to all keyboards
 */
struct AmigaKeycode {

    // Amiga keycodes 0x00 - 0x3F (Positional keys)
    
    /* "These are key codes assigned to specific positions on the main
     *  body of the keyboard. The letters on the tops of these keys are
     *  different for each country; not all countries use the QWERTY key
     *  layout. These keycodes are best described positionally [...]. The
     *  international keyboards have two more keys that are 'cut out' of
     *  larger keys on the USA version. These are $30, cut out from the the
     *  left shift, and $2B, cut out from the return key."
     *  [Amiga Hardware Reference, 3rd]
     */
    struct Ansi {
        static let grave         = 0x00
        static let digit1        = 0x01
        static let digit2        = 0x02
        static let digit3        = 0x03
        static let digit4        = 0x04
        static let digit5        = 0x05
        static let digit6        = 0x06
        static let digit7        = 0x07
        static let digit8        = 0x08
        static let digit9        = 0x09
        static let digit0        = 0x0A
        static let minus         = 0x0B
        static let equal         = 0x0C
        static let backslash     = 0x0D

        static let keypad0       = 0x0F
        
        static let q             = 0x10
        static let w             = 0x11
        static let e             = 0x12
        static let r             = 0x13
        static let t             = 0x14
        static let y             = 0x15
        static let u             = 0x16
        static let i             = 0x17
        static let o             = 0x18
        static let p             = 0x19
        static let lBracket      = 0x1A
        static let rBracket      = 0x1B
        
        static let keypad1       = 0x1D
        static let keypad2       = 0x1E
        static let keypad3       = 0x1F
        
        static let a             = 0x20
        static let s             = 0x21
        static let d             = 0x22
        static let f             = 0x23
        static let g             = 0x24
        static let h             = 0x25
        static let j             = 0x26
        static let k             = 0x27
        static let l             = 0x28
        static let semicolon     = 0x29
        static let quote         = 0x2A

        static let keypad4       = 0x2D
        static let keypad5       = 0x2E
        static let keypad6       = 0x2F
        
        static let z             = 0x31
        static let x             = 0x32
        static let c             = 0x33
        static let v             = 0x34
        static let b             = 0x35
        static let n             = 0x36
        static let m             = 0x37
        static let comma         = 0x38
        static let period        = 0x39
        static let slash         = 0x3A
        
        static let keypadDecimal = 0x3C
        static let keypad7       = 0x3D
        static let keypad8       = 0x3E
        static let keypad9       = 0x3F
    }
    
   // Extra keys on international Amigas (ISO style)
   struct Iso {
        static let hashtag       = 0x2B
        static let laceBrace     = 0x30
    }
    
    // Amiga keycodes 0x40 - 0x5F (Codes common to all keyboards)
    static let space             = 0x40
    static let backspace         = 0x41
    static let tab               = 0x42
    static let keypadEnter       = 0x43
    static let enter             = 0x44
    static let escape            = 0x45
    static let delete            = 0x46
    static let keypadMinus       = 0x4A
    static let cursorUp          = 0x4C
    static let cursorDown        = 0x4D
    static let cursorRight       = 0x4E
    static let cursorLeft        = 0x4F
    static let f1                = 0x50
    static let f2                = 0x51
    static let f3                = 0x52
    static let f4                = 0x53
    static let f5                = 0x54
    static let f6                = 0x55
    static let f7                = 0x56
    static let f8                = 0x57
    static let f9                = 0x58
    static let f10               = 0x59
    static let keypadLBracket    = 0x5A
    static let keypadRBracket    = 0x5B
    static let keypadDivide      = 0x5C
    static let keypadMultiply    = 0x5D
    static let keypadPlus        = 0x5E
    static let help              = 0x5F
    
    // 0x60 - 0x67 (Key codes for qualifier keys)
    static let leftShift         = 0x60
    static let rightShift        = 0x61
    static let capsLock          = 0x62
    static let control           = 0x63
    static let leftAlt           = 0x64
    static let rightAlt          = 0x65
    static let leftAmiga         = 0x66
    static let rightAmiga        = 0x67
}

// Country specific keycaps
let keycaps: [Int: [KBLayout: String]] = [
    
    AmigaKeycode.Ansi.grave:        [.generic: "~ `"],
    AmigaKeycode.Ansi.digit1:       [.generic: "! 1"],
    AmigaKeycode.Ansi.digit2:       [.generic: "@ 2", .german: "\" 2", .italian: "\" 2"],
    AmigaKeycode.Ansi.digit3:       [.generic: "# 3", .german: "§ 3", .italian: "\u{00A3} 3"],
    AmigaKeycode.Ansi.digit4:       [.generic: "$ 4"],
    AmigaKeycode.Ansi.digit5:       [.generic: "% 5"],
    AmigaKeycode.Ansi.digit6:       [.generic: "^ 6", .german: "& 6", .italian: "& 6"],
    AmigaKeycode.Ansi.digit7:       [.generic: "& 7", .german: "/ 7", .italian: "/ 7"],
    AmigaKeycode.Ansi.digit8:       [.generic: "* 8", .german: "( 8", .italian: "( 8"],
    AmigaKeycode.Ansi.digit9:       [.generic: "( 9", .german: ") 9", .italian: ") 9"],
    AmigaKeycode.Ansi.digit0:       [.generic: ") 0", .german: "= 0", .italian: "= 0"],
    AmigaKeycode.Ansi.minus:        [.generic: "_ -", .german: "? ß", .italian: "? ´"],
    AmigaKeycode.Ansi.equal:        [.generic: "+ =", .german: "` ´", .italian: "^ \u{00EC}"],
    AmigaKeycode.Ansi.backslash:    [.generic: "| \\"],
    
    AmigaKeycode.Ansi.keypad0:      [.generic: "0"],
        
    AmigaKeycode.Ansi.q:            [.generic: "Q"],
    AmigaKeycode.Ansi.w:            [.generic: "W"],
    AmigaKeycode.Ansi.e:            [.generic: "E"],
    AmigaKeycode.Ansi.r:            [.generic: "R"],
    AmigaKeycode.Ansi.t:            [.generic: "T"],
    AmigaKeycode.Ansi.y:            [.generic: "Y", .german: "Z"],
    AmigaKeycode.Ansi.u:            [.generic: "U"],
    AmigaKeycode.Ansi.i:            [.generic: "I"],
    AmigaKeycode.Ansi.o:            [.generic: "O"],
    AmigaKeycode.Ansi.p:            [.generic: "P"],
    AmigaKeycode.Ansi.lBracket:     [.generic: "{ [", .german: "Ü", .italian: "\u{00E9} \u{00E8}"],
    AmigaKeycode.Ansi.rBracket:     [.generic: "} ]", .german: "* +", .italian: "* +"],

    AmigaKeycode.Ansi.keypad1:      [.generic: "1"],
    AmigaKeycode.Ansi.keypad2:      [.generic: "2"],
    AmigaKeycode.Ansi.keypad3:      [.generic: "3"],

    AmigaKeycode.Ansi.a:            [.generic: "A"],
    AmigaKeycode.Ansi.s:            [.generic: "S"],
    AmigaKeycode.Ansi.d:            [.generic: "D"],
    AmigaKeycode.Ansi.f:            [.generic: "F"],
    AmigaKeycode.Ansi.g:            [.generic: "G"],
    AmigaKeycode.Ansi.h:            [.generic: "H"],
    AmigaKeycode.Ansi.j:            [.generic: "J"],
    AmigaKeycode.Ansi.k:            [.generic: "K"],
    AmigaKeycode.Ansi.l:            [.generic: "L"],
    AmigaKeycode.Ansi.semicolon:    [.generic: ": ;", .german: "Ö", .italian: "@ \u{00F2}"],
    AmigaKeycode.Ansi.quote:        [.generic: "\" ,", .german: "Ä", .italian: "# \u{00E0}"],

    AmigaKeycode.Ansi.keypad4:      [.generic: "4"],
    AmigaKeycode.Ansi.keypad5:      [.generic: "5"],
    AmigaKeycode.Ansi.keypad6:      [.generic: "6"],

    AmigaKeycode.Ansi.z:            [.generic: "Z", .german: "Y"],
    AmigaKeycode.Ansi.x:            [.generic: "X"],
    AmigaKeycode.Ansi.c:            [.generic: "C"],
    AmigaKeycode.Ansi.v:            [.generic: "V"],
    AmigaKeycode.Ansi.b:            [.generic: "B"],
    AmigaKeycode.Ansi.n:            [.generic: "N"],
    AmigaKeycode.Ansi.m:            [.generic: "M"],
    AmigaKeycode.Ansi.comma:        [.generic: "< ,", .german: "; ,", .italian: "; ,"],
    AmigaKeycode.Ansi.period:       [.generic: "> .", .german: ": .", .italian: ": ."],
    AmigaKeycode.Ansi.slash:        [.generic: "? /", .german: "_ -", .italian: "_ -"],

    AmigaKeycode.Ansi.keypadDecimal: [.generic: "."],
    AmigaKeycode.Ansi.keypad7:      [.generic: "7"],
    AmigaKeycode.Ansi.keypad8:      [.generic: "8"],
    AmigaKeycode.Ansi.keypad9:      [.generic: "9"],

    AmigaKeycode.Iso.hashtag:       [.generic: "", .german: "^ #", .italian: "§ \u{00F9}"],
    AmigaKeycode.Iso.laceBrace:     [.generic: "", .german: "> <", .italian: "> <"],

    AmigaKeycode.space:             [.generic: ""],
    AmigaKeycode.backspace:         [.generic: "\u{2190}"],
    AmigaKeycode.tab:               [.generic: "\u{21e4} \u{21e5}"],
    AmigaKeycode.keypadEnter:       [.generic: "\u{21b5}"],
    AmigaKeycode.enter:             [.generic: ""],
    AmigaKeycode.escape:            [.generic: "Esc"],
    AmigaKeycode.delete:            [.generic: "Del"],
    AmigaKeycode.keypadMinus:       [.generic: "-"],
    AmigaKeycode.cursorUp:          [.generic: "\u{2191}"],
    AmigaKeycode.cursorDown:        [.generic: "\u{2193}"],
    AmigaKeycode.cursorRight:       [.generic: "\u{2192}"],
    AmigaKeycode.cursorLeft:        [.generic: "\u{2190}"],
    AmigaKeycode.f1:                [.generic: "F1"],
    AmigaKeycode.f2:                [.generic: "F2"],
    AmigaKeycode.f3:                [.generic: "F3"],
    AmigaKeycode.f4:                [.generic: "F4"],
    AmigaKeycode.f5:                [.generic: "F5"],
    AmigaKeycode.f6:                [.generic: "F6"],
    AmigaKeycode.f7:                [.generic: "F7"],
    AmigaKeycode.f8:                [.generic: "F8"],
    AmigaKeycode.f9:                [.generic: "F9"],
    AmigaKeycode.f10:               [.generic: "F10"],
    AmigaKeycode.keypadLBracket:    [.generic: "(", .german: "{ [", .italian: "{ ["],
    AmigaKeycode.keypadRBracket:    [.generic: ")", .german: "} ]", .italian: "{ ["],
    AmigaKeycode.keypadDivide:      [.generic: "/"],
    AmigaKeycode.keypadMultiply:    [.generic: "*"],
    AmigaKeycode.keypadPlus:        [.generic: "+"],
    AmigaKeycode.help:              [.generic: "Help"],

    AmigaKeycode.leftShift:         [.generic: "\u{21e7}"],
    AmigaKeycode.rightShift:        [.generic: "\u{21e7}"],
    AmigaKeycode.capsLock:          [.generic: "Caps Lock"],
    AmigaKeycode.control:           [.generic: "Ctrl"],
    AmigaKeycode.leftAlt:           [.generic: "Alt"],
    AmigaKeycode.rightAlt:          [.generic: "Alt"],
    AmigaKeycode.leftAmiga:         [.generic: ""],
    AmigaKeycode.rightAmiga:        [.generic: ""]
]

/* This structure represents a physical keys on the Amiga keyboard.
 */
struct AmigaKey: Codable {
    
    // The unique identifier of this Amiga key
    var keyCode: Int = 0
    
    // The keycap label (country specific)
    var label: [KBLayout: String] = [:]
    
    // Initializers
    init(keyCode: Int) {
        
        self.keyCode = keyCode
        label = keycaps[keyCode] ?? [.generic: ""]
    }
}

extension AmigaKey: Equatable, Hashable {

    static func == (lhs: AmigaKey, rhs: AmigaKey) -> Bool {
        return lhs.keyCode == rhs.keyCode
    }
}

//
// Image factory
//

extension NSImage {

    func imprint(text: String,
                 x: CGFloat, y: CGFloat, font: NSFont, color: NSColor) {
                
        let w = size.width
        let h = size.height
        
        let textRect = CGRect(x: x, y: -y, width: w - x, height: h - y)
        let attributes: [NSAttributedString.Key: Any] = [
            .font: font,
            .foregroundColor: color
            ]
        lockFocus()
        text.draw(in: textRect, withAttributes: attributes)
        unlockFocus()
    }
    
    func imprint(text: String,
                 x: CGFloat, y: CGFloat, fontSize: CGFloat, tint: String) {
        
        let font = NSFont.systemFont(ofSize: fontSize)
        let color: NSColor = tint == "dark" ? .keyCapColor2 : .keyCapColor

        imprint(text: text, x: x, y: y, font: font, color: color)
    }
    
    func imprint(character c: Character,
                 x: CGFloat, y: CGFloat, fontSize: CGFloat, tint: String) {

        return imprint(text: String(c), x: x, y: y, fontSize: fontSize, tint: tint)
    }
}
            
extension AmigaKey {
    
    // Special keys (commons)
    private static let specialKeys: [Int: (String, String)] = [
        
        AmigaKeycode.Ansi.keypad0:   ("200x100", "white"),
        AmigaKeycode.escape:         ("100x100", "dark"),
        AmigaKeycode.f1:             ("125x100", "dark"),
        AmigaKeycode.f2:             ("125x100", "dark"),
        AmigaKeycode.f3:             ("125x100", "dark"),
        AmigaKeycode.f4:             ("125x100", "dark"),
        AmigaKeycode.f5:             ("125x100", "dark"),
        AmigaKeycode.f6:             ("125x100", "dark"),
        AmigaKeycode.f7:             ("125x100", "dark"),
        AmigaKeycode.f8:             ("125x100", "dark"),
        AmigaKeycode.f9:             ("125x100", "dark"),
        AmigaKeycode.f10:            ("125x100", "dark")
    ]
    
    // Special keys (A1000 commons)
    private static let a1000commons: [Int : (String, String)] = [
        
        AmigaKeycode.Ansi.grave:     ("125x100", "white"),
        AmigaKeycode.space:          ("750x100", "white"),
        AmigaKeycode.backspace:      ("175x100", "white"),
        AmigaKeycode.tab:            ("175x100", "white"),
        AmigaKeycode.keypadEnter:    ("200x100", "white"),
        AmigaKeycode.delete:         ("100x100", "dark"),
        AmigaKeycode.rightShift:     ("200x100", "white"),
        AmigaKeycode.leftAlt:        ("125x100", "white"),
        AmigaKeycode.rightAlt:       ("125x100", "white"),
        AmigaKeycode.leftAmiga:      ("150x100A", "white"),
        AmigaKeycode.rightAmiga:     ("150x100A", "white")
    ]
    
    // Special keys (A1000 ANSI like)
    private static let a1000ansi: [Int: (String, String)] = [
        
        AmigaKeycode.enter:          ("200x200", "white"),
        AmigaKeycode.leftShift:      ("250x100", "white")
    ]
    
    // Special keys (A1000 ISO like)
    private static let a1000iso: [Int: (String, String)] = [
        
        AmigaKeycode.enter:          ("200x200", "white"),
        AmigaKeycode.leftShift:      ("150x100", "white")
    ]
    
    // Special keys (A500 commons)
    private static let a500commons: [Int: (String, String)] = [
        
        AmigaKeycode.Ansi.grave:     ("150x100", "dark"),
        AmigaKeycode.space:          ("900x100", "white"),
        AmigaKeycode.backspace:      ("100x100", "dark"),
        AmigaKeycode.tab:            ("200x100", "dark"),
        AmigaKeycode.keypadEnter:    ("100x200", "dark"),
        AmigaKeycode.keypadMinus:    ("100x100", "dark"),
        AmigaKeycode.delete:         ("150x100", "dark"),
        AmigaKeycode.keypadLBracket: ("100x100", "dark"),
        AmigaKeycode.keypadRBracket: ("100x100", "dark"),
        AmigaKeycode.keypadDivide:   ("100x100", "dark"),
        AmigaKeycode.keypadMultiply: ("100x100", "dark"),
        AmigaKeycode.keypadPlus:     ("100x100", "dark"),
        AmigaKeycode.help:           ("150x100", "dark"),
        AmigaKeycode.rightShift:     ("250x100", "dark"),
        AmigaKeycode.control:        ("125x100", "dark"),
        AmigaKeycode.leftAlt:        ("125x100", "dark"),
        AmigaKeycode.rightAlt:       ("125x100", "dark"),
        AmigaKeycode.leftAmiga:      ("150x100A", "dark"),
        AmigaKeycode.rightAmiga:     ("150x100A", "dark")
    ]
    
    // Special keys (A500 ANSI like)
    private static let a500ansi: [Int: (String, String)] = [
        
        AmigaKeycode.enter:          ("225x200", "dark"),
        AmigaKeycode.leftShift:      ("275x100", "dark")
    ]
    
    // Special keys (A500 ISO like)
    private static let a500iso: [Int: (String, String)] = [
        
        AmigaKeycode.enter:          ("150x200", "dark"),
        AmigaKeycode.leftShift:      ("175x100", "dark")
    ]
    
    // Returns an unlabeled background image of the right shape
    private func bgImage(style: KBStyle, layout: KBLayout) -> (NSImage?, String) {
        
        var (shape, tint) = ("100x100", "white")
        
        // Determine physical keyboard layout (ignoring key labels)
        let a1000     = style == .narrow
        let a1000ansi = a1000 && layout == .us
        let a1000iso  = a1000 && layout != .us

        let a500     = style != .narrow
        let a500ansi = a500 && layout == .us
        let a500iso  = a500 && layout != .us
        
        // Crawl through the key descriptions
        if let info = AmigaKey.specialKeys[keyCode] {
            (shape, tint) = info
        } else if let info = AmigaKey.a1000commons[keyCode], a1000 {
            (shape, tint) = info
        } else if let info = AmigaKey.a1000ansi[keyCode], a1000ansi {
            (shape, tint) = info
        } else if let info = AmigaKey.a1000iso[keyCode], a1000iso {
            (shape, tint) = info
        } else if let info = AmigaKey.a500commons[keyCode], a500 {
            (shape, tint) = info
        } else if let info = AmigaKey.a500ansi[keyCode], a500ansi {
            (shape, tint) = info
        } else if let info = AmigaKey.a500iso[keyCode], a500iso {
            (shape, tint) = info
        }
        
        let image = NSImage(named: "shape" + shape)?.copy() as? NSImage
        if image == nil {
            warn("Missing shape \(shape)")
        }
        
        if tint == "dark" { image?.darken() }
        
        return (image, tint)
    }
    
    func image(style: KBStyle, layout: KBLayout) -> NSImage? {
        
        // Key label font sizes
        let large = CGFloat(15)
        let small = CGFloat(10)
        let tiny  = CGFloat(9) 

        // Get a background image
        let (image, tint) = bgImage(style: style, layout: layout)
        if image == nil { return nil }
        
        // Get the keycap label
        let label = self.label[layout] ?? self.label[.generic]!
        let parts = label.split(separator: " ")
    
        if parts.count == 1 {

            // Generate a standard label
            let size = (parts[0].count == 1) ? large : small
            image!.imprint(text: String(parts[0]), x: 8, y: 2, fontSize: size, tint: tint)
        }
        if parts.count == 2 {
            
            // Generate a stacked label
            let size = (parts[0].count == 1) ? small : tiny
            image!.imprint(text: String(parts[0]), x: 6, y: 2, fontSize: size, tint: tint)
            image!.imprint(text: String(parts[1]), x: 6, y: 9, fontSize: size, tint: tint)
        }
        
        return image
    }
}
