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

//
// Logging / Debugging
// 

public func track(_ message: String = "",
                  path: String = #file, function: String = #function, line: Int = #line ) {
    
    if let file = URL.init(string: path)?.deletingPathExtension().lastPathComponent {
        if (message == "") {
            print("\(file).\(line)::\(function)")
        } else {
            print("\(file).\(line)::\(function): \(message)")
        }
    }
}


//
// Manipulating Strings
//

extension String {
    
    init?(keyCode: UInt16, carbonFlags: Int) {
        
        let source = TISCopyCurrentASCIICapableKeyboardLayoutInputSource().takeUnretainedValue()
        let layoutData = TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)
        let dataRef = unsafeBitCast(layoutData, to: CFData.self)
        let keyLayout = UnsafePointer<CoreServices.UCKeyboardLayout>.self
        let keyLayoutPtr = unsafeBitCast(CFDataGetBytePtr(dataRef), to: keyLayout)
        let modifierKeyState = (carbonFlags >> 8) & 0xFF;
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


//
// Handling URLs
//

extension URL {
    
    func addTimeStamp() -> URL {
        
        let path = self.deletingPathExtension().path
        let suffix = self.pathExtension
        
        let date = Date.init()
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd"
        let dateString = formatter.string(from: date)
        formatter.dateFormat = "hh.mm.ss"
        let timeString = formatter.string(from: date)
        let timeStamp = dateString + " at " + timeString

        return URL(fileURLWithPath: path + " " + timeStamp + "." + suffix)
    }
    
    func makeUnique() -> URL {
        
        let path = self.deletingPathExtension().path
        let suffix = self.pathExtension
        let fileManager = FileManager.default
        
        for i in 0...127 {
            
            let numberStr = (i == 0) ? "." : " \(i)."
            let url = URL(fileURLWithPath: path + numberStr + suffix)

            if !fileManager.fileExists(atPath: url.path) {
                return url
            }
        }
        return self
    }
}


//
// Processing images
//

extension Data {
    var bitmap: NSBitmapImageRep? {
        return NSBitmapImageRep(data: self)
    }
}

extension NSImage {
    
    func representation(using: NSBitmapImageRep.FileType) -> Data? {
        
        let bitmap = tiffRepresentation?.bitmap
        return bitmap?.representation(using: using, properties: [:])
    }
        
    func resizeImage(width: CGFloat, height: CGFloat,
                     cutout: NSRect,
                     interpolation: NSImageInterpolation = .high) -> NSImage {
        
        let img = NSImage(size: CGSize(width:width, height:height))
        
        img.lockFocus()
        let ctx = NSGraphicsContext.current
        ctx?.imageInterpolation = interpolation // NSImageInterpolation.none // .high
        self.draw(in: cutout,
                  from: NSMakeRect(0, 0, size.width, size.height),
                  operation: .copy,
                  fraction: 1)
        img.unlockFocus()
        
        return img
    }
    
    func resize(width: CGFloat, height: CGFloat) -> NSImage {
        
        let cutout = NSMakeRect(0, 0, width, height)
        return resizeImage(width: width, height: height,
                           cutout: cutout)
    }

    func resizeSharp(width: CGFloat, height: CGFloat) -> NSImage {
        
        let cutout = NSMakeRect(0, 0, width, height)
        return resizeImage(width: width, height: height,
                           cutout: cutout,
                           interpolation: .none)
    }
    
    func roundCorners(withRadius radius: CGFloat) -> NSImage {
        
        let rect = NSRect(origin: NSPoint.zero, size: size)
        if
            let cgImage = self.cgImage,
            let context = CGContext(data: nil,
                                    width: Int(size.width),
                                    height: Int(size.height),
                                    bitsPerComponent: 8,
                                    bytesPerRow: 4 * Int(size.width),
                                    space: CGColorSpaceCreateDeviceRGB(),
                                    bitmapInfo: CGImageAlphaInfo.premultipliedFirst.rawValue) {
            context.beginPath()
            context.addPath(CGPath(roundedRect: rect, cornerWidth: radius, cornerHeight: radius, transform: nil))
            context.closePath()
            context.clip()
            context.draw(cgImage, in: rect)
            
            if let composedImage = context.makeImage() {
                return NSImage(cgImage: composedImage, size: size)
            }
        }
        
        return self
    }
    
    func makeGlossy() {
        
        let width  = size.width
        let height = size.height
        let glossy = NSImage(named: "glossy")
        let rect   = NSRect(x: 0, y: 0, width: width, height: height)
        
        lockFocus()
        let sourceOver = NSCompositingOperation.sourceOver
        draw(in: rect, from: NSZeroRect, operation: sourceOver, fraction: 1.0)
        glossy!.draw(in: rect, from: NSZeroRect, operation: sourceOver, fraction: 1.0)
        unlockFocus()
    }
    
    func tint(_ color: NSColor) {
        
        let imageRect = NSRect.init(origin: .zero, size: size)
        
        lockFocus()
        color.set()
        imageRect.fill(using: .sourceAtop)
        unlockFocus()
    }
    
    func darken() {
        
        tint(NSColor.init(red: 0.0, green: 0.0, blue: 0.0, alpha: 0.33))
    }

    func pressed() {
        
        tint(NSColor.init(red: 1.0, green: 0.0, blue: 0.0, alpha: 0.5))
        // tint(NSColor.init(red: 0.0, green: 0.0, blue: 0.0, alpha: 0.15))
    }

    func red() {
        
        tint(NSColor.init(red: 1.0, green: 0.0, blue: 0.0, alpha: 0.5))
    }
}


//
// Managing time
//


extension DispatchTime {

    static func diffNano(_ t : DispatchTime) -> UInt64 {
        return DispatchTime.now().uptimeNanoseconds - t.uptimeNanoseconds
    }

    static func diffMicroSec(_ t : DispatchTime) -> UInt64 { return diffNano(t) / 1_000 }
    static func diffMilliSec(_ t : DispatchTime) -> UInt64 { return diffNano(t) / 1_000_000 }
    static func diffSec(_ t : DispatchTime) -> UInt64 { return diffNano(t) / 1_000_000_000 }
}
    

