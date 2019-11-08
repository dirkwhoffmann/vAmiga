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
    
    @IBOutlet var dialogController: DialogController!

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

        dialogController.refresh()
    }
    
    override func prepareForDragOperation(_ sender: NSDraggingInfo) -> Bool {

        return true
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {

        guard let url = sender.url else { return false }
        guard let controller = myController else { return false }
        guard let amiga = amigaProxy else { return false }
        
        controller.romURL = url
        return amiga.mem.loadRom(fromFile: url)
    }
    
    override func concludeDragOperation(_ sender: NSDraggingInfo?) {

        dialogController.refresh()
    }
}

class RomDropView: DropView {

    override func acceptDragSource(url: URL) -> Bool {

        guard let amiga = amigaProxy else { return false }
        return amiga.mem.isRom(url) && amiga.isPoweredOff()
    }
}

class ExtRomDropView: DropView {

    override func acceptDragSource(url: URL) -> Bool {

        guard let amiga = amigaProxy else { return false }
        return amiga.mem.isExt(url) && amiga.isPoweredOff()
    }
}
