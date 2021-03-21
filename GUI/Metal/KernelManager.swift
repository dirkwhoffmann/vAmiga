//
//  KernelManager.swift
//  vAmiga
//
//  Created by Dirk Hoffmann on 21.03.21.
//  Copyright © 2021 Dirk Hoffmann. All rights reserved.
//

import Foundation

class KernelManager {
    
    let device: MTLDevice
    let renderer: Renderer
    
    var library: MTLLibrary! = nil
    
    // An instance of the merge filter
    var mergeFilter: MergeFilter! = nil

    // An instance of the merge bypass filter
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
    
    //
    // Initializing
    //
    
    init(view: MTKView, device: MTLDevice, renderer: Renderer) {
        
        self.device = device
        self.renderer = renderer
        
        buildKernels()
    }
    
    internal func buildKernels() {
    
        let device = renderer.device

        // Shader library
        library = device.makeDefaultLibrary()
        renderer.metalAssert(library != nil,
                             "The Shader Library could not be built.")
        
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
    
    //
    // Managing kernels
    //

    func makeFunction(name: String) -> MTLFunction? {
        return library.makeFunction(name: name)
    }
    
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

        var nr = Int(renderer.shaderOptions.bloom)
        if bloomFilterGallery.count <= nr || bloomFilterGallery[nr] == nil { nr = 0 }
        return bloomFilterGallery[nr]!
    }

    // Returns the compute kernel of the currently selected scanline filter
    func currentScanlineFilter() -> ComputeKernel {

        var nr = Int(renderer.shaderOptions.scanlines)
        if scanlineFilterGallery.count <= nr || scanlineFilterGallery[nr] == nil { nr = 0 }
        return scanlineFilterGallery[nr]!
    }
}
