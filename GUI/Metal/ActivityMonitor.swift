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
    var name = "Lorem ipsum" { didSet { updateTextures() } }
    
    // Colors
    var upperColor = NSColor.blue { didSet { updateTextures() } }
    var lowerColor = NSColor.red { didSet { updateTextures() } }

    // Scaling method on y axis
    var logScale = false { didSet { updateTextures() } }
    
    // If set to false, only the upper values are drawn
    var splitView = false
    
    //
    // Data
    //
    
    var upperValues: [Float] = []
    var lowerValues: [Float] = []
    var upperBars: [Node] = []
    var lowerBars: [Node] = []
    var bgRectangle: Node?
    
    // Variables needed inside addValue()
    var upperSum = Float(0)
    var lowerSum = Float(0)
    var upperSumCnt = 0
    var lowerSumCnt = 0

    // Surface textures
    var upperTexture: MTLTexture?
    var lowerTexture: MTLTexture?
    var background: MTLTexture?
    
    init(device: MTLDevice, name: String, logScale: Bool = false, splitView: Bool = false) {
        
        self.name = name
        self.logScale = logScale
        self.splitView = splitView
            
        upperValues = Array(repeating: 0.0, count: capacity)
        lowerValues = Array(repeating: 0.0, count: capacity)
        bgRectangle = Node.init(device: device, x: 0, y: 0, z: 0.001, w: 1.0, h: 1.0)
        
        super.init(device: device)
        
        updateBars()
        updateTextures()
        updateMatrix()
    }
    
    func updateUpperTexture() {
        
        let r = Int(upperColor.redComponent * 255)
        let g = Int(upperColor.greenComponent * 255)
        let b = Int(upperColor.blueComponent * 255)
        
        let size = MTLSizeMake(128, 128, 0)
        let data = UnsafeMutablePointer<UInt32>.allocate(capacity: size.width * size.height)
        
        data.drawGradient(size: size,
                          r1: r, g1: g, b1: b, a1: 255,
                          r2: 255, g2: 255, b2: 255, a2: 255)
        
        upperTexture = device.makeTexture(from: data, size: size)
        assert(upperTexture != nil)
    }
    
    // TODO: Merge with updateUpperTexture
    func updateLowerTexture() {
        
        let r = Int(lowerColor.redComponent * 255)
        let g = Int(lowerColor.greenComponent * 255)
        let b = Int(lowerColor.blueComponent * 255)
        
        let size = MTLSizeMake(128, 128, 0)
        let data = UnsafeMutablePointer<UInt32>.allocate(capacity: size.width * size.height)
        
        data.drawGradient(size: size,
                          r1: r, g1: g, b1: b, a1: 255,
                          r2: 255, g2: 255, b2: 255, a2: 255)
        
        lowerTexture = device.makeTexture(from: data, size: size)
        assert(upperTexture != nil)
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
    
    func makeBgBuffer(size: MTLSize) -> UnsafeMutablePointer<UInt32> {
        
        let (r1, g1, b1) = upperColor.integerComponents()
        let (r2, g2, b2) = (255, 255, 255)
        
        let capacity = size.width * size.height
        let data = UnsafeMutablePointer<UInt32>.allocate(capacity: capacity)
        
        data.drawGradient(size: size,
                          r1: r1 / 2, g1: g1 / 2, b1: b1 / 2, a1: 255,
                          r2: r2 / 2, g2: g2 / 2, b2: b2 / 2, a2: 255)
        
        let gridh = innerHeight * Float(size.height)
        let gridy1 = lowerBorder * Float(size.height)
        let gridy2 = gridy1 + gridh
        data.drawGrid(size: size,
                      y1: Int(gridy1), y2: Int(gridy2),
                      lines: 5, logScale: logScale)
        
        return data
    }
    
    func makeSplittedBgBuffer(size: MTLSize) -> UnsafeMutablePointer<UInt32> {
        
        let (r1, g1, b1) = upperColor.integerComponents()
        let (r2, g2, b2) = (255, 255, 255)
        let (r3, g3, b3) = lowerColor.integerComponents()
        
        let capacity = size.width * size.height
        let data1 = UnsafeMutablePointer<UInt32>.allocate(capacity: capacity)
        
        let data2 = data1 + capacity / 2
        let size2 = MTLSizeMake(size.width, size.height / 2, 0)
        
        data1.drawGradient(size: size2,
                           r1: r1 / 2, g1: g1 / 2, b1: b1 / 2, a1: 255,
                           r2: r2 / 2, g2: g2 / 2, b2: b2 / 2, a2: 255)
        data2.drawGradient(size: size2,
                           r1: r2 / 2, g1: g2 / 2, b1: b2 / 2, a1: 255,
                           r2: r3 / 2, g2: g3 / 2, b2: b3 / 2, a2: 255)
        
        let gridh = 0.5 * innerHeight * Float(size.height)
        let gridy1 = lowerBorder * Float(size.height)
        let gridy2 = gridy1 + gridh
        let gridy3 = gridy2 + gridh
        data1.drawGrid(size: size,
                       y1: Int(gridy2), y2: Int(gridy1),
                       lines: 5, logScale: logScale)
        data1.drawGrid(size: size,
                       y1: Int(gridy2), y2: Int(gridy3),
                       lines: 5, logScale: logScale)
        
        return data1
    }
    
    func updateBackgroundTexture() {
        
        let size = MTLSizeMake(300, 240, 0)
        let data = splitView ?
            makeSplittedBgBuffer(size: size) :
            makeBgBuffer(size: size)
        
        data.makeRoundCorner(size: size, radius: 10)
        data.imprint(size: size, text: name)
        
        background = device.makeTexture(from: data, size: size)
        assert(background != nil)
    }
    
    func updateTextures() {
        
        updateUpperTexture()
        updateLowerTexture()
        updateBackgroundTexture()
    }
    
    func setUpperColor(_ rgb: (Double, Double, Double) ) {
        
        upperColor = NSColor.init(r: rgb.0, g: rgb.1, b: rgb.2)
    }

    func setlowerColor(_ rgb: (Double, Double, Double) ) {
        
        lowerColor = NSColor.init(r: rgb.0, g: rgb.1, b: rgb.2)
    }

    func addUpperValue(_ value: Float) {

        if upperSumCnt == 0 { upperSum = 0 }
        upperSum += logScale ? log(1.0 + 19.0 * value) / log(20) : value
        upperSumCnt += 1
    }

    func addLowerValue(_ value: Float) {

        if lowerSumCnt == 0 { lowerSum = 0 }
        lowerSum += logScale ? log(1.0 + 19.0 * value) / log(20) : value
        lowerSumCnt += 1
    }
    
    func addValue(_ value: Float) {
        addUpperValue(value)
    }
    
    func addValues(_ value1: Float, _ value2: Float) {
        addUpperValue(value1)
        addLowerValue(value2)
    }

    func updateBars() {
        
        updateUpperBars()
        if splitView { updateLowerBars() }
    }
    
    func updateUpperBars() {
        
        upperBars = []
        let y = lowerBorder + (splitView ? innerHeight / 2 : 0)
        let w = thickness
        
        for n in 0 ..< upperValues.count {
            
            let v = Double(upperValues[n])
            let t = NSRect.init(x: 0, y: 1.0 - v, width: 1.0, height: v)
            let x = leftBorder + (Float(n) + 1) * barWidth
            let h = Float(max(v, 0.025) * (splitView ? 0.5 : 1)) * barHeight
            
            upperBars.append(Node.init(device: device,
                                       x: x, y: y, z: 0, w: w, h: h, t: t))
        }
    }
    
    func updateLowerBars() {
        
        lowerBars = []
        let y = lowerBorder + innerHeight / 2
        let w = thickness
        
        for n in 0 ..< lowerValues.count {
            
            let v = Double(lowerValues[n])
            let t = NSRect.init(x: 0, y: 1.0 - v, width: 1.0, height: v)
            let x = leftBorder + (Float(n) + 1) * barWidth
            let h = Float(max(v, 0.025) * -0.5) * barHeight
            
            lowerBars.append(Node.init(device: device,
                                       x: x, y: y, z: 0, w: w, h: h, t: t))
        }
    }
    
    override func draw(_ encoder: MTLRenderCommandEncoder, matrix: matrix_float4x4) {
        
        microStep += 1
        
        if microStep == microSteps {
            upperValues.remove(at: 0)
            lowerValues.remove(at: 0)
            upperValues.append(upperSumCnt == 0 ? upperSum : upperSum / Float(upperSumCnt))
            lowerValues.append(lowerSumCnt == 0 ? lowerSum : lowerSum / Float(lowerSumCnt))
            upperSumCnt = 0
            lowerSumCnt = 0
            microStep = 0
            updateBars()
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
        encoder.setFragmentTexture(upperTexture, index: 0)
        for rect in upperBars { rect.drawPrimitives(encoder) }

        encoder.setFragmentTexture(lowerTexture, index: 0)
        for rect in lowerBars { rect.drawPrimitives(encoder) }
    }
}
