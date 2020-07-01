// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@available(OSX 10.12.2, *)
extension NSTouchBarItem.Identifier {
    
    static let save = NSTouchBarItem.Identifier("com.vAmiga.TouchBarItem.save")
    static let load = NSTouchBarItem.Identifier("com.vAMiga.TouchBarItem.load")
    static let restore = NSTouchBarItem.Identifier("com.vAMiga.TouchBarItem.restore")

    static let click = NSTouchBarItem.Identifier("com.vAmiga.TouchBarItem.click")
    static let gallery = NSTouchBarItem.Identifier("com.vAmiga.TouchBarItem.gallery")
}

@available(OSX 10.12.2, *)
extension MyController: NSTouchBarDelegate {
    
    override open func makeTouchBar() -> NSTouchBar? {
 
        track()

        let touchBar = NSTouchBar()
        touchBar.delegate = self

        // Configure items
        touchBar.defaultItemIdentifiers =
            [ .save, .load, .restore, . click, .gallery ]
        
        // Make touchbar customizable
        touchBar.customizationIdentifier = NSTouchBar.CustomizationIdentifier("com.vAmiga.touchbar")
        touchBar.customizationAllowedItemIdentifiers =
            [ .save, .load, .restore, . click, .gallery ]
        
        return touchBar
    }
    
    public func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
                        
        case NSTouchBarItem.Identifier.save:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("pushTemplate"))!
            item.customizationLabel = "Save"
            item.view = NSButton(image: icon,
                                 target: self,
                                 action: #selector(takeSnapshotAction(_:)))
            return item
        
        case NSTouchBarItem.Identifier.load:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("popTemplate"))!
            item.customizationLabel = "Load"
            item.view = NSButton(image: icon,
                                 target: self,
                                 action: #selector(restoreSnapshotAction(_:)))
            return item
            
        case NSTouchBarItem.Identifier.restore:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("timeMachineTemplate"))!
            item.customizationLabel = "Restore"
            item.view = NSButton(image: icon,
                                 target: self,
                                 action: #selector(browseSnapshotsAction(_:)))
            return item

        case NSTouchBarItem.Identifier.click:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("clickTemplate"))!
            item.customizationLabel = "Click"
            item.view = NSButton(image: icon,
                                 target: self,
                                 action: #selector(takeScreenshotAction(_:)))
            return item
            
        case NSTouchBarItem.Identifier.gallery:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("folderTemplate"))!
            item.customizationLabel = "Gallery"
            item.view = NSButton(image: icon,
                                 target: self,
                                 action: #selector(browseScreenshotsAction(_:)))
            return item

        default:
            return nil
        }
    }
}
