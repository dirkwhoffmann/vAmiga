// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import MetalPerformanceShaders

@MainActor
class Canvas: Layer {
    
    var mergeFilter: ComputeKernel! { return ressourceManager.mergeFilter }
    var scaleFilter: ComputeKernel! { return ressourceManager.scaleFilter }
    var enhancer: ComputeKernel! { return ressourceManager.enhancer }
    var bloomFilter: ComputeKernel! { return ressourceManager.bloomFilter }
    var upscaler: ComputeKernel! { return ressourceManager.upscaler }
    var scanlineFilter: ComputeKernel! { return ressourceManager.scanlineFilter }
    
    // Indicates whether the recently drawn frames were long or short frames
    var currLOF = true
    var prevLOF = true

    // Used to determine if the GPU texture needs to be updated
    var prevNr = 0

    // Variable used to emulate interlace flickering
    var flickerCnt = 0

    // Tracked display window (adaptive viewport feature)
    var x1 = CGFloat(0)
    var y1 = CGFloat(0)
    var x2 = CGFloat(0)
    var y2 = CGFloat(0)

    //
    // Textures
    //
    
    /* Long and short frame textures (raw data from emulator, 1024 x 512)
     * This texture is filled with the screen buffer data from the emulator.
     * The texture is updated in function updateTexture() which is called
     * periodically in drawRect().
     */
    var lfTexture: MTLTexture! = nil
    var sfTexture: MTLTexture! = nil

    /* Merge texture
     * The long frame and short frame textures are merged into this one.
     */
    var mergeTexture: MTLTexture! = nil

    /* Bloom textures to emulate blooming (512 x 512)
     * To emulate a bloom effect, the emulator texture is first split into it's
     * R, G, and B parts. Each texture is then run through a Gaussian blur
     * filter with a large radius. These blurred textures are passed into
     * the fragment shader as secondary textures where they are recomposed
     * with the upscaled primary texture.
     */
    var bloomTextureR: MTLTexture! = nil
    var bloomTextureG: MTLTexture! = nil
    var bloomTextureB: MTLTexture! = nil

    /* Lowres enhancement texture (experimental)
     * Trying to use in-texture upscaling to enhance lowres mode
     */
    var lowresEnhancedTexture: MTLTexture! = nil

    /* Upscaled emulator texture (2048 x 2048)
     * In the first texture processing stage, the emulator texture is bumped up
     * by a factor of 4. The user can choose between bypass upscaling which
     * simply replaces each pixel by a 4x4 quad or more sophisticated upscaling
     * algorithms such as xBr.
     */
    var upscaledTexture: MTLTexture! = nil

    /* Upscaled texture with scanlines (2048 x 2048)
     * In the second texture processing stage, a scanline effect is applied to
     * the upscaled texture.
     */
    var scanlineTexture: MTLTexture! = nil

    /* The final texture. This texture is passed to the fragment shader.
     */
    var finalTexture: MTLTexture! = nil

    // Part of the texture that is currently visible
    var textureRect = CGRect.zero { didSet { buildVertexBuffers() } }

    //
    // Buffers and Uniforms
    //
    
    var quad2D: Node?
            
    var vertexUniforms2D = VertexUniforms(mvp: matrix_identity_float4x4)
    var fragmentUniforms = FragmentUniforms(alpha: 1.0,
                                            white: 0.0,
                                            dotMaskWidth: 0,
                                            dotMaskHeight: 0,
                                            scanlineDistance: 0)

    var mergeUniforms = MergeUniforms(longFrameScale: 1.0,
                                      shortFrameScale: 1.0)
    
    //
    // Initializing
    //
    
    override init(renderer: Renderer) {

        super.init(renderer: renderer)
        buildVertexBuffers()
        buildTextures()
        
        /* We start with a negative alpha value to give it some time until
         * it becomes greater than 0. During this time, the splash screen will
         * be fully visible. */
        alpha.set(-1.5)
    }
    
    func buildVertexBuffers() {
        
        quad2D = Node(device: device,
                      x: -1.0, y: -1.0, z: 0.98, w: 2.0, h: 2.0,
                      t: textureRect)        
    }
    
