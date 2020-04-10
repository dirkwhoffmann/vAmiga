// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// swiftlint:disable function_parameter_count

import Cocoa
import Metal

extension UInt32 {
    
    init(r: UInt8, g: UInt8, b: UInt8) {
        let red = UInt32(r) << 24
        let green = UInt32(g) << 16
        let blue = UInt32(b) << 8
        self.init(bigEndian: red | green | blue)
    }
}

//
// Extensions to CGImage
//

public extension CGImage {
    
    static func bitmapInfo() -> CGBitmapInfo {
        
        let noAlpha = CGImageAlphaInfo.noneSkipLast.rawValue
        let bigEn32 = CGBitmapInfo.byteOrder32Big.rawValue
    
        return CGBitmapInfo(rawValue: noAlpha | bigEn32)
    }
    
    static func dataProvider(data: UnsafeMutableRawPointer, size: CGSize) -> CGDataProvider? {
        
        let dealloc: CGDataProviderReleaseDataCallback = {
            
            (info: UnsafeMutableRawPointer?, data: UnsafeRawPointer, size: Int) -> Void in
            
            // Core Foundation objects are memory managed, aren't they?
            return
        }
        
        return CGDataProvider(dataInfo: nil,
                              data: data,
                              size: 4 * Int(size.width) * Int(size.height),
                              releaseData: dealloc)
    }
    
    /// Creates a CGImage from a raw data stream in 32 bit big endian format
    static func make(data: UnsafeMutableRawPointer, size: CGSize) -> CGImage? {
        
        let w = Int(size.width)
        let h = Int(size.height)
        
        return CGImage(width: w, height: h,
                       bitsPerComponent: 8,
                       bitsPerPixel: 32,
                       bytesPerRow: 4 * w,
                       space: CGColorSpaceCreateDeviceRGB(),
                       bitmapInfo: bitmapInfo(),
                       provider: dataProvider(data: data, size: size)!,
                       decode: nil,
                       shouldInterpolate: false,
                       intent: CGColorRenderingIntent.defaultIntent)
    }
    
    /// Creates a CGImage from a MTLTexture
    static func make(texture: MTLTexture, rect: CGRect) -> CGImage? {
        
        // Compute texture cutout
        //   (x,y) : upper left corner
        //   (w,h) : width and height
        let x = Int(CGFloat(texture.width) * rect.minX)
        let y = Int(CGFloat(texture.height) * rect.minY)
        let w = Int(CGFloat(texture.width) * rect.width)
        let h = Int(CGFloat(texture.height) * rect.height)
        
        // Get texture data as a byte stream
        guard let data = malloc(4 * w * h) else { return nil; }
        texture.getBytes(data,
                         bytesPerRow: 4 * w,
                         from: MTLRegionMake2D(x, y, w, h),
                         mipmapLevel: 0)
        
        // Copy data over to a new buffer of double horizontal width
        /*
         let w2 = 2 * w
         let h2 = h
         let bytesPerRow2 = 2 * bytesPerRow
         let size2 = bytesPerRow2 * h2
         guard let data2 = malloc(size2) else { return nil; }
         let ptr = data.assumingMemoryBound(to: UInt32.self)
         let ptr2 = data2.assumingMemoryBound(to: UInt32.self)
         for i in 0 ... (w * h) {
         ptr2[2 * i] = ptr[i]
         ptr2[2 * i + 1] = ptr[i]
         }
         */
        
        return make(data: data, size: CGSize.init(width: w, height: h))
    }
}

//
// Buffer utilities
//

extension UnsafeMutablePointer where Pointee == UInt32 {

    func darken(size: MTLSize, region: MTLRegion, factor: Double) {

        var index = size.width * region.origin.y + region.origin.x
        let skip = size.width - region.size.width
        assert(skip >= 0)
        
        for _ in 0 ..< region.size.height {
            for _ in 0 ..< region.size.width {
                
                let b = UInt32(Double(self[index] >> 16 & 0xFF) * factor) & 0xFF
                let g = UInt32(Double(self[index] >> 8 & 0xFF) * factor) & 0xFF
                let r = UInt32(Double(self[index] >> 0 & 0xFF) * factor) & 0xFF
                let c = self[index] & 0xFF000000 | b << 16 | g << 8 | r
                self[index] = c
                index += 1
            }
            index += skip
        }
    }
          
    func drawLine(size: MTLSize, y: Int, border: Int) {
        
        let width = size.width - 2 * border
        let origin = MTLOrigin.init(x: border, y: size.height - y, z: 0)
        let lineSize = MTLSize.init(width: width, height: 1, depth: 0)
        let region = MTLRegion.init(origin: origin, size: lineSize)
        
        darken(size: size, region: region, factor: 1.5)
    }
    
