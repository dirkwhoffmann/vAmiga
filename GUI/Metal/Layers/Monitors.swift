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

    var monitors: [ActivityMonitor] = []
    var monitorGlobalAlpha = Float(0.5)
    
    // Individual enable switch for each activity monitor
    var monitorEnabled: [Bool] = []
    
    // Layout scheme used for positioning the monitors
    var monitorLayout = 0 { didSet { updateMonitorPositions() } }
        
    // Indicates if at least one monitor is animating
    var animates = false
    
    override init(renderer: Renderer) {
        
        super.init(renderer: renderer)
        
        buildMonitors()
    }
    
    //
    // Managing activity monitors
    //

    /*
    func fadeIn(monitor nr: Int, steps: Int = 40) {

        assert(nr < monitors.count)
        
        monitorAlpha[nr].target = 1.0
        monitorAlpha[nr].steps = steps
        animates = true
    }
        
    func fadeOut(monitor nr: Int, steps: Int = 40) {

        assert(nr < monitors.count)
        
        monitorAlpha[nr].target = 0.0
        monitorAlpha[nr].steps = steps
        animates = true
    }

    func fadeOutMonitors() {

        for i in 0 ..< monitors.count { fadeOut(monitor: i) }
    }

    func updateMonitorAlphas() {
        
        for i in 0 ..< monitors.count where i < monitorEnabled.count {
            if monitorEnabled[i] {
                fadeIn(monitor: i)
            } else {
                fadeOut(monitor: i)
            }
        }
    }
    */
    
    override func update(frames: Int64) {
    
        super.update(frames: frames)

        /*
        if isAnimating {
            track("alpha = \(alpha.current)")
        }
        
        if animates {
            
            animates = false
            for i in 0 ..< monitors.count {
                
                monitorAlpha[i].move()
                if monitorAlpha[i].animates() { animates = true }
            }
        }
        */
    }
            
    func render(_ encoder: MTLRenderCommandEncoder) {
        
        for i in 0 ..< monitors.count where monitorEnabled[i] {
            
            if !amiga.paused { monitors[i].animate() }

            fragUniforms.alpha = monitorGlobalAlpha * alpha.current
            encoder.setFragmentBytes(&fragUniforms,
                                     length: MemoryLayout<FragmentUniforms>.stride,
                                     index: 1)
            monitors[i].draw(encoder, matrix: vertexUniforms3D.mvp)
        }
    }
}
