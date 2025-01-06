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

    var audioPort: AudioPortProxy { return monitor.emu.audioPort }

    // Waveform size
    var size: NSSize!
    var wordCount: Int { return Int(size.width) * Int(size.height) }

    // Waveform buffer
    var buffer: UnsafeMutablePointer<UInt32>!

    // Remembers the highest amplitude (used for auto scaling)
    // var maxAmp: Float = 0.001

    // Foreground color
    var color = UInt32(0x88888888)

    required init?(coder decoder: NSCoder) {

        super.init(coder: decoder)
    }

    required override init(frame frameRect: NSRect) {
        
        super.init(frame: frameRect)
    }

    override func awakeFromNib() {

        let w = visibleRect.width
        let h = visibleRect.height
        
        size = NSSize(width: w, height: h)
        buffer = UnsafeMutablePointer<UInt32>.allocate(capacity: wordCount)
    }

    func update() {

        needsDisplay = true
    }

    override func draw(_ dirtyRect: NSRect) {

        if tag == 0 {
            audioPort.drawWaveformL(buffer, size: size, color: color)
        } else {
            audioPort.drawWaveformR(buffer, size: size, color: color)
        }
        
        image = NSImage.make(data: buffer, rect: CGSize(width: size.width, height: size.height))
        super.draw(dirtyRect)
    }
}
