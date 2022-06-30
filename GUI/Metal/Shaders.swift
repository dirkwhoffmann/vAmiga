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

//
// Additional uniforms needed by the vertex shader
//

struct VertexUniforms {
    
    var mvp: simd_float4x4
}

//
// Uniforms passed to all compute shaders and the fragment shader
//
    
struct ShaderOptions: Codable {

    var blur: Int32
    var blurRadius: Float
    
    var bloom: Int32
    var bloomRadius: Float
    var bloomBrightness: Float
    var bloomWeight: Float

    var flicker: Int32
    var flickerWeight: Float

    var dotMask: Int32
    var dotMaskBrightness: Float
    
    var scanlines: Int32
    var scanlineBrightness: Float
    var scanlineWeight: Float
    
    var disalignment: Int32
    var disalignmentH: Float
    var disalignmentV: Float
}

//
// Uniforms passed to the merge shader
//

struct MergeUniforms {

    var longFrameScale: Float
    var shortFrameScale: Float
    var xScale: Float
}

//
// Additional uniforms needed by the fragment shader
//

struct FragmentUniforms {
    
    var alpha: Float
    var white: Float
    var dotMaskWidth: Int32
    var dotMaskHeight: Int32
    var scanlineDistance: Int32
}

//
// Base class for all compute kernels
//

class ComputeKernel: NSObject {

    var device: MTLDevice!
    var kernel: MTLComputePipelineState!

    // The rectangle the compute kernel is applied to
    var cutout = (256, 256)
    
    convenience init?(name: String, device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        
        self.init()
        
        self.device = device
        self.cutout = cutout
        
        // Lookup kernel function in library
        guard let function = library.makeFunction(name: name) else {
            warn("Cannot find kernel function '\(name)' in library.")
            return nil
        }
        
        // Create kernel
        do {
            try kernel = device.makeComputePipelineState(function: function)
        } catch {
            warn("Cannot create compute kernel '\(name)'.")
            let alert = NSAlert()
            alert.alertStyle = .informational
            alert.icon = NSImage(named: "metal")
            alert.messageText = "Failed to create compute kernel."
            alert.informativeText = "Kernel '\(name)' will be ignored when selected."
            alert.addButton(withTitle: "OK")
            alert.runModal()
            return nil
        }
    }
    
    func apply(commandBuffer: MTLCommandBuffer,
               source: MTLTexture, target: MTLTexture,
               options: UnsafeRawPointer? = nil, length: Int = 0) {
        
        if let encoder = commandBuffer.makeComputeCommandEncoder() {
            
            encoder.setTexture(source, index: 0)
            encoder.setTexture(target, index: 1)

            apply(encoder: encoder, options: options, length: length)
        }
    }
    
    func apply(commandBuffer: MTLCommandBuffer, textures: [MTLTexture],
               options: UnsafeRawPointer? = nil, length: Int = 0) {
        
        if let encoder = commandBuffer.makeComputeCommandEncoder() {
            
            for (index, texture) in textures.enumerated() {
                encoder.setTexture(texture, index: index)
            }
            apply(encoder: encoder, options: options, length: length)
        }
    }

    private func apply(encoder: MTLComputeCommandEncoder,
                       options: UnsafeRawPointer?, length: Int) {
        
        // Bind pipeline
        encoder.setComputePipelineState(kernel)
        
        // Pass in shader options
        if let opt = options { encoder.setBytes(opt, length: length, index: 0) }
        
        // Determine thread group size and number of groups
        let groupW = kernel.threadExecutionWidth
        let groupH = kernel.maxTotalThreadsPerThreadgroup / groupW
        let threadsPerGroup = MTLSizeMake(groupW, groupH, 1)
        
        let countW = (cutout.0) / groupW + 1
        let countH = (cutout.1) / groupH + 1
        let threadgroupCount = MTLSizeMake(countW, countH, 1)

        // Finally, we're ready to dispatch
        encoder.dispatchThreadgroups(threadgroupCount,
                                     threadsPerThreadgroup: threadsPerGroup)
        encoder.endEncoding()
    }
}

//
// Bypass filter
//

class BypassFilter: ComputeKernel {
    
    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "bypass",
                  device: device, library: library, cutout: cutout)
    }
}

//
// Mergers
//

class MergeFilter: ComputeKernel {
    
    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "merge",
                  device: device, library: library, cutout: cutout)
    }
}

class MergeBypassFilter: ComputeKernel {
    
    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "bypassmerger",
                  device: device, library: library, cutout: cutout)
    }
}

//
// Upscalers
//

class BypassUpscaler: ComputeKernel {
    
    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "bypassupscaler",
                  device: device, library: library, cutout: cutout)
    }
}

class InPlaceEpxScaler: ComputeKernel {
    
    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "inPlaceEpx",
                  device: device, library: library, cutout: cutout)
    }
}

class InPlaceXbrScaler: ComputeKernel {
    
    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "inPlaceXbr",
                  device: device, library: library, cutout: cutout)
    }
}
class EPXUpscaler: ComputeKernel {
    
    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "epxupscaler",
                  device: device, library: library, cutout: cutout)
    }
}

class XBRUpscaler: ComputeKernel {
    
    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "xbrupscaler",
                  device: device, library: library, cutout: cutout)
    }
}

//
// Split filter
//

class SplitFilter: ComputeKernel {

    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "split",
                  device: device, library: library, cutout: cutout)
    }
}

//
// Scanline filter
//

class SimpleScanlines: ComputeKernel {
    
    convenience init?(device: MTLDevice, library: MTLLibrary, cutout: (Int, Int)) {
        self.init(name: "scanlines",
                  device: device, library: library, cutout: cutout)
    }
}
