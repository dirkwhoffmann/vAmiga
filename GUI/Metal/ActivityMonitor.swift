// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// Base class for all activity monitors

enum Side {
    case lower
    case upper
    case left
    case right
}

class ActivityMonitor {
    
    // Reference to the owning MTLDevice
    let device: MTLDevice
    
    // Canvas dimensions on the xy plane
    var position = NSRect.init() { didSet { updateMatrix() } }

    // Rotation angle of the canvas
    var angle = Float(0.0) { didSet { updateMatrix() } }
    
    // Side of the canvas where the rotations is carried out
    var rotationSide = Side.lower { didSet { updateMatrix() } }
    
    // Transformation matrix computed out of the above parameters
    var matrix = matrix_identity_float4x4

    init (device: MTLDevice) {

        self.device = device
    }
    
    func updateMatrix() {
        
        let posx = Float(position.origin.x)
        let posy = Float(position.origin.y)
        let posw = Float(position.size.width)
        let posh = Float(position.size.height)
        
        let trans = Renderer.translationMatrix(x: posx, y: posy, z: -0.8)
        let scale = Renderer.scalingMatrix(xs: posw, ys: posh, zs: 1.0)
        
        let r = angle * .pi/180.0
        var t1: matrix_float4x4
        var t2: matrix_float4x4
        var rot: matrix_float4x4
        
        switch rotationSide {
            
        case .lower:
            t1 = matrix_identity_float4x4
            t2 = matrix_identity_float4x4
            rot = Renderer.rotationMatrix(radians: r, x: 1, y: 0, z: 0)
            
        case .upper:
            t1 = Renderer.translationMatrix(x: 0, y: -posh, z: 0)
            t2 = Renderer.translationMatrix(x: 0, y: posh, z: 0)
            rot = Renderer.rotationMatrix(radians: -r, x: 1, y: 0, z: 0)
            
        case .left:
            t1 = matrix_identity_float4x4
            t2 = matrix_identity_float4x4
            rot = Renderer.rotationMatrix(radians: -r, x: 0, y: 1, z: 0)
            
        case .right:
            t1 = Renderer.translationMatrix(x: -posw, y: 0, z: 0)
            t2 = Renderer.translationMatrix(x: posw, y: 0, z: 0)
            rot = Renderer.rotationMatrix(radians: r, x: 0, y: 1, z: 0)
        }
        
        matrix = trans * t2 * rot * t1 * scale
    }
    
    func draw(_ encoder: MTLRenderCommandEncoder, matrix: matrix_float4x4) {
    }
}
