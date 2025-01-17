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
    let signals = 6
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
    
    // Indicates if anything should be drawn
    var visible = true
     
    // Bit-width of the probed signals
    var bitWidth: [Int] = [ 24, 16, 16, 16, 16, 16 ]
    
    // Recorded data
    var data: [[Int?]] = Array(repeating: Array(repeating: 0, count: 228), count: 6)

    // Labels and colors for each segment
    var labels: [String?] = Array(repeating: nil, count: 228)
    var colors: [NSColor?] = Array(repeating: nil, count: 228)

    // Number formatter
    let formatter = LogicViewFormatter()

    // The graphics context used for drawing
    var context: CGContext!

    // Fonts
    let mono = NSFont.monospacedSystemFont(ofSize: 10, weight: .regular)
    let system = NSFont.systemFont(ofSize: 12)

    var lock = NSLock()

    func update() {

        needsDisplay = true
    }

    
    //
    // Managing the data source
    //

    func cacheData() {

        lock.lock()
        
        guard let emu = emu else { return }
        
        let hpos = emu.amiga.info.hpos
        let laInfo = emu.logicAnalyzer.info
        let owners = laInfo.busOwner!
        let addrBus = laInfo.addrBus!
        let dataBus = laInfo.dataBus!
        let channels = [ laInfo.channel.0, laInfo.channel.1, laInfo.channel.2, laInfo.channel.3 ]
        // Start from scratch
        for i in 0..<segments { labels[i] = nil }
        for i in 0..<segments { colors[i] = nil }
        for c in 0..<signals { for i in 0..<segments { data[c][i] = nil } }

        // Update with new data
        for i in 0..<hpos {
            
            // if (owners + i).pointee == .NONE { continue }
            
            switch (owners + i).pointee {
                
            case .CPU:      labels[i] = "CPU"; colors[i] = inspector.colCPU.color
            case .REFRESH:  labels[i] = "REF"; colors[i] = inspector.colRefresh.color
            case .DISK:     labels[i] = "DSK"; colors[i] = inspector.colDisk.color
            case .AUD0:     labels[i] = "AUD0"; colors[i] = inspector.colAudio.color
            case .AUD1:     labels[i] = "AUD1"; colors[i] = inspector.colAudio.color
            case .AUD2:     labels[i] = "AUD2"; colors[i] = inspector.colAudio.color
            case .AUD3:     labels[i] = "AUD3"; colors[i] = inspector.colAudio.color
            case .BPL1:     labels[i] = "BPL1"; colors[i] = inspector.colBitplanes.color
            case .BPL2:     labels[i] = "BPL2"; colors[i] = inspector.colBitplanes.color
            case .BPL3:     labels[i] = "BPL3"; colors[i] = inspector.colBitplanes.color
            case .BPL4:     labels[i] = "BPL4"; colors[i] = inspector.colBitplanes.color
            case .BPL5:     labels[i] = "BPL5"; colors[i] = inspector.colBitplanes.color
            case .BPL6:     labels[i] = "BPL6"; colors[i] = inspector.colBitplanes.color
            case .SPRITE0:  labels[i] = "SPR0"; colors[i] = inspector.colSprites.color
            case .SPRITE1:  labels[i] = "SPR1"; colors[i] = inspector.colSprites.color
            case .SPRITE2:  labels[i] = "SPR2"; colors[i] = inspector.colSprites.color
            case .SPRITE3:  labels[i] = "SPR3"; colors[i] = inspector.colSprites.color
            case .SPRITE4:  labels[i] = "SPR4"; colors[i] = inspector.colSprites.color
            case .SPRITE5:  labels[i] = "SPR5"; colors[i] = inspector.colSprites.color
            case .SPRITE6:  labels[i] = "SPR6"; colors[i] = inspector.colSprites.color
            case .SPRITE7:  labels[i] = "SPR7"; colors[i] = inspector.colSprites.color
            case .COPPER:   labels[i] = "COP"; colors[i] = inspector.colCopper.color
            case .BLITTER:  labels[i] = "BLT"; colors[i] = inspector.colBlitter.color
            case .BLOCKED:  labels[i] = "BLK"; colors[i] = .red
            default:        labels[i] = "-"; colors[i] = .clear; continue
            }
            
            // The first two channel display the address and data bus
            data[0][i] = Int((addrBus + i).pointee)
            data[1][i] = Int((dataBus + i).pointee)
        }
        
        // For the remaining channels, get the data from the logic analyzer
        for c in 2..<signals {
            
            if let values = channels[c - 2] {
                
                for i in 0..<hpos {
                    
                    let value = (values + i).pointee
                    data[c][i] = value >= 0 ? value : nil
                }
            }
        }
 
        lock.unlock()
    }

    func getData(cycle: Int, channel: Int) -> Int? {

        return cycle < segments ? data[channel][cycle] : nil
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
        context.setStrokeColor(NSColor.tertiaryLabelColor.cgColor)
        context.addPath(path)
        context.drawPath(using: .stroke)
    }
    
    func drawLabels() {
        
        formatter.hex = inspector.hex
        formatter.symbolic = inspector.busSymbolic.state == .on
        
        for i in 0..<segments {
            
            drawText(text: inspector.fmt8.string(for: i) ?? "?",
                     in: NSRect(x: CGFloat(i) * dx, y: bounds.maxY - 0.5 * headerHeight + 2, width: dx, height: 0.5 * headerHeight - 2),
                     font: system,
                     color: .labelColor)
            
            if let label = labels[i] {
                
                colors[i]?.setFill()
                
                CGRect(x: CGFloat(i) * dx, y: bounds.maxY - (headerHeight / 2) - 2, width: dx, height: 4).fill()
                
                drawText(text: label,
                         in: NSRect(x: CGFloat(i) * dx, y: bounds.maxY - headerHeight, width: dx, height: 0.5 * headerHeight - 2),
                         font: system,
                         color: .labelColor)
            }
        }
    }
        
    func drawSignal(_ channel: Int) {

        let bits = bitWidth[channel]
        var prev: Int?
        var curr: Int? = data[channel][0]
        var next: Int? = data[channel][1]
        
        // Compute the signal rect
        let sigrect = CGRect(x: bounds.minX,
                             y: bounds.maxY - headerHeight - CGFloat(channel + 1) * dy,
                             width: bounds.width,
                             height: dy)
        
        for i in 0..<segments {
            
            // Compute the rect for a single slot inside the signal rect
            let r = CGRect(x: CGFloat(i) * dx,
                           y: sigrect.minY + margin,
                           width: dx,
                           height: sigrect.height - 2 * margin)
            
            if bits == 1 {
                drawLineSegment(in: r, v: [prev, curr, next], color: .labelColor)
            } else {
                drawDataSegment(in: r, v: [prev, curr, next], color: .labelColor)
            }
            
            if let curr = curr {

                var label = formatter.string(from: curr, bitWidth: bits)
                
                if channel == 0 && formatter.symbolic {
                    if let symbolic = emu?.mem.symbolize(addr: curr) {
                        label = symbolic
                    }
                }
                
                drawText(text: label, in: r, font: mono, color: .labelColor, partially: true)
            }
            
            prev = curr
            curr = next
            next = i + 2 < segments ? data[channel][i + 2] : nil
        }
    }

    func drawText(text: String, in rect: NSRect, font: NSFont, color: NSColor, partially: Bool = false) {

        // Save the current graphics state
        context?.saveGState()

        // Set up the attributes
        let attributes: [NSAttributedString.Key: Any] = [ .font: font, .foregroundColor: color ]

        // Create the attributed string
        let attributedString = NSAttributedString(string: text, attributes: attributes)
        let size = attributedString.size()

        // Check if the string fits into the drawing area
        if size.width <= rect.width && size.height <= rect.height {
            
            // Draw the string centered
            let p = CGPoint(x: rect.midX - size.width / 2, y: rect.midY - size.height / 2)
            attributedString.draw(at: p)

        } else if partially {

            // Add a clipping area
            context?.addPath(CGPath(rect: rect, transform: nil))
            context?.clip()

            // Draw the string left aligned
            let p = CGPoint(x: rect.minX, y: rect.midY - size.height / 2)
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
