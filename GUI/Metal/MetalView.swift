// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation
import Metal
import MetalKit
import MetalPerformanceShaders

struct C64Texture {
    static let orig = NSSize.init(width: 1024, height: 512)
    static let upscaled = NSSize.init(width: 2048, height: 2048)
}

public class MetalView: MTKView {
    
    @IBOutlet weak var controller: MyController!
    
    /// Number of drawn frames since power up
    var frames: UInt64 = 0
    
    // Synchronization semaphore
    var semaphore: DispatchSemaphore!
    
    // Metal objects
    var library: MTLLibrary! = nil
    var queue: MTLCommandQueue! = nil
    var pipeline: MTLRenderPipelineState! = nil
    var depthState: MTLDepthStencilState! = nil
    var commandBuffer: MTLCommandBuffer! = nil
    var commandEncoder: MTLRenderCommandEncoder! = nil
    var drawable: CAMetalDrawable! = nil
    
    // Metal layer
    var metalLayer: CAMetalLayer! = nil
    var layerWidth = CGFloat(0.0)
    var layerHeight = CGFloat(0.0)
    var layerIsDirty = true
    
    // Buffers
    var positionBuffer: MTLBuffer! = nil

    var vertexUniforms2D = VertexUniforms(mvp: matrix_identity_float4x4)
    var vertexUniforms3D = VertexUniforms(mvp: matrix_identity_float4x4)
    var vertexUniformsBg = VertexUniforms(mvp: matrix_identity_float4x4)

    var fragmentUniforms = FragmentUniforms(alpha: 1.0,
                                            dotMaskWidth: 0,
                                            dotMaskHeight: 0,
                                            scanlineDistance: 0)
    
    // Textures
    
    /// Background image behind the cube
    var bgTexture: MTLTexture! = nil
    
    /// Texture to hold the pixel depth information
    var depthTexture: MTLTexture! = nil
    
    /// Long frame texture (raw data from emulator, 1024 x 512)
    /// This texture is filled with the screen buffer data from the emulator.
    /// The texture is updated in function updateLongFrameTexture() which is
    /// called periodically in drawRect().
    var longFrameTexture: MTLTexture! = nil

    /// Short frame texture (raw data from emulator, 1024 x 512)
    /// This texture is filled with the screen buffer data from the emulator.
    /// The texture is updated in function updateShortFrameTexture() which is
    /// called periodically in drawRect().
    var shortFrameTexture: MTLTexture! = nil
 
    /// Merge texture (1024 x 1024)
    /// The long frame and short frame textures are merged into this one.
    var mergeTexture: MTLTexture! = nil
    
    /// Bloom textures to emulate blooming (512 x 512)
    /// To emulate a bloom effect, the C64 texture is first split into it's
    /// R, G, and B parts. Each texture is then run through a Gaussian blur
    /// filter with a large radius. These blurred textures are passed into
    /// the fragment shader as a secondary textures where they are recomposed
    /// with the upscaled primary texture.
    var bloomTextureR: MTLTexture! = nil
    var bloomTextureG: MTLTexture! = nil
    var bloomTextureB: MTLTexture! = nil

    /// Upscaled emulator texture (2048 x 2048)
    /// In the first texture processing stage, the emulator texture is bumped up
    /// by a factor of 4. The user can choose between bypass upscaling which
    /// simply replaces each pixel by a 4x4 quad or more sophisticated upscaling
    /// algorithms such as xBr.
    var upscaledTexture: MTLTexture! = nil
    
    /// Upscaled texture with scanlines (2048 x 2048)
    /// In the second texture processing stage, a scanline effect is applied to
    /// the upscaled texture.
    var scanlineTexture: MTLTexture! = nil
    
    /// Dotmask texture (variable size)
    /// This texture is used by the fragment shader to emulate a dotmask
    /// effect.
    var dotMaskTexture: MTLTexture! = nil
    
    // An instance of the merge filter
    var mergeFilter : MergeFilter! = nil
    
    // Array holding all available upscalers
    var upscalerGallery = [ComputeKernel?](repeating: nil, count: 3)

    // Array holding all available bloom filters
    var bloomFilterGallery = [ComputeKernel?](repeating: nil, count: 3)

    // Array holding all available scanline filters
    var scanlineFilterGallery = [ComputeKernel?](repeating: nil, count: 3)

    // Array holding dotmask preview images
    var dotmaskImages = [NSImage?](repeating: nil, count: 5)

    
    //
    // Texture samplers
    //
    
