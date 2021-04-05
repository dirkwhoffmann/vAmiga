// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class Monitors: Layer {

    var vertexUniforms2D = VertexUniforms(mvp: matrix_identity_float4x4)
    var vertexUniforms3D = VertexUniforms(mvp: matrix_identity_float4x4)
    var fragUniforms = FragmentUniforms(alpha: 1.0,
                                        white: 0.0,
                                        dotMaskWidth: 0,
                                        dotMaskHeight: 0,
                                        scanlineDistance: 0)
    
    struct Monitor {
        
        static let copper = 0
        static let blitter = 1
        static let disk = 2
        static let audio = 3
        static let sprite = 4
        static let bitplane = 5
        
        static let chipRam = 6
        static let slowRam = 7
        static let fastRam = 8
        static let kickRom = 9

        static let waveformL = 10
        static let waveformR = 11
    }

    // Array holding all activity monitors
    var monitors: [ActivityMonitor] = []
    
    // Indicates if a monitor should be displayes
    var enabled: [Bool] = []

    // Maximum alpha value
    var maxAlpha = Float(0.5)
    
    // Layout scheme used for positioning the monitors
    var layout = 0 { didSet { updateMonitorPositions() } }

    override init(renderer: Renderer) {
        
        super.init(renderer: renderer)
        
        buildMonitors()
    }
    
    func render(_ encoder: MTLRenderCommandEncoder) {
        
        for i in 0 ..< monitors.count where enabled[i] {
            
            if !amiga.paused { monitors[i].animate() }

            fragUniforms.alpha = maxAlpha * alpha.current
            encoder.setFragmentBytes(&fragUniforms,
                                     length: MemoryLayout<FragmentUniforms>.stride,
                                     index: 1)
            monitors[i].draw(encoder, matrix: vertexUniforms3D.mvp)
        }
    }
}
