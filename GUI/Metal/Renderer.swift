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
        
    case emulator = 0
    case upscaler = 1
    case framebuffer = 2
}

enum ScreenshotCutout: Int {
        
    case visible = 0
    case entire = 1
    case automatic = 2
    case custom = 3
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
    
    var shaderOptions = ShaderOptions.zero

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
    
    func updateShaderOptions() {
        
        updateShaderOption(.MON_BLUR, value: Int64(config.blur))
        updateShaderOption(.MON_BLUR_RADIUS, value: Int64(config.blurRadius))
        updateShaderOption(.MON_BLOOM, value: Int64(config.bloom))
        updateShaderOption(.MON_BLOOM_RADIUS, value: Int64(config.bloomRadius))
        updateShaderOption(.MON_BLOOM_BRIGHTNESS, value: Int64(config.bloomBrightness))
        updateShaderOption(.MON_BLOOM_WEIGHT, value: Int64(config.bloomWeight))
        updateShaderOption(.MON_FLICKER, value: Int64(config.flicker))
        updateShaderOption(.MON_FLICKER_WEIGHT, value: Int64(config.flickerWeight))
        updateShaderOption(.MON_DOTMASK, value: Int64(config.dotMask))
        updateShaderOption(.MON_DOTMASK, value: Int64(config.dotMask))
        updateShaderOption(.MON_DOTMASK_BRIGHTNESS, value: Int64(config.dotMaskBrightness))
        updateShaderOption(.MON_SCANLINES, value: Int64(config.scanlines))
        updateShaderOption(.MON_SCANLINE_BRIGHTNESS, value: Int64(config.scanlineBrightness))
        updateShaderOption(.MON_SCANLINE_WEIGHT, value: Int64(config.scanlineWeight))
        updateShaderOption(.MON_DISALIGNMENT, value: Int64(config.disalignment))
        updateShaderOption(.MON_DISALIGNMENT_H, value: Int64(config.disalignmentH))
        updateShaderOption(.MON_DISALIGNMENT_V, value: Int64(config.disalignmentV))
    }
    
    func updateShaderOption(_ option: Option, value: Int64)
    {
        func map(_ value: Int64, from source: ClosedRange<Int64> = 0...1000, to target: ClosedRange<Float>) -> Float {
            
            let clamped = min(max(value, source.lowerBound), source.upperBound)
            let normalized = Float(clamped - source.lowerBound) / Float(source.upperBound - source.lowerBound)
            return target.lowerBound + normalized * (target.upperBound - target.lowerBound)
        }

        switch option {
            
        case .MON_ENHANCER:
            ressourceManager.selectEnhancer(Int(value))
        case .MON_UPSCALER:
            ressourceManager.selectUpscaler(Int(value))
        case .MON_BLUR:
            shaderOptions.blur = Int32(value)
        case .MON_BLUR_RADIUS:
            shaderOptions.blurRadius = map(value, to: 0...5.0)
        case .MON_BLOOM:
            shaderOptions.bloom = Int32(value)
            ressourceManager.selectBloomFilter(Int(value))
        case .MON_BLOOM_RADIUS:
            shaderOptions.bloomRadius = map(value, to: 0...5)
        case .MON_BLOOM_BRIGHTNESS:
            shaderOptions.bloomBrightness = map(value, to: 0...2)
        case .MON_BLOOM_WEIGHT:
            shaderOptions.bloomWeight = map(value, to: 0...3)
        case .MON_DOTMASK:
            shaderOptions.dotMask = Int32(value)
            ressourceManager.buildDotMasks()
            ressourceManager.selectDotMask(Int(value))
        case .MON_DOTMASK_BRIGHTNESS:
            shaderOptions.dotMaskBrightness = map(value, to: 0...1)
            ressourceManager.buildDotMasks()
            ressourceManager.selectDotMask(config.dotMask)
        case .MON_SCANLINES:
            shaderOptions.scanlines = Int32(value)
        case .MON_SCANLINE_BRIGHTNESS:
            shaderOptions.scanlineBrightness = map(value, to: 0...1)
        case .MON_SCANLINE_WEIGHT:
            shaderOptions.scanlineWeight = map(value, to: 0...1)
        case .MON_DISALIGNMENT:
            shaderOptions.disalignment = Int32(value)
        case .MON_DISALIGNMENT_H:
            shaderOptions.disalignmentH = map(value, to: -0.004...0.004)
        case .MON_DISALIGNMENT_V:
            shaderOptions.disalignmentV = map(value, to: -0.004...0.004)
        case .MON_FLICKER:
            shaderOptions.flicker = Int32(value)
        case .MON_FLICKER_WEIGHT:
            shaderOptions.flickerWeight = map(value, to: 0...1)
        default:
            break
        }
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

        if !BuildSettings.msgCallback {
            
            // Process all pending messages
            var msg = Message()
            while amiga.amiga.getMessage(&msg) {
                parent.process(message: msg)
            }
        }
        
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
    
    func processMessage(_ msg: Message) {
        
        let option = Option(rawValue: Int(msg.value))!

        switch msg.type {
            
        case .MON_SETTING:
            
            switch option {
                
            case .MON_CENTER, .MON_HCENTER, .MON_VCENTER, .MON_ZOOM, .MON_HZOOM, .MON_VZOOM:
                canvas.updateTextureRect()

            default:
                updateShaderOption(option, value: msg.value2)
            }
            
        default:
            break
        }
    }
            
    //
    // Methods from MTKViewDelegate
    //

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {

        reshape(withSize: size)
    }
    
    func draw(in view: MTKView) {
        
        MainActor.assertIsolated("Not isolated!!")
        
        frames += 1
        update(frames: frames)

        // Wait for the next frame
        semaphore.wait()

        // Get drawable
        guard let drawable = metalLayer.nextDrawable() else { semaphore.signal(); return }

        // Create the command buffer
        let buffer = makeCommandBuffer()
        
        // Create the command encoder
        guard let encoder = makeCommandEncoder(drawable, buffer) else { semaphore.signal(); return }
        
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
