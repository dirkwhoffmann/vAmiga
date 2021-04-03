// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import MetalPerformanceShaders

class Canvas: Layer {
    
    var kernelManager: KernelManager { return renderer.kernelManager }

    // Indicates whether the recently drawn frames were long or short frames
    var currLOF = true
    var prevLOF = true

    // Used to determine if the GPU texture needs to be updated
    var prevBuffer: ScreenBuffer?

    // Variable used to emulate interlace flickering
    var flickerCnt = 0

    //
    // Buffers and uniforms
    //
    
    var quad2D: Node?
    var quad3D: Quad?
            
    var vertexUniforms2D = VertexUniforms(mvp: matrix_identity_float4x4)
    var vertexUniforms3D = VertexUniforms(mvp: matrix_identity_float4x4)
    var fragmentUniforms = FragmentUniforms(alpha: 1.0,
                                            mix: 0.0,
                                            dotMaskWidth: 0,
                                            dotMaskHeight: 0,
                                            scanlineDistance: 0)

    var mergeUniforms = MergeUniforms(longFrameScale: 1.0,
                                      shortFrameScale: 1.0)
    
    //
    // Textures
    //
    
    /* Long and short frame textures (raw data from emulator, 1024 x 512)
     * This texture is filled with the screen buffer data from the emulator.
     * The texture is updated in function updateTexture() which is called
     * periodically in drawRect().
     */
    var longFrameTexture: MTLTexture! = nil
    var shortFrameTexture: MTLTexture! = nil

    /* Merge texture (1024 x 1024)
     * The long frame and short frame textures are merged into this one.
     */
    var mergeTexture: MTLTexture! = nil

    /* Bloom textures to emulate blooming (512 x 512)
     * To emulate a bloom effect, the C64 texture is first split into it's
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

    /* Dotmask texture (variable size)
     * This texture is used by the fragment shader to emulate a dotmask
     * effect.
     */
    var dotMaskTexture: MTLTexture! = nil

    // Array holding dotmask preview images
    var dotmaskImages = [NSImage?](repeating: nil, count: 5)
    
    //
    // Functions
    //
    
    override init(renderer: Renderer) {

        super.init(renderer: renderer)

        buildTextures()
        buildDotMasks()
        
        // Start with a negative alpha to present the start screen for a while
        alpha.set(-2.5)
    }
    
    //
    // Managing textures
    //
    
    func updateTexture() {
        
        if amiga.poweredOff {
        
            var buffer = amiga.denise.noise!
            longFrameTexture.replace(w: Int(HPIXELS),
                                     h: longFrameTexture.height,
                                     buffer: buffer)
            buffer = amiga.denise.noise!
            shortFrameTexture.replace(w: Int(HPIXELS),
                                     h: shortFrameTexture.height,
                                     buffer: buffer)
            return
        }
        
        let buffer = amiga.denise.stableBuffer
        
        // Only proceed if the emulator delivers a new texture
        if prevBuffer?.data == buffer.data { return }
        prevBuffer = buffer

        // Determine if the new texture is a long frame or a short frame
        prevLOF = currLOF
        currLOF = buffer.longFrame
        
        // Update the GPU texture
        if currLOF {
            // track("Updating long frame texture")
            longFrameTexture.replace(w: Int(HPIXELS),
                                     h: longFrameTexture.height,
                                     buffer: buffer.data + Int(HBLANK_MIN) * 4)
        } else {
            // track("Updating short frame texture")
            shortFrameTexture.replace(w: Int(HPIXELS),
                                      h: shortFrameTexture.height,
                                      buffer: buffer.data + Int(HBLANK_MIN) * 4)
        }
    }
    
    //
    // Rendering
    //
    
