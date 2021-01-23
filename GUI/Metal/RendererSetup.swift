// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import simd

struct TextureSize {

    static let background = MTLSizeMake(512, 512, 0)
    static let original   = MTLSizeMake(1024, 320, 0)
    static let merged     = MTLSizeMake(1024, 640, 0)
    static let upscaled   = MTLSizeMake(2048, 1280, 0)
}

extension Renderer {

    func setupMetal() {

        track()

        buildMetal()
        buildMonitors()
        buildTextures()
        buildSamplers()
        buildKernels()
        buildDotMasks()
        buildPipeline()
        buildVertexBuffer()

        reshape()
    }

    internal func buildMetal() {

        track()

        // Metal layer
        metalLayer = mtkView.layer as? CAMetalLayer
        metalAssert(metalLayer != nil,
                    "The Core Animation layer could not be accessed.")

        metalLayer.device = device
        metalLayer.pixelFormat = MTLPixelFormat.bgra8Unorm
        metalLayer.framebufferOnly = true
        metalLayer.frame = metalLayer.frame

        // Command queue
        queue = device.makeCommandQueue()
        metalAssert(queue != nil,
                    "The Command Queue could not be created.")

        // Shader library
        library = device.makeDefaultLibrary()
        metalAssert(library != nil,
                    "The Shader Library could not be built.")
    }
    
    func buildMonitors() {
        
        // Activity monitors are colorized with the bus debuggger colors
        let info = parent.amiga.dmaDebugger.getInfo()
        
        // DMA monitors
        monitors.append(BarChart.init(device: device, name: "Copper DMA", logScale: true))
        monitors.append(BarChart.init(device: device, name: "Blitter DMA", logScale: true))
        monitors.append(BarChart.init(device: device, name: "Disk DMA"))
        monitors.append(BarChart.init(device: device, name: "Audio DMA"))
        monitors.append(BarChart.init(device: device, name: "Sprite DMA"))
        monitors.append(BarChart.init(device: device, name: "Bitplane DMA"))
        
        monitors[0].setColor(rgb: info.copperColor)
        monitors[1].setColor(rgb: info.blitterColor)
        monitors[2].setColor(rgb: info.diskColor)
        monitors[3].setColor(rgb: info.audioColor)
        monitors[4].setColor(rgb: info.spriteColor)
        monitors[5].setColor(rgb: info.bitplaneColor)

        // Memory monitors
        monitors.append(BarChart.init(device: device, name: "CPU (Chip Ram)",
                                      splitView: true))
        monitors.append(BarChart.init(device: device, name: "CPU (Slow Ram)",
                                      splitView: true))
        monitors.append(BarChart.init(device: device, name: "CPU (Fast Ram)",
                                      splitView: true))
        monitors.append(BarChart.init(device: device, name: "CPU (Rom)",
                                      splitView: true))

        // Waveform monitors
        monitors.append(WaveformMonitor.init(device: device,
                                             paula: parent.amiga.paula,
                                             leftChannel: true))
        monitors.append(WaveformMonitor.init(device: device,
                                             paula: parent.amiga.paula,
                                             leftChannel: false))

        for _ in 0 ... monitors.count {
            monitorAlpha.append(AnimatedFloat(0))
            monitorEnabled.append(true)
        }
        updateMonitorPositions()        
    }

    func updateMonitorPositions() {

        updateMonitorPositions(aspectRatio: Double(size.width) / Double(size.height))
    }
    
