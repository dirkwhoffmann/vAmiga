// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Metal

enum ScreenshotSource: Int {
        
    case emulatorVisible = 0
    case emulatorEntire = 1
    case upscaledVisible = 2
    case upscaledEntire = 3
    case framebuffer = 4
}

@MainActor
class Renderer: NSObject, MTKViewDelegate {
    
    let view: MTKView
    let device: MTLDevice
    let parent: MyController
    
    var prefs: Preferences { return parent.pref }
    var config: Configuration { return parent.config }
    var amiga: EmulatorProxy { return parent.emu }

    // Number of drawn frames since power up
    var frames: Int64 = 0

    // The current GPU frame rate
    var fps = 60

    // Time stamp used for auto-detecting the frame rate
    var timestamp = CACurrentMediaTime()

    // Frame synchronization semaphore
    var semaphore = DispatchSemaphore(value: 1)

    //
    // Metal entities
    //
    
    var queue: MTLCommandQueue! = nil
    var pipeline: MTLRenderPipelineState! = nil
    var depthState: MTLDepthStencilState! = nil
    var descriptor: MTLRenderPassDescriptor! = nil
    
    //
    // Layers
    //
    
    var metalLayer: CAMetalLayer! = nil
    var splashScreen: SplashScreen! = nil
    var canvas: Canvas! = nil
    var console: Console! = nil
    var dropZone: DropZone! = nil

    //
    // Ressources
    //
    
    var ressourceManager: RessourceManager! = nil
    
    //
    // Uniforms
    //
    
    var shaderOptions: ShaderOptions!

    //
    // Animations
    //
    
    // Indicates if an animation is in progress
    var animates = 0
        
    // Color animation parameters
    var white = AnimatedFloat(0.0)
    
    // Texture animation parameters
    var cutoutX1 = AnimatedFloat(0.0)
    var cutoutY1 = AnimatedFloat(0.0)
    var cutoutX2 = AnimatedFloat(0.0)
    var cutoutY2 = AnimatedFloat(0.0)
            
    // Indicates if fullscreen mode is enabled
    var fullscreen = false
    
    //
    // Initializing
    //

    init(view: MTKView, device: MTLDevice, controller: MyController) {
        
        self.view = view
        self.device = device
        self.parent = controller

        super.init()
        
        self.view.device = device
        self.view.delegate = self
        
        setup()
    }
    
    func halt() {

        // Wait until the current frame has been completed
        semaphore.wait()
    }
    
    //
    // Managing layout
    //

    var size: CGSize {

        let frameSize = view.frame.size
        let scale = view.layer?.contentsScale ?? 1

        return CGSize(width: frameSize.width * scale,
                      height: frameSize.height * scale)
    }
    
    func reshape() {

        reshape(withSize: size)
    }
    
    func reshape(withSize size: CGSize) {

        // Rebuild matrices
        buildMatrices2D()

        // Rebuild depth buffer
        ressourceManager.buildDepthBuffer()
    }

    var recordingRect: CGRect {

        var result: CGRect

        switch prefs.captureSource {

        case .visible:  result = canvas.textureRectAbs
        case .entire:   result = canvas.entire
        }

        // Make sure the screen dimensions are even
        if Int(result.size.width) % 2 == 1 { result.size.width -= 1 }
        if Int(result.size.height) % 2 == 1 { result.size.height -= 1 }

        return result
    }

    //
    //  Drawing
    //

    func makeCommandBuffer() -> MTLCommandBuffer {
    
        let commandBuffer = queue.makeCommandBuffer()!
        canvas.makeCommandBuffer(buffer: commandBuffer)
        
        return commandBuffer
    }

    func makeCommandEncoder(_ drawable: CAMetalDrawable, _ buffer: MTLCommandBuffer) -> MTLRenderCommandEncoder? {
        
        // Update the render pass descriptor
        descriptor.colorAttachments[0].texture = drawable.texture
        descriptor.depthAttachment.texture = ressourceManager.depthTexture
        
        // Create a command encoder
        let encoder = buffer.makeRenderCommandEncoder(descriptor: descriptor)
        encoder?.setRenderPipelineState(pipeline)
        encoder?.setDepthStencilState(depthState)
        
        return encoder
    }

    //
    // Updating
    //
    
    func update(frames: Int64) {
                         
        if animates != 0 { animate() }

        splashScreen.update(frames: frames)
        dropZone.update(frames: frames)
        console.update(frames: frames)
        canvas.update(frames: frames)
        parent.update(frames: frames)

        measureFps(frames: frames)
    }

    func measureFps(frames: Int64) {

        let interval = Int64(32)

        if frames % interval == 0 {

            let now = CACurrentMediaTime()
            let elapsed = now - timestamp
            timestamp = now

            let newfps = Int(round(Double(interval) / elapsed))
            if newfps != fps {

                debug(.vsync, "Measured GPU frame rate: \(newfps)")

                if [50, 60, 100, 120, 200, 240].contains(newfps) {

                    fps = newfps
                    amiga.set(.HOST_REFRESH_RATE, value: Int(fps))
                    debug(.vsync, "New GPU frame rate: \(amiga.get(.HOST_REFRESH_RATE))")
                }
            }
        }
    }

    //
    // Methods from MTKViewDelegate
    //

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {

        amiga.set(.HOST_FRAMEBUF_WIDTH, value: Int(size.width))
        amiga.set(.HOST_FRAMEBUF_HEIGHT, value: Int(size.height))
        reshape(withSize: size)
    }
    
    func draw(in view: MTKView) {
        
        MainActor.assertIsolated("Not isolated!!")
        
        frames += 1
        update(frames: frames)

        semaphore.wait()

        if let drawable = metalLayer.nextDrawable() {

            // Create the command buffer
            let buffer = makeCommandBuffer()
            
            // Create the command encoder
            guard let encoder = makeCommandEncoder(drawable, buffer) else {

                semaphore.signal()
                return
            }

            // Render the scene
            if canvas.isTransparent { splashScreen.render(encoder) }
            if canvas.isVisible { canvas.render(encoder) }
            encoder.endEncoding()

            // Commit the command buffer
            buffer.addCompletedHandler { _ in self.semaphore.signal() }
            buffer.present(drawable)
            buffer.commit()
        }
    }
}
