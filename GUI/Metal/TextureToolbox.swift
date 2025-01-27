// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// RGBA values
//

extension UInt32 {

    init(rgba: (UInt8, UInt8, UInt8, UInt8)) {
        
        let r = UInt32(rgba.0)
        let g = UInt32(rgba.1)
        let b = UInt32(rgba.2)
        let a = UInt32(rgba.3)
        
        self.init(bigEndian: r << 24 | g << 16 | b << 8 | a)
    }

    init(rgba: (UInt8, UInt8, UInt8)) {
        
        self.init(rgba: (rgba.0, rgba.1, rgba.2, 0xFF))
     }
    
    init(r: UInt8, g: UInt8, b: UInt8, a: UInt8) { self.init(rgba: (r, g, b, a)) }
    init(r: UInt8, g: UInt8, b: UInt8) { self.init(rgba: (r, g, b)) }
}

//
// RGBA buffers
//

extension UnsafeMutablePointer where Pointee == UInt32 {

    func scale(size: MTLSize, region: MTLRegion, factor: Double) {

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
        let region = MTLRegionMake2D(border, size.height - y, width, 1)
        scale(size: size, region: region, factor: 1.5)
    }

    func drawGrid(size: MTLSize,
                  y1: Int, y2: Int, lines: Int, logScale: Bool, start: Int = 0) {
        
        let height = Double(y2 - y1)
        
        for i in start ... lines {
            
            var y = Double(i) / Double(lines)
            if logScale { y = log(1.0 + 19.0 * y) / log(20) }
            drawLine(size: size, y: Int(Double(y1) + height * y), border: 0)
        }
    }
 
    func drawDoubleGrid(size: MTLSize, y1: Int, y2: Int, lines: Int, logScale: Bool) {
        
        let middle = y1 + (y2 - y1) / 2
        
        drawGrid(size: size, y1: middle, y2: y1, lines: lines, logScale: logScale)
        drawGrid(size: size, y1: middle, y2: y2, lines: lines, logScale: logScale, start: 1)
    }
        
    func drawGradient(size: MTLSize, region: MTLRegion,
                      rgba1: (Int, Int, Int, Int), rgba2: (Int, Int, Int, Int)) {
        
        let w = region.size.width
        let h = region.size.height
        
        // Compute delta steps
        let dr = Double(rgba2.0 - rgba1.0) / Double(h); var r = Double(rgba1.0)
        let dg = Double(rgba2.1 - rgba1.1) / Double(h); var g = Double(rgba1.1)
        let db = Double(rgba2.2 - rgba1.2) / Double(h); var b = Double(rgba1.2)
        let da = Double(rgba2.3 - rgba1.3) / Double(h); var a = Double(rgba1.3)

        // Create gradient
        var index = size.width * region.origin.y + region.origin.x
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
    
    func drawGradient(size: MTLSize, region: MTLRegion, gradient: [ (Int, Int, Int, Int) ]) {
        
        let h = region.size.height / (gradient.count - 1)
        var r = region; r.size.height = h
        
        for i in 0 ..< gradient.count - 1 {
            drawGradient(size: size, region: r, rgba1: gradient[i], rgba2: gradient[i+1])
            r.origin.y += h
        }
    }
    
    func drawGradient(size: MTLSize, gradient: [ (Int, Int, Int, Int) ]) {
        
        let region = MTLRegionMake2D(0, 0, size.width, size.height)
        drawGradient(size: size, region: region, gradient: gradient)
    }
    
    func makeRoundCorners(size: MTLSize, radius: Int) {
        
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
        let context = CGContext(data: self,
                                width: size.width,
                                height: size.height,
                                bitsPerComponent: 8,
                                bytesPerRow: 4 * size.width,
                                space: CGColorSpaceCreateDeviceRGB(),
                                bitmapInfo: alphaInfo | orderInfo)
        
        // Create an attributed string
        let string = NSAttributedString(text, size: 28, color: .white)
       
        // Draw text centered on top
        let line = CTLineCreateWithAttributedString(string)
        let offset = CTLineGetPenOffsetForFlush(line, 0.5, Double(size.width))
        context!.textPosition = CGPoint(x: Int(offset), y: size.height - 38)
        CTLineDraw(line, context!)
    }
}

//
// Extensions to MTLTexture
//

extension MTLTexture {

    var size: MTLSize {

        return MTLSize(width: width, height: height, depth: 1)
    }

    func replace(region: MTLRegion, buffer: UnsafePointer<UInt32>?) {
        
        if buffer != nil {
            let bpr = 4 * region.size.width
            replace(region: region, mipmapLevel: 0, withBytes: buffer!, bytesPerRow: bpr)
        }
    }
    
    func replace(size: MTLSize, buffer: UnsafePointer<UInt32>?) {
        
        let region = MTLRegionMake2D(0, 0, size.width, size.height)
        replace(region: region, buffer: buffer)
    }
    
    func replace(w: Int, h: Int, buffer: UnsafePointer<UInt32>?) {
        
        let region = MTLRegionMake2D(0, 0, w, h)
        replace(region: region, buffer: buffer)
    }
    
