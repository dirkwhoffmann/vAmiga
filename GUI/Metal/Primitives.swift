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
    
    // Number of displayed values (bars)
    let capacity = 20
    
    // Number of scroll steps until a new bar shows up
    let steps = 20
    
    // Drawing dimensions
    let width = Float(0.01)
    let space = Float(0.005)
    let scale = Float(0.2)
    let border = Float(0.02)
    
    var totalWidth: Float { return Float(capacity + 1) * (width + space) - space }
    
    let device: MTLDevice
    var values: [Float]
    var rectangles: [Node]
    var bgRectangle: Node?
    var offset = 0
    var sum = Float(0)
        
    // Texture for drawing a single bar
    var foreground: MTLTexture?
    
    // Shared texture for drawing the background
    static var background: MTLTexture?

    init(device: MTLDevice) {
        
        self.device = device
        rectangles = []
        
        foreground = device.makeGradientTexture(width: 128, height: 128,
                                                r1: 255, g1: 0, b1: 0, a1: 255,
                                                r2: 255, g2: 255, b2: 255, a2: 128)
        
        if BarChart.background == nil {
            BarChart.background =
                device.makeGradientTexture(width: 256, height: 256,
                                           r1: 64, g1: 64, b1: 64, a1: 200,
                                           r2: 128, g2: 128, b2: 128, a2: 200,
                                           radius: 10)
        }
        
        values = Array(repeating: 0.0, count: capacity)
        updateRectangles()
    }
        
    func addValue(_ value: Float) {
        
        sum += value
        offset += 1
        
        if offset == steps {
            values.remove(at: 0)
            values.append(sum / Float(steps))
            sum = 0
            offset = 0
            updateRectangles()
        }
    }
    
    var xOffset: Float {
    
        return -(width + space) * (Float(offset) / Float(steps))
    }
    
    func updateRectangles() {
                
        rectangles = []
        for n in 0 ..< values.count {
            let rect = NSRect.init(x: 0,
                                   y: 1.0 - Double(values[n]),
                                   width: 1.0,
                                   height: Double(values[n]))
            let x = Float(n) * (width + space)
            rectangles.append(Node.init(device: device,
                                        x: x, y: 0, z: 0,
                                        w: width, h: values[n] * scale, t: rect))
        }
        bgRectangle = Node.init(device: device,
                                x: -border - width - space, y: -border, z: 0.01,
                                w: totalWidth + 2*border, h: scale + 2*border)
    }
    
    func draw(_ commandEncoder: MTLRenderCommandEncoder, matrix: matrix_float4x4) {
                
        let shift = Renderer.translationMatrix(x: xOffset, y: 0.0, z: 0.0)

        // Draw background
        var uniforms = VertexUniforms(mvp: matrix)
        commandEncoder.setVertexBytes(&uniforms,
                                      length: MemoryLayout<VertexUniforms>.stride,
                                      index: 1)
        commandEncoder.setFragmentTexture(BarChart.background, index: 0)
        bgRectangle!.drawPrimitives(commandEncoder)

        // Draw bars
        uniforms = VertexUniforms(mvp: shift * matrix)
        commandEncoder.setVertexBytes(&uniforms,
                                      length: MemoryLayout<VertexUniforms>.stride,
                                      index: 1)
        commandEncoder.setFragmentTexture(foreground, index: 0)
        for rect in rectangles { rect.drawPrimitives(commandEncoder) }
    }
}
