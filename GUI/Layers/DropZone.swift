// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

@MainActor
class DropZone: Layer {
    
    var window: NSWindow { return controller.window! }
    var contentView: NSView { return window.contentView! }
    var metal: MetalView { return controller.metal! }
    var mydocument: MyDocument { return controller.mydocument! }
    var mm: MediaManager { return controller.mm }
    
    var zones = [NSImageView(), NSImageView(), NSImageView(), NSImageView()]
    var labels = [NSImageView(), NSImageView(), NSImageView(), NSImageView()]
    var ul = [NSPoint(x: 0, y: 0), NSPoint(x: 0, y: 0),
              NSPoint(x: 0, y: 0), NSPoint(x: 0, y: 0)]
    var lr = [NSPoint(x: 0, y: 0), NSPoint(x: 0, y: 0),
              NSPoint(x: 0, y: 0), NSPoint(x: 0, y: 0)]
    
    static let unconnected = 0.92
    static let unselected = 0.92
    static let selected = 0.92
    
    var hideAll = false
    var enabled = [false, false, false, false]
    var inside = [false, false, false, false]
    var currentAlpha = [0.0, 0.0, 0.0, 0.0]
    var targetAlpha = [unselected, unselected, unselected, unselected]
    var maxAlpha = [0.0, 0.0, 0.0, 0.0]
    
    // URL properties
    var isFloppy35Image: Bool = false
    var isFloppy525Image: Bool = false
    var isHardDiskImage: Bool = false
    var isDirectory: Bool = false

    // Image pool
    var dfDisabled: NSImage { return NSImage(named: "dropDisabled")! }
    var dfEmpty: NSImage { return NSImage(named: "dropEmpty")! }
    var df35Hover: NSImage { return NSImage(named: "drop35")! }
    var df35Present: NSImage { return NSImage(named: "drop35Gray")! }
    var df525Hover: NSImage { return NSImage(named: "drop525")! }
    var df525Present: NSImage { return NSImage(named: "drop525Gray")! }
    var hdDisabled: NSImage { return NSImage(named: "dropDisabled")! }
    var hdEmpty: NSImage { return NSImage(named: "dropEmpty")! }
    var hdHover: NSImage { return NSImage(named: "dropHdr")! }
    var hdPresent: NSImage { return NSImage(named: "dropHdrGray")! }

    //
    // Initializing
    //
    
    override init(renderer: Renderer) {
        
        for i in 0...3 { zones[i].unregisterDraggedTypes() }
        for i in 0...3 { labels[i].unregisterDraggedTypes() }
        super.init(renderer: renderer)
        resize()
    }
    
    private func zoneImage(zone: Int) -> NSImage? {

        guard let emu = emu, enabled[zone] else {
            return isHardDiskImage ? hdDisabled : dfDisabled
        }
        if isHardDiskImage {
            return emu.hd(zone)!.info.hasDisk ? hdPresent : hdEmpty
        }
        if isFloppy35Image {
            return emu.df(zone)!.info.hasDisk ? df35Present : dfEmpty
        }
        if isFloppy525Image {
            return emu.df(zone)!.info.hasDisk ? df525Present : dfEmpty
        }
        return nil
    }

    private func labelImage(zone: Int) -> NSImage? {

        var name = "drop" + (isHardDiskImage ? "Hd" : "Df") + "\(zone)"
        if !enabled[zone] { name += "_disabled" }
        
        return NSImage(named: name)!
    }

    func open(url: URL, delay: Double) {

        guard let emu = emu else { return }

        let format = DiskImageProxy.about(url).format
        isDirectory = url.hasDirectoryPath
        isFloppy35Image = [.ADF, .EADF, .IMG, .ST, .DMS, .EXE].contains(format)
        isFloppy525Image = [.D64].contains(format)
        isHardDiskImage = [.HDF, .HDZ].contains(format)

        func hasHd(_ n: Int) -> Bool { emu.hd(n)!.info.isConnected }
        func hasDf(_ n: Int) -> Bool { emu.df(n)!.info.isConnected }
        func hasDf35(_ n: Int) -> Bool { hasDf(n) && emu.df(n)!.config.type != .DD_525 }
        func hasDf525(_ n: Int) -> Bool { hasDf(n) && emu.df(n)!.config.type == .DD_525 }

        if (isFloppy35Image) {

            enabled = [ hasDf35(0), hasDf35(1), hasDf35(2), hasDf35(3) ]
            hideAll = false

        } else if (isFloppy525Image) {

            enabled = [ hasDf525(0), hasDf525(1), hasDf525(2), hasDf525(3) ]
            hideAll = false

        } else if (isHardDiskImage) {

            enabled = [ true, hasHd(1), hasHd(2), hasHd(3) ]
            hideAll = false

        } else {
            enabled = [false, false, false, false]
            hideAll = true
        }

        // Assign zone images
        for i in 0...3 { zones[i].image = zoneImage(zone: i) }
        for i in 0...3 { labels[i].image = labelImage(zone: i) }

        open(delay: delay)
        resize()
    }

