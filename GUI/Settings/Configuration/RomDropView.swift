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
            return NSURL.init(from: pasteBoard) as URL?
        }
        return nil
    }
}

class DropView: NSImageView {
    
    @IBOutlet var parent: ConfigurationController!
    var amiga: AmigaProxy { return parent.amiga }

    override func awakeFromNib() {

        registerForDraggedTypes([NSPasteboard.PasteboardType.compatibleFileURL])
    }

    func acceptDragSource(url: URL) -> Bool { return false }

    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {

        if let url = sender.url {
            if acceptDragSource(url: url) {
                image = NSImage.init(named: "rom_medium")
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
        
        if !amiga.isPoweredOff { return false }
            
        let suffix = url.pathExtension
        return suffix == "zip" || suffix == "gz" || amiga.mem.isRom(url)
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {

        guard let url = sender.url?.unpacked else { return false }
        
        do {
            let rom = try Proxy.make(url: url) as RomFileProxy
            amiga.mem.loadRom(rom)
            return true
        } catch {
            let name = url.lastPathComponent
            (error as? VAError)?.warning("Cannot open Rom file \"\(name)\"")
        }
        return false
    }
}

class ExtRomDropView: DropView {

    override func acceptDragSource(url: URL) -> Bool {

        if !amiga.isPoweredOff { return false }

        let suffix = url.pathExtension
        return suffix == "zip" || suffix == "gz" || amiga.mem.isExt(url)
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {

        guard let url = sender.url?.unpacked else { return false }
        
        var ec: ErrorCode = .OK
        if amiga.mem.loadExt(fromFile: url, error: &ec) { return true }
        if ec != .FILE_TYPE_MISMATCH { ec.showAlert(url: url) }
        
        parent.refresh()
        return false
    }
}
