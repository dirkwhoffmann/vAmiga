// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Metal
import MetalKit
import MetalPerformanceShaders

class Renderer: NSObject, MTKViewDelegate {
    
    let mtkView: MTKView
    let device: MTLDevice
    let parent: MyController
    
    var prefs: Preferences { return parent.pref }
    var config: Configuration { return parent.config }
    
    /* Number of drawn frames since power up.
     */
    var frames: Int64 = 0
    
    /* Synchronization semaphore. The semaphore is locked in function draw()
     * and released in function endFrame(). It's puprpose is to prevent a new
     * frame from being drawn if the previous isn't finished yet. Not sure if
     * we really need it.
     */
    var semaphore = DispatchSemaphore(value: 1)

    //
    // Metal objects
    //
    
    var library: MTLLibrary! = nil
    var queue: MTLCommandQueue! = nil
    var pipeline: MTLRenderPipelineState! = nil
    var depthState: MTLDepthStencilState! = nil
    var commandBuffer: MTLCommandBuffer! = nil
    var commandEncoder: MTLRenderCommandEncoder! = nil
    var drawable: CAMetalDrawable! = nil
    
    //
    // Metal layers
    //
    
    var metalLayer: CAMetalLayer! = nil

    // Current canvas size
    var size: CGSize {

        let frameSize = mtkView.frame.size
        let scale = mtkView.layer?.contentsScale ?? 1

        return CGSize(width: frameSize.width * scale,
                      height: frameSize.height * scale)
    }

    //
    // Drawing layers
    //
    
    var splashScreen: SplashScreen! = nil
    var canvas: Canvas! = nil
    var monis: Monitors! = nil
    
    //
    // Buffers and uniforms
    //
    
    // var quad2D: Node?
    // var quad3D: Quad?
            
    // var vertexUniforms2D = VertexUniforms(mvp: matrix_identity_float4x4)
    // var vertexUniforms3D = VertexUniforms(mvp: matrix_identity_float4x4)
    
    /*
    var fragmentUniforms = FragmentUniforms(alpha: 1.0,
                                            dotMaskWidth: 0,
                                            dotMaskHeight: 0,
                                            scanlineDistance: 0)
    */
    
    var mergeUniforms = MergeUniforms(longFrameScale: 1.0,
                                      shortFrameScale: 1.0)

    //
    // Textures
    //

    // Background textures
    var bgTexture: MTLTexture! = nil
    var bgFullscreenTexture: MTLTexture! = nil

    // Texture to hold the pixel depth information
    var depthTexture: MTLTexture! = nil
    
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
        
    // An instance of the merge filter and the merge bypass filter
    var mergeFilter: MergeFilter! = nil
    var mergeBypassFilter: MergeBypassFilter! = nil

    // Array holding all available lowres enhancers
    var enhancerGallery = [ComputeKernel?](repeating: nil, count: 3)
    
    // The currently selected enhancer
    var enhancer: ComputeKernel!

    // Array holding all available upscalers
    var upscalerGallery = [ComputeKernel?](repeating: nil, count: 3)

    // The currently selected enhancer
    var upscaler: ComputeKernel!

    // Array holding all available bloom filters
    var bloomFilterGallery = [ComputeKernel?](repeating: nil, count: 2)
    
    // Array holding all available scanline filters
    var scanlineFilterGallery = [ComputeKernel?](repeating: nil, count: 3)
    
    // Array holding dotmask preview images
    var dotmaskImages = [NSImage?](repeating: nil, count: 5)
    
    //
    // Texture samplers
    //
    
    // Nearest neighbor sampler
    var samplerNearest: MTLSamplerState!
    
    // Linear interpolation sampler
    var samplerLinear: MTLSamplerState!
    
    // Shader options
    var shaderOptions: ShaderOptions!
    
    // Indicates if an animation is currently performed
    var animates = 0
    
    // Indicates if the splash screen should be rendered
    var drawSplashScreen = true
    
    // Animation parameters
    var angleX = AnimatedFloat(0.0)
    var angleY = AnimatedFloat(0.0)
    var angleZ = AnimatedFloat(0.0)
    
    var shiftX = AnimatedFloat(0.0)
    var shiftY = AnimatedFloat(0.0)
    var shiftZ = AnimatedFloat(0.0)
    
    var alpha = AnimatedFloat(0.0)
    var noise = AnimatedFloat(0.0)
    
    // Animation variables for smooth texture zooming
    var cutoutX1 = AnimatedFloat.init()
    var cutoutY1 = AnimatedFloat.init()
    var cutoutX2 = AnimatedFloat.init()
    var cutoutY2 = AnimatedFloat.init()

    // Part of the texture that is currently visible
    var textureRect = CGRect.init() { didSet { buildVertexBuffer() } }

