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
    
    @IBOutlet var parent: RomSettingsViewController!
    @IBOutlet var auxIcon: NSImageView!
    var emu: EmulatorProxy? { return parent.emu }

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

        guard let emu = emu else { return false }
        return !emu.poweredOff ? false : emu.mem.isRom(url)
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {

        guard let emu = emu, let url = sender.url else { return false }

        do {
            
            try emu.mem.loadRom(url)

            // Check if we should keep this Rom
            let hash = Int(emu.mem.romTraits.crc)

            for item in parent.presetPopup.itemArray {

                if item.tag == hash && hash != 0 {

                    if let url = UserDefaults.romUrl(fingerprint: hash) {
                        loginfo(1, "Saving \(url)")
                        try? emu.mem.saveRom(url)
                        parent.refreshRomSelector()
                    }
                }
            }
            return true
            
        } catch {

            image = oldImage
            parent.controller?.showAlert(.cantOpen(url: url),
                                         error: error,
                                         async: true,
                                         window: parent.controller!.window)
            return false
        }
    }
}

class ExtRomDropView: DropView {

    override func acceptDragSource(url: URL) -> Bool {

        guard let emu = emu else { return false }
        return !emu.poweredOff ? false : emu.mem.isRom(url)
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        
        guard let emu = emu, let url = sender.url else { return false }

        do {
            
            try emu.mem.loadExt(url)
            return true

        } catch {

            parent.controller?.showAlert(.cantOpen(url: url),
                                         error: error,
                                         async: true,
                                         window: parent.controller!.window)
            return false
        }
    }
}
