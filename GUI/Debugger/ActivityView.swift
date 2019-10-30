// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class ActivityView: NSView {

    var values = Array(repeating: 0.0, count: 128)
    var current: Double { return values[values.count - 1] }
    var maxval = 0.0
    var maxdisplayed = 0.5
    var w = 0.0, h = 0.0

    required init?(coder decoder: NSCoder) {
        super.init(coder: decoder)
    }

    required override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
    }

    func update() {

        needsDisplay = true
    }

    func add(value: Double) {

        maxval = 0
        for i in 0 ..< values.count - 1 {
            values[i] = values[i+1]
            maxval = max(maxval, values[i])
        }
        values[127] = 0.6 * values[126] + 0.4 * value
        maxval = max(maxval, values[127])

        if maxdisplayed < maxval { maxdisplayed += 0.03 }
        if maxdisplayed > max(maxval, 0.5) { maxdisplayed -= 0.03 }
    }

    func sample(x: Int) -> Int {

        let pos = Double(x) / Double(w)
        let val = values[Int(pos * Double(values.count - 1))]
        let scaled = Double(h) * val //  / maxdisplayed
        return Int(scaled)
    }

    override func draw(_ dirtyRect: NSRect) {

        // let upper = 255, lower = 102

        var r, g, b: Int
        if maxval <= 0.5 {
            r = 178 + Int(maxval * 144.0)
            g = 255
            b = 102
        } else {
            r = 255
            g = 255 - Int(maxval * 144.0)
            b = 102
        }
        let col1 = NSColor.init(r: r, g: g, b: b, a: 255)
        let col2 = col1.lighter()

        /*
        super.draw(dirtyRect)
        let offset1 = Int(((maxdisplayed - 0.5) / 0.5) * 255)
        let offset2 = Int(((maxdisplayed - 0.5) / 0.5) * 200)
        */
        let colors = [col1.cgColor, col2.cgColor]
        let colorSpace = CGColorSpaceCreateDeviceRGB()
        let locations: [CGFloat] = [0.0, 1.0]
        let gradient = CGGradient(colorsSpace: colorSpace,
                                  colors: colors as CFArray,
                                  locations: locations)!
        /*
        let gcol =  NSColor.init(r: lower, g: upper, b: lower, a: 255)
        let ycol =  NSColor.init(r: upper, g: upper, b: lower, a: 255)
        let rcol =  NSColor.init(r: upper, g: lower, b: lower, a: 255)
        let colors2 = [gcol.cgColor, ycol.cgColor, rcol.cgColor]
        let locations2: [CGFloat] = [0.0, 0.5, 1.0]
        let gradient2 = CGGradient(colorsSpace: colorSpace,
                                  colors: colors2 as CFArray,
                                  locations: locations2)!
        */

        let alphawhite = NSColor.init(r: 255, g: 255, b: 255, a: 100)
        let alphablack = NSColor.init(r: 0, g: 0, b: 0, a: 100)

        // guard let paula = amigaProxy?.paula else { return }
        let context = NSGraphicsContext.current?.cgContext

        let startPoint = CGPoint.zero
        let endPoint = CGPoint(x: 0, y: bounds.height)

        context?.drawLinearGradient(gradient,
                                    start: startPoint,
                                    end: endPoint,
                                    options: [])

        NSColor.white.setFill()
        NSColor.white.setStroke()

        // set up the points line
        let graphPath = NSBezierPath()

        w = Double(frame.width)
        h = Double(frame.height)

        // Create graph path
        var y = sample(x: 0)
        graphPath.move(to: CGPoint(x: 0, y: y))

        for x in 0...Int(w) {

            y = sample(x: x)
            let to = CGPoint(x: x, y: y)
            graphPath.line(to: to)
        }

        // Create clipping path
        let clippingPath = graphPath.copy() as? NSBezierPath
        clippingPath!.line(to: CGPoint(x: Int(w), y: 0))
        clippingPath!.line(to: CGPoint(x: 0, y: 0))
        clippingPath!.close()
        clippingPath!.addClip()

        // Draw gradient
        /*
        context?.drawLinearGradient(gradient,
                                    start: startPoint,
                                    end: endPoint,
                                    options: [])
        */

        let rect = CGRect(x: 0, y: 0, width: Int(w), height: Int(h))
        alphawhite.setFill()
        context?.fill(rect)

        graphPath.lineWidth = 2.0
        graphPath.stroke()
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