    func updateMonitorPositions(aspectRatio: Double) {
        
        //    w  d  w  d  w  d  w  d  w  d  w
        //   ___   ___   ___   ___   ___   ___
        //  |   |-|   |-|   |-|   |-|   |-|   | h
        //   ---   ---   ---   ---   ---   ---
        
        // Grid position and rotation side for all monitors
        var grid: [(Int, Int, Side)]
        
        switch monitorLayout {
            
        case 0: // Top and bottom

            grid = [ (0, 0, .lower), (1, 0, .lower), (2, 0, .lower),
                     (3, 0, .lower), (4, 0, .lower), (5, 0, .lower),
                     (1, 5, .upper), (2, 5, .upper), (3, 5, .upper), (4, 5, .upper),
                     (0, 5, .upper), (5, 5, .upper) ]

        case 1: // Top

            grid = [ (0, 5, .upper), (1, 5, .upper), (2, 5, .upper),
                     (3, 5, .upper), (4, 5, .upper), (5, 5, .upper),
                     (1, 4, .upper), (2, 4, .upper), (3, 4, .upper), (4, 4, .upper),
                     (0, 4, .upper), (5, 4, .upper) ]

        case 2: // Bottom
            
            grid = [ (0, 0, .lower), (1, 0, .lower), (2, 0, .lower),
                     (3, 0, .lower), (4, 0, .lower), (5, 0, .lower),
                     (1, 1, .lower), (2, 1, .lower), (3, 1, .lower), (4, 1, .lower),
                     (0, 1, .lower), (5, 1, .lower) ]
            
        case 3: // Left and right
            
            grid = [ (0, 5, .left ), (0, 4, .left ), (0, 3, .left ),
                     (0, 2, .left ), (0, 1, .left ), (0, 0, .left ),
                     (5, 4, .right), (5, 3, .right), (5, 2, .right), (5, 1, .right),
                     (5, 5, .right), (5, 0, .right) ]
            
        case 4: // Left
            
            grid = [ (0, 5, .left), (0, 4, .left), (0, 3, .left),
                     (0, 2, .left), (0, 1, .left), (0, 0, .left),
                     (1, 4, .left), (1, 3, .left), (1, 2, .left), (1, 1, .left),
                     (1, 5, .left), (1, 0, .left) ]
            
        case 5: // Right
            
            grid = [ (5, 5, .right), (5, 4, .right), (5, 3, .right),
                     (5, 2, .right), (5, 1, .right), (5, 0, .right),
                     (4, 4, .right), (4, 3, .right), (4, 2, .right), (4, 1, .right),
                     (4, 5, .right), (4, 0, .right) ]

        default: fatalError()
        }

        let ymax = 0.365
        let ymin = -ymax
        let yspan = ymax - ymin

        let xmax = ymax * aspectRatio // 0.49
        let xmin = -xmax
        let xspan = xmax - xmin

        let d = 0.02
        let w = (xspan - 5 * d) / 6
        let h = (yspan - 5 * d) / 6

        for i in 0 ..< monitors.count {
    
            let x = xmin + Double(grid[i].0) * (w + d)
            let y = ymin + Double(grid[i].1) * (h + d)
            monitors[i].position = NSRect.init(x: x, y: y, width: w, height: h)
            monitors[i].rotationSide = grid[i].2
        }
    }
    
    func buildTextures() {

        track()
                
        // Texture usages
        let r: MTLTextureUsage = [ .shaderRead ]
        let rwt: MTLTextureUsage = [ .shaderRead, .shaderWrite, .renderTarget ]
        let rwtp: MTLTextureUsage = [ .shaderRead, .shaderWrite, .renderTarget, .pixelFormatView ]

        // Background texture used in window mode
        bgTexture = device.makeTexture(size: TextureSize.background, usage: r)
        metalAssert(bgTexture != nil,
                    "The background texture could not be allocated.")
        
        // Background texture used in fullscreen mode
        let c1 = (0x00, 0x00, 0x00, 0xFF)
        let c2 = (0x44, 0x44, 0x44, 0xFF)
        bgFullscreenTexture = device.makeTexture(size: TextureSize.background,
                                                 gradient: [c1, c2], usage: r)
        metalAssert(bgFullscreenTexture != nil,
                    "The fullscreen texture could not be allocated.")

        // Emulator texture (long frames)
        longFrameTexture = device.makeTexture(size: TextureSize.original, usage: r)
        metalAssert(longFrameTexture != nil,
                    "The frame texture (long frames) could not be allocated.")

        // Emulator texture (short frames)
        shortFrameTexture = device.makeTexture(size: TextureSize.original, usage: r)
        metalAssert(shortFrameTexture != nil,
                    "The frame texture (short frames) could not be allocated.")

        // Merged emulator texture (long frame + short frame)
        mergeTexture = device.makeTexture(size: TextureSize.merged, usage: rwt)
        metalAssert(mergeTexture != nil,
                    "The merge texture could not be allocated.")

        // Bloom textures
        bloomTextureR = device.makeTexture(size: TextureSize.merged, usage: rwt)
        bloomTextureG = device.makeTexture(size: TextureSize.merged, usage: rwt)
        bloomTextureB = device.makeTexture(size: TextureSize.merged, usage: rwt)
        metalAssert(bloomTextureR != nil,
                    "The bloom texture (R channel) could not be allocated.")
        metalAssert(bloomTextureG != nil,
                    "The bloom texture (G channel) could not be allocated.")
        metalAssert(bloomTextureB != nil,
                    "The bloom texture (B channel) could not be allocated.")

        // Target for in-texture upscaling
        lowresEnhancedTexture = device.makeTexture(size: TextureSize.merged, usage: rwt)
        metalAssert(lowresEnhancedTexture != nil,
                    "The lowres enhancer texture could not be allocated.")

        // Upscaled merge texture
        upscaledTexture = device.makeTexture(size: TextureSize.upscaled, usage: rwtp)
        scanlineTexture = device.makeTexture(size: TextureSize.upscaled, usage: rwtp)
        metalAssert(upscaledTexture != nil,
                    "The upscaling texture could not be allocated.")
        metalAssert(scanlineTexture != nil,
                    "The scanline texture could not be allocated.")
    }