    func drawGrid(size: MTLSize, y1: Int, y2: Int, lines: Int, logScale: Bool) {
        
        let height = Double(y2 - y1)
        
        for i in 0 ... lines {
            
            var y = Double(i) / Double(lines)
            // if logScale { y = log10(1.0 + 9.0 * y) }
            if logScale { y = log(1.0 + 19.0 * y) / log(20) }
            drawLine(size: size, y: Int(Double(y1) + height * y), border: 20)
        }
    }
    
    func drawGradient(size: MTLSize,
                      region: MTLRegion? = nil,
                      r1: Int, g1: Int, b1: Int, a1: Int,
                      r2: Int, g2: Int, b2: Int, a2: Int) {
        
        let origin = region?.origin ?? MTLOriginMake(0, 0, 0)
        let w = region?.size.width ?? size.width
        let h = region?.size.height ?? size.height
        
        let dr = Double(r2 - r1) / Double(h); var r = Double(r1)
        let dg = Double(g2 - g1) / Double(h); var g = Double(g1)
        let db = Double(b2 - b1) / Double(h); var b = Double(b1)
        let da = Double(a2 - a1) / Double(h); var a = Double(a1)

        // Create gradient
        var index = size.width * origin.y + origin.x
        let skip = Int(size.width) - w
        assert(skip >= 0)
        
        for _ in 0 ..< h {
            let c = UInt32(a) << 24 | UInt32(b) << 16 | UInt32(g) << 8 | UInt32(r)
            for _ in 0 ..< w {
                self[index] = c
                index += 1
            }
            r += dr; g += dg; b += db; a += da
            index += skip
        }
    }
    
    func makeRoundCorner(size: MTLSize, radius: Int) {
        
        let w = size.width
        let h = size.height
        
        for row in 0 ..< radius {
            let dy = radius - row
            for col in 0 ..< radius {
                let dx = radius - col
                if dx*dx + dy*dy >= radius*radius {
                    self[row * w + col] &= 0xFFFFFF
                    self[(h - 1 - row) * w + col] &= 0xFFFFFF
                    self[(h - 1 - row) * w + (w - 1 - col)] &= 0xFFFFFF
                    self[row * w + (w - 1 - col)] &= 0xFFFFFF
                }
            }
        }
    }
    
    func imprint(size: MTLSize, text: String) {
        
        // Wrap this buffer into a CGContext
        let alphaInfo = CGImageAlphaInfo.premultipliedLast.rawValue
        let orderInfo = CGBitmapInfo.byteOrder32Big.rawValue
        let context = CGContext.init(data: self,
                                     width: size.width,
                                     height: size.height,
                                     bitsPerComponent: 8,
                                     bytesPerRow: 4 * size.width,
                                     space: CGColorSpaceCreateDeviceRGB(),
                                     bitmapInfo: alphaInfo | orderInfo)
        
        // Create an attributed string
        let string = NSAttributedString.init(text, size: 28, color: .white)
       
        // Draw text centered on top
        let line = CTLineCreateWithAttributedString(string)
        let offset = CTLineGetPenOffsetForFlush(line, 0.5, Double(size.width))
        context!.textPosition = CGPoint(x: Int(offset), y: size.height - 38)
        CTLineDraw(line, context!)
    }
}

//
// Extensions to MTLDevice
//

extension MTLTexture {

    func replace(region: MTLRegion,
                 r1: Int, g1: Int, b1: Int, a1: Int,
                 r2: Int, g2: Int, b2: Int, a2: Int,
                 radius: Int) {
        
        let w = region.size.width
        let h = region.size.height
        
        // Allocate working buffer
        let data = UnsafeMutablePointer<UInt32>.allocate(capacity: w * h)
        
        // Create gradient
        data.drawGradient(size: region.size,
                          r1: r1, g1: g1, b1: b1, a1: a1,
                          r2: r2, g2: g2, b2: b2, a2: a2)
        
        // Create round corners
        data.makeRoundCorner(size: region.size, radius: radius)
        
        // Copy working buffer into texture
        replace(region: region, mipmapLevel: 0, withBytes: data, bytesPerRow: 4 * w)
        data.deallocate()
    }
}

extension MTLDevice {
    
