// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

struct Vertex {
    
    var x, y, z, tx, ty: Float // Position and texture data
    
    init(x: Float, y: Float, z: Float, t: NSPoint) {
        self.x = x
        self.y = y
        self.z = z
        tx = Float(t.x)
        ty = Float(t.y)
    }
    
    func floatBuffer() -> [Float] {

        return [x, y, z, 1.0, tx, ty, 0.0, 0.0]
    }
}

class Node {
    
    let device: MTLDevice
    var vertexCount: Int
    var vertexBuffer: MTLBuffer
    
    init(device: MTLDevice, vertices: [Vertex]) {

        var vertexData = [Float]()
        for vertex in vertices {
            vertexData += vertex.floatBuffer()
        }
        
        let dataSize = vertexData.count * MemoryLayout.size(ofValue: vertexData[0])
        vertexBuffer = device.makeBuffer(bytes: vertexData,
                                         length: dataSize,
                                         options: [])!
        self.device = device
        vertexCount = vertices.count
    }
    
    convenience init(device: MTLDevice,
                     x: Float, y: Float, z: Float, w: Float, h: Float,
                     t: NSRect = NSRect.init(x: 0, y: 0, width: 1.0, height: 1.0)) {
        
        let upperLeft = NSPoint(x: t.minX, y: t.minY)
        let lowerLeft = NSPoint(x: t.minX, y: t.maxY)
        let upperRight = NSPoint(x: t.maxX, y: t.minY)
        let lowerRight = NSPoint(x: t.maxX, y: t.maxY)
        
        let v0 = Vertex(x: x, y: y, z: z, t: lowerLeft)
        let v1 = Vertex(x: x, y: y + h, z: z, t: upperLeft)
        let v2 = Vertex(x: x + w, y: y + h, z: z, t: upperRight)
        
        let v3 = Vertex(x: x, y: y, z: z, t: lowerLeft)
        let v4 = Vertex(x: x + w, y: y + h, z: z, t: upperRight)
        let v5 = Vertex(x: x + w, y: y, z: z, t: lowerRight)
        
        self.init(device: device, vertices: [v0, v1, v2, v3, v4, v5])
    }
 
    convenience init(device: MTLDevice,
                     x: Float, y: Float, z: Float, w: Float, d: Float, t: NSRect) {
        
        let upperLeft = NSPoint(x: t.minX, y: t.minY)
        let lowerLeft = NSPoint(x: t.minX, y: t.maxY)
        let upperRight = NSPoint(x: t.maxX, y: t.minY)
        let lowerRight = NSPoint(x: t.maxX, y: t.maxY)
        
        let v0 = Vertex(x: x, y: y, z: z, t: lowerLeft)
        let v1 = Vertex(x: x, y: y, z: z + d, t: upperLeft)
        let v2 = Vertex(x: x + w, y: y, z: z + d, t: upperRight)
        
        let v3 = Vertex(x: x, y: y, z: z, t: lowerLeft)
        let v4 = Vertex(x: x + w, y: y, z: z + d, t: upperRight)
        let v5 = Vertex(x: x + w, y: y, z: z, t: lowerRight)
        
        self.init(device: device, vertices: [v0, v1, v2, v3, v4, v5])
    }
 
    convenience init(device: MTLDevice,
                     x: Float, y: Float, z: Float, h: Float, d: Float, t: NSRect) {
        
        let upperLeft = NSPoint(x: t.minX, y: t.minY)
        let lowerLeft = NSPoint(x: t.minX, y: t.maxY)
        let upperRight = NSPoint(x: t.maxX, y: t.minY)
        let lowerRight = NSPoint(x: t.maxX, y: t.maxY)
        
        let v0 = Vertex(x: x, y: y, z: z, t: lowerLeft)
        let v1 = Vertex(x: x, y: y + h, z: z, t: upperLeft)
        let v2 = Vertex(x: x, y: y + h, z: z + d, t: upperRight)
        
        let v3 = Vertex(x: x, y: y, z: z, t: lowerLeft)
        let v4 = Vertex(x: x, y: y + h, z: z + d, t: upperRight)
        let v5 = Vertex(x: x, y: y, z: z + d, t: lowerRight)
        
        self.init(device: device, vertices: [v0, v1, v2, v3, v4, v5])
    }
    
