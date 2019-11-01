// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class ActivityView: NSView {

    // Values of the upper graph
    var values = [Array(repeating: 0.0, count: 128),
                  Array(repeating: 0.0, count: 128)]

    //
    // Configuring the view
    //

    // Number of horizontal help lines
    var numlines = 8

    // Set to true to use a logarithmic scale
    var logscale = true

    // Set to true if the view should cover an upper and a lower part
    var splitview = false

    // Colors for drawing both graphs
    // var color1: NSColor?
    // var color2: NSColor?
    var color1 = NSColor.init(r: 0, g: 128, b: 255, a: 255)
    var alpha1 = NSColor.init(r: 0, g: 128, b: 255, a: 100)
    var color2 = NSColor.init(r: 0, g: 128, b: 255, a: 255)
    var alpha2 = NSColor.init(r: 0, g: 128, b: 255, a: 100)

    //
    // Computed properties
    //

    // Scale factor
    var scale: Double { return Double(frame.height - 2) / (splitview ? 2.0 : 1.0) }

    // Vertical location of the zero point
    var zero: Double { return splitview ? Double(frame.height - 2) / 2.0 : 0.0 }

    // Colors for positive and negative values
    var posCol: NSColor?
    var posColAlpha: NSColor?
    var negCol: NSColor?
    var negColAlpha: NSColor?

    func setPosColor(r: Int, g: Int, b: Int) {
        posCol = NSColor.init(r: r, g: g, b: b, a: 255)
        posColAlpha = NSColor.init(r: r, g: g, b: b, a: 128)
    }
    func setPosColor(r: Double, g: Double, b: Double) {
        setPosColor(r: Int(r * 255.0), g: Int(g * 255.0), b: Int(b * 255.0))
     }
    func setNegColor(r: Int, g: Int, b: Int) {
        negCol = NSColor.init(r: r, g: g, b: b, a: 255)
        negColAlpha = NSColor.init(r: r, g: g, b: b, a: 128)
    }
    func setNegColor(r: Double, g: Double, b: Double) {
        setNegColor(r: Int(r * 255.0), g: Int(g * 255.0), b: Int(b * 255.0))
     }

    // MAKE LOCAL
    var w = 0.0, h = 0.0

    required init?(coder decoder: NSCoder) {
        splitview = true
        super.init(coder: decoder)
    }

    required override init(frame frameRect: NSRect) {
        splitview = true
        super.init(frame: frameRect)
    }

    func update() {

        needsDisplay = true
    }

    func add(value: Double, storage: Int) {

        assert(storage == 0 || storage == 1)
        assert(value >= 0.0)

        let v = logscale ? log10(1.0 + 10.0 * value) : value
        let max = values[storage].count

        for i in 0 ..< max-1 {
            values[storage][i] = values[storage][i+1]
            values[storage][max-1] = 0.4 * values[storage][max-2] + 0.6 * v
        }
    }

    /*
    func add(val1: Double, val2: Double = 0.0) {

        add(value: val1, storage: 0)
        if splitview { add(value: val2, storage: 1) }

        needsDisplay = true
    }
    */

    func add(val1: Double, val2: Double = 0.0) {

        add(value: 1.0, storage: 0)
        if splitview { add(value: 1.0, storage: 1) }

        needsDisplay = true
    }

    func sample(x: Int, storage: Int) -> Double {

        assert(storage == 0 || storage == 1)

        let pos = Double(x) / Double(w)
        let val = values[storage][Int(pos * Double(values[storage].count - 1))]

        return storage == 0 ? val : -val
    }

    func drawGrid() {

        let delta = 1.0 / Double(numlines)
        let c = scale, z = Int(zero)

        if let context = NSGraphicsContext.current?.cgContext {

            NSColor.lightGray.setStroke()
            context.setLineWidth(0.5)

            for i in 1..<numlines {
                var y = Double(i) * delta
                if logscale { y = log10(1.0 + 9*y) }
                context.move(to: CGPoint(x: 0, y: z + Int(c * y)))
                context.addLine(to: CGPoint(x: Int(w), y: z + Int(c * y)))
                context.strokePath()
            }

            if splitview {
                for i in 1..<numlines {
                     var y = Double(i) * delta
                     if logscale { y = log10(1.0 + 9*y) }
                     context.move(to: CGPoint(x: 0, y: z - Int(c * y)))
                     context.addLine(to: CGPoint(x: Int(w), y: z - Int(c * y)))
                     context.strokePath()
                 }
            }
        }
    }

    func drawZeroLine() {

        if let context = NSGraphicsContext.current?.cgContext {

            context.setLineWidth(1)
            NSColor.textColor.setStroke()
            context.move(to: CGPoint(x: 0, y: Int(zero)))
            context.addLine(to: CGPoint(x: Int(w), y: Int(zero)))
            context.strokePath()
        }
    }

    func createGraph(storage s: Int) -> NSBezierPath {

        let graph = NSBezierPath()
        let c = scale, z = zero

        var y = sample(x: 0, storage: s)
        graph.move(to: CGPoint(x: 0, y: Int(z + y * c)))
        for x in 0...Int(w) {
            y = sample(x: x, storage: s)
            graph.line(to: CGPoint(x: x, y: Int(z + y * c)))
        }

        return graph
    }

    func draw(upper: Bool) {

        let context = NSGraphicsContext.current?.cgContext
        var graph1, graph2, clip1, clip2: NSBezierPath?

        // Create gradient
        // let c1 = NSColor.init(r: 0, g: 192, b: 0, a: 128).cgColor
        let c2 = NSColor.init(r: 255, g: 255, b: 0, a: 128).cgColor
        let c3 = NSColor.init(r: 255, g: 0, b: 0, a: 128).cgColor
        let grad1 = CGGradient(colorsSpace: CGColorSpaceCreateDeviceRGB(),
                               colors: [alpha1.cgColor, alpha1.cgColor, c2, c3] as CFArray,
                               locations: [0.5, 0.8, 0.9, 1.0] as [CGFloat])!

        // Create graph lines
        graph1 = createGraph(storage: 0)
        if splitview { graph2 = createGraph(storage: 1) }

        // Create clipping areas
        clip1 = graph1?.copy() as? NSBezierPath
        clip1?.line(to: CGPoint(x: Int(w), y: Int(zero)))
        clip1?.line(to: CGPoint(x: 0, y: Int(zero)))
        clip1?.close()

        if splitview {
            clip2 = graph2?.copy() as? NSBezierPath
            clip2?.line(to: CGPoint(x: Int(w), y: Int(zero)))
            clip2?.line(to: CGPoint(x: 0, y: Int(zero)))
            clip2?.close()
        }

        // Fill area
        /*
         context?.saveGState()
         clip1?.addClip()
         let fillColor = color1.transparent(alpha: 0.4)
         let rect = CGRect(x: 0, y: Int(zero), width: Int(frame.width), height: Int(frame.height))
         fillColor.setFill()
         context?.fill(rect)
         context?.restoreGState()
         */

        // Apply a gradient fill
        context?.saveGState()
        clip1?.addClip()
        context?.drawLinearGradient(grad1,
                                    start: CGPoint(x: 0, y: zero),
                                    end: CGPoint(x: 0, y: bounds.height),
                                    options: [])
        context?.restoreGState()

        // Draw graph line
        color1.setStroke()
        graph1?.lineWidth = 2.0
        graph1?.stroke()

        // Draw lower graph
        if splitview {

            // Apply a gradient fill
            context?.saveGState()
            clip2?.addClip()
            context?.drawLinearGradient(grad1,
                                        start: CGPoint(x: 0, y: zero),
                                        end: CGPoint(x: 0, y: 0),
                                        options: [])
            context?.restoreGState()

            // Draw graph line
            color2.setStroke()
            graph2?.lineWidth = 2.0
            graph2?.stroke()
        }
    }

    override func draw(_ dirtyRect: NSRect) {

        let context = NSGraphicsContext.current?.cgContext

        // Store width and height
        w = Double(frame.width)
        h = Double(frame.height - 1)

        // Clear the view
        NSColor.clear.set()
        context?.fill(dirtyRect)

        // Draw background
        drawGrid()

        // Draw positive values
        draw(upper: true)
        draw(upper: false)

        // track("h = \(h) \(Int(h / 2))")
        drawZeroLine()
    }
}

extension NSColor {

    func lighter(by percentage: CGFloat = 30.0) -> NSColor {
        return self.adjust(by: abs(percentage) )
    }

    func darker(by percentage: CGFloat = 30.0) -> NSColor {
        return self.adjust(by: -1 * abs(percentage) )
    }

    func adjust(by percentage: CGFloat = 30.0) -> NSColor {
        var red: CGFloat = 0, green: CGFloat = 0, blue: CGFloat = 0, alpha: CGFloat = 0
        getRed(&red, green: &green, blue: &blue, alpha: &alpha)
        return NSColor(red: min(red + percentage/100, 1.0),
                       green: min(green + percentage/100, 1.0),
                       blue: min(blue + percentage/100, 1.0),
                       alpha: alpha)
    }

    func transparent(alpha: CGFloat = 0.3) -> NSColor {
        var r: CGFloat = 0, g: CGFloat = 0, b: CGFloat = 0, a: CGFloat = 0
        getRed(&r, green: &g, blue: &b, alpha: &a)
        return NSColor(red: r, green: g, blue: b, alpha: alpha)
    }
}
