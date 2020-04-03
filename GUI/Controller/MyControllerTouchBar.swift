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
    
    static let snap = NSTouchBarItem.Identifier("com.vAmiga.TouchBarItem.snap")
    static let revert = NSTouchBarItem.Identifier("com.vAMiga.TouchBarItem.revert")
    static let click = NSTouchBarItem.Identifier("com.vAmiga.TouchBarItem.click")
    static let browse = NSTouchBarItem.Identifier("com.vAmiga.TouchBarItem.browse")
}

@available(OSX 10.12.2, *)
extension MyController: NSTouchBarDelegate {
    
    override open func makeTouchBar() -> NSTouchBar? {
 
        track()

        let touchBar = NSTouchBar()
        touchBar.delegate = self

        // Configure items
        touchBar.defaultItemIdentifiers =
            [ .snap, .revert, .click, .browse ]
        
        // Make touchbar customizable
        touchBar.customizationIdentifier = NSTouchBar.CustomizationIdentifier("com.vAmiga.touchbar")
        touchBar.customizationAllowedItemIdentifiers =
            [ .snap, .revert, .click, .browse ]
        
        return touchBar
    }
    
    public func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
                        
        case NSTouchBarItem.Identifier.snap:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("ttStoreTemplate"))!
            let resizedIcon = icon.resize(width: 24, height: 24)
            item.customizationLabel = "Snap"
            item.view = NSButton(image: resizedIcon,
                                 target: self,
                                 action: #selector(takeUserSnapshotAction(_:)))
            return item
        
        case NSTouchBarItem.Identifier.revert:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("ttRestoreTemplate"))!
            let resizedIcon = icon.resize(width: 24, height: 24)
            item.customizationLabel = "Revert"
            item.view = NSButton(image: resizedIcon,
                                 target: self,
                                 action: #selector(restoreLatestUserSnapshotAction(_:)))
            return item
            
        case NSTouchBarItem.Identifier.click:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("ttCameraTemplate"))!
            let resizedIcon = icon.resize(width: 24, height: 24)
            item.customizationLabel = "Click"
            item.view = NSButton(image: resizedIcon,
                                 target: self,
                                 action: #selector(takeUserScreenshotAction(_:)))
            return item
            
        case NSTouchBarItem.Identifier.browse:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("ttFolderTemplate"))!
            let resizedIcon = icon.resize(width: 24, height: 24)
            item.customizationLabel = "Browse"
            item.view = NSButton(image: resizedIcon,
                                 target: self,
                                 action: #selector(browseStorageAction(_:)))
            return item

        default:
            return nil
        }
    }
}
