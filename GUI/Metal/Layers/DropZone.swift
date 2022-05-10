// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class DropZone: Layer {
    
    class func createImage(_ name: String, label: String) -> NSImage {
        
        let img = NSImage(named: name)!.copy() as! NSImage
        img.imprint(text: label, x: 235, y: 330, fontSize: 100, tint: "light")
        return img
    }
    
    let controller: MyController
    
    var window: NSWindow { return controller.window! }
    var contentView: NSView { return window.contentView! }
    var metal: MetalView { return controller.metal! }
    var mydocument: MyDocument { return controller.mydocument! }
    
    var zones = [NSImageView(), NSImageView(), NSImageView(), NSImageView()]
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
        
    var type: FileType?
    
    // Image pool
    var dfDisabled: [NSImage] =
    [ createImage("dropZoneDisabled", label: "DF0"),
      createImage("dropZoneDisabled", label: "DF1"),
      createImage("dropZoneDisabled", label: "DF2"),
      createImage("dropZoneDisabled", label: "DF3") ]
    
    var dfEmpty: [NSImage] =
    [ createImage("dropZoneEmpty", label: "DF0"),
      createImage("dropZoneEmpty", label: "DF1"),
      createImage("dropZoneEmpty", label: "DF2"),
      createImage("dropZoneEmpty", label: "DF3") ]

    var dfInUse: [NSImage] =
    [ createImage("dropZoneFloppy1", label: "DF0"),
      createImage("dropZoneFloppy1", label: "DF1"),
      createImage("dropZoneFloppy1", label: "DF2"),
      createImage("dropZoneFloppy1", label: "DF3") ]

    var dfSelected: [NSImage] =
    [ createImage("dropZoneFloppy2", label: "DF0"),
      createImage("dropZoneFloppy2", label: "DF1"),
      createImage("dropZoneFloppy2", label: "DF2"),
      createImage("dropZoneFloppy2", label: "DF3") ]

    var hdDisabled: [NSImage] =
    [ createImage("dropZoneDisabled", label: "HD0"),
      createImage("dropZoneDisabled", label: "HD1"),
      createImage("dropZoneDisabled", label: "HD2"),
      createImage("dropZoneDisabled", label: "HD3") ]
        
    var hdEmpty: [NSImage] =
    [ createImage("dropZoneEmpty", label: "HD0"),
      createImage("dropZoneEmpty", label: "HD1"),
      createImage("dropZoneEmpty", label: "HD2"),
      createImage("dropZoneEmpty", label: "HD3") ]

    var hdInUse: [NSImage] =
    [ createImage("dropZoneHdr1", label: "HD0"),
      createImage("dropZoneHdr1", label: "HD1"),
      createImage("dropZoneHdr1", label: "HD2"),
      createImage("dropZoneHdr1", label: "HD3") ]

    var hdSelected: [NSImage] =
    [ createImage("dropZoneHdr2", label: "HD0"),
      createImage("dropZoneHdr2", label: "HD1"),
      createImage("dropZoneHdr2", label: "HD2"),
      createImage("dropZoneHdr2", label: "HD3") ]

    //
    // Initializing
    //
    
    override init(renderer: Renderer) {
        
        controller = renderer.parent
        
        for i in 0...3 { zones[i].unregisterDraggedTypes() }
        super.init(renderer: renderer)
        resize()
    }

    private func image(zone: Int) -> NSImage {

        if !enabled[zone] {
            return type == .HDF ? hdDisabled[zone] : dfDisabled[zone]
        } else if amiga.df(zone)!.hasDisk {
            return type == .HDF ? hdInUse[zone] : dfInUse[zone]
        } else {
            return type == .HDF ? hdEmpty[zone] : dfEmpty[zone]
        }
    }
    
    func open(type: FileType, delay: Double) {

        self.type = type
        
        switch type {
        
        case .ADF, .EXT, .IMG, .DMS, .EXE, .DIR:
            let connected = amiga.diskController.getConfig().connected
            enabled = [ connected.0, connected.1, connected.2, connected.3 ]
                        
        case .HDF:
            enabled = [ true, true, true, true ]
            
        default:
            enabled = [false, false, false, false]
        }
                
        // Assign zone images
        for i in 0...3 { zones[i].image = image(zone: i) }
        
        // Hide all drop zones if none is enabled
        hideAll = !enabled[0] && !enabled[1] && !enabled[2] && !enabled[3]
        
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
                        
        if hideAll { return }
        
        for i in 0...3 {

            if !enabled[i] {
                targetAlpha[i] = DropZone.unconnected
                return
            }
            
            let isIn = isInside(sender, zone: i)

            if isIn && !inside[i] {
                
                inside[i] = true
                zones[i].image = type == .HDF ? hdSelected[i] : dfSelected[i]
                targetAlpha[i] = DropZone.selected
            }

            if !isIn && inside[i] {
                
                inside[i] = false
                zones[i].image = image(zone: i)
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
            }
            if alpha.current == 0 && zones[i].superview != nil {
                zones[i].removeFromSuperview()
            }
        }
        
        resize()
    }
    
    override func layerDidClose() {
        
        guard let url = metal.dropUrl else { return }
        guard let type = metal.dropType else { return }
        let n = metal.dropZone

        do {
                        
            switch type {
                
            case .SNAPSHOT, .SCRIPT:
                try mydocument.addMedia(url: url,
                                        allowedTypes: [type])
                
            case .ADF, .HDF, .EXT, .IMG, .DMS, .EXE, .DIR:
                try mydocument.addMedia(url: url,
                                        allowedTypes: [type], df: n!, hd: n!)
            default:
                fatalError()
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
            }
        }
    }
    
    func resize() {
                          
        let size = controller.metal.frame.size
        let origin = controller.metal.frame.origin
        let midx = origin.x + (size.width / 2)

        let w = size.width / 6
        let h = w * 1.2
        let y = size.height + origin.y - 24 - h * CGFloat(alpha.current)
        let margin = w / 8
        let iconSize = NSSize(width: w, height: h)

        ul[0] = CGPoint(x: midx - 2 * w - 1.5 * margin, y: y)
        lr[0] = CGPoint(x: ul[0].x + w, y: ul[0].y + h)

        ul[1] = CGPoint(x: midx - w - 0.5 * margin, y: y)
        lr[1] = CGPoint(x: ul[1].x + w, y: ul[1].y + h)

        ul[2] = CGPoint(x: midx + 0.5 * margin, y: y)
        lr[2] = CGPoint(x: ul[2].x + w, y: ul[2].y + h)

        ul[3] = CGPoint(x: midx + w + 1.5 * margin, y: y)
        lr[3] = CGPoint(x: ul[3].x + w, y: ul[3].y + h)

        for i in 0...3 {

            zones[i].setFrameSize(iconSize)
            zones[i].frame.origin = ul[i]
        }
    }
}
