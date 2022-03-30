// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension NSDraggingInfo {
    
    var url: URL? {
        let pasteBoard = draggingPasteboard
        let types = [NSPasteboard.PasteboardType.compatibleFileURL]
        if pasteBoard.availableType(from: types) != nil {
            return NSURL(from: pasteBoard) as URL?
        }
        return nil
    }
}

class DropView: NSImageView {
    
    @IBOutlet var parent: ConfigurationController!
    var amiga: AmigaProxy { return parent.amiga }

    var oldImage: NSImage?
    
    override func awakeFromNib() {

        registerForDraggedTypes([NSPasteboard.PasteboardType.compatibleFileURL])
    }

    func acceptDragSource(url: URL) -> Bool { return false }

    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {

        if let url = sender.url {
            if acceptDragSource(url: url) {
                oldImage = image
                image = NSImage(named: "rom_medium")
                return .copy
            }
        }
        return NSDragOperation()
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        
        return false
    }

    override func draggingExited(_ sender: NSDraggingInfo?) {

        parent.refresh()
    }
    
    override func prepareForDragOperation(_ sender: NSDraggingInfo) -> Bool {

        return true
    }
        
    override func concludeDragOperation(_ sender: NSDraggingInfo?) {

        parent.refresh()
    }
}

class RomDropView: DropView {

    override func acceptDragSource(url: URL) -> Bool {
        
        if !amiga.poweredOff { return false }
            
        let suffix = url.pathExtension
        return suffix == "zip" || suffix == "gz" || amiga.mem.isRom(url)
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {

        guard let url = sender.url?.unpacked else { return false }
        
        do {
            let rom = try RomFileProxy.make(with: url)
            try amiga.mem.loadRom(rom)
            return true
        } catch {
            image = oldImage
            let name = url.lastPathComponent
            (error as? VAError)?.warning("Cannot open Rom file \"\(name)\"", async: true)
        }
        return false
    }
}

class ExtRomDropView: DropView {

    override func acceptDragSource(url: URL) -> Bool {

        if !amiga.poweredOff { return false }

        let suffix = url.pathExtension
        return suffix == "zip" || suffix == "gz" || amiga.mem.isExt(url)
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        
        guard let url = sender.url?.unpacked else { return false }
        
        do {
            let ext = try ExtendedRomFileProxy.make(with: url)
            try amiga.mem.loadExt(ext)
            return true
        } catch {
            let name = url.lastPathComponent
            (error as? VAError)?.warning("Cannot open extended Rom file \"\(name)\"")
        }
        return false        
    }
}