    func makeTexture(from data: UnsafeMutablePointer<UInt32>,
                     size: MTLSize) -> MTLTexture? {
        
        // Create texture
        let descriptor = MTLTextureDescriptor.texture2DDescriptor(
            pixelFormat: MTLPixelFormat.rgba8Unorm,
            width: size.width,
            height: size.height,
            mipmapped: false)
        descriptor.usage = [ .shaderRead ]
        let texture = makeTexture(descriptor: descriptor)
        
        // Copy buffer into texture
        let region = MTLRegionMake2D(0, 0, size.width, size.height)
        texture?.replace(region: region,
                         mipmapLevel: 0,
                         withBytes: data,
                         bytesPerRow: 4 * size.width)
        
        return texture
    }

    /*
    func makeGradientTexture(width: Int, height: Int,
                             r1: Int, g1: Int, b1: Int, a1: Int,
                             r2: Int, g2: Int, b2: Int, a2: Int,
                             radius: Int = 0) -> MTLTexture? {
        
        let d = MTLTextureDescriptor.texture2DDescriptor(
              pixelFormat: MTLPixelFormat.rgba8Unorm,
              width: width,
              height: height,
              mipmapped: false)
        d.usage = [ .shaderRead ]
        
        let data = UnsafeMutablePointer<UInt32>.allocate(capacity: width * height)
        
        var r = Double(r1)
        var g = Double(g1)
        var b = Double(b1)
        var a = Double(a1)
        let dr = Double(r2 - r1) / Double(height)
        let dg = Double(g2 - g1) / Double(height)
        let db = Double(b2 - b1) / Double(height)
        let da = Double(a2 - a1) / Double(height)
        
        for row in 0 ..< height {
            let c = UInt32(a) << 24 | UInt32(b) << 16 | UInt32(g) << 8 | UInt32(r)
            for col in 0 ..< width {
                data[row * width + col] = c
            }
            r += dr; g += dg; b += db; a += da
        }
        
        for row in 0 ..< radius {
            let dy = radius - row
            for col in 0 ..< radius {
                let dx = radius - col
                if dx*dx + dy*dy >= radius*radius {
                    data[row * width + col] &= 0xFFFFFF
                    data[(height - 1 - row) * width + col] &= 0xFFFFFF
                    data[(height - 1 - row) * width + (width - 1 - col)] &= 0xFFFFFF
                    data[row * width + (width - 1 - col)] &= 0xFFFFFF
                }
            }
        }
        
        let context = CGContext.init(data: data,
                                     width: width,
                                     height: height,
                                     bitsPerComponent: 8,
                                     bytesPerRow: 4 * width,
                                     space: CGColorSpaceCreateDeviceRGB(),
                                     bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue | CGBitmapInfo.byteOrder32Big.rawValue
        )
        
        let font = NSFont.systemFont(ofSize: 32)
        let paragraphStyle = NSMutableParagraphStyle()
        paragraphStyle.alignment = .center
        let attr: [NSAttributedString.Key: Any] = [
            .font: font,
            .foregroundColor: NSColor.white,
            .paragraphStyle: paragraphStyle
        ]
        let astr = NSAttributedString.init(string: "Blitter", attributes: attr)
        let line = CTLineCreateWithAttributedString(astr)
        
        let offset = CTLineGetPenOffsetForFlush(line, 0.5, Double(width))
        context!.textPosition = CGPoint(x: Int(offset), y: height - 48)
        CTLineDraw(line, context!)
        
        let region = MTLRegionMake2D(0, 0, width, height)
        let texture = makeTexture(descriptor: d)
        texture!.replace(region: region,
                         mipmapLevel: 0,
                         withBytes: data,
                         bytesPerRow: 4 * width)
        
        data.deallocate()
        /*
        let buffer = makeBuffer(bytes: data, length: bytes, options: d.resourceOptions)
        return buffer?.makeTexture(descriptor: d, offset: 0, bytesPerRow: bytesPerRow)
        */
        return texture
    }
     */
}

//
// Extensions to NSColor
//

extension NSColor {
    
    convenience init(_ rgb: (Double, Double, Double)) {
        
        self.init(r: rgb.0, g: rgb.1, b: rgb.2)
    }
}

//
// Extensions to NSImage
//

public extension NSImage {
    
    convenience init(color: NSColor, size: NSSize) {
        
        self.init(size: size)
        lockFocus()
        color.drawSwatch(in: NSRect(origin: .zero, size: size))
        unlockFocus()
    }

    static func make(texture: MTLTexture, rect: CGRect) -> NSImage? {
        
        guard let cgImage = CGImage.make(texture: texture, rect: rect) else {
            track("Failed to create CGImage.")
            return nil
        }
        
        let size = NSSize(width: cgImage.width, height: cgImage.height)
        return NSImage(cgImage: cgImage, size: size)
    }