    override func update(frames: Int64) {
        
        super.update(frames: frames)
        if alpha.current > 0 { updateAlpha() }
    }
    
    func isInside(_ sender: NSDraggingInfo, zone i: Int) -> Bool {
        
        assert(i >= 0 && i <= 3)
        
        if !enabled[i] { return false }
        
        let x = sender.draggingLocation.x
        let y = sender.draggingLocation.y
        
        return x > ul[i].x && x < lr[i].x && y > ul[i].y && y < lr[i].y
    }
    
    func draggingUpdated(_ sender: NSDraggingInfo) {

        var hoverImage: NSImage? {

            if isHardDiskImage { return hdHover }
            if isFloppy35Image { return df35Hover }
            if isFloppy525Image { return df525Hover }
            return nil
        }

        if hideAll { return }
        
        for i in 0...3 {
            
            if enabled[i] {

                let isIn = isInside(sender, zone: i)

                if isIn && !inside[i] {

                    inside[i] = true
                    zones[i].image = hoverImage
                    targetAlpha[i] = DropZone.selected
                }

                if !isIn && inside[i] {

                    inside[i] = false
                    zones[i].image = zoneImage(zone: i)
                    targetAlpha[i] = DropZone.unselected
                }

            } else {

                targetAlpha[i] = DropZone.unconnected
                continue
            }
        }
    }
    
    override func alphaDidChange() {
        
        if hideAll { return }
        
        for i in 0...3 {
            
            maxAlpha[i] = Double(alpha.current)
            
            if alpha.current > 0 && zones[i].superview == nil {
                contentView.addSubview(zones[i])
                contentView.addSubview(labels[i])
            }
            if alpha.current == 0 && zones[i].superview != nil {
                zones[i].removeFromSuperview()
                labels[i].removeFromSuperview()
            }
        }
        
        resize()
    }
    
    override func layerDidClose() {
        
        guard let url = metal.dropUrl else { return }

        do {

            if let nr = metal.dropZone {

                if isHardDiskImage {
                    try mm.mount(hd: nr, url: url, options: [.remember])
                } else {
                    try mm.mount(df: nr, url: url, options: [.remember])
                }

            } else {
                
                switch url.pathExtension.lowercased() {
                    
                case "vamiga":
                    
                    try mydocument.processWorkspaceFile(url: url)
                    
                case "vasnap":
                    
                    try mm.loadSnapshot(url: url)
                    break
                    
                case "retrosh":
                    
                    try mm.runScript(url: url)
                    mm.console.open()
                    
                default:
                    
                    NSSound.beep()
                }
            }
            
        } catch {
            
            controller.showAlert(.cantOpen(url: url), error: error, async: true)
        }
    }
    
    func updateAlpha() {
        
        for i in 0...3 {
            
            let current = currentAlpha[i]
            var delta = 0.0
            
            if current < targetAlpha[i] && current < maxAlpha[i] { delta = 0.05 }
            if current > targetAlpha[i] || current > maxAlpha[i] { delta = -0.05 }
            
            if delta != 0.0 {
                
                currentAlpha[i] += delta
                zones[i].alphaValue = CGFloat(currentAlpha[i])
                labels[i].alphaValue = CGFloat(currentAlpha[i])
            }
        }
    }
    
    func resize() {
        
        let size = controller.metal.frame.size
        let origin = controller.metal.frame.origin
        let midx = origin.x + (size.width / 2)
        
        // Compute the drop zone size
        let zw = size.width / 6
        let zh = zw * 0.9
        let zy = size.height + origin.y - 24 - zh * CGFloat(alpha.current)
        let margin = zw / 8
        let zoneSize = NSSize(width: zw, height: zh)
        
        // Compute the drop label size
        let lw = zw
        let lh = lw / 4
        let ly = zy - lh - 0.5 * margin
        let labelSize = NSSize(width: lw, height: lh)
        
        // Compute the drop zone bounding boxes
        ul[0] = CGPoint(x: midx - 2 * zw - 1.5 * margin, y: zy)
        lr[0] = CGPoint(x: ul[0].x + zw, y: ul[0].y + zh)
        
        ul[1] = CGPoint(x: midx - zw - 0.5 * margin, y: zy)
        lr[1] = CGPoint(x: ul[1].x + zw, y: ul[1].y + zh)
        
        ul[2] = CGPoint(x: midx + 0.5 * margin, y: zy)
        lr[2] = CGPoint(x: ul[2].x + zw, y: ul[2].y + zh)
        
        ul[3] = CGPoint(x: midx + zw + 1.5 * margin, y: zy)
        lr[3] = CGPoint(x: ul[3].x + zw, y: ul[3].y + zh)
        
        for i in 0...3 {
            
            zones[i].imageScaling = .scaleAxesIndependently
            zones[i].setFrameSize(zoneSize)
            zones[i].frame.origin = ul[i]
            
            labels[i].imageScaling = .scaleAxesIndependently
            labels[i].setFrameSize(labelSize)
            labels[i].frame.origin = CGPoint(x: ul[i].x, y: ly)
        }
    }
}
