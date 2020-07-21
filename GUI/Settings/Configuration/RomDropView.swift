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
    
    override func draggingExited(_ sender: NSDraggingInfo?) {

        parent.refresh()
    }
    
    override func prepareForDragOperation(_ sender: NSDraggingInfo) -> Bool {

        return true
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {

        guard let url = sender.url else { return false }

        // Check for a standard ROM
        if amiga.mem.loadRom(fromFile: url) { return true }

        // Check for an encrypted ROM
        var error = DECRYPT_ROM_KEY_ERROR
        if amiga.mem.loadEncryptedRom(fromFile: url, error: &error) { return true }

        // Report error
        parent.parent.mydocument.showDecryptionAlert(error: error)
        return false
    }
    
    override func concludeDragOperation(_ sender: NSDraggingInfo?) {

        parent.refresh()
    }
}

class RomDropView: DropView {

    override func acceptDragSource(url: URL) -> Bool {

        if !amiga.isPoweredOff() { return false }
        return amiga.mem.isRom(url) || amiga.mem.isEncryptedRom(url)
    }
}

class ExtRomDropView: DropView {

    override func acceptDragSource(url: URL) -> Bool {

        if !amiga.isPoweredOff() { return false }
        return amiga.mem.isExt(url)
    }
}
