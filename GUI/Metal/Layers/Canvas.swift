// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Canvas: Layer {
        
    //
    // Buffers and uniforms
    //
    
    var quad2D: Node?
    var quad3D: Quad?
            
    var vertexUniforms2D = VertexUniforms(mvp: matrix_identity_float4x4)
    var vertexUniforms3D = VertexUniforms(mvp: matrix_identity_float4x4)
    var fragmentUniforms = FragmentUniforms(alpha: 1.0,
                                            dotMaskWidth: 0,
                                            dotMaskHeight: 0,
                                            scanlineDistance: 0)
    /*
    var mergeUniforms = MergeUniforms(longFrameScale: 1.0,
                                      shortFrameScale: 1.0)
     */
    
    //
    // Textures
    //
    
    /* Bloom textures to emulate blooming (512 x 512)
     * To emulate a bloom effect, the C64 texture is first split into it's
     * R, G, and B parts. Each texture is then run through a Gaussian blur
     * filter with a large radius. These blurred textures are passed into
     * the fragment shader as secondary textures where they are recomposed
     * with the upscaled primary texture.
     */
    // var bloomTextureR: MTLTexture! = nil
    // var bloomTextureG: MTLTexture! = nil
    // var bloomTextureB: MTLTexture! = nil
    var bloomTextureR: MTLTexture { return renderer.bloomTextureR }
    var bloomTextureG: MTLTexture { return renderer.bloomTextureG }
    var bloomTextureB: MTLTexture { return renderer.bloomTextureB }

    /* Upscaled texture with scanlines (2048 x 2048)
     * In the second texture processing stage, a scanline effect is applied to
     * the upscaled texture.
     */
    // var scanlineTexture: MTLTexture! = nil
    var scanlineTexture: MTLTexture { return renderer.scanlineTexture }
    
    //
    // Functions
    //
    
    override init(view: MTKView, device: MTLDevice, renderer: Renderer) {

        super.init(view: view, device: device, renderer: renderer)

        track()
    }
    
    func buildVertexBuffer() {

        quad2D = Node.init(device: device,
                           x: -1.0, y: -1.0, z: 0.0, w: 2.0, h: 2.0,
                           t: renderer.textureRect)

        quad3D = Quad.init(device: device,
                           x1: -0.64, y1: -0.48, z1: -0.64,
                           x2: 0.64, y2: 0.48, z2: 0.64,
                           t: renderer.textureRect)
    }
        
    func render2D() {
    
        // Configure the vertex shader
        commandEncoder.setVertexBytes(&vertexUniforms2D,
                                      length: MemoryLayout<VertexUniforms>.stride,
                                      index: 1)

        // Configure the fragment shader
        commandEncoder.setFragmentTexture(scanlineTexture, index: 0)
        commandEncoder.setFragmentBytes(&fragmentUniforms,
                                        length: MemoryLayout<FragmentUniforms>.stride,
                                        index: 1)

        // Render
        quad2D!.drawPrimitives(commandEncoder)
    }
    
    func render3D() {
        
        let animates = renderer.animates
        
        // Configure the vertex shader
        commandEncoder.setVertexBytes(&vertexUniforms3D,
                                      length: MemoryLayout<VertexUniforms>.stride,
                                      index: 1)
        
        // Configure the fragment shader
        fragmentUniforms.alpha = amiga.paused ? Float(0.5) : Float(alpha) / 0xFF
        commandEncoder.setFragmentTexture(scanlineTexture, index: 0)
        commandEncoder.setFragmentTexture(bloomTextureR, index: 1)
        commandEncoder.setFragmentTexture(bloomTextureG, index: 2)
        commandEncoder.setFragmentTexture(bloomTextureB, index: 3)
        commandEncoder.setFragmentBytes(&fragmentUniforms,
                                        length: MemoryLayout<FragmentUniforms>.stride,
                                        index: 1)
        
        // Render (part of) the cube
        quad3D!.draw(commandEncoder, allSides: animates != 0)
    }
}
