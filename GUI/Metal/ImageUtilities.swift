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
// Extensions to MTLDevice
//

extension MTLDevice {
    
    func makeGradientTexture(ptr: UnsafeMutablePointer<UInt32>,
                             width: Int, height: Int,
                             r1: Int, g1: Int, b1: Int, a1: Int,
                             r2: Int, g2: Int, b2: Int, a2: Int) -> MTLTexture? {
        
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