    // Indicates whether the recently drawn frames were long or short frames
    var currLOF = true
    var prevLOF = true

    // Used to determine if the GPU texture needs to be updated
    var prevBuffer: ScreenBuffer?
    
    // Variable used to emulate interlace flickering
    var flickerCnt = 0
        
    // Is set to true when fullscreen mode is entered
    var fullscreen = false
    
    //
    // Initializing
    //

    init(view: MTKView, device: MTLDevice, controller: MyController) {
        
        self.mtkView = view
        self.device = device
        self.parent = controller
        super.init()

        setupMetal()

        mtkView.device = device
        mtkView.delegate = self
    }

    //
    // Managing textures
    //
    
    func updateBgTexture(bytes: UnsafeMutablePointer<UInt32>) {

        // bgTexture.replace(w: 512, h: 512, buffer: bytes)
    }
    
    func updateTexture() {
        
        if parent.amiga.poweredOff {
        
            var buffer = parent.amiga.denise.noise!
            longFrameTexture.replace(w: Int(HPIXELS),
                                     h: longFrameTexture.height,
                                     buffer: buffer)
            buffer = parent.amiga.denise.noise!
            shortFrameTexture.replace(w: Int(HPIXELS),
                                     h: shortFrameTexture.height,
                                     buffer: buffer)
            return
        }
        
        let buffer = parent.amiga.denise.stableBuffer
        
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
    // Managing kernels
    //

    // Tries to select a new enhancer
    func selectEnhancer(_ nr: Int) -> Bool {
        
        if nr < enhancerGallery.count && enhancerGallery[nr] != nil {
            enhancer = enhancerGallery[nr]!
            return true
        }
        return false
    }
  
    // Tries to select a new upscaler
    func selectUpscaler(_ nr: Int) -> Bool {
        
        if nr < upscalerGallery.count && upscalerGallery[nr] != nil {
            upscaler = upscalerGallery[nr]!
            return true
        }
        return false
    }
    
    // Returns the compute kernel of the currently selected bloom filter
    func currentBloomFilter() -> ComputeKernel {

        var nr = Int(shaderOptions.bloom)
        if bloomFilterGallery.count <= nr || bloomFilterGallery[nr] == nil { nr = 0 }
        return bloomFilterGallery[nr]!
    }

    // Returns the compute kernel of the currently selected scanline filter
    func currentScanlineFilter() -> ComputeKernel {

        var nr = Int(shaderOptions.scanlines)
        if scanlineFilterGallery.count <= nr || scanlineFilterGallery[nr] == nil { nr = 0 }
        return scanlineFilterGallery[nr]!
    }
    
    //
    //  Drawing
    //

    func runTexturePipeline() {
        
        // Compute the merge texture
        if currLOF != prevLOF {
            
            // Case 1: Interlace drawing
            var weight = Float(1.0)
            if shaderOptions.flicker > 0 { weight -= shaderOptions.flickerWeight }
            flickerCnt += 1
            mergeUniforms.longFrameScale = (flickerCnt % 4 >= 2) ? 1.0 : weight
            mergeUniforms.shortFrameScale = (flickerCnt % 4 >= 2) ? weight : 1.0
            
            mergeFilter.apply(commandBuffer: commandBuffer,
                              textures: [longFrameTexture,
                                         shortFrameTexture,
                                         mergeTexture],
                              options: &mergeUniforms,
                              length: MemoryLayout<MergeUniforms>.stride)
            
        } else if currLOF {
            
            // Case 2: Non-interlace drawing (two long frames in a row)
            mergeBypassFilter.apply(commandBuffer: commandBuffer,
                                    textures: [longFrameTexture, mergeTexture])
        } else {
            
            // Case 3: Non-interlace drawing (two short frames in a row)
            mergeBypassFilter.apply(commandBuffer: commandBuffer,
                                    textures: [shortFrameTexture, mergeTexture])
        }
        
        // Compute upscaled texture (first pass, in-texture upscaling)
        enhancer.apply(commandBuffer: commandBuffer,
                       source: mergeTexture,
                       target: lowresEnhancedTexture)

        // Compute the bloom textures
        if shaderOptions.bloom != 0 {
            let bloomFilter = currentBloomFilter()
            bloomFilter.apply(commandBuffer: commandBuffer,
                              textures: [mergeTexture,
                                         bloomTextureR,
                                         bloomTextureG,
                                         bloomTextureB],
                              options: &shaderOptions,
                              length: MemoryLayout<ShaderOptions>.stride)

            func applyGauss(_ texture: inout MTLTexture, radius: Float) {

                if #available(OSX 10.13, *) {
                    let gauss = MPSImageGaussianBlur(device: device, sigma: radius)
                    gauss.encode(commandBuffer: commandBuffer,
                                 inPlaceTexture: &texture, fallbackCopyAllocator: nil)
                }
            }
            applyGauss(&bloomTextureR, radius: shaderOptions.bloomRadius)
            applyGauss(&bloomTextureG, radius: shaderOptions.bloomRadius)
            applyGauss(&bloomTextureB, radius: shaderOptions.bloomRadius)
        }
        
        // Compute upscaled texture (second pass)
        upscaler.apply(commandBuffer: commandBuffer,
                       source: lowresEnhancedTexture,
                       target: upscaledTexture)

        // Blur the upscaled texture
        if #available(OSX 10.13, *), shaderOptions.blur > 0 {
            let gauss = MPSImageGaussianBlur(device: device,
                                             sigma: shaderOptions.blurRadius)
            gauss.encode(commandBuffer: commandBuffer,
                         inPlaceTexture: &upscaledTexture,
                         fallbackCopyAllocator: nil)
        }