    func blit() {

        // Use the blitter to copy the texture data back from the GPU
        let queue = device.makeCommandQueue()!
        let commandBuffer = queue.makeCommandBuffer()!
        let blitEncoder = commandBuffer.makeBlitCommandEncoder()!
        blitEncoder.synchronize(texture: self, slice: 0, level: 0)
        blitEncoder.endEncoding()
        commandBuffer.commit()
        commandBuffer.waitUntilCompleted()
    }

    func blitTextureToBuffer(device: MTLDevice, texture: MTLTexture) -> MTLBuffer? {
        
        let textureWidth = texture.width
        let textureHeight = texture.height
        let pixelFormat = texture.pixelFormat

        // Ensure the pixel format is compatible
        guard pixelFormat == .rgba8Unorm else {
            print("Unsupported pixel format: \(pixelFormat)")
            return nil
        }

        let bytesPerPixel = 4
        let bytesPerRow = textureWidth * bytesPerPixel
        let bufferSize = bytesPerRow * textureHeight

        // Create a buffer on the GPU
        guard let buffer = device.makeBuffer(length: bufferSize, options: .storageModeShared) else {
            print("Failed to create buffer")
            return nil
        }

        // Create a command queue and command buffer
        guard let commandQueue = device.makeCommandQueue(),
              let commandBuffer = commandQueue.makeCommandBuffer(),
              let blitEncoder = commandBuffer.makeBlitCommandEncoder() else {
            print("Failed to create Metal objects")
            return nil
        }

        // Copy texture data into the buffer
        blitEncoder.copy(from: texture,
                         sourceSlice: 0,
                         sourceLevel: 0,
                         sourceOrigin: MTLOrigin(x: 0, y: 0, z: 0),
                         sourceSize: MTLSize(width: textureWidth, height: textureHeight, depth: 1),
                         to: buffer,
                         destinationOffset: 0,
                         destinationBytesPerRow: bytesPerRow,
                         destinationBytesPerImage: bytesPerRow * textureHeight)

        blitEncoder.endEncoding()
        commandBuffer.commit()
        commandBuffer.waitUntilCompleted()

        return buffer
    }
    
    // MOVE TO CUSTOM CODE. TOO SPECIALIZED
    /*
    func innerArea(device: MTLDevice) -> CGRect {
    
        guard let buffer = blitTextureToBuffer(device: device, texture: self) else {

            print("Failed to access texture data");
            return CGRect.zero
        }

        let w = width
        let h = height
        let p = buffer.contents().bindMemory(to: Int32.self, capacity: w * h)
        
        let first = p[2 * width + 2]

        func empty(row: Int) -> Bool {
            print("first = \(first)")
            for x in 0..<w { if p[row * w + x] != first { print("diff = \(p[row * w + x])"); return false } }
            return true
        }
 
        func empty(col: Int) -> Bool {
            for y in 0..<h { if p[y * w + col] != first { return false } }
            return true
        }
        
        var x1 = 2
        var x2 = width - 2
        var y1 = 2
        var y2 = height - 2
        
        while x2 > 0 && empty(col: x2) { x2 -= 1 }
        while x1 < x2 && empty(col: x1) { x1 += 1 }
        while y2 > 0 && empty(row: y2) { y2 -= 1 }
        while y1 < y2 && empty(row: y1) { y1 += 1 }

        return CGRect.init(x: x1, y: y1, width: x2 - x1, height: y2 - y1)
    }
    */
}

//
// Extensions to MTLDevice
//

extension MTLDevice {
    
    func makeTexture(size: MTLSize,
                     buffer: UnsafeMutablePointer<UInt32>? = nil,
                     usage: MTLTextureUsage = [.shaderRead]) -> MTLTexture? {
        
        let descriptor = MTLTextureDescriptor.texture2DDescriptor(
            pixelFormat: MTLPixelFormat.rgba8Unorm,
            width: size.width,
            height: size.height,
            mipmapped: false)
        descriptor.usage = usage
        
        let texture = makeTexture(descriptor: descriptor)
        texture?.replace(size: size, buffer: buffer)
        return texture
    }
    
    func makeTexture(w: Int, h: Int,
                     buffer: UnsafeMutablePointer<UInt32>? = nil,
                     usage: MTLTextureUsage = [.shaderRead]) -> MTLTexture? {
        
        let size = MTLSizeMake(w, h, 0)
        return makeTexture(size: size, buffer: buffer, usage: usage)
    }
    
    func makeTexture(size: MTLSize,
                     gradient: [ (Int, Int, Int, Int) ],
                     usage: MTLTextureUsage = [.shaderRead]) -> MTLTexture? {
        
        let capacity = size.width * size.height
        let buffer = UnsafeMutablePointer<UInt32>.allocate(capacity: capacity)
        buffer.drawGradient(size: size, gradient: gradient)
        
        return makeTexture(size: size, buffer: buffer, usage: usage)
    }
    
    func makeTexture(w: Int, h: Int,
                     gradient: [ (Int, Int, Int, Int) ],
                     usage: MTLTextureUsage = [.shaderRead]) -> MTLTexture? {
        
        let size = MTLSizeMake(w, h, 0)
        return makeTexture(size: size, gradient: gradient, usage: usage)
    }
}