    func drawPrimitives(_ commandEncoder: MTLRenderCommandEncoder, count: Int) {
        
        commandEncoder.setVertexBuffer(vertexBuffer, offset: 0, index: 0)
        commandEncoder.drawPrimitives(type: .triangle,
                                      vertexStart: 0,
                                      vertexCount: count,
                                      instanceCount: 1)
    }

    func drawPrimitives(_ commandEncoder: MTLRenderCommandEncoder) {
        
        drawPrimitives(commandEncoder, count: vertexCount)
    }
}

class Quad {
    
    var front: Node
    var back: Node
    var left: Node
    var right: Node
    var top: Node
    var bottom: Node
    
    init(device: MTLDevice,
         x1: Float, y1: Float, z1: Float,
         x2: Float, y2: Float, z2: Float, t: NSRect) {
        
        front  = Node.init(device: device,
                           x: x1, y: y1, z: z1, w: (x2 - x1), h: (y2 - y1), t: t)
        back   = Node.init(device: device,
                           x: x2, y: y1, z: z2, w: (x1 - x2), h: (y2 - y1), t: t)
        left   = Node.init(device: device,
                           x: x1, y: y1, z: z2, h: (y2 - y1), d: (z1 - z2), t: t)
        right  = Node.init(device: device,
                           x: x2, y: y1, z: z1, h: (y2 - y1), d: (z2 - z1), t: t)
        top    = Node.init(device: device,
                           x: x1, y: y2, z: z1, w: (x2 - x1), d: (y2 - y1), t: t)
        bottom = Node.init(device: device,
                           x: x1, y: y1, z: z2, w: (x2 - x1), d: (y1 - y2), t: t)
    }

    func draw(_ commandEncoder: MTLRenderCommandEncoder, allSides: Bool) {
        
        front.drawPrimitives(commandEncoder)
        if allSides {
            back.drawPrimitives(commandEncoder)
            left.drawPrimitives(commandEncoder)
            right.drawPrimitives(commandEncoder)
            top.drawPrimitives(commandEncoder)
            bottom.drawPrimitives(commandEncoder)
        }
    }
}

class BarChart {
    
    let device: MTLDevice

    //
    // Dimensions in normalized rectangle (0,0) - (1,1)
    //
    
    let leftBorder  = Float(0.1)
    let rightBorder = Float(0.1)
    let upperBorder = Float(0.275)
    let lowerBorder = Float(0.1)

    var borderWidth: Float { return leftBorder + rightBorder }
    var innerWidth: Float { return 1.0 - borderWidth }
    var borderHeight: Float { return upperBorder + lowerBorder }
    var innerHeight: Float { return 1.0 - borderHeight }

    let bars        = 20
    let thickness   = Float(0.03)

    let barHeight   = Float(0.625)
    var barWidth: Float { innerWidth / Float(bars + 1) }
    
    //
    // Transformations
    //
        
    var posm = matrix_identity_float4x4

    var position: NSRect = NSRect.init() { didSet { updateMatrices() } }
    var angle: Float = 0.0 { didSet { if angle != oldValue { updateMatrices() } } }

    //
    // Appearance
    //
    
    // Number of displayed values (bars)
    let capacity = 20
    
    // Number of scroll steps until a new bar shows up
    let steps = 20
    
    // Name of this view
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
    var offset = Int.random(in: 0 ... 19)
    
    // Variables needed inside addValue()
    var sum = Float(0)
    var sumCnt = 0

    // Surface textures
    var foreground: MTLTexture?
    var background: MTLTexture?

