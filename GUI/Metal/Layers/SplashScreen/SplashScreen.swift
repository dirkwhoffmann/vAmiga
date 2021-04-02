// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class SplashScreen: Layer {
        
    var bgTexture: MTLTexture! = nil
    var bgRect: Node?
        
    var vertUniforms = VertexUniforms(mvp: matrix_identity_float4x4)
    var fragUniforms = FragmentUniforms(alpha: 1.0,
                                        dotMaskWidth: 0,
                                        dotMaskHeight: 0,
                                        scanlineDistance: 0)
    
    override init(view: MTKView, device: MTLDevice, renderer: Renderer) {

        super.init(view: view, device: device, renderer: renderer)

        alpha.set(1.0)        
        let img = NSImage.init(named: "background")!
        bgTexture = img.toTexture(device: device, vflip: false)
        
        renderer.metalAssert(bgTexture != nil,
                             "The background texture could not be allocated.")
    }
    
    override func render(buffer: MTLCommandBuffer) {
        
    }
    
    override func render(encoder: MTLRenderCommandEncoder, flat: Bool) {
        
        // Configure vertex shader
        encoder.setVertexBytes(&vertUniforms,
                               length: MemoryLayout<VertexUniforms>.stride,
                               index: 1)
        
        // Configure fragment shader
        encoder.setFragmentTexture(bgTexture, index: 0)
        encoder.setFragmentTexture(bgTexture, index: 1)
        encoder.setFragmentBytes(&fragUniforms,
                                 length: MemoryLayout<FragmentUniforms>.stride,
                                 index: 1)
        
        // Render
        bgRect!.drawPrimitives(encoder)
    }
}
