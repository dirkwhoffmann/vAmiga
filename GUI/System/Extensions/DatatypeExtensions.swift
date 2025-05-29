// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Carbon.HIToolbox

//
// Size
//

extension NSSize {
    
    /*
    func scaled(x: CGFloat = CGFloat(1.0), y: CGFloat = CGFloat(1.0)) -> NSSize {
        return NSSize(width: self.width * x, height: self.height * y)
    }
    */
    
    func scaled(x: Float = 1.0, y: Float = 1.0) -> NSSize {
        return NSSize(width: self.width * CGFloat(x), height: self.height * CGFloat(y))
    }
}

//
// Array
//

extension Array {
    
    func at(_ index: Int) -> Element? {
        
        indices.contains(index) ? self[index] : nil
    }
}

//
// Comparable
//

extension Comparable {
    
    func clamped(_ f: Self, _ t: Self) -> Self {
        
        var r = self
        if r > t { r = t }
        if r < f { r = f }
        return r
    }
}

//
// Data
//

extension Data {
    
    var bitmap: NSBitmapImageRep? {
        return NSBitmapImageRep(data: self)
    }
}

//
// Double
//

extension Double {
   
    func truncate(digits: Int) -> Double {
        let factor = Double(truncating: pow(10, digits) as NSNumber)
        return (self * factor).rounded() / factor
    }
}

//
// Strings
//

extension String {
    
    func indicesOf(string: String) -> [Int] {
        
        var indices = [Int]()
        var searchStartIndex = self.startIndex
        
        while searchStartIndex < self.endIndex,
              let range = self.range(of: string, range: searchStartIndex..<self.endIndex),
              !range.isEmpty {
            
            let index = distance(from: self.startIndex, to: range.lowerBound)
            indices.append(index)
            searchStartIndex = range.upperBound
        }
        
        return indices
    }

    init(capacity: Int) {

        let kb = capacity / 1024
        let mb = capacity / (1024 * 1024)
        let gb = capacity / (1024 * 1024 * 1024)
        let kbfrac = (capacity * 100 / 1024) % 100
        let mbfrac = (capacity * 100 / (1024 * 1024)) % 100
        let gbfrac = (capacity * 100 / (1024 * 1024 * 1024)) % 100

        if (capacity < 1024) {
            self.init("\(capacity) Bytes")
        } else if (capacity < 1024 * 1024) {
            self.init(kbfrac == 0 ? "\(kb) KB" : "\(kb).\(kbfrac) KB")
        } else if (capacity < 1024 * 1024 * 1024) {
            self.init(mbfrac == 0 ? "\(mb) MB" : "\(mb).\(mbfrac) MB")
        } else {
            self.init(gbfrac == 0 ? "\(gb) GB" : "\(gb).\(gbfrac) GB")
        }
    }

    init?(keyCode: UInt16, carbonFlags: Int) {
        
        let source = TISCopyCurrentASCIICapableKeyboardLayoutInputSource().takeRetainedValue()
        let layoutData = TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)
        let dataRef = unsafeBitCast(layoutData, to: CFData.self)
        let keyLayout = UnsafePointer<CoreServices.UCKeyboardLayout>.self
        let keyLayoutPtr = unsafeBitCast(CFDataGetBytePtr(dataRef), to: keyLayout)
        let modifierKeyState = (carbonFlags >> 8) & 0xFF
        let keyTranslateOptions = OptionBits(CoreServices.kUCKeyTranslateNoDeadKeysBit)
        var deadKeyState: UInt32 = 0
        let maxChars = 1
        var length = 0
        var chars = [UniChar](repeating: 0, count: maxChars)
        
        let error = CoreServices.UCKeyTranslate(keyLayoutPtr,
                                                keyCode,
                                                UInt16(CoreServices.kUCKeyActionDisplay),
                                                UInt32(modifierKeyState),
                                                UInt32(LMGetKbdType()),
                                                keyTranslateOptions,
                                                &deadKeyState,
                                                maxChars,
                                                &length,
                                                &chars)
        if error == noErr {
            self.init(NSString(characters: &chars, length: length))
        } else {
            return nil
        }
    }
}

extension NSAttributedString {
    
    convenience init(_ text: String, size: CGFloat, color: NSColor) {
        
        let paraStyle = NSMutableParagraphStyle()
        paraStyle.alignment = .center

        let attr: [NSAttributedString.Key: Any] = [
            .font: NSFont.systemFont(ofSize: size),
            .foregroundColor: color,
            .paragraphStyle: paraStyle
        ]
        
        self.init(string: text, attributes: attr)
    }
}
