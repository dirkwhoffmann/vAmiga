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

    // Modulo counter to trigger image rendering
    // var delayCounter = 0

    required init?(coder decoder: NSCoder) {
        super.init(coder: decoder)
    }

    required override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
    }

    func update() {

        needsDisplay = true
    }

    func add(value val: Double) {

        for i in 0 ..< values.count - 1 {
            values[i] = values[i+1]
        }
        values[127] = val

        update()
    }

    override func draw(_ dirtyRect: NSRect) {

        super.draw(dirtyRect)

        // guard let paula = amigaProxy?.paula else { return }
        let context = NSGraphicsContext.current?.cgContext

        NSColor.clear.set()
        context?.fill(dirtyRect)

        let w = Double(frame.width), h = Double(frame.height)

        for x in 0...Int(w) {

            let val = values[Int(Double(x) * 127.0 / w )]
            let scaled = min(Int(val * h), 1)

            let from = CGPoint(x: x, y: 0)
            let to = CGPoint(x: x, y: scaled)
            context?.move(to: from)
            context?.addLine(to: to)
        }
        NSColor.gray.setStroke()
        context?.strokePath()
    }
}