    init(device: MTLDevice, name: String,
         position: NSRect,
         color: NSColor,
         logScale: Bool = false) {
        
        self.device = device
        self.name = name
        self.position = position
        self.color = color
        self.logScale = logScale
        
        self.rectangles = []
        values = Array(repeating: 0.0, count: capacity)

        updateRectangles()
        updateTextures()
        updateMatrices()
    }
     
    func updateForegroundTexture() {
        
        let r = Int(color.redComponent * 255)
        let g = Int(color.greenComponent * 255)
        let b = Int(color.blueComponent * 255)
        
        let size = MTLSizeMake(128, 128, 0)
        let data = UnsafeMutablePointer<UInt32>.allocate(capacity: size.width * size.height)
        
        data.drawGradient(size: size,
                          r1: r, g1: g, b1: b, a1: 128,
                          r2: 255, g2: 255, b2: 255, a2: 255)

        foreground = device.makeTexture(from: data, size: size)
        assert(foreground != nil)
    }
 
    func updateBackgroundTexture() {
        
        /*
        let r1 = 64
        let g1 = 64
        let b1 = 64
        let r2 = 128
        let g2 = 128
        let b2 = 128
        */
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
                          r1: r1, g1: g1, b1: b1, a1: 128,
                          r2: r2, g2: g2, b2: b2, a2: 128)

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
    
    func updateMatrices() {
    
        let trans = Renderer.translationMatrix(x: Float(position.origin.x),
                                               y: Float(position.origin.y),
                                               z: -0.7)
        let scale = Renderer.scalingMatrix(xs: Float(position.size.width),
                                           ys: Float(position.size.height),
                                           zs: 1.0)
        let t1 = Renderer.translationMatrix(x: -0.5 * Float(position.size.width),
                                            y: -0.5 * Float(position.size.height),
                                            z: 0)
        let t2 = Renderer.translationMatrix(x: 0.5 * Float(position.size.width),
                                            y: 0.5 * Float(position.size.height),
                                            z: 0)
        let rot = Renderer.rotationMatrix(radians: angle * .pi/180.0, x: 0, y: 1, z: 0)
     
        posm = trans * t2 * rot * t1 * scale
    }
    
    func setColor(_ rgb: (Double, Double, Double) ) {
        
        color = NSColor.init(r: rgb.0, g: rgb.1, b: rgb.2)
    }
    
    func addValue(_ value: Float) {
        
        if sumCnt == 0 { sum = 0 }
        sum += logScale ? log10(1.0 + 9.0 * value) : value // log2(1.0 + 1.0 * value) : value
        sumCnt += 1
    }
    
    var xOffset: Float {
    
        return -barWidth * (Float(offset) / Float(steps))
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
        bgRectangle = Node.init(device: device,
                                x: 0, y: 0, z: 0.01, w: 1.0, h: 1.0)
    }

    func draw(_ commandEncoder: MTLRenderCommandEncoder, matrix: matrix_float4x4) {
                
        offset += 1
        
        if offset == steps {
            values.remove(at: 0)
            values.append(sumCnt == 0 ? sum : sum / Float(sumCnt))
            sumCnt = 0
            offset = 0
            updateRectangles()
        }

        let shift = Renderer.translationMatrix(x: xOffset, y: 0.0, z: 0.0)

        // angle += 2
        // updateMatrices()
        
        // Draw background
        var uniforms = VertexUniforms(mvp: matrix * posm)
        commandEncoder.setVertexBytes(&uniforms,
                                      length: MemoryLayout<VertexUniforms>.stride,
                                      index: 1)
        commandEncoder.setFragmentTexture(background, index: 0)
        bgRectangle!.drawPrimitives(commandEncoder)

        // Draw bars
        uniforms = VertexUniforms(mvp: matrix * posm * shift)
        commandEncoder.setVertexBytes(&uniforms,
                                      length: MemoryLayout<VertexUniforms>.stride,
                                      index: 1)
        commandEncoder.setFragmentTexture(foreground, index: 0)
        for rect in rectangles { rect.drawPrimitives(commandEncoder) }
    }
}