    // Nearest neighbor sampler
    var samplerNearest : MTLSamplerState! = nil

    // Linear interpolation sampler
    var samplerLinear : MTLSamplerState! = nil
    
    // Shader options
    var shaderOptions = Defaults.shaderOptions
    
    // Animation parameters
    var currentXAngle = Float(0.0)
    var targetXAngle = Float(0.0)
    var deltaXAngle = Float(0.0)
    var currentYAngle = Float(0.0)
    var targetYAngle = Float(0.0)
    var deltaYAngle = Float(0.0)
    var currentZAngle = Float(0.0)
    var targetZAngle = Float(0.0)
    var deltaZAngle = Float(0.0)
    var currentEyeX = Defaults.eyeX
    var targetEyeX = Defaults.eyeX
    var deltaEyeX = Float(0.0)
    var currentEyeY = Defaults.eyeY
    var targetEyeY = Defaults.eyeY
    var deltaEyeY = Float(0.0)
    var currentEyeZ = Defaults.eyeZ
    var targetEyeZ = Defaults.eyeZ
    var deltaEyeZ = Float(0.0)
    var currentAlpha = Float(0.0)
    var targetAlpha = Float(1.0)  // Start with an invisible screen
    var deltaAlpha = Float(0.0)
        
    /// Texture cut-out (normalized)
    var textureRect = CGRect.init(x: 0.0, y: 0.0, width: 0.0, height: 0.0)
 
    /// Currently selected texture upscaler
    var upscaler = Defaults.upscaler {
        didSet {
            if upscaler >= upscalerGallery.count || upscalerGallery[upscaler] == nil {
                track("Sorry, the selected GPU upscaler is unavailable.")
                upscaler = 0
            }
        }
    }
    
    //! If true, no GPU drawing is performed (for performance profiling olny)
    var enableMetal = false
    
    //! Is set to true when fullscreen mode is entered (usually enables the 2D renderer)
    var fullscreen = false
    
    //! If true, the 3D renderer is also used in fullscreen mode
    var keepAspectRatio = Defaults.keepAspectRatio
    
    required public init(coder: NSCoder) {
    
        super.init(coder: coder)
    }
    
    required public override init(frame frameRect: CGRect, device: MTLDevice?) {
        
        super.init(frame: frameRect, device: device)
    }
    
    override open func awakeFromNib() {

        track()
        
        // Create semaphore
        semaphore = DispatchSemaphore(value: 1);
        
        // Check if machine is capable to run the Metal graphics interface
        checkForMetal()
    
        // Register for drag and drop
        setupDragAndDrop()
    }
    
    override public var acceptsFirstResponder: Bool
    {
        get { return true }
    }
    
    //! Adjusts view height by a certain number of pixels
    func adjustHeight(_ height: CGFloat) {
    
        var newFrame = frame
        newFrame.origin.y -= height
        newFrame.size.height += height
        frame = newFrame
    }
    
    //! Shrinks view vertically by the height of the status bar
    public func shrink() { adjustHeight(-24.0) }
    
    //! Expand view vertically by the height of the status bar
    public func expand() { adjustHeight(24.0) }

    public func updateScreenGeometry() {
    
        var rect: CGRect
        
        // Setup parameters that look good for a PAL texture
        rect = CGRect.init(x: CGFloat(0),
                           y: CGFloat(0),
                           width: CGFloat(HPIXELS - 82),
                           height: CGFloat((2*VPIXELS) - 4))
        
        textureRect = CGRect.init(x: rect.minX / 1024,
                                  y: rect.minY / 1024,
                                  width: rect.width / 1024,
                                  height: rect.height / 1024)
        
        // Enable this for debugging (will display the whole texture)
        // textureRect = CGRect.init(x: 0.0, y: 0.0, width: 1.0, height: 1.0)
        
        // Update texture coordinates in vertex buffer
        buildVertexBuffer()
    }
    
    func updateTexture() {
        
        let buf = controller.amiga.denise.screenBuffer()
        assert(buf != nil)
        
        let pixelSize = 4
        let width = Int(HPIXELS)
        let height = Int(VPIXELS)
        let rowBytes = width * pixelSize
        let imageBytes = rowBytes * height
        let region = MTLRegionMake2D(0,0,width,height)
        
        if (controller.amiga.denise.longFrameIsReady()) {
            longFrameTexture.replace(region: region,
                                     mipmapLevel: 0,
                                     slice: 0,
                                     withBytes: buf!,
                                     bytesPerRow: rowBytes,
                                     bytesPerImage: imageBytes)
        } else {
            shortFrameTexture.replace(region: region,
                                     mipmapLevel: 0,
                                     slice: 0,
                                     withBytes: buf!,
                                     bytesPerRow: rowBytes,
                                     bytesPerImage: imageBytes)
        }
    }
    
