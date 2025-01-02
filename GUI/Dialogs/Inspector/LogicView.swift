// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class LogicView: NSView {

    // Constants
    let segments = 228
    let signals = 4
    // let headerHeight = CGFloat(24)
    let signalHeight = CGFloat(24)
    
    // Derived constants
    var headerHeight: CGFloat { return bounds.height / CGFloat(signals + 1) }
    var dx: CGFloat { return bounds.width / CGFloat(segments) }
    var dy: CGFloat { return (bounds.height - headerHeight) / CGFloat(signals) }
    var margin: CGFloat { return (dy - signalHeight) / 2 }
    
    // Reference to the logic analyzer this view is embedded in
    @IBOutlet weak var inspector: Inspector!

    // Reference to the emulator
    var emu: EmulatorProxy? { return inspector.emu }

    var lock = NSLock()
    
    // Indicates if anything should be drawn
    var visible = true
     
    // The probed signal
    var probe: [Probe] = [ .BUS_OWNER, .ADDR_BUS, .DATA_BUS, .NONE ]

    // The recorded data
    var data:[[Int?]] = Array(repeating: Array(repeating: 0, count: 228), count: 4)

    // Number formatter
    let formatter = LogicViewFormatter()

    // The graphics context used for drawing
    var context: CGContext!
    var gradient: [CGGradient?] = [nil, nil, nil, nil]

    // Fonts and colors
    let box = NSBox()
    
    // var bgColor = [ NSColor.lightGray, NSColor.gray, NSColor.darkGray, NSColor.systemGray]
    let mono = NSFont.monospacedSystemFont(ofSize: 10, weight: .regular)
    let system = NSFont.systemFont(ofSize: 12)

    func update() {

        needsDisplay = true
    }

    func signalRect(_ nr: Int) -> CGRect {
        
        return CGRect(x: bounds.minX,
                      y: bounds.maxY - headerHeight - CGFloat(nr + 1) * dy,
                      width: bounds.width,
                      height: dy)
    }
    
    
    //
    // Managing the data source
    //

    func cacheData() {

        lock.lock()
        
        guard let emu = emu else { return }

        let hpos = emu.amiga.info.hpos
        
        for c in 0...3 {
            
            if let values = emu.logicAnalyzer.getData(c) {
                
                for i in 0..<hpos {
                    
                    let value = (values + i).pointee
                    data[c][i] = value >= 0 ? value : nil
                }
                for i in (hpos + 1)..<228 {
                    
                    data[c][i] = nil
                }
            }
        }
 
        lock.unlock()
    }

    func getData(cycle: Int, channel: Int) -> Int? {

        return cycle < 228 ? data[channel][cycle] : nil
    }
    
    //
    // Drawing
    //

    override func draw(_ dirtyRect: NSRect) {

        lock.lock()
        
        super.draw(dirtyRect)

        context = NSGraphicsContext.current?.cgContext
        
        NSColor.clear.setFill()
        bounds.fill()

        if visible {
            
            drawHairlines()
            drawLabels()
            for i in 0..<signals { drawSignal(i) }
        }
        
        lock.unlock()
    }
    
    func drawHairlines() {

        let path = CGMutablePath()
    
        for i in 1..<segments {

            path.move(to: CGPoint(x: CGFloat(i) * dx, y: bounds.minY))
            path.addLine(to: CGPoint(x: CGFloat(i) * dx, y: bounds.maxY))
        }
        context.setLineWidth(0.5)
        context.setStrokeColor(NSColor.secondaryLabelColor.cgColor)

        context.addPath(path)
        context.drawPath(using: .stroke)
    }
    
    func drawLabels() {
        
        for i in 0..<segments {

            drawText(text: "\(i)",
                     in: NSRect(x: CGFloat(i) * dx, y: bounds.maxY - headerHeight, width: dx, height: headerHeight),
                     font: system,
                     color: NSColor.labelColor)
        }
    }
    
    func drawSignal(_ channel: Int) {

        let rect = signalRect(channel)

        // if (channel % 2 == 0) { NSColor.red.setFill() } else { NSColor.blue.setFill() }
        // rect.fill()
        
        var prev: Int?
        var curr: Int? = getData(cycle: 0, channel: channel)
        var next: Int? = getData(cycle: 1, channel: channel)

        for i in 0..<segments {
            
            let r = CGRect(x: CGFloat(i) * dx,
                           y: rect.minY + margin,
                           width: dx,
                           height: rect.height - 2 * margin)
            
            drawDataSegment(in: r, v: [prev, curr, next], color: .labelColor)
            /*
             if bitWidth[channel] == 1 {
             drawLineSegment(in: r, v: [prev, curr, next], color: .black)
             } else {
             drawDataSegment(in: r, v: [prev, curr, next], color: .black)
             }
             */
            if curr != nil {
                drawText(text: formatter.string(from: curr!, bitWidth: 16),
                         in: r,
                         font: mono,
                         color: .labelColor)
            }
            
            prev = curr
            curr = next
            next = getData(cycle: i+2, channel: channel)
        }
    }

    func drawText(text: String, in rect: NSRect, font: NSFont, color: NSColor) {

        // Save the current graphics state
        context?.saveGState()

        // Set up the attributes
        let attributes: [NSAttributedString.Key: Any] = [ .font: font, .foregroundColor: color ]

        // Create the attributed string
        let attributedString = NSAttributedString(string: text, attributes: attributes)
        let size = attributedString.size()

        // Check if the string fits into the drawing area
        if size.width <= rect.width && size.height <= rect.height {

            // Draw the string
            let p = CGPoint(x: rect.midX - size.width / 2, y: rect.midY - size.height / 2)
            attributedString.draw(at: p)
        }

        // Restore the graphics state
        context?.restoreGState()
    }
    
    func drawLineSegment(in rect: CGRect, v: [Int?], color: NSColor) {

        if v[1] == nil { return }

        let path = CGMutablePath()

        let x1 = rect.minX
        let y1 = rect.minY
        let x2 = rect.maxX
        let y2 = rect.maxY

        /*          p2                  p3
         *          .-------------------.
         *          |                   |
         *          |                   |
         *          p1                  p4
         */

        let p2 = CGPoint(x: x1, y: v[1] == 1 ? y2 : y1)
        let p1 = v[0] != nil ? CGPoint(x: x1, y: v[0] == 1 ? y2 : y1) : p2
        let p3 = CGPoint(x: x2, y: v[1] == 1 ? y2 : y1)
        let p4 = v[2] != nil ? CGPoint(x: x2, y: v[2] == 1 ? y2 : y1) : p3

        path.move(to: p1)
        path.addLine(to: p2)
        path.addLine(to: p3)
        path.addLine(to: p4)

        context.setLineWidth(1.5)
        context.setStrokeColor(color.cgColor)

        context.addPath(path)
        context.drawPath(using: .stroke)
    }

    func drawDataSegment(in rect: CGRect, v: [Int?], color: NSColor) {

        let path = CGMutablePath()

        let x1 = rect.minX
        let y1 = rect.minY
        let x2 = rect.maxX
        let y2 = rect.maxY

        if v[1] == nil {
            
            let p1 = CGPoint(x: x1, y: rect.midY)
            let p2 = CGPoint(x: x2, y: rect.midY)
            path.move(to: p1)
            path.addLine(to: p2)
            
        } else {
            
            /*           p2               p3
             *           /-----------------\
             *          /                   \
             *   p1/p8 .                     . p4/p5
             *          \                   /
             *           \-----------------/
             *           p7               p6
             */
            
            let m = 0.1 * rect.width
            let p1 = CGPoint(x: x1, y: (v[0] == v[1]) ? y1 : rect.midY)
            let p2 = CGPoint(x: x1 + m, y: y1)
            let p3 = CGPoint(x: x2 - m, y: y1)
            let p4 = CGPoint(x: x2, y: (v[1] == v[2]) ? y1 : rect.midY)
            let p5 = CGPoint(x: x2, y: (v[1] == v[2]) ? y2 : rect.midY)
            let p6 = CGPoint(x: x2 - m, y: y2)
            let p7 = CGPoint(x: x1 + m, y: y2)
            let p8 = CGPoint(x: x1, y: (v[0] == v[1]) ? y2 : rect.midY)
            
            path.move(to: p1)
            path.addLine(to: p2)
            path.addLine(to: p3)
            path.addLine(to: p4)
            path.move(to: p5)
            path.addLine(to: p6)
            path.addLine(to: p7)
            path.addLine(to: p8)
        }
        
        context.setLineWidth(1.5)
        context.setStrokeColor(color.cgColor)

        context.addPath(path)
        context.drawPath(using: .stroke)
    }

}