    func buildTextures() {
     
        // Texture usages
        let r: MTLTextureUsage = [ .shaderRead ]
        let rwt: MTLTextureUsage = [ .shaderRead, .shaderWrite, .renderTarget ]
        let rwtp: MTLTextureUsage = [ .shaderRead, .shaderWrite, .renderTarget, .pixelFormatView ]

        // Emulator texture (long frames)
        lfTexture = device.makeTexture(size: TextureSize.original, usage: r)
        renderer.metalAssert(lfTexture != nil,
                             "The frame texture (long frames) could not be allocated.")
        
        // Emulator texture (short frames)
        sfTexture = device.makeTexture(size: TextureSize.original, usage: r)
        renderer.metalAssert(sfTexture != nil,
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

    //
    // Updating
    //
    
    override func update(frames: Int64) {
            
        super.update(frames: frames)

        // Grab the current texture
        updateTexture()

        // Let the emulator compute the next frame
        amiga.wakeUp()
    }

    func updateTexture() {

        precondition(lfTexture != nil)
        precondition(sfTexture != nil)

        // Get the emulator texture
        var buffer: UnsafePointer<UInt32>!
        var nr = 0

        // Prevent the stable texture from changing
        amiga.videoPort.lockTexture()

        // Grab the stable texture
        amiga.videoPort.texture(&buffer, nr: &nr, lof: &currLOF, prevlof: &prevLOF)

        // Check for duplicated or dropped frames
        if nr != prevNr + 1 {

            debug(.vsync, "Frame sync mismatch (\(prevNr) -> \(nr))")
        }
        prevNr = nr

        // Update the GPU texture
        if currLOF {
            lfTexture.replace(w: Int(TPP) * VAMIGA.HPIXELS, h: VAMIGA.VPIXELS, buffer: buffer)
        } else {
            sfTexture.replace(w: Int(TPP) * VAMIGA.HPIXELS, h: VAMIGA.VPIXELS, buffer: buffer)
        }

        // Release the texture lock
        amiga.videoPort.unlockTexture()
    }

    //
    // Rendering
    //
    
    func makeCommandBuffer(buffer: MTLCommandBuffer) {

        func applyGauss(_ texture: inout MTLTexture, radius: Float) {
            
            let gauss = MPSImageGaussianBlur(device: device, sigma: radius)
            gauss.encode(commandBuffer: buffer,
                         inPlaceTexture: &texture, fallbackCopyAllocator: nil)
        }

        // Compute the merge texture
        if currLOF == prevLOF {

            if currLOF {

                // Case 1: Non-interlace mode, two long frames in a row
                scaleFilter.apply(commandBuffer: buffer,
                                  textures: [lfTexture, mergeTexture])

            } else {

                // Case 2: Non-interlace mode, two short frames in a row
                scaleFilter.apply(commandBuffer: buffer,
                                  textures: [sfTexture, mergeTexture])
            }

        } else {

            // Case 3: Interlace mode, long frame followed by a short frame
            if renderer.shaderOptions.flicker > 0 {

                let weight = 1.0 - renderer.shaderOptions.flickerWeight
                mergeUniforms.longFrameScale = (flickerCnt % 4 >= 2) ? 1.0 : weight
                mergeUniforms.shortFrameScale = (flickerCnt % 4 >= 2) ? weight : 1.0
                flickerCnt += 1

            } else {

                mergeUniforms.longFrameScale = 1.0
                mergeUniforms.shortFrameScale = 1.0
            }
            
            mergeFilter.apply(commandBuffer: buffer,
                              textures: [lfTexture, sfTexture, mergeTexture],
                              options: &mergeUniforms,
                              length: MemoryLayout<MergeUniforms>.stride)
        }
        finalTexture = mergeTexture

        // Compute the upscaled texture (first pass, in-texture upscaling)
        if renderer.config.enhancer != 0 {

            enhancer.apply(commandBuffer: buffer,
                           source: finalTexture,
                           target: lowresEnhancedTexture)

            finalTexture = lowresEnhancedTexture
        }

        // Compute the bloom textures
        if renderer.shaderOptions.bloom != 0 {
            bloomFilter.apply(commandBuffer: buffer,
                              textures: [mergeTexture,
                                         bloomTextureR,
                                         bloomTextureG,
                                         bloomTextureB],
                              options: &renderer.shaderOptions,
                              length: MemoryLayout<ShaderOptions>.stride)
            
            applyGauss(&bloomTextureR, radius: renderer.shaderOptions.bloomRadius)
            applyGauss(&bloomTextureG, radius: renderer.shaderOptions.bloomRadius)
            applyGauss(&bloomTextureB, radius: renderer.shaderOptions.bloomRadius)
        }
        
        // Compute the upscaled texture (second pass)
        if renderer.config.upscaler != 0 {

            upscaler.apply(commandBuffer: buffer,
                           source: finalTexture,
                           target: upscaledTexture)
            finalTexture = upscaledTexture
        } else {
            upscaledTexture = finalTexture
        }

        // Blur the upscaled texture
        if renderer.shaderOptions.blur != 0 {
            
            applyGauss(&finalTexture, radius: renderer.shaderOptions.blurRadius)
        }

        // Emulate scanlines
        if renderer.shaderOptions.scanlines == 1 {

            scanlineFilter.apply(commandBuffer: buffer,
                                 source: mergeTexture,
                                 target: scanlineTexture,
                                 options: &renderer.shaderOptions,
                                 length: MemoryLayout<ShaderOptions>.stride)
            finalTexture = scanlineTexture
        }
    }
    
    func setupFragmentShader(encoder: MTLRenderCommandEncoder) {
        
        // Setup textures
        encoder.setFragmentTexture(finalTexture, index: 0)
        encoder.setFragmentTexture(bloomTextureR, index: 1)
        encoder.setFragmentTexture(bloomTextureG, index: 2)
        encoder.setFragmentTexture(bloomTextureB, index: 3)
        encoder.setFragmentTexture(ressourceManager.dotMask, index: 4)

        // Select the texture sampler
        if renderer.shaderOptions.blur > 0 {
            encoder.setFragmentSamplerState(ressourceManager.samplerLinear, index: 0)
        } else {
            encoder.setFragmentSamplerState(ressourceManager.samplerNearest, index: 0)
        }
        
        // Setup uniforms
        fragmentUniforms.alpha = amiga.paused ? 0.5 : alpha.current
        fragmentUniforms.white = renderer.white.current
        fragmentUniforms.dotMaskHeight = Int32(ressourceManager.dotMask.height)
        fragmentUniforms.dotMaskWidth = Int32(ressourceManager.dotMask.width)
        fragmentUniforms.scanlineDistance = Int32(renderer.size.height / 256)
        encoder.setFragmentBytes(&renderer.shaderOptions,
                                 length: MemoryLayout<ShaderOptions>.stride,
                                 index: 0)
        encoder.setFragmentBytes(&fragmentUniforms,
                                 length: MemoryLayout<FragmentUniforms>.stride,
                                 index: 1)
    }

    func render(_ encoder: MTLRenderCommandEncoder) {
        
        // Configure the vertex shader
        encoder.setVertexBytes(&vertexUniforms2D,
                               length: MemoryLayout<VertexUniforms>.stride,
                               index: 1)
        
        // Configure the fragment shader
        setupFragmentShader(encoder: encoder)
        
        // Render
        quad2D!.drawPrimitives(encoder)
    }
}

//
// Screenshots
//

extension Canvas {
        
    func screenshot(source: ScreenshotSource, cutout: ScreenshotCutout, width: Int, height: Int) -> NSImage? {

        // print("screenshot(source: \(source), cutout: \(cutout), width: \(width), height: \(height)")
        
        // Handle the framebuffer option first
        if source == .framebuffer { return framebuffer }
        
        var texture: MTLTexture
        var mtlreg: MTLRegion
        
        // Analyze the emulator texture
        var x1 = 0, x2 = 0, y1 = 0, y2 = 0
        amiga.videoPort.innerArea(&x1, x2: &x2, y1: &y1, y2: &y2)
        var width = x2 - x1 + 1, height = y2 - y1 + 1
        
        // Scale to texture coordinates
        x1 *= 2; x2 *= 2; width *= 2; y1 *= 4; y2 *= 4; height *= 4
        
        func region(width: Int, height: Int) -> MTLRegion {
        
            // Compute the center coordinate
            let cx = Int((x1 + x2) / 2)
            let cy = Int((y1 + y2) / 2)

            // Assemble the region
            let origin = MTLOrigin(x: cx - width / 2, y: cy - height / 2, z: 0)
            let size = MTLSize(width: width, height: height, depth: 1)
            return MTLRegion(origin: origin, size: size)
        }

        func region(cgRect rect: CGRect) -> MTLRegion {
        
            // Compute scaling factors
            let w = CGFloat(texture.width)
            let h = CGFloat(texture.height)

            // Assemble the region
            let origin = MTLOrigin(x: Int(rect.minX * w), y: Int(rect.minY * h), z: 0)
            let size = MTLSize(width: Int(rect.width * w), height: Int(rect.height * h), depth: 1)
            return MTLRegion(origin: origin, size: size)
        }
        
        func region(x1: Int, x2: Int, y1: Int, y2: Int) -> MTLRegion {
        
            // Revert to the entire texture if a zero rect is given
            if x1 == x2 || y1 == y2 { return region(cgRect: largestVisibleNormalized) }
            
            // Assemble the region
            let origin = MTLOrigin(x: x1, y: y1, z: 0)
            let size = MTLSize(width: width, height: height, depth: 1)
            return MTLRegion(origin: origin, size: size)
        }
        
        switch source {
        case .emulator: texture = mergeTexture
        case .upscaler: texture = upscaledTexture
        case .framebuffer: fatalError()
        }
        
        switch cutout {
        case .visible: mtlreg = region(cgRect: visibleNormalized)
        case .entire: mtlreg = region(cgRect: largestVisibleNormalized)
        case .automatic: mtlreg = region(x1: x1, x2: x2, y1: y1, y2: y2)
        case .custom: mtlreg = region(width: width, height: height)
        }
             
        return screenshot(texture: texture, region: mtlreg)
    }

    private func screenshot(texture: MTLTexture, rect: CGRect) -> NSImage? {

        texture.blit()
        return NSImage.make(texture: texture, rect: rect)
    }

    private func screenshot(texture: MTLTexture, region: MTLRegion) -> NSImage? {

        texture.blit()
        return NSImage.make(texture: texture, region: region)
    }

    private var framebuffer: NSImage? {
        
        guard let drawable = renderer.metalLayer.nextDrawable() else { return nil }
        
        // Create target texture
        let texture = device.makeTexture(size: drawable.texture.size, usage: [.shaderRead, .shaderWrite])!
        
        // Use the blitter to copy the framebuffer back from the GPU
        let queue = renderer.device.makeCommandQueue()!
        let commandBuffer = queue.makeCommandBuffer()!
        let blitEncoder = commandBuffer.makeBlitCommandEncoder()!
        blitEncoder.copy(from: drawable.texture,
                         sourceSlice: 0,
                         sourceLevel: 0,
                         sourceOrigin: MTLOrigin(x: 0, y: 0, z: 0),
                         sourceSize: MTLSize(width: texture.width,
                                             height: texture.height,
                                             depth: 1),
                         to: texture,
                         destinationSlice: 0,
                         destinationLevel: 0,
                         destinationOrigin: MTLOrigin(x: 0, y: 0, z: 0))
        blitEncoder.endEncoding()
        commandBuffer.commit()
        commandBuffer.waitUntilCompleted()
                
        // Convert the texture into an NSImage
        let alpha = CGImageAlphaInfo.premultipliedFirst.rawValue
        let leEn32 = CGBitmapInfo.byteOrder32Little.rawValue
        let bitmapInfo =  CGBitmapInfo(rawValue: alpha | leEn32)
        
        return NSImage.make(texture: texture, bitmapInfo: bitmapInfo)
    }
}
