// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class ActivityView: NSView {

    var n = 0

    // Value storage
    var values = Array(repeating: 0.0, count: 128)
    // var current: Double { return values[values.count - 1] }

    // Scaling factor and vertical zero point
    var scale = 0.0
    var zero = 0.0

    //
    // Configuring the view
    //

    // Number of horizontal help lines
    var numlines = 5

    // Set to true to use a logarithmic scale
    var logscale = true

    // Set to true if the view should cover negative values, too
    var negative: Bool {
        didSet {
            scale = negative ? Double(frame.height) / 2.0 : Double(frame.height)
            zero = negative ? Double(frame.height) / 2.0 : 0.0
        }
    }

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
        negative = true
        super.init(coder: decoder)
    }

    required override init(frame frameRect: NSRect) {
        negative = true
        super.init(frame: frameRect)
    }

    func update() {

        needsDisplay = true
    }

    func add(value: Double) {

        var v = value
        if logscale {
            v *= 1024.0
            v = (v < 1.0) ? 0.0 : (log2(v) / 10.0)
        }

        n += 1
        if n > 20 { n = 0 }

        for i in 0 ..< values.count - 1 { values[i] = values[i+1] }

        values[127] = 0.6 * values[126] + 0.4 * v * (n > 10 ? 1.0 : 1.0)
        needsDisplay = true
    }

    func sample(x: Int) -> Double {

        let pos = Double(x) / Double(w)
        let val = values[Int(pos * Double(values.count - 1))]
        return val
    }

    func drawGrid() {

        let context = NSGraphicsContext.current?.cgContext

        NSColor.lightGray.setStroke()
        context?.setLineWidth(0.5)
        let delta = 1.0 / Double(numlines)

        for i in 1..<numlines {
            var y = Double(i) * delta
            if logscale { y = log2(1.0 + y) }
            context?.move(to: CGPoint(x: 0, y: Int(y * h)))
            context?.addLine(to: CGPoint(x: Int(w), y: Int(y * h)))
            context?.strokePath()
        }

        // Lower line
        context?.move(to: CGPoint(x: 0, y: 1))
        context?.addLine(to: CGPoint(x: Int(frame.width), y: 1))
        context?.strokePath()

        // Upper line
        // context?.move(to: CGPoint(x: 0, y: Int(frame.height) - 1))
        // context?.addLine(to: CGPoint(x: Int(frame.width), y: Int(frame.height) - 1))
        // context?.strokePath()

        // Zero line
        context?.setLineWidth(1)
        NSColor.darkGray.setStroke()
        context?.move(to: CGPoint(x: 0, y: Int(zero)))
        context?.addLine(to: CGPoint(x: Int(frame.width), y: Int(zero)))
        context?.strokePath()
    }

    func draw(upper: Bool) {

        let context = NSGraphicsContext.current?.cgContext

        // Create gradient
        let v = 255
        let c1 = NSColor.init(r: 0, g: v, b: 0, a: 128).cgColor
        let c2 = NSColor.init(r: v, g: v, b: 0, a: 128).cgColor
        let c3 = NSColor.init(r: v, g: 0, b: 0, a: 128).cgColor
        let grad = CGGradient(colorsSpace: CGColorSpaceCreateDeviceRGB(),
                              colors: [c1, c1, c2, c3] as CFArray,
                              locations: [0.0, 0.4, 0.6, 1.0] as [CGFloat])!
        
        context?.saveGState()

        // Convert data points to a graph
        let graphPath = NSBezierPath()
        var y = sample(x: 0)
        y = y > 0 ? (upper ? y : 0) : (upper ? 0 : y)
        graphPath.move(to: CGPoint(x: 0, y: Int(zero + y * scale)))
        for x in 0...Int(w) {
            y = sample(x: x)
            y = y > 0 ? (upper ? y : 0) : (upper ? 0 : y)
            graphPath.line(to: CGPoint(x: x, y: Int(zero + y * scale)))
        }

        // Create clipping path
        let clippingPath = graphPath.copy() as? NSBezierPath
        clippingPath!.line(to: CGPoint(x: Int(w), y: Int(zero)))
        clippingPath!.line(to: CGPoint(x: 0, y: Int(zero)))
        clippingPath!.close()
        clippingPath!.addClip()

        // Draw gradient
        context?.drawLinearGradient(grad,
                                    start: CGPoint(x: 0, y: 0),
                                    end: CGPoint(x: 0, y: bounds.height),
                                    options: [])

        // Fill area
        /*
        let fillColor = upper ? posColAlpha : negColAlpha
        let rect = CGRect(x: 0, y: 0, width: Int(frame.width), height: Int(frame.height))
        fillColor?.setFill()
        context?.fill(rect)
        */

        // Draw graph
        let lineColor = upper ? posCol : negCol
        lineColor?.setStroke()
        graphPath.lineWidth = 3.0
        graphPath.stroke()

        context?.restoreGState()
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
}