    internal func buildSamplers() {

        let descriptor = MTLSamplerDescriptor()
        descriptor.sAddressMode = MTLSamplerAddressMode.clampToEdge
        descriptor.tAddressMode = MTLSamplerAddressMode.clampToEdge
        descriptor.mipFilter = MTLSamplerMipFilter.notMipmapped

        // Nearest neighbor sampler
        descriptor.minFilter = MTLSamplerMinMagFilter.linear
        descriptor.magFilter = MTLSamplerMinMagFilter.linear
        samplerLinear = device.makeSamplerState(descriptor: descriptor)

        // Linear sampler
        descriptor.minFilter = MTLSamplerMinMagFilter.nearest
        descriptor.magFilter = MTLSamplerMinMagFilter.nearest
        samplerNearest = device.makeSamplerState(descriptor: descriptor)
    }

    internal func buildKernels() {
        
        precondition(library != nil)
        
        shaderOptions = ShaderOptions.init(
            blur: config.blur,
            blurRadius: config.blurRadius,
            bloom: config.bloom,
            bloomRadius: config.bloomRadius,
            bloomBrightness: config.bloomBrightness,
            bloomWeight: config.bloomWeight,
            flicker: config.flicker,
            flickerWeight: config.flickerWeight,
            dotMask: config.dotMask,
            dotMaskBrightness: config.dotMaskBrightness,
            scanlines: config.scanlines,
            scanlineBrightness: config.scanlineBrightness,
            scanlineWeight: config.scanlineWeight,
            disalignment: config.disalignment,
            disalignmentH: config.disalignmentH,
            disalignmentV: config.disalignmentV
        )
        
        let mc = (TextureSize.merged.width, TextureSize.merged.height)
        let uc = (TextureSize.upscaled.width, TextureSize.upscaled.height)

        // Build the merge filters
        mergeFilter = MergeFilter.init(device: device, library: library, cutout: mc)
        mergeBypassFilter = MergeBypassFilter.init(device: device, library: library, cutout: mc)
        
        // Build low-res enhancers (first-pass, in-texture upscaling)
        enhancerGallery[0] = BypassFilter.init(device: device, library: library, cutout: mc)
        enhancerGallery[1] = InPlaceEpxScaler.init(device: device, library: library, cutout: mc)
        enhancerGallery[2] = InPlaceXbrScaler.init(device: device, library: library, cutout: mc)
        enhancer = enhancerGallery[0]

        // Build upscalers (second-pass upscaling)
        upscalerGallery[0] = BypassUpscaler.init(device: device, library: library, cutout: uc)
        upscalerGallery[1] = EPXUpscaler.init(device: device, library: library, cutout: uc)
        upscalerGallery[2] = XBRUpscaler.init(device: device, library: library, cutout: uc)
        upscaler = upscalerGallery[0]
        
        // Build bloom filters
        bloomFilterGallery[0] = BypassFilter.init(device: device, library: library, cutout: uc)
        bloomFilterGallery[1] = SplitFilter.init(device: device, library: library, cutout: uc)

        // Build scanline filters
        scanlineFilterGallery[0] = BypassFilter.init(device: device, library: library, cutout: uc)
        scanlineFilterGallery[1] = SimpleScanlines(device: device, library: library, cutout: uc)
        scanlineFilterGallery[2] = BypassFilter.init(device: device, library: library, cutout: uc)
    }

