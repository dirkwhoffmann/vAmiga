// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// Extensions to NSColor
//

extension NSColor {
    
    convenience init(r: Int, g: Int, b: Int, a: Int = 255) {
        
        self.init(red: CGFloat(r) / 255,
                  green: CGFloat(g) / 255,
                  blue: CGFloat(b) / 255,
                  alpha: CGFloat(a) / 255)
    }
    
    convenience init(r: Double, g: Double, b: Double, a: Double = 1.0) {

        self.init(red: CGFloat(r),
                  green: CGFloat(g),
                  blue: CGFloat(b),
                  alpha: CGFloat(a))
    }
    
    convenience init(_ rgb: (Double, Double, Double)) {
        
        self.init(r: rgb.0, g: rgb.1, b: rgb.2)
    }
    
    convenience init(rgba: UInt32) {
        
        let r = CGFloat(rgba & 0xFF)
        let g = CGFloat((rgba >> 8) & 0xFF)
        let b = CGFloat((rgba >> 16) & 0xFF)
        
        self.init(red: r / 255.0, green: g / 255.0, blue: b / 255.0, alpha: 1.0)
    }
    
    convenience init(amigaRGB: UInt16) {
        
        let r = CGFloat((amigaRGB >> 8) & 0xF)
        let g = CGFloat((amigaRGB >> 4) & 0xF)
        let b = CGFloat(amigaRGB & 0xF)
        
        self.init(red: r / 15.0, green: g / 15.0, blue: b / 15.0, alpha: 1.0)
    }
    
    func rgb() -> (Int, Int, Int) {
        
        let r = Int(redComponent * 255)
        let g = Int(greenComponent * 255)
        let b = Int(blueComponent * 255)
        
        return (r, g, b)
    }
    
    func rgba() -> (Int, Int, Int, Int) {
        
        let r = Int(redComponent * 255)
        let g = Int(greenComponent * 255)
        let b = Int(blueComponent * 255)
        
        return (r, g, b, 255)
    }
    
    func amigaRGB() -> UInt16 {
        
        let r = UInt16(redComponent * 15.0)
        let g = UInt16(greenComponent * 15.0)
        let b = UInt16(blueComponent * 15.0)
        
        return (r << 8) | (g << 4) | b
    }
    
    func adjust(brightness: CGFloat, saturation: CGFloat) -> NSColor {

        guard let colorInHSB = usingColorSpace(.deviceRGB) else { return self }

        var h: CGFloat = 0
        var s: CGFloat = 0
        var b: CGFloat = 0
        var a: CGFloat = 0

        colorInHSB.getHue(&h, saturation: &s, brightness: &b, alpha: &a)
        b = brightness
        s = saturation

        return NSColor(hue: h, saturation: s, brightness: b, alpha: a)
    }
}

//
// Extensions to NSColorWell
//

extension NSColorWell {
    
    func setColor(_ rgb: (Double, Double, Double) ) {
        
        color = NSColor(r: rgb.0, g: rgb.1, b: rgb.2)
    }
}

//
// Extensions to Renderer
//

extension Renderer {

    //
    // Image handling
    //
    
    func createBackgroundTexture() -> MTLTexture? {

        let size = NSSize(width: 256, height: 256)
        let wallpaper = NSImage(color: .lightGray, size: size)
        return wallpaper.toTexture(device: device)
    }
}
