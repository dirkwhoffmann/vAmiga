// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
public extension MetalView {

    func acceptedTypes() -> [NSPasteboard.PasteboardType] {

        return [.fileURL, .string, .fileContents]
    }

    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {
        
        debug(.dragndrop, "draggingEntered \(sender)\n")
        
        dropZone = nil
        dropUrl = nil
        dropType = nil
        
        let pasteBoard = sender.draggingPasteboard
        guard let type = pasteBoard.availableType(from: acceptedTypes()) else {
            return NSDragOperation()
        }
        
        switch type {
            
        case .string:
            return NSDragOperation.copy
        
        case .fileContents:
            return NSDragOperation.copy
            
        case .fileURL:
            
            if let url = NSURL.init(from: pasteBoard) as URL? {
            
                // Unpack the file if it is compressed
                dropUrl = url // url.unpacked(maxSize: 2048 * 1024)

                // Analyze the file type
                let type = MediaFileProxy.type(of: dropUrl)

                // Open the drop zone layer
                parent.renderer.dropZone.open(type: type, delay: 0.25)
            }

            return NSDragOperation.copy
            
        default:
            return NSDragOperation()
        }
    }
    
    override func draggingUpdated(_ sender: NSDraggingInfo) -> NSDragOperation {
        
        debug(.dragndrop, "draggingUpdated \(sender)\n")

        parent.renderer.dropZone.draggingUpdated(sender)
        return NSDragOperation.copy
    }

    override func draggingExited(_ sender: NSDraggingInfo?) {
    
        debug(.dragndrop, "draggingExited \(String(describing: sender))")
        
        parent.renderer.dropZone.close(delay: 0.25)
    }
    
    override func prepareForDragOperation(_ sender: NSDraggingInfo) -> Bool {
        
        debug(.dragndrop, "prepareForDragOperation \(sender)\n")
        
        parent.renderer.dropZone.close(delay: 0.25)
        return true
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        
        debug(.dragndrop, "performDragOperation \(sender)\n")
        
        let pasteBoard = sender.draggingPasteboard
        
        if let type = pasteBoard.availableType(from: acceptedTypes()) {
            
            switch type {
                
            case .string:
                return performStringDrag(sender)
                
            case .fileURL:
                return performUrlDrag(sender)
                
            default:
                break
            }
        }
        
        return false
    }
    
    func performStringDrag(_ sender: NSDraggingInfo) -> Bool {
        
        let pasteBoard = sender.draggingPasteboard
        
        // Type text on virtual keyboard
        guard let text = pasteBoard.string(forType: .string) else {
            return false
        }
        parent.keyboard.autoType(text, max: 256)
        return true
    }

    func performUrlDrag(_ sender: NSDraggingInfo) -> Bool {
                
        if dropUrl == nil { return false }
        
        // Check drop zones
        var zone: Int?
        for i in 0...3 {
            if renderer.dropZone.isInside(sender, zone: i) { zone = i }
        }

        // Check file types
        let type = FileType(url: dropUrl)
        switch type {
            
        case .WORKSPACE, .SNAPSHOT, .SCRIPT:
            break
            
        case .ADF, .ADZ, .DIR, .DMS, .EADF, .EXE, .HDF, .IMG, .ST:
            if zone == nil { return false }
            
        default:
            return false
        }

        dropZone = zone
        dropType = type
        return true
    }
            
    override func concludeDragOperation(_ sender: NSDraggingInfo?) {

    }
}
