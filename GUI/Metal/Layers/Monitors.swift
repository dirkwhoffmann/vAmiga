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
    
    // Maximum alpha value
    var maxAlpha = Float(0.5)
    
    // Layout scheme used for positioning the monitors
    var layout = 0 { didSet { updateMonitorPositions() } }

    //
    // Initializing
    //
    
    override init(renderer: Renderer) {
        
        super.init(renderer: renderer)

        // Activity monitors are colorized with the bus debuggger colors
        let info = amiga.dmaDebugger.info
        
        // DMA monitors
        monitors.append(BarChart(device: device, name: "Copper DMA", logScale: true))
        monitors.append(BarChart(device: device, name: "Blitter DMA", logScale: true))
        monitors.append(BarChart(device: device, name: "Disk DMA"))
        monitors.append(BarChart(device: device, name: "Audio DMA"))
        monitors.append(BarChart(device: device, name: "Sprite DMA"))
        monitors.append(BarChart(device: device, name: "Bitplane DMA"))
        
        monitors[0].setColor(rgb: info.copperColor)
        monitors[1].setColor(rgb: info.blitterColor)
        monitors[2].setColor(rgb: info.diskColor)
        monitors[3].setColor(rgb: info.audioColor)
        monitors[4].setColor(rgb: info.spriteColor)
        monitors[5].setColor(rgb: info.bitplaneColor)

        // Memory monitors
        monitors.append(BarChart(device: device, name: "CPU (Chip Ram)", splitView: true))
        monitors.append(BarChart(device: device, name: "CPU (Slow Ram)", splitView: true))
        monitors.append(BarChart(device: device, name: "CPU (Fast Ram)", splitView: true))
        monitors.append(BarChart(device: device, name: "CPU (Rom)", splitView: true))

        // Waveform monitors
        monitors.append(WaveformMonitor(device: device, audioPort: amiga.audioPort, leftChannel: true))
        monitors.append(WaveformMonitor(device: device, audioPort: amiga.audioPort, leftChannel: false))

        updateMonitorPositions()
    }

    func updateMonitorPositions() {

        let ratio = Double(renderer.size.width) / Double(renderer.size.height)
        updateMonitorPositions(aspectRatio: ratio)
    }
    
    func updateMonitorPositions(aspectRatio: Double) {
        
        //    w  d  w  d  w  d  w  d  w  d  w
        //   ___   ___   ___   ___   ___   ___
        //  |   |-|   |-|   |-|   |-|   |-|   | h
        //   ---   ---   ---   ---   ---   ---
        
        // Grid position and rotation side for all monitors
        var grid: [(Int, Int, Side)]
        
        switch layout {
            
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
            monitors[i].position = NSRect(x: x, y: y, width: w, height: h)
            monitors[i].rotationSide = grid[i].2
        }
    }
    
    //
    // Rendering
    //
    
    func render(_ encoder: MTLRenderCommandEncoder) {

        for i in 0 ..< monitors.count {
            
            if !amiga.paused { monitors[i].animate() }

            fragUniforms.alpha = maxAlpha * alpha.current
            encoder.setFragmentBytes(&fragUniforms,
                                     length: MemoryLayout<FragmentUniforms>.stride,
                                     index: 1)
            monitors[i].draw(encoder, matrix: vertexUniforms3D.mvp)
        }
    }
}