    func buildDotMasks() {

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

    func buildPipeline() {

        precondition(library != nil)

        // Get vertex and fragment shader from library
        let vertexFunc = library.makeFunction(name: "vertex_main")
        let fragmentFunc = library.makeFunction(name: "fragment_main")
        metalAssert(vertexFunc != nil && fragmentFunc != nil,
                    "Cannot create Render Pipeline function objects.")
        
        // Create depth stencil state
        let stencilDescriptor = MTLDepthStencilDescriptor.init()
        stencilDescriptor.depthCompareFunction = MTLCompareFunction.less
        stencilDescriptor.isDepthWriteEnabled = true
        depthState = device.makeDepthStencilState(descriptor: stencilDescriptor)

        // Setup vertex descriptor
        let vertexDescriptor = MTLVertexDescriptor.init()

        // Positions
        vertexDescriptor.attributes[0].format = MTLVertexFormat.float4
        vertexDescriptor.attributes[0].offset = 0
        vertexDescriptor.attributes[0].bufferIndex = 0

        // Texture coordinates
        vertexDescriptor.attributes[1].format = MTLVertexFormat.half2
        vertexDescriptor.attributes[1].offset = 16
        vertexDescriptor.attributes[1].bufferIndex = 1

        // Single interleaved buffer
        vertexDescriptor.layouts[0].stride = 24
        vertexDescriptor.layouts[0].stepRate = 1
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunction.perVertex

        // Render pipeline
        let pipelineDescriptor = MTLRenderPipelineDescriptor.init()
        pipelineDescriptor.label = "vAmiga Metal pipeline"
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormat.depth32Float
        pipelineDescriptor.vertexFunction = vertexFunc
        pipelineDescriptor.fragmentFunction = fragmentFunc

        // Color attachment
        let colorAttachment = pipelineDescriptor.colorAttachments[0]!
        colorAttachment.pixelFormat = MTLPixelFormat.bgra8Unorm
        colorAttachment.isBlendingEnabled = true
        colorAttachment.rgbBlendOperation = MTLBlendOperation.add
        colorAttachment.alphaBlendOperation = MTLBlendOperation.add
        colorAttachment.sourceRGBBlendFactor = MTLBlendFactor.sourceAlpha
        colorAttachment.sourceAlphaBlendFactor = MTLBlendFactor.sourceAlpha
        colorAttachment.destinationRGBBlendFactor = MTLBlendFactor.oneMinusSourceAlpha
        colorAttachment.destinationAlphaBlendFactor = MTLBlendFactor.oneMinusSourceAlpha
        do {
            try pipeline = device.makeRenderPipelineState(descriptor: pipelineDescriptor)
        } catch {
            metalAssert(false, "Cannot create the Metal GPU pipeline.")
        }
    }

    func buildVertexBuffer() {

        bgRect = Node.init(device: device,
                           x: -1.0, y: -1.0, z: 0.99, w: 2.0, h: 2.0,
                           t: NSRect.init(x: 0.0, y: 0.0, width: 1.0, height: 1.0))

        quad2D = Node.init(device: device,
                           x: -1.0, y: -1.0, z: 0.0, w: 2.0, h: 2.0,
                           t: textureRect)

        quad3D = Quad.init(device: device,
                           x1: -0.64, y1: -0.48, z1: -0.64,
                           x2: 0.64, y2: 0.48, z2: 0.64,
                           t: textureRect)
    }

    func buildMatricesBg() {

        let model  = matrix_identity_float4x4
        let view   = matrix_identity_float4x4
        let aspect = Float(size.width) / Float(size.height)
        let proj   = Renderer.perspectiveMatrix(fovY: (Float(65.0 * (.pi / 180.0))),
                                       aspect: aspect,
                                       nearZ: 0.1,
                                       farZ: 100.0)

        vertexUniformsBg.mvp = proj * view * model
    }

    func buildMatrices2D() {

        let model = Renderer.translationMatrix(x: 0, y: 0, z: 0.99)
        let view  = matrix_identity_float4x4
        let proj  = matrix_identity_float4x4

        vertexUniforms2D.mvp = proj * view * model
    }

    func buildMatrices3D() {

        let xAngle = -angleX.current / 180.0 * .pi
        let yAngle = angleY.current / 180.0 * .pi
        let zAngle = angleZ.current / 180.0 * .pi

        let xShift = -shiftX.current
        let yShift = -shiftY.current
        let zShift = shiftZ.current

        let aspect = Float(size.width) / Float(size.height)
        
        let view = matrix_identity_float4x4
        let proj = Renderer.perspectiveMatrix(fovY: Float(65.0 * (.pi / 180.0)),
                                     aspect: aspect,
                                     nearZ: 0.1,
                                     farZ: 100.0)

        let transEye = Renderer.translationMatrix(x: xShift,
                                         y: yShift,
                                         z: zShift + 1.393 - 0.16)

        let transRotX = Renderer.translationMatrix(x: 0.0,
                                          y: 0.0,
                                          z: 0.16)

        let rotX = Renderer.rotationMatrix(radians: xAngle, x: 0.5, y: 0.0, z: 0.0)
        let rotY = Renderer.rotationMatrix(radians: yAngle, x: 0.0, y: 0.5, z: 0.0)
        let rotZ = Renderer.rotationMatrix(radians: zAngle, x: 0.0, y: 0.0, z: 0.5)

        // Chain all transformations
        let model = transEye * rotX * transRotX * rotY * rotZ

        vertexUniforms3D.mvp = proj * view * model
    }

    func buildDepthBuffer() {

        let width = Int(size.width)
        let height = Int(size.height)

        precondition(width != 0)
        precondition(height != 0)

        let descriptor = MTLTextureDescriptor.texture2DDescriptor(
            pixelFormat: MTLPixelFormat.depth32Float,
            width: width,
            height: height,
            mipmapped: false)
        descriptor.resourceOptions = MTLResourceOptions.storageModePrivate
        descriptor.usage = MTLTextureUsage.renderTarget

        depthTexture = device.makeTexture(descriptor: descriptor)
        metalAssert(depthTexture != nil,
                    "Could not create depth texture.")
    }
    
    //
    // Matrix utilities
    //
    
    static func perspectiveMatrix(fovY: Float,
                                  aspect: Float,
                                  nearZ: Float,
                                  farZ: Float) -> matrix_float4x4 {
        
        // Variant 1: Keeps correct aspect ratio independent of window size
        let yscale = 1.0 / tanf(fovY * 0.5) // 1 / tan == cot
        let xscale = yscale / aspect
        let q = farZ / (farZ - nearZ)
        
        // Alternative: Adjust to window size
        // float yscale = 1.0f / tanf(fovY * 0.5f);
        // float xscale = 0.75 * yscale;
        // float q = farZ / (farZ - nearZ);
        
        var m = matrix_float4x4()
        m.columns.0 = SIMD4<Float>(xscale, 0.0, 0.0, 0.0)
        m.columns.1 = SIMD4<Float>(0.0, yscale, 0.0, 0.0)
        m.columns.2 = SIMD4<Float>(0.0, 0.0, q, 1.0)
        m.columns.3 = SIMD4<Float>(0.0, 0.0, q * -nearZ, 0.0)
        
        return m
    }
    
    static func scalingMatrix(xs: Float, ys: Float, zs: Float) -> matrix_float4x4 {
        
        var m = matrix_float4x4()
        m.columns.0 = SIMD4<Float>(xs, 0.0, 0.0, 0.0)
        m.columns.1 = SIMD4<Float>(0.0, ys, 0.0, 0.0)
        m.columns.2 = SIMD4<Float>(0.0, 0.0, zs, 0.0)
        m.columns.3 = SIMD4<Float>(0.0, 0.0, 0.0, 1.0)
        
        return m
    }
    
    static func translationMatrix(x: Float,
                                  y: Float,
                                  z: Float) -> matrix_float4x4 {
        
        var m = matrix_identity_float4x4
        m.columns.3 = SIMD4<Float>(x, y, z, 1.0)
        
        return m
    }
    
    static func rotationMatrix(radians: Float,
                               x: Float,
                               y: Float,
                               z: Float) -> matrix_float4x4 {
        
        var v = vector_float3(x, y, z)
        v = normalize(v)
        let cos = cosf(radians)
        let cosp = 1.0 - cos
        let sin = sinf(radians)
        
        var m = matrix_float4x4()
        m.columns.0 = SIMD4<Float>(cos + cosp * v.x * v.x,
                                   cosp * v.x * v.y + v.z * sin,
                                   cosp * v.x * v.z - v.y * sin,
                                   0.0)
        m.columns.1 = SIMD4<Float>(cosp * v.x * v.y - v.z * sin,
                                   cos + cosp * v.y * v.y,
                                   cosp * v.y * v.z + v.x * sin,
                                   0.0)
        m.columns.2 = SIMD4<Float>(cosp * v.x * v.z + v.y * sin,
                                   cosp * v.y * v.z - v.x * sin,
                                   cos + cosp * v.z * v.z,
                                   0.0)
        m.columns.3 = SIMD4<Float>(0.0,
                                   0.0,
                                   0.0,
                                   1.0)
        return m
    }
    
    //
    // Error handling
    //
    
    func metalAssert(_ cond: Bool, _ msg: String) {
        
        if !cond {
            
            let alert = NSAlert()
            
            alert.alertStyle = .critical
            alert.icon = NSImage.init(named: "metal")
            alert.messageText = "Failed to initialize Metal Hardware"
            alert.informativeText = msg
            alert.addButton(withTitle: "OK")
            alert.runModal()
            
            exit(1)
        }
    }
}
