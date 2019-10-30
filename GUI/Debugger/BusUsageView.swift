// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class BusUsageView: NSView {

    var values = Array(repeating: Array(repeating: 0.0, count: Int(BUS_OWNER_COUNT.rawValue)), count: 128)
    // var current: Double { return values[values.count - 1] }
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

    func shiftValues() {

        for i in 0 ..< values.count - 1 {
            values[i] = values[i+1]
        }
    }

    func add(value: Int, busOwner: Int) {

        values[127][busOwner] = Double(value) / (312 * 226)
        update()
    }

    func sample(x: Int, busOwner: Int) -> Int {

        // track("count = \(values.count)")
        let cnt = values.count - 1
        let pos = Int(Double(cnt) * Double(x) / Double(w))
        // track("pos = \(pos)")
        let val = values[pos][busOwner]
        let scaled = Double(h) * val
        return Int(scaled)
    }

    override func draw(_ dirtyRect: NSRect) {

        let context = NSGraphicsContext.current?.cgContext

        NSColor.clear.set()
        context?.fill(dirtyRect)

        w = Double(frame.width)
        h = Double(frame.height)

        // Create graph
        for x in 0...Int(w) {

            context?.move(to: CGPoint(x: x, y: 0))
            var y = 0

            for i in 0...Int(BUS_OWNER_COUNT.rawValue)-1 {

                y += sample(x: x, busOwner: i)
                if i % 2 == 0 {
                    NSColor.red.setStroke()
                } else {
                    NSColor.yellow.setStroke()
                }
                let to = CGPoint(x: x, y: y)
                context?.addLine(to: to)
            }
        }
        context?.strokePath()
    }
}
