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

class BarChart: ActivityMonitor {
    
    // Number of stored values (number of displayed bars)
    let capacity = 20

    // Dimensions in normalized rectangle (0,0) - (1,1)
    let leftBorder  = Float(0.1)
    let rightBorder = Float(0.1)
    let upperBorder = Float(0.275)
    let lowerBorder = Float(0.1)
    let thickness = Float(0.03)
    let barHeight = Float(0.625)
    var borderWidth: Float { return leftBorder + rightBorder }
    var innerWidth: Float { return 1.0 - borderWidth }
    var borderHeight: Float { return upperBorder + lowerBorder }
    var innerHeight: Float { return 1.0 - borderHeight }
    var barWidth: Float { innerWidth / Float(capacity + 1) }
            
    //  Number of scroll steps until a new bar shows up
    let microSteps = 20
    
    // Current scroll step
    var microStep = Int.random(in: 0 ... 19)
    
    var xOffset: Float { return -barWidth * (Float(microStep) / Float(microSteps)) }
    
    // Name of this monitor
    var name = "Lorem ipsum"
    
    // Color
    var color: NSColor! { didSet { updateTextures() } }
    
    // Scaling method on y axis
    var logScale = false
    
    //
    // Data
    //
    
    var values: [Float]
    var rectangles: [Node]
    var bgRectangle: Node?
    
    // Variables needed inside addValue()
    var sum = Float(0)
    var sumCnt = 0
    
    // Surface textures
    var foreground: MTLTexture?
    var background: MTLTexture?
    
    init(device: MTLDevice, name: String, color: NSColor, logScale: Bool = false) {
        
        self.name = name
        self.color = color
        self.logScale = logScale
        
        values = Array(repeating: 0.0, count: capacity)
        rectangles = []
        bgRectangle = Node.init(device: device, x: 0, y: 0, z: 0.001, w: 1.0, h: 1.0)
        
        super.init(device: device)
        
        updateRectangles()
        updateTextures()
        updateMatrix()
    }
    
    func updateForegroundTexture() {
        
        let r = Int(color.redComponent * 255)
        let g = Int(color.greenComponent * 255)
        let b = Int(color.blueComponent * 255)
        
        let size = MTLSizeMake(128, 128, 0)
        let data = UnsafeMutablePointer<UInt32>.allocate(capacity: size.width * size.height)
        
        data.drawGradient(size: size,
                          r1: r, g1: g, b1: b, a1: 255,
                          r2: 255, g2: 255, b2: 255, a2: 255)
        
        foreground = device.makeTexture(from: data, size: size)
        assert(foreground != nil)
    }
    
    var oldScale = Float(0.1)
    func experimental(amiga: AmigaProxy) {
    
        let bgWidth = 300
        let bgHeight = 240
        
        let size = MTLSizeMake(bgWidth, bgHeight, 0)
        let data = UnsafeMutablePointer<UInt32>.allocate(capacity: size.width * size.height)
        
        let s = NSSize(width: bgWidth, height: bgHeight)
        oldScale = amiga.paula.drawWaveformL(data, size: s, scale: oldScale)
        
        background = device.makeTexture(from: data, size: size)
        assert(background != nil)
    }
    
    func updateBackgroundTexture() {
        
        let r1 = Int(color.redComponent * 255) / 2
        let g1 = Int(color.greenComponent * 255) / 2
        let b1 = Int(color.blueComponent * 255) / 2
        let r2 = 128
        let g2 = 128
        let b2 = 128
        
        let bgWidth = 300
        let bgHeight = 240
        
        let size = MTLSizeMake(bgWidth, bgHeight, 0)
        let data = UnsafeMutablePointer<UInt32>.allocate(capacity: size.width * size.height)
        
        data.drawGradient(size: size,
                          r1: r1, g1: g1, b1: b1, a1: 255,
                          r2: r2, g2: g2, b2: b2, a2: 255)
        
        let gridy = lowerBorder * Float(bgHeight)
        let gridh = innerHeight * Float(bgHeight)
        data.drawGrid(size: size,
                      y1: Int(gridy), y2: Int(gridy + gridh),
                      lines: 5, logScale: logScale)
        
        data.makeRoundCorner(size: size, radius: 10)
        data.imprint(size: size, text: name)
        
        background = device.makeTexture(from: data, size: size)
        assert(background != nil)
    }
    
    func updateTextures() {
        
        updateForegroundTexture()
        updateBackgroundTexture()
    }
    
    func setColor(_ rgb: (Double, Double, Double) ) {
        
        color = NSColor.init(r: rgb.0, g: rgb.1, b: rgb.2)
    }
    
    func addValue(_ value: Float) {
        
        if sumCnt == 0 { sum = 0 }
        sum += logScale ? log(1.0 + 19.0 * value) / log(20) : value
        sumCnt += 1
    }
        
    func updateRectangles() {
        
        rectangles = []
        for n in 0 ..< values.count {
            
            let t = NSRect.init(x: 0,
                                y: 1.0 - Double(values[n]),
                                width: 1.0,
                                height: Double(values[n]))
            
            let x = leftBorder + (Float(n) + 1) * barWidth
            let y = lowerBorder
            let w = thickness
            let h = max(values[n], 0.025) * barHeight
            
            rectangles.append(Node.init(device: device,
                                        x: x, y: y, z: 0, w: w, h: h, t: t))
        }
    }
    
    override func draw(_ encoder: MTLRenderCommandEncoder, matrix: matrix_float4x4) {
        
        microStep += 1
        
        if microStep == microSteps {
            values.remove(at: 0)
            values.append(sumCnt == 0 ? sum : sum / Float(sumCnt))
            sumCnt = 0
            microStep = 0
            updateRectangles()
        }
        
        let shift = Renderer.translationMatrix(x: xOffset, y: 0.0, z: 0.0)
        
        // Draw background
        var uniforms = VertexUniforms(mvp: matrix * self.matrix)
        encoder.setVertexBytes(&uniforms,
                               length: MemoryLayout<VertexUniforms>.stride,
                               index: 1)
        encoder.setFragmentTexture(background, index: 0)
        bgRectangle!.drawPrimitives(encoder)
        
        // Draw bars
        uniforms = VertexUniforms(mvp: matrix * self.matrix * shift)
        encoder.setVertexBytes(&uniforms,
                               length: MemoryLayout<VertexUniforms>.stride,
                               index: 1)
        encoder.setFragmentTexture(foreground, index: 0)
        for rect in rectangles { rect.drawPrimitives(encoder) }
    }
}
