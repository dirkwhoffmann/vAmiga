// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Canvas {

    func buildTextures() {
     
        // Texture usages
        let r: MTLTextureUsage = [ .shaderRead ]
        let rwt: MTLTextureUsage = [ .shaderRead, .shaderWrite, .renderTarget ]
        let rwtp: MTLTextureUsage = [ .shaderRead, .shaderWrite, .renderTarget, .pixelFormatView ]

        // Emulator texture (long frames)
        longFrameTexture = device.makeTexture(size: TextureSize.original, usage: r)
        renderer.metalAssert(longFrameTexture != nil,
                             "The frame texture (long frames) could not be allocated.")
        
        // Emulator texture (short frames)
        shortFrameTexture = device.makeTexture(size: TextureSize.original, usage: r)
        renderer.metalAssert(shortFrameTexture != nil,
                             "The frame texture (short frames) could not be allocated.")
        
        // Merged emulator texture (long frame + short frame)
        mergeTexture = device.makeTexture(size: TextureSize.merged, usage: rwt)
        renderer.metalAssert(mergeTexture != nil,
                             "The merge texture could not be allocated.")
        
        // Bloom textures
        bloomTextureR = device.makeTexture(size: TextureSize.merged, usage: rwt)
        bloomTextureG = device.makeTexture(size: TextureSize.merged, usage: rwt)
        bloomTextureB = device.makeTexture(size: TextureSize.merged, usage: rwt)
        
        renderer.metalAssert(bloomTextureR != nil,
                             "The bloom texture (R channel) could not be allocated.")
        renderer.metalAssert(bloomTextureG != nil,
                             "The bloom texture (G channel) could not be allocated.")
        renderer.metalAssert(bloomTextureB != nil,
                             "The bloom texture (B channel) could not be allocated.")
        
        // Target for in-texture upscaling
        lowresEnhancedTexture = device.makeTexture(size: TextureSize.merged, usage: rwt)
        renderer.metalAssert(lowresEnhancedTexture != nil,
                             "The lowres enhancer texture could not be allocated.")
        
        // Upscaled merge texture
        upscaledTexture = device.makeTexture(size: TextureSize.upscaled, usage: rwtp)
        scanlineTexture = device.makeTexture(size: TextureSize.upscaled, usage: rwtp)
        renderer.metalAssert(upscaledTexture != nil,
                             "The upscaling texture could not be allocated.")
        renderer.metalAssert(scanlineTexture != nil,
                             "The scanline texture could not be allocated.")
    }

    func buildVertexBuffer() {

        quad2D = Node.init(device: device,
                           x: -1.0, y: -1.0, z: 0.0, w: 2.0, h: 2.0,
                           t: renderer.textureRect)

        quad3D = Quad.init(device: device,
                           x1: -0.64, y1: -0.48, z1: -0.64,
                           x2: 0.64, y2: 0.48, z2: 0.64,
                           t: renderer.textureRect)
    }
    
    func buildDotMasks() {

        let shaderOptions = renderer.shaderOptions!
        
        let selected = shaderOptions.dotMask
        let max  = UInt8(85 + shaderOptions.dotMaskBrightness * 170)
        let base = UInt8((1 - shaderOptions.dotMaskBrightness) * 85)
        let none = UInt8(30 + (1 - shaderOptions.dotMaskBrightness) * 55)

        let R = UInt32.init(r: max, g: base, b: base)
        let G = UInt32.init(r: base, g: max, b: base)
        let B = UInt32.init(r: base, g: base, b: max)
        let M = UInt32.init(r: max, g: base, b: max)
        let W = UInt32.init(r: max, g: max, b: max)
        let N = UInt32.init(r: none, g: none, b: none)

        let maskSize = [
            CGSize.init(width: 1, height: 1),
            CGSize.init(width: 3, height: 1),
            CGSize.init(width: 4, height: 1),
            CGSize.init(width: 3, height: 9),
            CGSize.init(width: 4, height: 8)
        ]

        let maskData = [

            [ W ],
            [ M, G, N ],
            [ R, G, B, N ],
            [ M, G, N,
              M, G, N,
              N, N, N,
              N, M, G,
              N, M, G,
              N, N, N,
              G, N, M,
              G, N, M,
              N, N, N],
            [ R, G, B, N,
              R, G, B, N,
              R, G, B, N,
              N, N, N, N,
              B, N, R, G,
              B, N, R, G,
              B, N, R, G,
              N, N, N, N]
        ]

        for n in 0 ... 4 {

            // Create image representation in memory
            let cap = Int(maskSize[n].width) * Int(maskSize[n].height)
            let mask = calloc(cap, MemoryLayout<UInt32>.size)!
            let ptr = mask.bindMemory(to: UInt32.self, capacity: cap)
            for i in 0 ... cap - 1 {
                ptr[i] = maskData[n][i]
            }

            // Create image
            let image = NSImage.make(data: mask, rect: maskSize[n])

            // Create texture if the dotmask is the currently selected mask
            if n == selected {
                dotMaskTexture = image?.toTexture(device: device)
            }

            // Store preview image
            dotmaskImages[n] = image?.resizeSharp(width: 12, height: 12)
        }
    }
}
