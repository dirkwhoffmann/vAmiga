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

enum ScreenshotSource: Int {
        
    case visible = 0
    case visibleUpscaled = 1
    case entire = 2
    case entireUpscaled = 3
}

class Renderer: NSObject, MTKViewDelegate {
    
    let mtkView: MTKView
    let device: MTLDevice
    let parent: MyController
    
    var prefs: Preferences { return parent.pref }
    var config: Configuration { return parent.config }

    // Number of drawn frames since power up
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
    
    var queue: MTLCommandQueue! = nil
    var pipeline: MTLRenderPipelineState! = nil
    var depthState: MTLDepthStencilState! = nil
    
    //
    // Layers
    //
    
    var metalLayer: CAMetalLayer! = nil
    
    var size: CGSize {

        let frameSize = mtkView.frame.size
        let scale = mtkView.layer?.contentsScale ?? 1

        return CGSize(width: frameSize.width * scale,
                      height: frameSize.height * scale)
    }

    var splashScreen: SplashScreen! = nil
    var canvas: Canvas! = nil
    var monitors: Monitors! = nil
    var console: Console! = nil
    
    // Texture to hold the pixel depth information
    var depthTexture: MTLTexture! = nil

    // Kernels (shaders)
    var kernelManager: KernelManager! = nil

    // Nearest neighbor sampler
    var samplerNearest: MTLSamplerState!
    
    // Linear interpolation sampler
    var samplerLinear: MTLSamplerState!
    
    // Shader options
    var shaderOptions: ShaderOptions!
    
    // Indicates if an animation is currently performed
    var animates = 0
    
    // Indicates if the splash screen should be rendered
    var renderSplash = true
    
    // Animation parameters
    var angleX = AnimatedFloat(0.0)
    var angleY = AnimatedFloat(0.0)
    var angleZ = AnimatedFloat(0.0)
    
    var shiftX = AnimatedFloat(0.0)
    var shiftY = AnimatedFloat(0.0)
    var shiftZ = AnimatedFloat(0.0)
        
    var white = AnimatedFloat(0.0)
    
    // Animation variables for smooth texture zooming
    var cutoutX1 = AnimatedFloat.init()
    var cutoutY1 = AnimatedFloat.init()
    var cutoutX2 = AnimatedFloat.init()
    var cutoutY2 = AnimatedFloat.init()

    // Part of the texture that is currently visible
    var textureRect = CGRect.init() { didSet { buildVertexBuffers() } }
            
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
        
        self.mtkView.device = device
        self.mtkView.delegate = self

        setup()
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
    // Screenshots
    //
    
    func screenshot(source: ScreenshotSource) -> NSImage? {

        track("source: \(source)")

        switch source {
            
        case .entire:
            return screenshot(texture: canvas.mergeTexture, rect: largestVisibleNormalized)
            
        case .entireUpscaled:
            return screenshot(texture: canvas.upscaledTexture, rect: largestVisibleNormalized)
            
        case .visible:
            return screenshot(texture: canvas.mergeTexture, rect: visibleNormalized)
            
        case .visibleUpscaled:
            return screenshot(texture: canvas.upscaledTexture, rect: visibleNormalized)
        }
    }

    func screenshot(texture: MTLTexture, rect: CGRect) -> NSImage? {
        
        // Use the blitter to copy the texture data back from the GPU
        let queue = texture.device.makeCommandQueue()!
        let commandBuffer = queue.makeCommandBuffer()!
        let blitEncoder = commandBuffer.makeBlitCommandEncoder()!
        blitEncoder.synchronize(texture: texture, slice: 0, level: 0)
        blitEncoder.endEncoding()
        commandBuffer.commit()
        commandBuffer.waitUntilCompleted()
        
        return NSImage.make(texture: texture, rect: rect)
    }

    //
    //  Drawing
    //

    func makeCommandEncoder(_ commandBuffer: MTLCommandBuffer,
                            _ drawable: CAMetalDrawable) -> MTLRenderCommandEncoder? {
        
        let descriptor = MTLRenderPassDescriptor.init()
        descriptor.colorAttachments[0].texture = drawable.texture
        descriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1)
        descriptor.colorAttachments[0].loadAction = MTLLoadAction.clear
        descriptor.colorAttachments[0].storeAction = MTLStoreAction.store

        descriptor.depthAttachment.texture = depthTexture
        descriptor.depthAttachment.clearDepth = 1
        descriptor.depthAttachment.loadAction = MTLLoadAction.clear
        descriptor.depthAttachment.storeAction = MTLStoreAction.dontCare
        
        // Create a command encoder
        let commandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: descriptor)
        commandEncoder?.setRenderPipelineState(pipeline)
        commandEncoder?.setDepthStencilState(depthState)
        commandEncoder?.setFragmentBytes(&shaderOptions,
                                         length: MemoryLayout<ShaderOptions>.stride,
                                         index: 0)
        
        /* Finally, we have to decide for a texture sampler. We use a linear
         * interpolation sampler, if Gaussian blur is enabled, and a nearest
         * neighbor sampler if Gaussian blur is disabled.
         */
        let sampler = shaderOptions.blur > 0 ? samplerLinear : samplerNearest
        commandEncoder?.setFragmentSamplerState(sampler, index: 0)
        
        return commandEncoder
    }

    //
    // Methods from MTKViewDelegate
    //

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {

        reshape(withSize: size)
    }
    
    func update(frames: Int64) {
                         
        if animates != 0 { animate() }
    }
    
    func draw(in view: MTKView) {
        
        frames += 1
        update(frames: frames)
        splashScreen.update(frames: frames)
        canvas.update(frames: frames)
        monitors.update(frames: frames)
        console.update(frames: frames)
        
        semaphore.wait()
        
        if let drawable = metalLayer.nextDrawable() {
            
            renderSplash = renderSplash && canvas.isTransparent
            let renderCanvas = canvas.isVisible
            let renderMonitors = monitors.drawActivityMonitors
            
            let flat = fullscreen && !parent.pref.keepAspectRatio
            
            let buffer = queue.makeCommandBuffer()!
            if renderSplash { splashScreen.render(buffer: buffer) }
            if renderCanvas { canvas.render(buffer: buffer) }
            if renderMonitors { monitors.render(buffer: buffer) }
            
            if let commandEncoder = makeCommandEncoder(buffer, drawable) {
                if renderSplash { splashScreen.render(encoder: commandEncoder, flat: flat) }
                if renderCanvas { canvas.render(encoder: commandEncoder, flat: flat) }
                if renderMonitors { monitors.render(encoder: commandEncoder, flat: flat) }
                commandEncoder.endEncoding()
                
                buffer.addCompletedHandler { _ in self.semaphore.signal() }
                buffer.present(drawable)
                buffer.commit()
            }
        }
    }
}
