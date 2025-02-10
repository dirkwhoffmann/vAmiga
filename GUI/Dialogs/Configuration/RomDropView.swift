// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
extension NSDraggingInfo {
    
    var url: URL? {
        let pasteBoard = draggingPasteboard
        let types = [NSPasteboard.PasteboardType.fileURL]
        if pasteBoard.availableType(from: types) != nil {
            return NSURL(from: pasteBoard) as URL?
        }
        return nil
    }
}

@MainActor
class DropView: NSImageView {
    
    @IBOutlet var parent: ConfigurationController!
    var amiga: EmulatorProxy { return parent.emu }

    var oldImage: NSImage?
    
    override init(frame frameRect: NSRect) { super.init(frame: frameRect); commonInit() }
    required init?(coder: NSCoder) { super.init(coder: coder); commonInit() }
    
    func commonInit() {

        registerForDraggedTypes([NSPasteboard.PasteboardType.fileURL])
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

        // guard let url = sender.url?.unpacked else { return false }
        guard let url = sender.url else { return false }

        do {
            
            let rom = try MediaFileProxy.make(with: url)
            try amiga.mem.loadRom(rom)

            // Check if we should keep this Rom
            let hash = Int(amiga.mem.romTraits.crc)

            for item in parent.romArosPopup.itemArray {

                if item.tag == hash && hash != 0 {

                    if let url = UserDefaults.romUrl(fingerprint: hash) {
                        debug(1, "Saving \(url)")
                        try? amiga.mem.saveRom(url)
                        parent.refreshRomSelector()
                    }
                }
            }
            return true
            
        } catch {
            
            image = oldImage
            parent.parent.showAlert(.cantOpen(url: url),
                                    error: error,
                                    async: true,
                                    window: parent.window)
            return false
        }
    }
}

class ExtRomDropView: DropView {

    override func acceptDragSource(url: URL) -> Bool {

        if !amiga.poweredOff { return false }

        let suffix = url.pathExtension
        return suffix == "zip" || suffix == "gz" || amiga.mem.isExt(url)
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        
        // guard let url = sender.url?.unpacked else { return false }
        guard let url = sender.url else { return false }

        do {
            
            let ext = try MediaFileProxy.make(with: url)
            try amiga.mem.loadExt(ext)
            return true
            
        } catch {
            
            parent.parent.showAlert(.cantOpen(url: url),
                                    error: error,
                                    async: true,
                                    window: parent.window)
            return false
        }
    }
}
