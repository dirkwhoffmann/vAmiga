// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import simd

//
// Static texture parameters
//

// Parameters of a short / long frame texture delivered by the emulator
struct EmulatorTexture {

    static let width = 1024
    static let height = 320
}

// Parameters of a textures that combines a short and a long frame
struct MergedTexture {

    static let width = EmulatorTexture.width
    static let height = 2 * EmulatorTexture.height
    static let cutout = (width, height)
}

// Parameters of a (merged) texture that got upscaled
struct UpscaledTexture {

    static let width = 2 * MergedTexture.width
    static let height = 2 * MergedTexture.height
    static let cutout = (width, height)
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
        assert(metalLayer != nil, "Metal layer must not be nil")

        metalLayer.device = device
        metalLayer.pixelFormat = MTLPixelFormat.bgra8Unorm
        metalLayer.framebufferOnly = true
        metalLayer.frame = metalLayer.frame

        // Command queue
        queue = device.makeCommandQueue()
        assert(queue != nil, "Metal command queue must not be nil")

        // Shader library
        library = device.makeDefaultLibrary()
        assert(library != nil, "Metal library must not be nil")
    }
    
    func buildMonitors() {
        
        // Activity monitors are colorized with the bus debuggger colors
        let info = controller.amiga.agnus.getDebuggerInfo()
        let rgb = [
            NSColor.init(info.colorRGB.0),
            NSColor.init(info.colorRGB.1),
            NSColor.init(info.colorRGB.2),
            NSColor.init(info.colorRGB.3),
            NSColor.init(info.colorRGB.4),
            NSColor.init(info.colorRGB.5),
            NSColor.init(info.colorRGB.6),
            NSColor.init(info.colorRGB.7),
            NSColor.init(info.colorRGB.8)
        ]

        dmaMonitors[Monitor.copper] =
            BarChart.init(device: device, name: "Copper DMA",
                          color: rgb[Int(BUS_COPPER.rawValue)], logScale: true)
        dmaMonitors[Monitor.blitter] =
            BarChart.init(device: device, name: "Blitter DMA",
                          color: rgb[Int(BUS_BLITTER.rawValue)], logScale: true)
        dmaMonitors[Monitor.disk] =
            BarChart.init(device: device, name: "Disk DMA",
                          color: rgb[Int(BUS_DISK.rawValue)])
        dmaMonitors[Monitor.audio] =
            BarChart.init(device: device, name: "Audio DMA",
                          color: rgb[Int(BUS_AUDIO.rawValue)])
        dmaMonitors[Monitor.sprite] =
            BarChart.init(device: device, name: "Sprite DMA",
                          color: rgb[Int(BUS_SPRITE.rawValue)])
        dmaMonitors[Monitor.bitplane] =
            BarChart.init(device: device, name: "Bitplane DMA",
                          color: rgb[Int(BUS_BITPLANE.rawValue)])
            
        for _ in 0 ... 5 { monitorAlpha.append(AnimatedFloat(0)) }
        
        updateMonitorPositions()
    }

    func updateMonitorPositions() {
        
        //    w  d  w  d  w  d  w  d  w  d  w
        //   ___   ___   ___   ___   ___   ___
        //  |   |-|   |-|   |-|   |-|   |-|   | h
        //   ---   ---   ---   ---   ---   ---
        
        let xmax = 0.49
        let xmin = -xmax
        let xspan = xmax - xmin

        let ymax = 0.365
        let ymin = -ymax
        let yspan = ymax - ymin

        let d = 0.02
        let w = (xspan - 5 * d) / 6
        let h = (yspan - 5 * d) / 6 // w * 2/3
        
        var bx, by, dx, dy: Double
        
        switch dmaMonitors[0]!.alignment {
        case 0: bx = xmin;     by = ymin;     dx = w + d; dy = 0    // bottom
        case 1: bx = xmin;     by = ymax - h; dx = w + d; dy = 0    // top
        case 2: bx = xmin;     by = ymax - h; dx = 0; dy = -(h + d) // left
        case 3: bx = xmax - w; by = ymax - h; dx = 0; dy = -(h + d) // right
        default: fatalError()
        }

        for i in 0 ..< dmaMonitors.count {
            
            let rect = NSRect.init(x: bx + Double(i) * dx,
                                   y: by + Double(i) * dy, width: w, height: h)
            dmaMonitors[i]!.position = rect
        }
    }
    
    internal func buildTextures() {

        track()

        // let loader = MTKTextureLoader.init(device: device)
                   
        //
        // Background texture
        //

        let descriptor = MTLTextureDescriptor.texture2DDescriptor(
            pixelFormat: MTLPixelFormat.rgba8Unorm,
            width: 512,
            height: 512,
            mipmapped: false)

        // Background texture (drawn behind the cube)
        bgTexture = device.makeTexture(descriptor: descriptor)

        //
        // Emulator textures (one for short frames, one for long frames)
        //

        descriptor.width = EmulatorTexture.width
        descriptor.height = EmulatorTexture.height

        // Emulator textures (raw data of long and short frames)
        descriptor.usage = [ .shaderRead ]
        longFrameTexture = device.makeTexture(descriptor: descriptor)
        assert(longFrameTexture != nil, "Failed to create long frame texture.")
        shortFrameTexture = device.makeTexture(descriptor: descriptor)
        assert(shortFrameTexture != nil, "Failed to create short frame texture.")

        //
        // Textures that combine a short and a long frame (not yet upscaled)
        //

        descriptor.width = MergedTexture.width
        descriptor.height = MergedTexture.height

        // Merged emulator texture (long frame + short frame)
        descriptor.usage = [ .shaderRead, .shaderWrite, .renderTarget ]
        mergeTexture = device.makeTexture(descriptor: descriptor)
        assert(mergeTexture != nil, "Failed to create merge texture.")

        // Bloom textures
        descriptor.usage = [ .shaderRead, .shaderWrite, .renderTarget ]
        bloomTextureR = device.makeTexture(descriptor: descriptor)
        bloomTextureG = device.makeTexture(descriptor: descriptor)
        bloomTextureB = device.makeTexture(descriptor: descriptor)
        assert(bloomTextureR != nil, "Failed to create bloom texture (R).")
        assert(bloomTextureG != nil, "Failed to create bloom texture (G).")
        assert(bloomTextureB != nil, "Failed to create bloom texture (B).")

        descriptor.usage = [ .shaderRead, .shaderWrite, .renderTarget ]
        lowresEnhancedTexture = device.makeTexture(descriptor: descriptor)
        assert(lowresEnhancedTexture != nil, "Failed to create lowres enhancement texture.")

        //
        // Upscaled textures
        //

        descriptor.width  = UpscaledTexture.width
        descriptor.height = UpscaledTexture.height

        // Upscaled emulator texture
        descriptor.usage = [ .shaderRead, .shaderWrite, .pixelFormatView, .renderTarget ]
        upscaledTexture = device.makeTexture(descriptor: descriptor)
        assert(upscaledTexture != nil, "Failed to create upscaling texture.")

        // Scanline texture
        scanlineTexture = device.makeTexture(descriptor: descriptor)
        assert(scanlineTexture != nil, "Failed to create scanline texture.")
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

        assert(library != nil)

        let mc = MergedTexture.cutout
        let uc = UpscaledTexture.cutout

        // Build the mergefilter
        mergeFilter = MergeFilter.init(device: device, library: library, cutout: mc)

        // Build low-res enhancers (first-pass, in-texture upscaling)
        enhancerGallery[0] = BypassFilter.init(device: device, library: library, cutout: mc)
        enhancerGallery[1] = InPlaceEpxScaler.init(device: device, library: library, cutout: mc)
        enhancerGallery[2] = InPlaceXbrScaler.init(device: device, library: library, cutout: mc)

        // Build upscalers (second-pass upscaling)
        upscalerGallery[0] = BypassUpscaler.init(device: device, library: library, cutout: uc)
        upscalerGallery[1] = EPXUpscaler.init(device: device, library: library, cutout: uc)
        upscalerGallery[2] = XBRUpscaler.init(device: device, library: library, cutout: uc)

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

        track()
        assert(library != nil)

        // Get vertex and fragment shader from library
        let vertexFunc = library.makeFunction(name: "vertex_main")
        let fragmentFunc = library.makeFunction(name: "fragment_main")
        assert(vertexFunc != nil)
        assert(fragmentFunc != nil)

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
            fatalError("Cannot create Metal graphics pipeline")
        }
    }

    func buildVertexBuffer() {

        bgRect = Node.init(device: device,
                           x: -6.4, y: -4.8, z: 6.8, w: 12.8, h: 9.6,
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

        let model = matrix_identity_float4x4
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
        assert(depthTexture != nil, "Failed to create depth texture")
    }
}
