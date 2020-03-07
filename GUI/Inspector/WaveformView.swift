// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class WaveformView: NSView {

    @IBOutlet weak var monitor: Monitor!
    var amiga: AmigaProxy!

    // Remembers the highest amplitude (used for auto scaling)
    var highestAmplitude = 0.001
    
    // Modulo counter to trigger image rendering
    var delayCounter = 0
    
    required init?(coder decoder: NSCoder) {
        super.init(coder: decoder)
    }
 
    required override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
    }

    override func awakeFromNib() {

        amiga = monitor.amiga
    }

    // Restarts the auto scaling mechanism
    func initAutoScaler() {

        highestAmplitude = 0.001
    }
    
    func update() {

        needsDisplay = true
    }
    
    override func draw(_ dirtyRect: NSRect) {
        
        super.draw(dirtyRect)

        let context = NSGraphicsContext.current?.cgContext
        
        NSColor.clear.set()
        context?.fill(dirtyRect)
        
        let w = Int(frame.width)
        let baseline = Double(frame.height / 2)
        let normalizer = highestAmplitude
        highestAmplitude = 0.001
        
        for x in 0...w {

            // Read samples from ringbuffer
            let sampleL = abs(amiga.paula.ringbufferDataL(40 * x))
            let sampleR = abs(amiga.paula.ringbufferDataR(40 * x))

            // Remember the highest amplitude
            highestAmplitude = max(highestAmplitude, max(sampleL, sampleR))

            // Scale the samples
            var scaledL = sampleL / normalizer * baseline
            var scaledR = sampleR / normalizer * baseline

            // Add some noise to make it look sexy
            if scaledL == 0 && scaledR == 0 {
                if drand48() > 0.5 { scaledL = 1.0 } else { scaledR = 1.0 }
            }

            // Draw lines
            let from = CGPoint(x: x, y: Int(baseline + scaledL))
            let to = CGPoint(x: x, y: Int(baseline - scaledR))
            context?.move(to: from)
            context?.addLine(to: to)
        }
        NSColor.gray.setStroke()
        context?.strokePath()
    }
}
