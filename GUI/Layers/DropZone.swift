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
    
    // var type: FileType? // DEPRECATED

    // URL properties
    var isFloppyImage: Bool = false
    var isHardDiskImage: Bool = false
    var isDirectory: Bool = false

    // Image pool
    var dfDisabled: NSImage { return NSImage(named: "dropZoneDisabled")! }
    var dfEmpty: NSImage { return NSImage(named: "dropZoneEmpty")! }
    var dfInUse: NSImage { return NSImage(named: "dropZoneFloppy1")! }
    var dfSelected: NSImage { return NSImage(named: "dropZoneFloppy2")! }
    var hdDisabled: NSImage { return NSImage(named: "dropZoneDisabled")! }
    var hdEmpty: NSImage { return NSImage(named: "dropZoneEmpty")! }
    var hdInUse: NSImage { return NSImage(named: "dropZoneHdr1")! }
    var hdSelected: NSImage { return NSImage(named: "dropZoneHdr2")! }
    
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
        
        guard let emu = emu else { return nil }

        if !enabled[zone] {
            return isHardDiskImage ? hdDisabled : dfDisabled
        } else if emu.df(zone)!.info.hasDisk {
            return isHardDiskImage ? hdInUse : dfInUse
        } else {
            return isHardDiskImage ? hdEmpty : dfEmpty
        }
    }
    
    private func labelImage(zone: Int) -> NSImage? {

        var name = "drop" + (isHardDiskImage ? "Hd" : "Df") + "\(zone)"
        if !enabled[zone] { name += "_disabled" }
        
        return NSImage(named: name)!
    }

    func open(url: URL, delay: Double) {

        guard let emu = emu else { return }

        isFloppyImage = FloppyDiskImageProxy.about(url).format != .UNKNOWN
        isHardDiskImage = HardDiskImageProxy.about(url).format != .UNKNOWN
        isDirectory = url.hasDirectoryPath

        if (isFloppyImage) {

            enabled = [ emu.df0.info.isConnected,
                        emu.df1.info.isConnected,
                        emu.df2.info.isConnected,
                        emu.df3.info.isConnected ]

        } else if (isHardDiskImage) {

            enabled = [ true,
                        emu.hd1.info.isConnected,
                        emu.hd2.info.isConnected,
                        emu.hd3.info.isConnected ]

        } else {

            enabled = [false, false, false, false]
        }

        // Assign zone images
        for i in 0...3 { zones[i].image = zoneImage(zone: i) }
        for i in 0...3 { labels[i].image = labelImage(zone: i) }

        // Hide all drop zones if none is enabled
        hideAll = !enabled[0] && !enabled[1] && !enabled[2] && !enabled[3]

        open(delay: delay)
        resize()
    }

    /*
    func open(type: FileType, delay: Double) {
        
        guard let emu = emu else { return }
        
        self.type = type

        switch type {
            
        case .ADF, .ADZ, .EADF, .IMG, .ST, .DMS, .EXE, .DIR:
            enabled = [ emu.df0.info.isConnected,
                        emu.df1.info.isConnected,
                        emu.df2.info.isConnected,
                        emu.df3.info.isConnected ]
            
        case .HDF, .HDZ:
            enabled = [ true,
                        emu.hd1.info.isConnected,
                        emu.hd2.info.isConnected,
                        emu.hd3.info.isConnected ]
            
        default:
            enabled = [false, false, false, false]
        }
        
        // Assign zone images
        for i in 0...3 { zones[i].image = zoneImage(zone: i) }
        for i in 0...3 { labels[i].image = labelImage(zone: i) }
        
        // Hide all drop zones if none is enabled
        hideAll = !enabled[0] && !enabled[1] && !enabled[2] && !enabled[3]
        
        open(delay: delay)
        resize()
    }
    */

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
        
        if hideAll { return }
        
        for i in 0...3 {
            
            if !enabled[i] {
                targetAlpha[i] = DropZone.unconnected
                return
            }
            
            let isIn = isInside(sender, zone: i)
            
            if isIn && !inside[i] {
                
                inside[i] = true
                zones[i].image = isHardDiskImage ? hdSelected : dfSelected
                targetAlpha[i] = DropZone.selected
            }
            
            if !isIn && inside[i] {
                
                inside[i] = false
                zones[i].image = zoneImage(zone: i)
                targetAlpha[i] = DropZone.unselected
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
        guard let type = metal.dropType else { return }
        
        do {
            
            if let nr = metal.dropZone {
                try mm.mount(url: url, allowedTypes: [type], drive: nr)
            } else {
                try mm.mount(url: url, allowedTypes: [type])
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