        // Emulate scanlines
        let scanlineFilter = currentScanlineFilter()
        scanlineFilter.apply(commandBuffer: commandBuffer,
                             source: upscaledTexture,
                             target: scanlineTexture,
                             options: &shaderOptions,
                             length: MemoryLayout<ShaderOptions>.stride)
    }
    
    func startFrame() {

        commandBuffer = queue.makeCommandBuffer()
        assert(commandBuffer != nil, "Command buffer must not be nil")

        // Set uniforms for the fragment shader
        /*
        fragmentUniforms.alpha = 1.0
        fragmentUniforms.dotMaskHeight = Int32(dotMaskTexture.height)
        fragmentUniforms.dotMaskWidth = Int32(dotMaskTexture.width)
        fragmentUniforms.scanlineDistance = Int32(size.height / 256)
        */
        
        // Create a render pass descriptor
        let descriptor = MTLRenderPassDescriptor.init()
        descriptor.colorAttachments[0].texture = drawable.texture
        descriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.95, 0.95, 0.95, 1)
        descriptor.colorAttachments[0].loadAction = MTLLoadAction.clear
        descriptor.colorAttachments[0].storeAction = MTLStoreAction.store

        descriptor.depthAttachment.texture = depthTexture
        descriptor.depthAttachment.clearDepth = 1
        descriptor.depthAttachment.loadAction = MTLLoadAction.clear
        descriptor.depthAttachment.storeAction = MTLStoreAction.dontCare

        runTexturePipeline()
        
        // Create a command encoder
        commandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: descriptor)
        commandEncoder.setRenderPipelineState(pipeline)
        commandEncoder.setDepthStencilState(depthState)
        commandEncoder.setFragmentTexture(dotMaskTexture, index: 4)
        commandEncoder.setFragmentBytes(&shaderOptions,
                                        length: MemoryLayout<ShaderOptions>.stride,
                                        index: 0)
        
        // Finally, we have to decide for a texture sampler. We use a linear
        // interpolation sampler, if Gaussian blur is enabled, and a nearest
        // neighbor sampler if Gaussian blur is disabled.
        let sampler = shaderOptions.blur > 0 ? samplerLinear : samplerNearest
        commandEncoder.setFragmentSamplerState(sampler, index: 0)
    }

    func draw2D() {

        if canvas.isTransparent { splashScreen.render() }
        if canvas.isVisible { canvas.render2D() }
        if monis.drawActivityMonitors { monis.render2D() }
    }
    
    func draw3D() {

        // Perform a single animation step
        if animates != 0 { performAnimationStep() }

        if canvas.isTransparent { splashScreen.render() }
        if canvas.isVisible { canvas.render3D() }
        if monis.drawActivityMonitors { monis.render3D() }
    }
    
    func endFrame() {

        commandEncoder.endEncoding()

        commandBuffer.addCompletedHandler { _ in
            self.semaphore.signal()
        }

        if drawable != nil {
            commandBuffer.present(drawable)
            commandBuffer.commit()
        }
    }

    //
    // Managing layout
    //

    func reshape() {

        reshape(withSize: size)
    }

    func reshape(withSize size: CGSize) {

        // Rebuild matrices
        buildMatrices2D()
        buildMatrices3D()

        // Rebuild depth buffer
        buildDepthBuffer()
    }
    
    //
    // Methods from MTKViewDelegate
    //

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {

        reshape(withSize: size)
    }
    
    func draw(in view: MTKView) {
        
        frames += 1
        
        updateTexture()
        splashScreen.update(frames: frames)
        canvas.update(frames: frames)

        semaphore.wait()
        drawable = metalLayer.nextDrawable()
        
        if drawable != nil {
            startFrame()
            fullscreen && !parent.pref.keepAspectRatio ? draw2D() : draw3D()
            endFrame()
        } 
    }
}