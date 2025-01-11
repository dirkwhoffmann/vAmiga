// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class WaveformPanel: NSImageView {

    @IBOutlet weak var monitor: Dashboard!

    var audioPort: AudioPortProxy? //  { return nil } //  monitor.emu.audioPort }

    // Waveform size
    var imageSize: NSSize!
    var wordCount: Int { return Int(imageSize.width) * Int(imageSize.height) }

    // Waveform buffer
    var buffer: UnsafeMutablePointer<UInt32>!

    // Remembers the highest amplitude (used for auto scaling)
    // var maxAmp: Float = 0.001

    // Foreground color
    var color = UInt32(0xFF888888)

    required init?(coder decoder: NSCoder) {

        super.init(coder: decoder)
        setup()
    }

    required override init(frame frameRect: NSRect) {
        
        super.init(frame: frameRect)
        setup()
    }
    
    func setup() {
                        
        imageSize = NSSize(width: 300, height: 100)
        buffer = UnsafeMutablePointer<UInt32>.allocate(capacity: wordCount)
        imageScaling = .scaleAxesIndependently
        
    }

    func update() {

        needsDisplay = true
    }

    override var intrinsicContentSize: NSSize {
        
        // Let the image scale with the ImageView
        return .zero
    }
    
    override func draw(_ dirtyRect: NSRect) {

        if tag == 0 {
            audioPort?.drawWaveformL(buffer, size: imageSize, color: color)
        } else {
            audioPort?.drawWaveformR(buffer, size: imageSize, color: color)
        }
                
        image = NSImage.make(data: buffer, rect: CGSize(width: imageSize.width, height: imageSize.height))
        super.draw(dirtyRect)
    }
}
