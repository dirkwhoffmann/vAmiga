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
    
    var prefs: Preferences { return parent.prefs }
    var config: Configuration { return parent.config }
    
    /* Number of drawn frames since power up
     * Used to determine the fps value shown in the emulator's bottom bar.
     */
    var frames: Int64 = 0
    
    /* Synchronization semaphore
     * The semaphore is locked in function draw() and released in function
     * endFrame(). It's puprpose is to prevent a new frame from being drawn
     * if the previous isn't finished yet. Not sure if we really need it.
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
    // Metal buffers and uniforms
    //
    
    var bgRect: Node?
    var quad2D: Node?
    var quad3D: Quad?
            
    var vertexUniforms2D = VertexUniforms(mvp: matrix_identity_float4x4)
    var vertexUniforms3D = VertexUniforms(mvp: matrix_identity_float4x4)
    var vertexUniformsBg = VertexUniforms(mvp: matrix_identity_float4x4)
    
    var fragmentUniforms = FragmentUniforms(alpha: 1.0,
                                            dotMaskWidth: 0,
                                            dotMaskHeight: 0,
                                            scanlineDistance: 0)
    
    var mergeUniforms = MergeUniforms(interlace: 0,
                                      longFrameScale: 1.0,
                                      shortFrameScale: 1.0)

    //
    // Activity monitors
    //

    struct Monitor {
        
        static let copper = 0
        static let blitter = 1
        static let disk = 2
        static let audio = 3
        static let sprite = 4
        static let bitplane = 5
        
        static let chipRam = 6
        static let slowRam = 7
        static let fastRam = 8
        static let kickRom = 9

        static let waveformL = 10
        static let waveformR = 11
    }

    var monitors: [ActivityMonitor] = []

    // Global enable switch for all activity monitors
    var drawActivityMonitors = false { didSet { updateMonitorAlphas() } }

    // Individual enable switch for each activity monitor
    var monitorEnabled: [Bool] = [] { didSet { updateMonitorAlphas() } }
    
    // Global alpha value of activity monitors
    var monitorGlobalAlpha = Float(0.5)
    
    // Layout scheme used for positioning the monitors
    var monitorLayout = 0 { didSet { updateMonitorPositions() } }
    
    //
    // Textures
    //

    // Background textures
    var bgTexture: MTLTexture! = nil
    var bgFullscreenTexture: MTLTexture! = nil

    // Texture to hold the pixel depth information
    var depthTexture: MTLTexture! = nil
    
    /* Long frame texture (raw data from emulator, 1024 x 512)
     * This texture is filled with the screen buffer data from the emulator.
     * The texture is updated in function updateLongFrameTexture() which is
     * called periodically in drawRect().
     */
    var longFrameTexture: MTLTexture! = nil
    
    /* Short frame texture (raw data from emulator, 1024 x 512)
     * This texture is filled with the screen buffer data from the emulator.
     * The texture is updated in function updateShortFrameTexture() which is
     * called periodically in drawRect().
     */
    var shortFrameTexture: MTLTexture! = nil
    
    /* Merge texture (1024 x 1024)
     * The long frame and short frame textures are merged into this one.
     */
    var mergeTexture: MTLTexture! = nil
    
    /* Bloom textures to emulate blooming (512 x 512)
     * To emulate a bloom effect, the C64 texture is first split into it's
     * R, G, and B parts. Each texture is then run through a Gaussian blur
     * filter with a large radius. These blurred textures are passed into
     * the fragment shader as a secondary textures where they are recomposed
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
        
    /* An instance of the merge filter
     */
    var mergeFilter: MergeFilter! = nil
    
    // Array holding all available lowres enhancers
    var enhancerGallery = [ComputeKernel?](repeating: nil, count: 3)
    
    // Array holding all available upscalers
    var upscalerGallery = [ComputeKernel?](repeating: nil, count: 3)
    
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
    
    // Animation parameters
    var angleX = AnimatedFloat(0.0)
    var angleY = AnimatedFloat(0.0)
    var angleZ = AnimatedFloat(0.0)
    
    var shiftX = AnimatedFloat(0.0)
    var shiftY = AnimatedFloat(0.0)
    var shiftZ = AnimatedFloat(0.0)
    
    var alpha = AnimatedFloat(0.0)
    var noise = AnimatedFloat(0.0)
    
    var monitorAlpha: [AnimatedFloat] = []
    
    // Animation variables for smooth texture zooming
    var cutoutX1 = AnimatedFloat.init()
    var cutoutY1 = AnimatedFloat.init()
    var cutoutX2 = AnimatedFloat.init()
    var cutoutY2 = AnimatedFloat.init()

    // Part of the texture that is currently visible
    var textureRect = CGRect.init()

    // Indicates if the current frame is a long frame or a short frame (DEPRECATED)
    var flickerToggle = 0

    // Indicates the type of the frame that is read next
    var requestLongFrame = true
    
    // Is set to true when fullscreen mode is entered (usually enables the 2D renderer)
    var fullscreen = false
    
    //
    // Initializing
    //

    init(view: MTKView, device: MTLDevice, controller: MyController) {
        
        self.mtkView = view
        self.device = device
        self.parent = controller
        super.init()

        textureRect = computeTextureRect()
        setupMetal()

        mtkView.delegate = self
        mtkView.device = device
    }

    //
    // Managing textures
    //

    func updateBgTexture(bytes: UnsafeMutablePointer<UInt32>) {

        bgTexture.replace(w: 512, h: 512, buffer: bytes)
    }

    func clearBgTexture() {
        
        let w = 512
        let h = 512

        let bytes = UnsafeMutablePointer<UInt32>.allocate(capacity: w * h)
        bytes.initialize(repeating: 0xFFFF0000, count: w * h)
        
        updateBgTexture(bytes: bytes)
        bytes.deallocate()
    }
    
    func updateTexture(bytes: UnsafeMutablePointer<UInt32>, longFrame: Bool) {

        let w = Int(HPIXELS)
        let h = longFrameTexture.height
        let offset = Int(HBLANK_MIN) * 4
        
        if longFrame {
            longFrameTexture.replace(w: w, h: h, buffer: bytes + offset)
        } else {
            shortFrameTexture.replace(w: w, h: h, buffer: bytes + offset)
        }

        /*
        let region = MTLRegionMake2D(0, 0, w, h)

        if longFrame {
            longFrameTexture.replace(region: region,
                                     mipmapLevel: 0,
                                     withBytes: bytes + (Int(HBLANK_MIN) * 4),
                                     bytesPerRow: 4 * w)
        } else {
            shortFrameTexture.replace(region: region,
                                      mipmapLevel: 0,
                                      withBytes: bytes + (Int(HBLANK_MIN) * 4),
                                      bytesPerRow: 4 * w)
        }
        */
    }

    func updateTexture() {

        if requestLongFrame {

            let buffer = parent.amiga.denise.stableLongFrame()
            updateTexture(bytes: buffer.data, longFrame: true)

            // If interlace mode is on, the next frame will be a short frame
            if parent.amiga.agnus.interlaceMode() { requestLongFrame = false }
            
        } else {

            let buffer = parent.amiga.denise.stableShortFrame()
            updateTexture(bytes: buffer.data, longFrame: false)

            // The next frame will be a long frame
            requestLongFrame = true
        }
    }
    
    func computeTextureRect() -> CGRect {
        
        /*
         *       aw <--------- maxWidth --------> dw
         *    ah |-----|---------------------|-----|
         *     ^ |     bw                   cw     |
         *     | -  bh *<----- width  ------>*     -
         *     | |     ^                     ^     |
         *     | |     |                     |     |
         *     | |   height                height  |
         *     | |     |                     |     |
         *     | |     v                     v     |
         *     | -  ch *<----- width  ------>*     -
         *     v |                                 |
         *    dh |-----|---------------------|-----|
         *
         *      aw/ah - dw/dh = largest posible texture cutout
         *      bw/bh - cw/ch = currently used texture cutout
         */
        let aw = Float(HBLANK_CNT) * 4
        let dw = Float(HPOS_CNT) * 4
        let ah = Float(VBLANK_CNT)
        let dh = Float(VPOS_CNT)
        
        let maxWidth = dw - aw
        let maxHeight = dh - ah
        
        let width = (1 - config.hZoom) * maxWidth
        let bw = aw + config.hCenter * (maxWidth - width)
        let height = (1 - config.vZoom) * maxHeight
        let bh = ah + config.vCenter * (maxHeight - height)
                
        let texW = EmulatorTexture.width
        let texH = EmulatorTexture.height
        
        return CGRect.init(x: CGFloat(bw) / CGFloat(texW),
                           y: CGFloat(bh) / CGFloat(texH),
                           width: CGFloat(width) / CGFloat(texW),
                           height: CGFloat(height) / CGFloat(texH))
    }
    
    // DEPRECATED
    func updateTextureRect() {
        
        textureRect = computeTextureRect()
        buildVertexBuffer()
    }
    
    //
    // Managing kernels
    //

    // Returns the compute kernel of the currently selected upscaler (first pass)
    func currentEnhancer() -> ComputeKernel {

        var nr = config.enhancer
        if enhancerGallery.count <= nr || enhancerGallery[nr] == nil { nr = 0 }
        return enhancerGallery[nr]!
    }

    // Returns the compute kernel of the currently selected upscaler (second pass)
    func currentUpscaler() -> ComputeKernel {

        var nr = config.upscaler
        if upscalerGallery.count <= nr || upscalerGallery[nr] == nil { nr = 0 }
        return upscalerGallery[nr]!
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
    // Managing activity monitors
    //

    func fadeIn(monitor nr: Int, steps: Int = 40) {

        assert(nr < monitors.count)
        
        monitorAlpha[nr].target = 1.0
        monitorAlpha[nr].steps = steps
        animates |= AnimationType.monitors
    }
        
    func fadeOut(monitor nr: Int, steps: Int = 40) {

        assert(nr < monitors.count)
        
        monitorAlpha[nr].target = 0.0
        monitorAlpha[nr].steps = steps
        animates |= AnimationType.monitors
    }

    func fadeOutMonitors() {

        for i in 0 ..< monitors.count { fadeOut(monitor: i) }
    }

    func updateMonitorAlphas() {
        
        for i in 0 ..< monitors.count where i < monitorEnabled.count {
            if drawActivityMonitors && monitorEnabled[i] {
                fadeIn(monitor: i)
            } else {
                fadeOut(monitor: i)
            }
        }
    }
    
    //
    //  Drawing
    //

    func startFrame() {

        commandBuffer = queue.makeCommandBuffer()
        assert(commandBuffer != nil, "Command buffer must not be nil")

        // Set uniforms for the fragment shader
        fragmentUniforms.alpha = 1.0
        fragmentUniforms.dotMaskHeight = Int32(dotMaskTexture.height)
        fragmentUniforms.dotMaskWidth = Int32(dotMaskTexture.width)
        fragmentUniforms.scanlineDistance = Int32(size.height / 256)

        // Set uniforms for the merge shader
        if parent.amiga.agnus.interlaceMode() {

            let weight = shaderOptions.flicker > 0 ? (1.0 - shaderOptions.flickerWeight) : Float(1.0)
            mergeUniforms.interlace = 1
            mergeUniforms.longFrameScale = (flickerToggle & 2) == 0 ? 1.0 : weight
            mergeUniforms.shortFrameScale = (flickerToggle & 2) == 0 ? weight : 1.0
            flickerToggle += 1
        } else {
            mergeUniforms.interlace = 0
            mergeUniforms.longFrameScale = 1.0
            mergeUniforms.shortFrameScale = 1.0
        }

        // Compute the merge texture
        mergeFilter.apply(commandBuffer: commandBuffer,
                          textures: [longFrameTexture, shortFrameTexture, mergeTexture],
                          options: &mergeUniforms)

        // Compute upscaled texture (first pass, in-texture upscaling)
        let enhancer = currentEnhancer()
        enhancer.apply(commandBuffer: commandBuffer,
                       source: mergeTexture,
                       target: lowresEnhancedTexture)

        // Compute the bloom textures
        if shaderOptions.bloom != 0 {
            let bloomFilter = currentBloomFilter()
            bloomFilter.apply(commandBuffer: commandBuffer,
                              textures: [mergeTexture, bloomTextureR, bloomTextureG, bloomTextureB],
                              options: &shaderOptions)

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

        let upscaler = currentUpscaler()
        upscaler.apply(commandBuffer: commandBuffer,
                       source: lowresEnhancedTexture, // mergeTexture,
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
                             options: &shaderOptions)

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

    func drawScene2D() {

        startFrame()

        // Configure vertex shader
        commandEncoder.setVertexBytes(&vertexUniforms2D,
                                      length: MemoryLayout<VertexUniforms>.stride,
                                      index: 1)

        // Configure fragment shader
        commandEncoder.setFragmentTexture(scanlineTexture, index: 0)
        commandEncoder.setFragmentBytes(&fragmentUniforms,
                                        length: MemoryLayout<FragmentUniforms>.stride,
                                        index: 1)

        // Draw
        quad2D!.drawPrimitives(commandEncoder)
        
        // Draw activity monitors
        if drawActivityMonitors {
            
            for i in 0 ... monitors.count where monitorAlpha[i].current != 0.0 {
                
                fragmentUniforms.alpha = monitorAlpha[i].current * monitorGlobalAlpha
                commandEncoder.setFragmentBytes(&fragmentUniforms,
                                                length: MemoryLayout<FragmentUniforms>.stride,
                                                index: 1)
                monitors[i].draw(commandEncoder, matrix: vertexUniforms3D.mvp)
            }
        }
        
        endFrame()
    }
    
    var angle = 0
 
    func drawScene3D() {

        let paused = parent.amiga.isPaused()
        let poweredOff = parent.amiga.isPoweredOff()
        let renderBackground = poweredOff || fullscreen
        let renderForeground = alpha.current > 0.0

        // Perform a single animation step
        if animates != 0 { performAnimationStep() }

        startFrame()

        // Render background
        if renderBackground {
            
            // Update background texture
            if !fullscreen {
                let buffer = parent.amiga.denise.noise()
                updateBgTexture(bytes: buffer!)
            }

            // Configure vertex shader
            vertexUniformsBg.mvp = matrix_identity_float4x4
            commandEncoder.setVertexBytes(&vertexUniformsBg,
                                          length: MemoryLayout<VertexUniforms>.stride,
                                          index: 1)

            // Configure fragment shader
            if fullscreen {
                fragmentUniforms.alpha = 1.0
                commandEncoder.setFragmentTexture(bgFullscreenTexture, index: 0)
                commandEncoder.setFragmentTexture(bgFullscreenTexture, index: 1)
            } else {
                fragmentUniforms.alpha = noise.current
                commandEncoder.setFragmentTexture(bgTexture, index: 0)
                commandEncoder.setFragmentTexture(bgTexture, index: 1)
            }
            commandEncoder.setFragmentBytes(&fragmentUniforms,
                                            length: MemoryLayout<FragmentUniforms>.stride,
                                            index: 1)

            // Draw
            bgRect!.drawPrimitives(commandEncoder)
        }

        // Render emulator texture
        if renderForeground {

            // Configure vertex shader
            commandEncoder.setVertexBytes(&vertexUniforms3D,
                                          length: MemoryLayout<VertexUniforms>.stride,
                                          index: 1)
            // Configure fragment shader
            fragmentUniforms.alpha = paused ? 0.5 : alpha.current
            commandEncoder.setFragmentTexture(scanlineTexture, index: 0)
            commandEncoder.setFragmentTexture(bloomTextureR, index: 1)
            commandEncoder.setFragmentTexture(bloomTextureG, index: 2)
            commandEncoder.setFragmentTexture(bloomTextureB, index: 3)
            commandEncoder.setFragmentBytes(&fragmentUniforms,
                                            length: MemoryLayout<FragmentUniforms>.stride,
                                            index: 1)

            // Draw (part of) cube
            quad3D!.draw(commandEncoder, allSides: animates != 0)
            
            // Draw activity monitors
            if drawActivityMonitors {
                
                for i in 0 ... monitors.count where monitorAlpha[i].current != 0.0 {
                    
                    fragmentUniforms.alpha = monitorAlpha[i].current * monitorGlobalAlpha
                    commandEncoder.setFragmentBytes(&fragmentUniforms,
                                                    length: MemoryLayout<FragmentUniforms>.stride,
                                                    index: 1)
                    monitors[i].draw(commandEncoder, matrix: vertexUniforms3D.mvp)
                }
            }
        }
        
        endFrame()
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

        frames += 1
    }

    //
    // Managing layout
    //

    func reshape(withSize size: CGSize) {

        // Rebuild matrices
        buildMatricesBg()
        buildMatrices2D()
        buildMatrices3D()

        // Rebuild depth buffer
        buildDepthBuffer()
    }

    func reshape() {

        reshape(withSize: size)
    }

    //
    // Methods from MTKViewDelegate
    //

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {

        reshape(withSize: size)
    }
    
    func draw(in view: MTKView) {
        
        semaphore.wait()

        drawable = metalLayer.nextDrawable()
        if drawable != nil {
            updateTexture()
            if fullscreen && !parent.prefs.keepAspectRatio {
                drawScene2D()
            } else {
                drawScene3D()
            }
        } 
    }
}