    override func render(buffer: MTLCommandBuffer) {
                
        // Get the most recent texture from the emulator
        updateTexture()
        
        // Compute the merge texture
        if currLOF != prevLOF {
            
            // Case 1: Interlace drawing
            var weight = Float(1.0)
            if renderer.shaderOptions.flicker > 0 {
                weight -= renderer.shaderOptions.flickerWeight
            }
            flickerCnt += 1
            mergeUniforms.longFrameScale = (flickerCnt % 4 >= 2) ? 1.0 : weight
            mergeUniforms.shortFrameScale = (flickerCnt % 4 >= 2) ? weight : 1.0
            
            kernelManager.mergeFilter.apply(commandBuffer: buffer,
                                            textures: [longFrameTexture,
                                                       shortFrameTexture,
                                                       mergeTexture],
                                            options: &mergeUniforms,
                                            length: MemoryLayout<MergeUniforms>.stride)
            
        } else if currLOF {
            
            // Case 2: Non-interlace drawing (two long frames in a row)
            kernelManager.mergeBypassFilter.apply(commandBuffer: buffer,
                                    textures: [longFrameTexture, mergeTexture])
        } else {
            
            // Case 3: Non-interlace drawing (two short frames in a row)
            kernelManager.mergeBypassFilter.apply(commandBuffer: buffer,
                                    textures: [shortFrameTexture, mergeTexture])
        }
        
        // Compute upscaled texture (first pass, in-texture upscaling)
        kernelManager.enhancer.apply(commandBuffer: buffer,
                                     source: mergeTexture,
                                     target: lowresEnhancedTexture)
        
        // Compute the bloom textures
        if renderer.shaderOptions.bloom != 0 {
            kernelManager.bloomFilter.apply(commandBuffer: buffer,
                                            textures: [mergeTexture,
                                                       bloomTextureR,
                                                       bloomTextureG,
                                                       bloomTextureB],
                                            options: &renderer.shaderOptions,
                                            length: MemoryLayout<ShaderOptions>.stride)
            
            func applyGauss(_ texture: inout MTLTexture, radius: Float) {
                
                if #available(OSX 10.13, *) {
                    let gauss = MPSImageGaussianBlur(device: device, sigma: radius)
                    gauss.encode(commandBuffer: buffer,
                                 inPlaceTexture: &texture, fallbackCopyAllocator: nil)
                }
            }
            applyGauss(&bloomTextureR, radius: renderer.shaderOptions.bloomRadius)
            applyGauss(&bloomTextureG, radius: renderer.shaderOptions.bloomRadius)
            applyGauss(&bloomTextureB, radius: renderer.shaderOptions.bloomRadius)
        }
        
        // Compute upscaled texture (second pass)
        kernelManager.upscaler.apply(commandBuffer: buffer,
                                     source: lowresEnhancedTexture,
                                     target: upscaledTexture)

        // Blur the upscaled texture
        if #available(OSX 10.13, *), renderer.shaderOptions.blur > 0 {
            let gauss = MPSImageGaussianBlur(device: device,
                                             sigma: renderer.shaderOptions.blurRadius)
            gauss.encode(commandBuffer: buffer,
                         inPlaceTexture: &upscaledTexture,
                         fallbackCopyAllocator: nil)
        }
        
        // Emulate scanlines
        kernelManager.scanlineFilter.apply(commandBuffer: buffer,
                                           source: upscaledTexture,
                                           target: scanlineTexture,
                                           options: &renderer.shaderOptions,
                                           length: MemoryLayout<ShaderOptions>.stride)
    }
    
    override func render(encoder: MTLRenderCommandEncoder, flat: Bool) {
        
        encoder.setFragmentTexture(scanlineTexture, index: 0)
        encoder.setFragmentTexture(bloomTextureR, index: 1)
        encoder.setFragmentTexture(bloomTextureG, index: 2)
        encoder.setFragmentTexture(bloomTextureB, index: 3)
        encoder.setFragmentTexture(dotMaskTexture, index: 4)
        
        if flat {
            
            // Configure the vertex shader
            encoder.setVertexBytes(&vertexUniforms2D,
                                   length: MemoryLayout<VertexUniforms>.stride,
                                   index: 1)
            
            // Configure the fragment shader
            fragmentUniforms.alpha = amiga.paused ? Float(0.5) : alpha.clamped
            fragmentUniforms.mix = renderer.white.current
            fragmentUniforms.dotMaskHeight = Int32(dotMaskTexture.height)
            fragmentUniforms.dotMaskWidth = Int32(dotMaskTexture.width)
            fragmentUniforms.scanlineDistance = Int32(renderer.size.height / 256)
            encoder.setFragmentBytes(&fragmentUniforms,
                                     length: MemoryLayout<FragmentUniforms>.stride,
                                     index: 1)
            
            // Render
            quad2D!.drawPrimitives(encoder)
            
        } else {
            
            let animates = renderer.animates
            
            // Configure the vertex shader
            encoder.setVertexBytes(&vertexUniforms3D,
                                   length: MemoryLayout<VertexUniforms>.stride,
                                   index: 1)
            
            // Configure the fragment shader
            fragmentUniforms.alpha = amiga.paused ? Float(0.5) : alpha.clamped
            fragmentUniforms.mix = renderer.white.current
            fragmentUniforms.dotMaskHeight = Int32(dotMaskTexture.height)
            fragmentUniforms.dotMaskWidth = Int32(dotMaskTexture.width)
            fragmentUniforms.scanlineDistance = Int32(renderer.size.height / 256)
            encoder.setFragmentBytes(&fragmentUniforms,
                                     length: MemoryLayout<FragmentUniforms>.stride,
                                     index: 1)
            
            // Render (part of) the cube
            quad3D!.draw(encoder, allSides: animates != 0)
        }
    }
}
