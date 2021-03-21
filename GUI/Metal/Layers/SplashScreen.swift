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

        track()
        alpha = 0xFF
        targetAlpha = 0xFF
        
        let img = NSImage.init(named: "background")!
        bgTexture = img.toTexture(device: device, vflip: false)
        
        renderer.metalAssert(bgTexture != nil,
                             "The background texture could not be allocated.")
        
        bgRect = Node.init(device: device,
                           x: -1.0, y: -1.0, z: 0.99, w: 2.0, h: 2.0,
                           t: NSRect.init(x: 0.0, y: 0.0, width: 1.0, height: 1.0))
    }
    
    override func render() {
        
        // Configure vertex shader
        commandEncoder.setVertexBytes(&vertUniforms,
                                      length: MemoryLayout<VertexUniforms>.stride,
                                      index: 1)

        // Configure fragment shader
        commandEncoder.setFragmentTexture(bgTexture, index: 0)
        commandEncoder.setFragmentTexture(bgTexture, index: 1)
        commandEncoder.setFragmentBytes(&fragUniforms,
                                        length: MemoryLayout<FragmentUniforms>.stride,
                                        index: 1)

        // Render
        bgRect!.drawPrimitives(commandEncoder)
        
    }
}
