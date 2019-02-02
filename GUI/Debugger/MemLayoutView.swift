// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class MemLayoutView: NSView {
    
    required init?(coder decoder: NSCoder) {
        super.init(coder: decoder)
    }
    
    required override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
    }
    
    func update() {
  
        needsDisplay = true
    }
    
    override func draw(_ dirtyRect: NSRect) {
        
        super.draw(dirtyRect)
        
        guard let memory = amigaProxy?.mem else { return }
        let context = NSGraphicsContext.current?.cgContext
        
        NSColor.clear.set()
        context?.fill(dirtyRect)
        
        let w = Int(frame.width)
        let h = Int(frame.height)

        for x in 0...w {
            
            let bank = Int(255.0 * Float(x) / Float(w))
            let src = memory.memSrc(bank << 16).rawValue
            var color : NSColor
            
            switch (src) {
                
            case MEM_UNMAPPED.rawValue:
                color = .gray
            case MEM_CHIP.rawValue:
                color = .green
            case MEM_FAST.rawValue:
                color = .green
            case MEM_CIA.rawValue:
                color = .yellow
            case MEM_SLOW.rawValue:
                color = .green
            case MEM_RTC.rawValue:
                color = .yellow
            case MEM_CUSTOM.rawValue:
                color = .orange
            case MEM_BOOT.rawValue:
                color = .blue
            case MEM_KICK.rawValue:
                color = .blue
            default:
                color = .gray
            }
            
            let from = CGPoint(x: x, y: 0)
            let to = CGPoint(x: x, y: h)
            context?.move(to: from)
            context?.setStrokeColor(color.cgColor)
            context?.addLine(to: to)
            context?.strokePath()
        }
        // NSColor.gray.setStroke()
        // context?.strokePath()
    }
}
