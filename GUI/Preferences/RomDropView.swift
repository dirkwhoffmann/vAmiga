// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension NSDraggingInfo
{
    var url: URL? {
        let pasteBoard = draggingPasteboard
        let types = [NSPasteboard.PasteboardType.compatibleFileURL]
        if let _ = pasteBoard.availableType(from: types) {
            return NSURL.init(from: pasteBoard) as URL?
        }
        return nil
    }
}

class RomDropView : NSImageView
{
    @IBOutlet var dialogController: DialogController!

    func acceptDragSource(url: URL) -> Bool {
        return false
    }
    
    override func awakeFromNib()
    {
        registerForDraggedTypes([NSPasteboard.PasteboardType.compatibleFileURL])
    }
    
    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation
    {
        if let url = sender.url {
            if acceptDragSource(url: url) {
                image = NSImage.init(named: "rom_medium")
                return .copy
            }
        }
        return NSDragOperation()
    }
    
    override func draggingExited(_ sender: NSDraggingInfo?)
    {
        dialogController.refresh()
    }
    
    override func prepareForDragOperation(_ sender: NSDraggingInfo) -> Bool
    {
        return true
    }
    
    override func concludeDragOperation(_ sender: NSDraggingInfo?)
    {
        dialogController.refresh()
    }
}

class BootRomDropView : RomDropView
{
    
    override func acceptDragSource(url: URL) -> Bool
    {
        return amigaProxy?.isBootRom(url) ?? false
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool
    {
        guard let url = sender.url else { return false }
        myController?.bootRomURL = url
        return amigaProxy?.loadBootRom(fromFile: url) ?? false
    }
}

class KickRomDropView : RomDropView
{
    override func acceptDragSource(url: URL) -> Bool
    {
        return amigaProxy?.isKickRom(url) ?? false
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool
    {
        guard let url = sender.url else { return false }
        myController?.kickRomURL = url
        return amigaProxy?.loadKickRom(fromFile: url) ?? false
    }
}