    /// Returns the compute kernel of the currently selected pixel upscaler
    func currentUpscaler() -> ComputeKernel {
    
        assert(upscaler < upscalerGallery.count)
        assert(upscalerGallery[0] != nil)
        
        return upscalerGallery[upscaler]!
    }

    /// Returns the compute kernel of the currently selected bloom filter
    func currentBloomFilter() -> ComputeKernel {
        
        assert(shaderOptions.bloom < bloomFilterGallery.count)
        assert(bloomFilterGallery[0] != nil)
        
        return bloomFilterGallery[Int(shaderOptions.bloom)]!
    }

    /// Returns the compute kernel of the currently selected scanline filter
    func currentScanlineFilter() -> ComputeKernel {
        
        assert(shaderOptions.scanlines < scanlineFilterGallery.count)
        assert(scanlineFilterGallery[0] != nil)
        
        return scanlineFilterGallery[Int(shaderOptions.scanlines)]!
    }
    
    func startFrame() {
    
        commandBuffer = queue.makeCommandBuffer()
        assert(commandBuffer != nil, "Command buffer must not be nil")
    
        // Set uniforms for the fragment shader
        // fillFragmentShaderUniforms(uniformFragment)
        fragmentUniforms.alpha = 1.0
        fragmentUniforms.dotMaskHeight = Int32(dotMaskTexture.height)
        fragmentUniforms.dotMaskWidth = Int32(dotMaskTexture.width)
        fragmentUniforms.scanlineDistance = Int32(layerHeight / 256);
       
        // Compute the merge texture
        mergeFilter.apply(commandBuffer: commandBuffer,
                          textures: [longFrameTexture, shortFrameTexture, mergeTexture])
        
        // Compute the bloom textures
        shaderOptions.bloom = 0;
        /*
        if shaderOptions.bloom != 0 {
            let bloomFilter = currentBloomFilter()
            bloomFilter.apply(commandBuffer: commandBuffer,
                              textures: [emulatorTexture, bloomTextureR, bloomTextureG, bloomTextureB],
                              options: shaderOptions)
            
            func applyGauss(_ texture: inout MTLTexture, radius: Float) {
                
                if #available(OSX 10.13, *) {
                    let gauss = MPSImageGaussianBlur(device: device!, sigma: radius)
                    gauss.encode(commandBuffer: commandBuffer,
                                 inPlaceTexture: &texture, fallbackCopyAllocator: nil)
                }
            }
            applyGauss(&bloomTextureR, radius: shaderOptions.bloomRadiusR)
            applyGauss(&bloomTextureG, radius: shaderOptions.bloomRadiusG)
            applyGauss(&bloomTextureB, radius: shaderOptions.bloomRadiusB)
        }
        */
        
        // Compute upscaled texture
        let upscaler = currentUpscaler()
        upscaler.apply(commandBuffer: commandBuffer,
                       source: mergeTexture,
                       target: upscaledTexture)
        
        // Blur the upscaled texture
        if #available(OSX 10.13, *), shaderOptions.blur > 0 {
            let gauss = MPSImageGaussianBlur(device: device!,
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
                                options: shaderOptions)
        
        // Create render pass descriptor
        let descriptor = MTLRenderPassDescriptor.init()
        descriptor.colorAttachments[0].texture = drawable.texture
        descriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1)
        descriptor.colorAttachments[0].loadAction = MTLLoadAction.clear
        descriptor.colorAttachments[0].storeAction = MTLStoreAction.store
        
        descriptor.depthAttachment.texture = depthTexture
        descriptor.depthAttachment.clearDepth = 1
        descriptor.depthAttachment.loadAction = MTLLoadAction.clear
        descriptor.depthAttachment.storeAction = MTLStoreAction.dontCare
        