    static func make(data: UnsafeMutableRawPointer, rect: CGSize) -> NSImage? {
        
        guard let cgImage = CGImage.make(data: data, size: rect) else {
            track("Failed to create CGImage.")
            return nil
        }
        
        let size = NSSize(width: cgImage.width, height: cgImage.height)
        return NSImage(cgImage: cgImage, size: size)
    }

    func expand(toSize size: NSSize) -> NSImage? {
 
        let newImage = NSImage.init(size: size)
    
        NSGraphicsContext.saveGraphicsState()
        newImage.lockFocus()

        let t = NSAffineTransform()
        t.translateX(by: 0.0, yBy: size.height)
        t.scaleX(by: 1.0, yBy: -1.0)
        t.concat()
        
        let inRect = NSRect.init(x: 0, y: 0, width: size.width, height: size.height)
        let fromRect = NSRect.init(x: 0, y: 0, width: self.size.width, height: self.size.height)
        let operation = NSCompositingOperation.copy
        self.draw(in: inRect, from: fromRect, operation: operation, fraction: 1.0)
        
        newImage.unlockFocus()
        NSGraphicsContext.restoreGraphicsState()
        
        return newImage
    }
    
    var cgImage: CGImage? {
        var rect = CGRect.init(origin: .zero, size: self.size)
        return self.cgImage(forProposedRect: &rect, context: nil, hints: nil)
    }
    
    func cgImageWH() -> (CGImage, Int, Int)? {
        
        if let cgi = cgImage(forProposedRect: nil, context: nil, hints: nil) {
            if cgi.width != 0 && cgi.height != 0 {
                return (cgi, cgi.width, cgi.height)
            }
        }
        return nil
    }
    
    func toData(vflip: Bool = false) -> UnsafeMutableRawPointer? {
        
        guard let (cgimage, width, height) = cgImageWH() else { return nil }
    
        // Allocate memory
        guard let data = malloc(height * width * 4) else { return nil; }
        let rawBitmapInfo =
            CGImageAlphaInfo.noneSkipLast.rawValue |
                CGBitmapInfo.byteOrder32Big.rawValue
        let bitmapContext = CGContext(data: data,
                                      width: width,
                                      height: height,
                                      bitsPerComponent: 8,
                                      bytesPerRow: 4 * width,
                                      space: CGColorSpaceCreateDeviceRGB(),
                                      bitmapInfo: rawBitmapInfo)
        
        // Flip image vertically if requested
        if vflip {
            bitmapContext?.translateBy(x: 0.0, y: CGFloat(height))
            bitmapContext?.scaleBy(x: 1.0, y: -1.0)
        }
        
        // Call 'draw' to fill the data array
        let rect = CGRect.init(x: 0, y: 0, width: width, height: height)
        bitmapContext?.draw(cgimage, in: rect)
        return data
    }
    
    func toTexture(device: MTLDevice) -> MTLTexture? {
 
        guard let (_, width, height) = cgImageWH() else { return nil }
        guard let data = toData(vflip: true) else { return nil }

        // Use a texture descriptor to create a texture
        let textureDescriptor = MTLTextureDescriptor.texture2DDescriptor(
            pixelFormat: MTLPixelFormat.rgba8Unorm,
            width: width,
            height: height,
            mipmapped: false)
        let texture = device.makeTexture(descriptor: textureDescriptor)
        
        // Copy data
        let region = MTLRegionMake2D(0, 0, width, height)
        texture?.replace(region: region, mipmapLevel: 0, withBytes: data, bytesPerRow: 4 * width)

        free(data)
        return texture
    }
}

//
// Extensions to Renderer
//

extension Renderer {

    //
    // Image handling
    //

    func screenshot(texture: MTLTexture) -> NSImage? {

        // Use the blitter to copy the texture data back from the GPU
        let queue = texture.device.makeCommandQueue()!
        let commandBuffer = queue.makeCommandBuffer()!
        let blitEncoder = commandBuffer.makeBlitCommandEncoder()!
        blitEncoder.synchronize(texture: texture, slice: 0, level: 0)
        blitEncoder.endEncoding()
        commandBuffer.commit()
        commandBuffer.waitUntilCompleted()
        
        return NSImage.make(texture: texture, rect: textureRect)
    }
    
    func screenshot(afterUpscaling: Bool = true) -> NSImage? {
        
        if afterUpscaling {
            return screenshot(texture: upscaledTexture)
        } else {
            return screenshot(texture: mergeTexture)
        }
    }
    
    func createBackgroundTexture() -> MTLTexture? {

        let size = NSSize(width: 256, height: 256)
        let wallpaper = NSImage(color: .lightGray, size: size)
        return wallpaper.toTexture(device: device)
    }
}