        // Create command encoder
        commandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: descriptor)
        commandEncoder.setRenderPipelineState(pipeline)
        commandEncoder.setDepthStencilState(depthState)
        commandEncoder.setFragmentTexture(dotMaskTexture, index: 4)
        commandEncoder.setFragmentBytes(&shaderOptions,
                                        length: MemoryLayout<ShaderOptions>.stride,
                                        index: 0);
        
        commandEncoder.setVertexBuffer(positionBuffer, offset: 0, index: 0)

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
        commandEncoder.drawPrimitives(type: MTLPrimitiveType.triangle,
                                      vertexStart: 42,
                                      vertexCount: 6,
                                      instanceCount: 1)
        endFrame()
    }
    
    func drawScene3D() {
    
        let animates = self.animates()
        let paused = controller.amiga.isPaused()
        let renderBackground = !fullscreen && !paused && (animates || (currentAlpha < 1.0))
        let renderForeground = currentAlpha > 0.0
        
        if animates {
            updateAngles()
            buildMatrices3D()
        }

        startFrame()
            
        // Render background
        if renderBackground {
            
            // Configure vertex shader
            // commandEncoder.setVertexBuffer(uniformBufferBg, offset: 0, index: 1)
            commandEncoder.setVertexBytes(&vertexUniformsBg,
                                          length: MemoryLayout<VertexUniforms>.stride,
                                          index: 1)
            
            // Configure fragment shader
            fragmentUniforms.alpha = 1
            commandEncoder.setFragmentTexture(bgTexture, index: 0)
            commandEncoder.setFragmentTexture(bgTexture, index: 1)
            commandEncoder.setFragmentBytes(&fragmentUniforms,
                                            length: MemoryLayout<FragmentUniforms>.stride,
                                            index: 1);
            
            // Draw
            commandEncoder.drawPrimitives(type: MTLPrimitiveType.triangle,
                                          vertexStart: 0,
                                          vertexCount: 6,
                                          instanceCount: 1)
        }
        
        // Render emulator texture
        if renderForeground {
            
            // Configure vertex shader
            commandEncoder.setVertexBytes(&vertexUniforms3D,
                                          length: MemoryLayout<VertexUniforms>.stride,
                                          index: 1)
            // Configure fragment shader
            fragmentUniforms.alpha = paused ? 0.5 : currentAlpha
            commandEncoder.setFragmentTexture(scanlineTexture, index: 0)
            commandEncoder.setFragmentTexture(bloomTextureR, index: 1)
            commandEncoder.setFragmentTexture(bloomTextureG, index: 2)
            commandEncoder.setFragmentTexture(bloomTextureB, index: 3)
            commandEncoder.setFragmentBytes(&fragmentUniforms,
                                            length: MemoryLayout<FragmentUniforms>.stride,
                                            index: 1);
            
            // Draw
            commandEncoder.drawPrimitives(type: MTLPrimitiveType.triangle,
                                          vertexStart: 6,
                                          vertexCount: (animates ? 24 : 6),
                                          instanceCount: 1)
        }
                
        endFrame()
    }

    func endFrame() {
    
        commandEncoder.endEncoding()
    
        commandBuffer.addCompletedHandler { cb in
            self.semaphore.signal()
        }
        
        if (drawable != nil) {
            commandBuffer.present(drawable)
            commandBuffer.commit()
        }
        
        frames += 1
    }
    
    override public func setFrameSize(_ newSize: NSSize) {
        
        super.setFrameSize(newSize)
        layerIsDirty = true
    }
    
    func reshape(withFrame frame: CGRect) {
    
        reshape()
    }

    func reshape() {

        let drawableSize = metalLayer.drawableSize
        
        if layerWidth == drawableSize.width && layerHeight == drawableSize.height {
            return
        }
    
        layerWidth = drawableSize.width
        layerHeight = drawableSize.height
    
        // Rebuild matrices
        buildMatricesBg()
        buildMatrices2D()
        buildMatrices3D()
    
        // Rebuild depth buffer
        buildDepthBuffer()
    }
    
    override public func draw(_ rect: NSRect) {
        
        if !enableMetal {
            return
        }

        // Wait until it's save to go ...
        // let result semaphore.wait (timeout: .distantFuture)
        semaphore.wait()
        
        // Refresh size dependent items if needed
        if layerIsDirty {
            reshape(withFrame: frame)
            layerIsDirty = false
        }
    
        // Draw scene
        drawable = metalLayer.nextDrawable()
        if (drawable != nil) {
            updateTexture()
            if fullscreen && !keepAspectRatio {
                drawScene2D()
            } else {
                drawScene3D()
            }
        }
    }
   
    public func cleanup() {
    
        track()
    }
    
}

